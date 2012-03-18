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

#include <QApplication>
#include <QSplashScreen>
#include <QCleanlooksStyle>
#include <QCDEStyle>
#include <QMotifStyle>
#include <QPlastiqueStyle>
#include <QWindowsStyle>
#include <QWindowsVistaStyle>
#include <QWindowsXPStyle>
#include <QTranslator>
#include <QObject>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QDir>
#include <QUiLoader>
#include <QThread>
//#include <stdio.h>
#include "configuracion.h"
#include "globales.h"
#include "forms/logindlg.h"
#include "forms/changepassworddlg.h"
#include "dao/database.h"
#include "dao/beans/beansfactory.h"
#include "dao/userdao.h"
#include "scripts/perpscriptengine.h"
#include "checkstatus.h"
#include "forms/perpmainwindow.h"
#include <QPointer>

void establecerEstilo(QApplication *app, int estilo);
PERPMainWindow * loadMainUi();

int main(int argc, char *argv[])
{
	PERPMainWindow *mainWin = 0;

	QApplication app(argc, argv);
	Q_INIT_RESOURCE(resources);

	// Cargamos el archivo con las traducciones
	QTranslator translator;
	if (translator.load("qt_es", app.applicationDirPath()) == false) {
		QMessageBox::warning(0, QString::fromUtf8(APP_NAME), 
							 QObject::trUtf8("Ha sido imposible cargar el fichero con las traducciones."));
	} else {
		app.installTranslator(&translator);
	}
	
	establecerEstilo(&app, configuracion.lookAndFeel());

    QString dirPlugins = QString("%1/plugins").arg(QCoreApplication::applicationDirPath());
	QString scriptDirPlugins = QString("%1").arg(configuracion.tempPath());
	qApp->addLibraryPath(dirPlugins);
	qApp->addLibraryPath(scriptDirPlugins);
	qDebug() << "Lista de directorios con plugins: ";
	qDebug() << qApp->libraryPaths();

	if ( !Database::createConnection(BASE_CONNECTION) ) {
		QString errorMessage = QObject::trUtf8("Ha sido imposible conectar con la base de datos. No es posible iniciar AlephERP.\r\nError: %1").
				arg(Database::openMessage);
		QMessageBox::critical(0, QString::fromUtf8(APP_NAME), errorMessage);
		return 0;
	}

	// http://blogs.kde.org/node/3919
	QPointer<LoginDlg> loginDlg = new LoginDlg;
	loginDlg->setModal(true);
	loginDlg->exec();
    qApp->setProperty("userName", loginDlg->userName());
    if ( loginDlg->loginOk() == LoginDlg::NOT_LOGIN ) {
		return 0;
	} else if ( loginDlg->loginOk() == LoginDlg::EMPTY_PASSWORD ) {
		QPointer<ChangePasswordDlg> passDlg = new ChangePasswordDlg(true);
		passDlg->setModal(true);
		passDlg->exec();
		delete passDlg;
	}
    configuracion.setLastLoggerUser(loginDlg->userName());
    delete loginDlg;

	QSplashScreen *splash = new QSplashScreen (
		QPixmap(":/aplicacion/images/splashscreenimg.png"));
	splash->show();
	QCoreApplication::processEvents();

	splash->showMessage(QObject::trUtf8("Comprobando e inicializando las definiciones de sistema..."), Qt::AlignRight | Qt::AlignBottom);
	if ( !Database::initSystemsBeans() ) {
		QMessageBox::critical(0, QString::fromUtf8(APP_NAME),
							  QObject::trUtf8("No se han podido leer las definiciones de sistema. "
									 "Los datos en base de datos están corruptos o faltan. "
									 "El sistema no puede iniciarse. Póngase en contacto con <i>"
									 "Aleph Sistemas de Información</i>."));
		splash->close();
		delete splash;
		return 0;
	} else {
		qApp->setProperty("permissions", UserDAO::permission(qApp->property("userName").toString()));
		splash->showMessage(QObject::trUtf8("Creando el intérprete de código..."), Qt::AlignRight | Qt::AlignBottom);
		// La primera vez que se ejecuta el intérprete de javascript debe cargar una serie
		// de librerias y demás que lleva un tiempo. Lo hacemos en la carga.
		PERPScriptEngine::engine();
		splash->showMessage(QObject::trUtf8("Creando ventana principal..."), Qt::AlignRight | Qt::AlignBottom);
		mainWin = loadMainUi();
		if ( mainWin == NULL ) {
			return 0;
		}
		qApp->setProperty("MainWindowPointer", qVariantFromValue((void *) mainWin));
		splash->showMessage(QObject::trUtf8("Iniciando la aplicación..."), Qt::AlignRight | Qt::AlignBottom);
		mainWin->init();
		configuracion.applyDimensionForm(mainWin);
		configuracion.applyPosForm(mainWin);
		mainWin->show();

		splash->finish(mainWin);
	
		QThread thread;
		CheckStatus chk;
		thread.start();
		chk.moveToThread(&thread);
		chk.init();

		splash->close();
		delete splash;

		bool rc = app.exec();

		thread.quit();
		delete mainWin;
		return rc;
	}
}

void establecerEstilo(QApplication *app, int estilo)
{
	switch (estilo ) {
		case LOOK_CDE:
			app->setStyle(new QCDEStyle);
			break;
			
		case LOOK_CLEANLOOK:
			app->setStyle(new QCleanlooksStyle);
			break;
		case LOOK_MOTIF:
			app->setStyle(new QMotifStyle);
			break;
		case LOOK_PLASTIQUE:
			app->setStyle(new QPlastiqueStyle);
			break;
		case LOOK_WINDOWS:
			app->setStyle(new QWindowsStyle);
			break;
#ifdef WINDOWS		
		case LOOK_VISTA:
			app->setStyle(new QWindowsVistaStyle);
			break;
		case LOOK_XP:
			app->setStyle(new QWindowsXPStyle);
			break;
#endif
		default:
			app->setStyle(new QPlastiqueStyle);
			break;
	}
}

PERPMainWindow * loadMainUi()
{
	QString fileName = QString("%1/main.qmaindlg.ui").
					   arg(QDir::fromNativeSeparators(configuracion.tempPath()));
	QUiLoader uiLoader;
	QString pluginDir = QString("%1/../plugins/designer").arg(qApp->applicationDirPath());
	qDebug() << QString("Buscando plugins en '%1'").arg(pluginDir);
	QFile file (fileName);
	QWidget *tmp_widget = NULL;
	PERPMainWindow *widget = NULL;
	QString mensaje = QObject::trUtf8("No se ha podido cargar la interfaz de usuario de este formulario. Existe un problema en la definición de las tablas de sistema de su programa.");

	if ( file.exists() ) {
		uiLoader.addPluginPath(pluginDir);
		file.open( QFile::ReadOnly );
		tmp_widget = uiLoader.load(&file);
		if ( tmp_widget ) {
			widget = qobject_cast<PERPMainWindow *>(tmp_widget);
			if ( widget == NULL ) {
				qDebug() << QString("Fallo en la transformacion (cast) del widget en el fichero %1").arg(fileName);
			}
		} else {
			qDebug() << QString("Fallo en la carga del widget en el fichero %1").arg(fileName);
		}
		if ( widget == NULL ) {
			QMessageBox::warning(0, QObject::trUtf8(APP_NAME), mensaje, QMessageBox::Ok);
		}
	} else {
		qDebug() << QString("El fichero %1 no existe").arg(fileName);
		
		QMessageBox::warning(0, QObject::trUtf8(APP_NAME), mensaje, QMessageBox::Ok);
	}
	return widget;
}
