#include "checkstatus.h"
#include "dao/database.h"
#include "configuracion.h"
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QApplication>
#include <QMessageBox>
#include <QTimer>

CheckStatus::CheckStatus(QObject *parent) : QObject(parent)
{
	m_manager = NULL;
	m_dbOpen = false;
	m_manager = NULL;
	m_timer = NULL;
    if ( configuracion.check().isEmpty() ) {
        QMessageBox::critical(0, QString::fromUtf8(APP_NAME),
                              QString::fromUtf8("Aplicación no configurada. Se cerrara."), QMessageBox::Ok);
		QTimer::singleShot(250, qApp, SLOT(quit()));
        return;
    }
    if ( configuracion.check() == "1qazxsw23edcvfr45tgbnhy67ujm" ){
        m_check = false;
	} else {
        m_url = configuracion.check();
        m_check = true;
	}
}

CheckStatus::~CheckStatus() {
    if ( m_manager != NULL ) {
        delete m_manager;
    }
	if ( m_timer != NULL ) {
		delete m_timer;
	}
}

void CheckStatus::init() {
	m_manager = new QNetworkAccessManager();
	connect(m_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(setState(QNetworkReply*)));

	m_timer = new QTimer();
	m_timer->setInterval(1*60*1000);
	QObject::connect(m_timer, SIGNAL(timeout()), this, SLOT(check()));
	m_timer->start();
}

void CheckStatus::check() {
    if ( m_check ) {
        QString urlToGet = QString("http://www.alephsistemas.es/%1").arg(m_url);
        if ( m_manager != NULL ) {
            m_manager->get(QNetworkRequest(QUrl(urlToGet)));
        }
    }
}

void CheckStatus::setState(QNetworkReply *reply)
{
    if ( !m_check ) {
		return;
	}
	if ( !m_dbOpen ) {
		if ( Database::createConnection("CheckConnection") ) {
			m_dbOpen = true;
		}
	}
	QScopedPointer<QSqlQuery> qry (new QSqlQuery(Database::getQDatabase("CheckConnection")));
	if ( reply->error() == QNetworkReply::NoError ) {
		QByteArray data = reply->readAll();
		QString dataStr = QString(data);
		if ( dataStr.contains("0") ) {
			QString sql = QString("DELETE FROM %1_envvars WHERE variable='available'").arg(configuracion.systemTablePrefix());
			qry->prepare(sql);
			if ( qry->exec() ) {
				sql = QString("INSERT INTO %1_envvars (variable, data) VALUES('available', 'false')").
						arg(configuracion.systemTablePrefix());
				qry->prepare(sql);
				if ( !qry->exec() ) {
					qDebug() << "CheckState:setState: " << qry->lastError();
				}
			} else {
				qDebug() << "CheckState:setState: " << qry->lastError();
			}
			QTimer::singleShot(250, qApp, SLOT(quit()));
		} else if ( dataStr.contains("1") ) {
			QString sql = QString("DELETE FROM %1_envvars WHERE variable='available'").arg(configuracion.systemTablePrefix());
			qry->prepare(sql);
			if ( qry->exec() ) {
				sql = QString("INSERT INTO %1_envvars (variable, data) VALUES('available', 'true')").
						arg(configuracion.systemTablePrefix());
				qry->prepare(sql);
				if ( !qry->exec() ) {
					qDebug() << "CheckState:setState: " << qry->lastError();
				}
			} else {
				qDebug() << "CheckState:setState: " << qry->lastError();
			}
		}
	} else {
		QString sql = QString("SELECT data FROM %1_envvars WHERE variable='available'").arg(configuracion.systemTablePrefix());
		qry->prepare(sql);
		if ( qry->exec() && qry->first() ) {
			QString av = qry->value(0).toString();
			if ( av == "false" ) {
				QTimer::singleShot(250, qApp, SLOT(quit()));
				exit(1);
			}
		} else {
		   qDebug() << "CheckState:setState: " << qry->lastError();
		}
	}
}
