/***************************************************************************
 *   Copyright (C) 2007 by David Pinelo   *
 *   david.pinelo@alephsistemas.es   *
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
#include "configuracion.h"
#include "globales.h"
#include <QDateTime>
#include <QResource>
#include <QFile>
#include <QTableView>
#include <QHeaderView>
#include <QMainWindow>
#include <QPair>
#include <QDir>

Configuracion configuracion;

#define KEY_SERVIDORDB					"db/servidor"
#define KEY_USUARIODB					"db/usuario"
#define KEY_PASSWORDDB					"db/password"
#define KEY_NOMBREDB					"db/nombre"
#define KEY_PORTDB                      "db/port"
#define KEY_TIPO_CONEXION				"db/TipoConexion"
#define KEY_DSN_ODBC					"db/DSN_ODBC"
#define KEY_CODIFICACION_BBDD			"db/codificacion"
#define KEY_ESQUEMA_BBDD				"db/esquema_bbdd"
#define KEY_SYSTEM_TABLE_PREFIX			"db/system_table_prefix"
#define KEY_FILESYSTEM_ENCODING			"db/filesystem_encoding"

#define KEY_FORMS_DIMENSIONS			"forms/size/"
#define KEY_FORMS_POS					"forms/pos/"
#define KEY_MAIN_WINDOW_STATE			"forms/mainWindow"

#define KEY_DEBUGGER_ENABLED			"generales/debuggerEnabled"
#define KEY_TEMP_DIRECTORY				"generales/DirectorioTemporal"
#define KEY_LOOK_AND_FEEL				"generales/LookAndFeel"
#define KEY_FIRST_DAY_OF_WEEK			"generales/PrimerDiaSemana"
#define KEY_CHECK                       "generales/business"

#define KEY_EXTERNAL_RESOURCE			"generales/externalResource"
#define KEY_MDI_TAB_VIEW				"generales/mdiTabView"

#define KEY_TABLEVIEW_CONFIG			"generales/tableView"
#define KEY_TABLEVIEW_CONFIG_ORDERS		"generales/tableViewOrders"
#define KEY_OTHERS						"generales/otros"

#define KEY_LAST_LOGGED_USER			"generales/ultimoUsuarioLogado"

#define KEY_MODEL_REFRESH_TIMEOUT		"generales/modelRefreshTimeOut"
#define KEY_STRONG_FILTER_ROW_COUNT 	"generales/strongFilterRowCountLimit"
#define KEY_MODEL_TIME_BETWEEN_RELOADS	"generales/timeBetweenReloads"

#define KEY_MAIL_SERVER					"mail/server"
#define KEY_MAIL_FROM					"mail/from"	
#define KEY_MAIL_RESPONDER_A			"mail/responder_a"
#define KEY_MAIL_COPIA_A				"mail/copia_a"

#define KEY_VISOR_PDF_INTERNO				"generales/usarVisorInternoPDF"
#define KEY_VISOR_PDF_EXTERNO				"generales/visorPDF"

#define KEY_FONT_SIZE_REFERENCE_REPORTS		"generales/fontSizeReferenceReports"
#define FONT_SIZE_REFERENCE_REPORTS_POR_DEFECTO	10

#define KEY_SCRIPT						"script/%1"

Configuracion::Configuracion(QObject *parent) : QObject(parent)
{
	settings = new QSettings(COMPANY, APP_NAME);
	locale = new QLocale(QLocale::Spanish, QLocale::Spain);	
	init();
	m_codec = QTextCodec::codecForName(m_BBDDEncoding.toAscii());
	if ( m_fileSystemEncoding.isEmpty() ) {
		m_fileSystemCodec = QTextCodec::codecForLocale();
	} else {
		m_fileSystemCodec = QTextCodec::codecForName(m_fileSystemEncoding.toLatin1());
	}
}


Configuracion::~Configuracion()
{
	almacenar();
	if ( !m_externalResource.isEmpty() && QFile::exists(m_externalResource) ) {
		qDebug() << "Closing external resource file: " << QResource::unregisterResource(m_externalResource);
	}
	delete settings;
	delete locale;
}

QString Configuracion::check()
{
    return m_check;
}

QDate Configuracion::minimunDate()
{
	QDate d(2009, 01, 01);
	return d;
}

QString Configuracion::key(const QString &value)
{
	return settings->value(value).toString();
}

QString Configuracion::scriptKey(const QString &value)
{
	QString key = QString(KEY_SCRIPT).arg(value);
	return settings->value(key).toString();
}

void Configuracion::setScriptKey(const QString &key, const QVariant &value)
{
	QString k = QString(KEY_SCRIPT).arg(key);
	settings->setValue(k, value);
}

void Configuracion::init (void)
{
	m_dbServer = settings->value(KEY_SERVIDORDB).toString();
	m_userDb = settings->value(KEY_USUARIODB).toString();
	m_passwordDb = settings->value(KEY_PASSWORDDB).toString();
	m_dbName = settings->value(KEY_NOMBREDB).toString();
    m_portDb = settings->value(KEY_PORTDB, 5432).toInt();
	m_BBDDEncoding = settings->value(KEY_CODIFICACION_BBDD, "UTF-8").toString();
	m_dbSchema = settings->value(KEY_ESQUEMA_BBDD, "public").toString();
	m_tipoConexion = settings->value(KEY_TIPO_CONEXION).toString();
	m_dsnODBC = settings->value(KEY_DSN_ODBC).toString();
	m_systemTablePrefix = settings->value(KEY_SYSTEM_TABLE_PREFIX).toString();
	m_fileSystemEncoding = settings->value(KEY_FILESYSTEM_ENCODING).toString();
	m_timeBetweenReloads = settings->value(KEY_MODEL_TIME_BETWEEN_RELOADS, 2000).toInt();

    m_check = settings->value(KEY_CHECK, "").toString();

	m_debuggerEnabled = settings->value(KEY_DEBUGGER_ENABLED, true).toBool();

	m_firstDayOfWeek = settings->value(KEY_FIRST_DAY_OF_WEEK, "monday").toString();

	m_tempPath = settings->value(KEY_TEMP_DIRECTORY).toString();

	m_lastLoggedUser = settings->value(KEY_LAST_LOGGED_USER).toString();

	m_modelRefreshTimeout = settings->value(KEY_MODEL_REFRESH_TIMEOUT, 30000).toInt();
	m_strongFilterRowCountLimit = settings->value(KEY_STRONG_FILTER_ROW_COUNT, 200).toInt();

	m_externalResource = settings->value(KEY_EXTERNAL_RESOURCE, "").toString();
	if ( !m_externalResource.isEmpty() && QFile::exists(m_externalResource) ) {
		qDebug() << "Open external resource file: " << QResource::registerResource(m_externalResource);
	}

	m_mainWindowState = settings->value(KEY_MAIN_WINDOW_STATE).toByteArray();

	// Leemos las dimensiones de todas las ventanas.
	settings->beginGroup(KEY_FORMS_DIMENSIONS);
	QStringList forms = settings->childKeys();
	for ( int i = 0 ; i < forms.size() ; i++ ) {
		m_dimensionesForms[forms[i]] = settings->value(forms[i]).toByteArray();
	}
	settings->endGroup();

	// Leemos las dimensiones de todas las ventanas.
	settings->beginGroup(KEY_FORMS_POS);
	forms = settings->childKeys();
	for ( int i = 0 ; i < forms.size() ; i++ ) {
		m_posForms[forms[i]] = settings->value(forms[i], QPoint(0,0)).toPoint();
	}
	settings->endGroup();

	m_internalPDFViewer = settings->value(KEY_VISOR_PDF_INTERNO, true).toBool();
	m_externalPDFViewer = settings->value(KEY_VISOR_PDF_EXTERNO, "").toString();
	
	mailServer = settings->value(KEY_MAIL_SERVER).toString();
	mailResponderA = settings->value(KEY_MAIL_RESPONDER_A).toString();
	mailCopiaA = settings->value(KEY_MAIL_COPIA_A).toString();
	mailFrom = settings->value(KEY_MAIL_FROM).toString();

	m_mdiTabView = settings->value(KEY_MDI_TAB_VIEW, false).toBool();
	
	m_lookAndFeel = settings->value(KEY_LOOK_AND_FEEL, 0).toInt();

	m_fontSizeReferenceReports = settings->value(KEY_FONT_SIZE_REFERENCE_REPORTS, FONT_SIZE_REFERENCE_REPORTS_POR_DEFECTO).toInt();
}


QString Configuracion::dbServer() const
{
	return m_dbServer;
}


void Configuracion::setDbServer ( const QString& theValue )
{
	m_dbServer = theValue;
}


QString Configuracion::userDb() const
{
	return m_userDb;
}


void Configuracion::setUserDb ( const QString& theValue )
{
	m_userDb = theValue;
}


QString Configuracion::passwordDb() const
{
	return m_passwordDb;
}


void Configuracion::setPasswordDb ( const QString& theValue )
{
	m_passwordDb = theValue;
}


QString Configuracion::dbName() const
{
	return m_dbName;
}


void Configuracion::setDbName ( const QString& theValue )
{
	m_dbName = theValue;
}

int Configuracion::portDb() const
{
    return m_portDb;
}


void Configuracion::setPortDb ( int theValue )
{
    m_portDb = theValue;
}

void Configuracion::applyDimensionForm(QWidget *form) const
{
	QByteArray size;
	
	if ( m_dimensionesForms.contains(form->objectName()) ) {
		size = m_dimensionesForms[form->objectName()];
		form->restoreGeometry(size);
		QMainWindow *main = qobject_cast<QMainWindow *> (form);
		if ( main != NULL ) {
			main->restoreState(m_mainWindowState);
		}
	}
}

void Configuracion::applyPosForm(QWidget *form) const
{
	QPoint point;
	
	if ( m_posForms.contains(form->objectName()) ) {
		point = m_posForms[form->objectName()];
	} else {
		point.setX(0);
		point.setY(0);
	}
	form->move(point);
}

void Configuracion::savePosForm(QWidget *form)
{
	m_posForms[form->objectName()] = form->pos();
}

void Configuracion::saveDimensionForm(QWidget *form)
{
	m_dimensionesForms[form->objectName()] = form->saveGeometry();
	QMainWindow *main = qobject_cast<QMainWindow *> (form);
	if ( main != NULL ) {
		m_mainWindowState = main->saveState();
	}
}

QString Configuracion::externalResource()
{
	return m_externalResource;
}

void Configuracion::setExternalResource(const QString &temp)
{
	m_externalResource = temp;
}

QString Configuracion::firstDayOfWeek()
{
	return m_firstDayOfWeek;
}

void Configuracion::setFirstDayOfWeek(const QString &temp)
{
	m_firstDayOfWeek = temp;
}

QStringList Configuracion::tableViewColumnOrder(QTableView *tw)
{
	QString key = KEY_TABLEVIEW_CONFIG;
	key = key + "/" + tw->objectName();
	key = key + "Order";

	QVariant valor = settings->value(key);
	return valor.toStringList();
}

QStringList Configuracion::tableViewColumnSort(QTableView *tw)
{
	QString key = KEY_TABLEVIEW_CONFIG_ORDERS;
	key = key + "/" + tw->objectName();
	key = key + "Sort";

	QVariant valor = settings->value(key);
	return valor.toStringList();
}

/*!
  Salva el orden de las columnas del QTableView \a tw. \a columnNames hace referencia
  al DBField (dbFieldName) o nombre de la columna del modelo BaseBean. Se guarda el nombre
  de la columna por un caso raro que se ve en DBRecordDlg. Se almacena, también el orden
  de las mismas, "ASC", "DESC"...
  */
void Configuracion::saveTableViewColumnOrder(QTableView * tw, QStringList columnNames, QStringList columnOrder)
{
	QString key = KEY_TABLEVIEW_CONFIG;
	key = key + "/" + tw->objectName();
	key = key + "Order";
	settings->setValue(key, columnNames);
	key = KEY_TABLEVIEW_CONFIG_ORDERS;
	key = key + "/" + tw->objectName();
	key = key + "Sort";
	settings->setValue(key, columnOrder);
	settings->sync();
}

void Configuracion::applyTableViewState(QTableView *tw)
{
	QString key = KEY_TABLEVIEW_CONFIG;
	key = key + "/" + tw->objectName();

	QVariant valor = settings->value(key);
	if ( !valor.isNull() && valor.isValid() && tw->model() != NULL ) {
		QList<QVariant> anchos = valor.toList();
		if ( anchos.size() == tw->model()->columnCount() ) {
			for ( int i = 0 ; i < tw->model()->columnCount() ; i++ ) {
				if ( anchos.at(i).canConvert<int>() ) {
					tw->setColumnWidth(i, anchos.at(i).toInt());
				}
			}
		}
	}
}

void Configuracion::saveTableViewState(QTableView *tw)
{
	QList<QVariant> widths;
	QString key = KEY_TABLEVIEW_CONFIG;
	key = key + "/" + tw->objectName();
/*	if ( tw != NULL ) {
		QByteArray state = tw->horizontalHeader()->saveState();
		settings->setValue(key, state);
		settings->sync();
	}*/

	if ( tw->model() != NULL ) {
		for ( int i = 0 ; i < tw->model()->columnCount() ; i++ ) {
			widths << tw->columnWidth(i);
		}
		settings->setValue(key, widths);
		settings->sync();
	}
}

QString Configuracion::tempPath() const
{
	QDir dir;
	if ( !dir.exists(m_tempPath) ) {
		if ( !dir.mkdir(m_tempPath) ) {
			qDebug() << "Configuracion::tempPath(): No se ha podido crear el directorio de archivos temporales: " << m_tempPath;
		}
	}
	return m_tempPath;
}

void Configuracion::saveRegistryValue(const QString &key, const QVariant &value)
{
	QString registryKey = QString("%1/%2").arg(KEY_OTHERS).arg(key);
	settings->setValue(registryKey, value);
}

QVariant Configuracion::loadRegistryValue(const QString &key)
{
	QString registryKey = QString("%1/%2").arg(KEY_OTHERS).arg(key);
	return settings->value(registryKey);
}

QString Configuracion::lastLoggedUser()
{
	return m_lastLoggedUser;
}

void Configuracion::setLastLoggerUser(const QString &user)
{
	m_lastLoggedUser = user;
}

void Configuracion::almacenar(void )
{
	QString strForms;
	QMapIterator<QString, QByteArray> i(m_dimensionesForms);
	QMapIterator<QString, QPoint> pos(m_posForms);
	
	settings->setValue(KEY_SERVIDORDB, m_dbServer);
	settings->setValue(KEY_USUARIODB, m_userDb);
	settings->setValue(KEY_PASSWORDDB, m_passwordDb);
    settings->setValue(KEY_PORTDB, m_portDb);
	settings->setValue(KEY_NOMBREDB, m_dbName);
	settings->setValue(KEY_CODIFICACION_BBDD, m_BBDDEncoding);
	settings->setValue(KEY_ESQUEMA_BBDD, m_dbSchema);
	settings->setValue(KEY_TIPO_CONEXION, m_tipoConexion);
	settings->setValue(KEY_DSN_ODBC, m_dsnODBC);
	settings->setValue(KEY_SYSTEM_TABLE_PREFIX, m_systemTablePrefix);
	settings->setValue(KEY_FILESYSTEM_ENCODING, m_fileSystemEncoding);;

	settings->setValue(KEY_DEBUGGER_ENABLED, m_debuggerEnabled);
	settings->setValue(KEY_EXTERNAL_RESOURCE, m_externalResource);

	settings->setValue(KEY_FIRST_DAY_OF_WEEK, m_firstDayOfWeek);

	settings->setValue(KEY_MAIN_WINDOW_STATE, m_mainWindowState);

	settings->setValue(KEY_LAST_LOGGED_USER, m_lastLoggedUser);

	while (i.hasNext())
	{
		i.next();
		strForms = KEY_FORMS_DIMENSIONS + i.key();
		settings->setValue(strForms, i.value());
	}

	while (pos.hasNext())
	{
		pos.next();
		strForms = KEY_FORMS_POS + pos.key();
		settings->setValue(strForms, pos.value());
	}

	settings->setValue(KEY_TEMP_DIRECTORY, m_tempPath);
	settings->setValue(KEY_LOOK_AND_FEEL, m_lookAndFeel);

	settings->setValue(KEY_VISOR_PDF_INTERNO, m_internalPDFViewer );
	settings->setValue(KEY_MAIL_SERVER, mailServer);
	settings->setValue(KEY_MAIL_RESPONDER_A, mailResponderA);
	settings->setValue(KEY_MAIL_COPIA_A, mailCopiaA);
	settings->setValue(KEY_MAIL_FROM, mailFrom);
	settings->setValue(KEY_VISOR_PDF_EXTERNO, m_externalPDFViewer);
	settings->setValue(KEY_FONT_SIZE_REFERENCE_REPORTS, m_fontSizeReferenceReports);
	
	settings->sync();
}

QLocale* Configuracion::getLocale() const
{
	return locale;
}

QString Configuracion::getConnectionType() const
{
	return m_tipoConexion;
}


void Configuracion::setConnectionType ( const QString& theValue )
{
	m_tipoConexion = theValue;
}

QString Configuracion::dsnODBC() const
{
	return m_dsnODBC;
}


void Configuracion::setDsnODBC ( const QString& theValue )
{
	m_dsnODBC = theValue;
}

QString Configuracion::systemTablePrefix() const
{
	return m_systemTablePrefix;
}

void Configuracion::setSystemTablePrevix (const QString &value)
{
	m_systemTablePrefix = value;
}

QString Configuracion::fileSystemEncoding() const
{
	return m_fileSystemEncoding;
}

void Configuracion::setSileSystemEncoding (const QString &value)
{
	m_fileSystemEncoding = value;
}

int Configuracion::lookAndFeel() const
{
	return m_lookAndFeel;
}


void Configuracion::setLookAndFeel ( int theValue )
{
	m_lookAndFeel = theValue;
}

QString Configuracion::bbddEncoding() const
{
	return m_BBDDEncoding;
}


void Configuracion::setBBDDEnconding ( const QString& theValue )
{
	m_BBDDEncoding = theValue;
}


QTextCodec* Configuracion::bbddCodec() const
{
	return m_codec;
}

QTextCodec* Configuracion::fileSystemCodec() const
{
	return m_fileSystemCodec;
}

QString Configuracion::dbSchema() const
{
	return m_dbSchema;
}


void Configuracion::setDbSchema ( const QString& theValue )
{
	m_dbSchema = theValue;
}

bool Configuracion::debuggerEnabled()
{
	return m_debuggerEnabled;
}

void Configuracion::setDebuggerEnabled(bool value)
{
	m_debuggerEnabled = value;
}

bool Configuracion::mdiTabView()
{
	return m_mdiTabView;
}

bool Configuracion::internalPDFViewer() const
{
	return m_internalPDFViewer;
}

int Configuracion::fontSizeReferenceReports()
{
	return m_fontSizeReferenceReports;
}

void Configuracion::setFontSizeReferenceReports( int theValue )
{
	m_fontSizeReferenceReports = theValue;
}

int Configuracion::modelRefreshTimeout()
{
	return m_modelRefreshTimeout;
}

void Configuracion::setModelRefreshTimeout(int value)
{
	m_modelRefreshTimeout = value;
}

void Configuracion::setInternalPDFViewer ( bool theValue )
{
	m_internalPDFViewer = theValue;
}


QString Configuracion::getMailServer() const
{
	return mailServer;
}


void Configuracion::setMailServer ( const QString& theValue )
{
	mailServer = theValue;
}


QString Configuracion::getMailResponderA() const
{
	return mailResponderA;
}


void Configuracion::setMailResponderA ( const QString& theValue )
{
	mailResponderA = theValue;
}


QString Configuracion::getMailCopiaA() const
{
	return mailCopiaA;
}


void Configuracion::setMailCopiaA ( const QString& theValue )
{
	mailCopiaA = theValue;
}


QString Configuracion::getMailFrom() const
{
	return mailFrom;
}


void Configuracion::setMailFrom ( const QString& theValue )
{
	mailFrom = theValue;
}


QString Configuracion::externalPDFViewer() const
{
	return m_externalPDFViewer;
}


void Configuracion::setExternalPDFViewer ( const QString& theValue )
{
	m_externalPDFViewer = theValue;
}

QString Configuracion::formatToMoney(const QString &value)
{
	QString money;
	money = QString("%1 %2").arg(value).arg(EUR);
	return money;
}

/*!
  Para una ejecución de un programa, genera un número único. Esto es muy útil para darle
  un valor predefinido a los DBField con serial, y que se utilicen en sentencias de INSERT
  */
int Configuracion::uniqueId()
{
	static int lastId;
	if ( lastId == 0 ) {
		QDateTime now = QDateTime::currentDateTime();
		QDateTime orig (QDate(now.date().year(), 1, 1), QTime(0, 01));
		lastId = orig.secsTo(now);
	}
	QMutexLocker locker(&m_mutex);
	lastId++;
	return lastId;
}

int Configuracion::timeBetweenReloads()
{
	return m_timeBetweenReloads;
}

void Configuracion::setTimeBetweenReloads(int time)
{
	m_timeBetweenReloads = time;
}

int Configuracion::strongFilterRowCountLimit()
{
	return m_strongFilterRowCountLimit;
}

void Configuracion::setStrongFilterRowCountLimit(int limit)
{
	m_strongFilterRowCountLimit = limit;
}
