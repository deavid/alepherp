/***************************************************************************
 *   Copyright (C) 2007 by David Pinelo   *
 *   david@pinelo.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "jasperserver.h"
#include "configuracion.h"
#include "proxy/JasperServerrepositorySoapBindingProxy.h"
#include "dao/database.h"
#include <QProgressDialog>
#include <QByteArray>
#include <QFile>
#include <QTemporaryFile>
#include <QDesktopServices>
#include <QApplication>
#include <QUrl>
#include <QDir>

class JasperServerWSPrivate {
public:
	QString m_endPoint;
    QString m_user;
    QString m_password;

    JasperServerWSPrivate(){
    }
};

JasperServerWS::JasperServerWS(QObject * parent) : QObject ( parent ), d(new JasperServerWSPrivate())
{
    QScopedPointer<QSqlQuery> qry(new QSqlQuery(Database::getQDatabase()));
	QString sql = QString("SELECT data FROM %1_envvars WHERE variable='jasperserver_endpoint'").arg(configuracion.systemTablePrefix());
    qry->prepare(sql);
    if ( qry->exec() && qry->first() ) {
        QByteArray av = QByteArray::fromBase64(qry->value(0).toByteArray());
        d->m_endPoint = QString(av);
    }
	qDebug() << "JasperServerWS: " << qry->lastQuery();

    sql = QString("SELECT data FROM %1_envvars WHERE variable='jasperserver_user'").arg(configuracion.systemTablePrefix());
    qry->prepare(sql);
    if ( qry->exec() && qry->first() ) {
        QByteArray av = QByteArray::fromBase64(qry->value(0).toByteArray());
        d->m_user = QString(av);
    }
	qDebug() << "JasperServerWS: " << qry->lastQuery();

    sql = QString("SELECT data FROM %1_envvars WHERE variable='jasperserver_password'").arg(configuracion.systemTablePrefix());
    qry->prepare(sql);
    if ( qry->exec() && qry->first() ) {
        QByteArray av = QByteArray::fromBase64(qry->value(0).toByteArray());
        d->m_password = QString(av);
    }
	qDebug() << "JasperServerWS: " << qry->lastQuery();
}

JasperServerWS::~JasperServerWS()
{
	delete d;
}

/**
	Esta función es específica para bajar archivos de JasperServer.
	Debe proporcionársele
	@param xmlRequest archivo XML con el request específico de JasperServer
	@param isTempFile El archivo obtenido se guarda como archivo temporal, con lo que hay que pasar obligatoriamente tempFile
	@param rutaDestino Donde se guardará el fichero si isTempFile es false
	@param tempFile Fichero abierto para su escritura.
 */
QScriptValue JasperServerWS::downloadJasperServerFile(const QString & xmlRequest,  bool isTempFile, const QString & rutaDestino, QFile *tempFile)
{
	int r;
	std::string result;
	std::string xml = xmlRequest.toStdString();
	QScriptValue returnResult;

    if ( d->m_endPoint.isEmpty() ) {
        qDebug() << "downloadJasperServerFile:: JasperServer no configurado.";
		returnResult = QScriptValue(trUtf8("Motor de informes JasperServer no configurado."));
		return returnResult;
    }
	JasperServer::repositorySoapBindingProxy rep;
	QByteArray qba = d->m_user.toAscii();
	const char *uid = qba.constData();

	QByteArray qbb = d->m_password.toAscii();
	const char *pas = qbb.constData();

	QByteArray qbc = d->m_endPoint.toAscii();
	char *endP = qbc.data();

    rep.soap->userid = uid;
    rep.soap->passwd = pas;
    strcpy(endP, rep.soap->endpoint);

	r = rep.runReport(xml, result);
	if ( r == SOAP_OK ) {
        soap_multipart::iterator attachment = rep.soap->mime.begin();
		++attachment;
        if ( attachment != rep.soap->mime.end() ) {
			if ( isTempFile ) {
				tempFile->write((*attachment).ptr, (*attachment).size);
			} else {
				saveDocument (*attachment, rutaDestino);
			}
			returnResult = QScriptValue(QString("ok"));
		} else {
			returnResult = QScriptValue(QString::fromStdString ( result ));
            qDebug() << QString::fromStdString ( result );
		}
	} else {
		char error[1024];
		rep.soap_sprint_fault(error, 1024);
		returnResult = QScriptValue(QString(error));
        qDebug() << QString(error);
    }
	return returnResult;
}

void JasperServerWS::saveDocument(const struct soap_multipart & attachment, const QString & path)
{
	QFile file (path);
	file.open ( QIODevice::WriteOnly | QIODevice::Truncate );
	file.write(attachment.ptr, attachment.size);
	file.close();
}

/*!
  Baja y abre un archivo JasperServer, segun el tipo especificado
  */
QScriptValue JasperServerWS::downloadAndOpenJasperServerFile (const QString &xmlRequest, const QString &fileSuffix)
{
	QString fileTemplate = QString("%1/XXXXXX.%2").arg(QDir::tempPath()).arg(fileSuffix);
	QTemporaryFile file(fileTemplate);
	file.setAutoRemove(false);
	QScriptValue v;
	if ( file.open() ) {
		QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
		v = downloadJasperServerFile(xmlRequest, true, "", &file);
		QApplication::restoreOverrideCursor();
		file.close();
		if ( v.toString() == "ok" ) {
			QDesktopServices::openUrl(QUrl::fromLocalFile(file.fileName()));
		}
	} else {
		v = QScriptValue(trUtf8("No se pudo abrir un archivo temporal"));
	}
	return v;
}

QScriptValue JasperServerWS::toScriptValueSharedPointer(QScriptEngine *engine, const QSharedPointer<JasperServerWS> &in)
{
	return engine->newQObject(in.data(), QScriptEngine::AutoOwnership);
}

void JasperServerWS::fromScriptValueSharedPointer(const QScriptValue &object, QSharedPointer<JasperServerWS> &out)
{
	out = QSharedPointer<JasperServerWS>(qobject_cast<JasperServerWS *>(object.toQObject()));
}
