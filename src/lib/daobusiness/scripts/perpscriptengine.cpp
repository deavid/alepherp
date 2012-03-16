/***************************************************************************
 *   Copyright (C) 2011 by David Pinelo   *
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
#include "perpscriptengine.h"
#include <QScriptEngine>
#include <QCoreApplication>
#include <QDir>
#include <QButtonGroup>
#include <QProcess>
#include <QScriptEngine>
#ifndef QT_NO_SCRIPTTOOLS
 #include <QScriptEngineDebugger>
 #include <QMainWindow>
#endif
#include "scripts/perpscriptdialog.h"
#include "scripts/perpscriptsqlquery.h"
#include "scripts/perpscriptcommon.h"
#include "scripts/dbcommonsscriptprototypes.h"
#include "dao/beans/dbfield.h"
#include "dao/beans/dbrelation.h"
#include "dao/beans/basebean.h"
#include "forms/dbformdlg.h"
#include "widgets/dbcheckbox.h"
#include "widgets/dbchooserecordbutton.h"
#include "widgets/dbcodeedit.h"
#include "widgets/dbcombobox.h"
#include "widgets/dbdatetimeedit.h"
#include "widgets/dbdetailview.h"
#include "widgets/dbfileupload.h"
#include "widgets/dbframebuttons.h"
#include "widgets/dbhtmleditor.h"
#include "widgets/dblabel.h"
#include "widgets/dblineedit.h"
#include "widgets/dbnumberedit.h"
#include "widgets/dbtableview.h"
#include "widgets/dbtextedit.h"
#include "widgets/dbtreeview.h"
#include "widgets/dbfiltertableview.h"
#include "widgets/dblistview.h"
#include "configuracion.h"

#ifndef QT_NO_SCRIPTTOOLS
/** Debugger */
QScriptEngineDebugger *PERPScriptEngine::m_debugger;
/** Ventana de debug */
QMainWindow *PERPScriptEngine::m_debugWindow;
#endif

/** Esto nos va a permitir crear objetos QSqlQuery desde QtScript */
Q_SCRIPT_DECLARE_QMETAOBJECT(PERPScriptSqlQuery, QObject*)
/** También podremos crear formularios de búsqueda desde QtScript */
Q_SCRIPT_DECLARE_QMETAOBJECT(PERPScriptDialog, QObject*)
Q_SCRIPT_DECLARE_QMETAOBJECT(DBField, QObject*)
Q_SCRIPT_DECLARE_QMETAOBJECT(DBRelation, QObject*)
Q_SCRIPT_DECLARE_QMETAOBJECT(BaseBean, QObject*)
Q_SCRIPT_DECLARE_QMETAOBJECT(Configuracion, QObject*)
Q_SCRIPT_DECLARE_QMETAOBJECT(PERPScriptCommon, QObject*)

PERPScriptEngine::PERPScriptEngine()
{
}

PERPScriptEngine::~PERPScriptEngine()
{
}

QScriptEngine * PERPScriptEngine::engine()
{
	static QScriptEngine *singleton = 0;

	if ( singleton == 0 ) {
		singleton = new QScriptEngine();
		// Esto garantiza que el objeto se borra al cerrar la aplicación
		singleton->setParent(qApp);
		// Tipos disponibles para el motor de scripts
		PERPScriptEngine::registerScriptsTypes(singleton);
		// Para poder traducir los scripts
		singleton->installTranslatorFunctions();
		// Función común en los scripts
		PERPScriptEngine::importExtensions(singleton);
		qDebug() << "Listado de extensiones disponibles: " << singleton->availableExtensions ();
	}
	return singleton;
}

QScriptValue loadJS(QScriptContext *context, QScriptEngine *engine)
{
	QString result;
	for ( int i = 0; i < context->argumentCount(); ++i ) {
		if ( i > 0 ) {
			result.append(" ");
		}
		result.append(context->argument(i).toString());
	}
	QFile scriptFile(result);

	// check file is exited or not
	if ( !scriptFile.open(QIODevice::ReadOnly) ) {
		return QScriptValue();
	}

	// load file
	QTextStream stream (&scriptFile);
	QString s = stream.readAll();
	scriptFile.close();

	// set ScriptContext
	QScriptContext *parent = context->parentContext();
	if( parent != 0 ) {
		context->setActivationObject(context->parentContext()->activationObject());
		context->setThisObject(context->parentContext()->thisObject());
	}
	// execute script
	return engine->evaluate(s, result);
}

QScriptEngine * PERPScriptEngine::newEngine(QObject *parent, QScriptEngineDebugger *debugger)
{
	QScriptEngine *engine = new QScriptEngine(parent);
	if ( debugger != NULL ) {
		debugger->attachTo(engine);
	}
	PERPScriptEngine::registerScriptsTypes(engine);
	engine->installTranslatorFunctions();
	PERPScriptEngine::importExtensions(engine);
	return engine;
}

#ifndef QT_NO_SCRIPTTOOLS
QScriptEngineDebugger *PERPScriptEngine::debugger()
{
	if ( configuracion.debuggerEnabled() ) {
		if ( !PERPScriptEngine::m_debugger ) {
			PERPScriptEngine::m_debugger = new QScriptEngineDebugger(qApp);
			PERPScriptEngine::m_debugWindow = PERPScriptEngine::m_debugger->standardWindow();
			PERPScriptEngine::m_debugWindow->setWindowModality(Qt::ApplicationModal);
			//PERPScriptEngine::m_debugWindow->resize(1280, 704);
			PERPScriptEngine::m_debugger->attachTo(PERPScriptEngine::engine());
		}
	}
	return PERPScriptEngine::m_debugger;
}

QMainWindow *PERPScriptEngine::debugWindow()
{
	return PERPScriptEngine::m_debugWindow;
}
#endif


/*!
  Función disponible a los scripts para lanzar información de debug en los scripts
  */
QScriptValue printFunction(QScriptContext *context, QScriptEngine *engine)
{
	QString result;
	for ( int i = 0; i < context->argumentCount(); ++i ) {
		if ( i > 0 ) {
			result.append(" ");
		}
		result.append(context->argument(i).toString());
	}
	qDebug() << "Script: " << result;
	return engine->undefinedValue();
}

/*!
  Permitirá a los scripts cerrar la aplicación
  */
QScriptValue quitApplication(QScriptContext *context, QScriptEngine *engine)
{
	Q_UNUSED(context)
	Q_UNUSED(engine)
	exit(0);
	return QScriptValue();
}

/*!
  Esta función lee los scripts creados dentro del directorio script creado temporalmente
  segun los archivos guardados en BBDD. Esos scripts estarán disponibles
  a todos los que se ejecuten y estén en base de datos. De esta manera, conseguimos
  reutilización del código.
  */
void PERPScriptEngine::importExtensions(QScriptEngine *engine)
{
	QString path = QString("%1/script").arg(configuracion.tempPath());
	QDir exeDir (path);
	// Obtenemos los plugins que el usuario puede introducir
	QStringList scriptPlugins = exeDir.entryList(QDir::Dirs);
	QStringList importedPlugins = engine->importedExtensions();
	foreach ( QString plugin, scriptPlugins ) {
		if ( plugin != ".." && !importedPlugins.contains(plugin) ) {
			engine->importExtension(plugin);
			if ( engine->hasUncaughtException() ) {
				qDebug() << "PERPScriptEngine: importExtensions(): " << plugin << ". Error en linea: " <<
							engine->uncaughtExceptionLineNumber() <<
						" ERROR: " << engine->uncaughtException().toString();
				QStringList errors = engine->uncaughtExceptionBacktrace();
				foreach ( QString error, errors ) {
					qDebug() << "PERPScriptEngine: importExtensions(): " << error;
				}
			}
		}
	}
}

/*!
  Permite cargar dinámicamente, desde Javascript, extensiones adicionales
  que hubiese
  */
QScriptValue loadExtension(QScriptContext *context, QScriptEngine *engine)
{
	QString binding;
	if ( context->argumentCount() == 1 ) {
		binding = context->argument(0).toString();
	}
	QStringList importedExt = engine->importedExtensions();
	if ( importedExt.contains(binding) ) {
		return true;
	}
    QStringList availableExt = engine->availableExtensions();
    if ( !availableExt.contains(binding) ) {
        qDebug() << "loadExtension: binding: " << binding << " no está disponible";
        return false;
    }
	qDebug() << "loadExtension:: importing extension bindings " << binding;
	QScriptValue result = engine->importExtension( binding );
	if( result.isUndefined() ) {
		return true;
    } else if ( engine->hasUncaughtException() ) {
		qDebug() << "loadExtension: error: " << engine->uncaughtException().toString();
    }
	return false;
}

/*!
  Registra tipos comunes a los scripts, como pueden ser una función debug, o bien
  querys, dialogos... Se registran en globalObject, con lo cual estarán disponible
  para todos los contextos que existan de Engines.
  */
void PERPScriptEngine::registerScriptsTypes(QScriptEngine *engine)
{
	// Hacemos visible una función de debug
	QScriptValue fun = engine->newFunction(printFunction);
	engine->globalObject().setProperty("debug", fun);

	QScriptValue funQuit = engine->newFunction(quitApplication);
	engine->globalObject().setProperty("quitApplication", funQuit);

	QScriptValue funLoad = engine->newFunction(loadExtension);
	engine->globalObject().setProperty("loadExtension", funLoad);

	QScriptValue funLoadJS = engine->newFunction(loadJS);
	engine->globalObject().setProperty("loadJS", funLoadJS);

	QScriptValue config = engine->newQObject(&configuracion);
	engine->globalObject().setProperty("Config", config);

	// También hacemos posible la creación de querys a base de datos.
	QScriptValue ctorPERPScriptSqlQuery = engine->newFunction(PERPScriptSqlQuery::specialPERPScriptSqlQueryConstructor);
	QScriptValue metaObject = engine->newQMetaObject(&QObject::staticMetaObject, ctorPERPScriptSqlQuery);
	engine->globalObject().setProperty("PERPSqlQuery", metaObject);

	// scriptValueFromQMetaObject crea un constructor razonablemente bueno, aunque
	// no válido para nuestro caso
	/*
	QScriptValue processValue = engine->scriptValueFromQMetaObject<PERPScriptSqlQuery>();
	engine->globalObject().setProperty("PERPSqlQuery", processValue);
	qScriptRegisterMetaType(engine, PERPScriptSqlQuery::toScriptValue, PERPScriptSqlQuery::fromScriptValue);
	*/

	// Desde los scripts también se podrá llamar a los formularios de búsqueda
	QScriptValue dialogValue = engine->scriptValueFromQMetaObject<PERPScriptDialog>();
	engine->globalObject().setProperty("DBDialog", dialogValue);
	qScriptRegisterMetaType(engine, PERPScriptDialog::toScriptValue, PERPScriptDialog::fromScriptValue);

	qScriptRegisterSequenceMetaType<BaseBeanPointerList>(engine);
	qScriptRegisterSequenceMetaType<QList<DBField*> >(engine);
	qScriptRegisterSequenceMetaType<QList<DBRelation*> >(engine);
	qScriptRegisterSequenceMetaType<BaseBeanPointerList>(engine);

	// También registramos objetos como DBFields, DBRelations y BaseBean

	// Debemos registrar dos posibles objetos: El DBObject* y el QSharedPointer<DBObject *>
	QScriptValue dbFieldValue = engine->scriptValueFromQMetaObject<DBField>();
	engine->globalObject().setProperty("DBField", dbFieldValue);
	qScriptRegisterMetaType(engine, DBField::toScriptValue, DBField::fromScriptValue);
	qScriptRegisterMetaType(engine, DBField::toScriptValueSharedPointer, DBField::fromScriptValueSharedPointer);

	QScriptValue dbRelationValue = engine->scriptValueFromQMetaObject<DBRelation>();
	engine->globalObject().setProperty("DBRelation", dbRelationValue);
	qScriptRegisterMetaType(engine, DBRelation::toScriptValue, DBRelation::fromScriptValue);
	qScriptRegisterMetaType(engine, DBRelation::toScriptValueSharedPointer, DBRelation::fromScriptValueSharedPointer);

	QScriptValue baseBeanValue = engine->scriptValueFromQMetaObject<BaseBean>();
	engine->globalObject().setProperty("BaseBean", baseBeanValue);
	qScriptRegisterMetaType(engine, BaseBean::toScriptValue, BaseBean::fromScriptValue);
	qScriptRegisterMetaType(engine, BaseBean::toScriptValueSharedPointer, BaseBean::fromScriptValueSharedPointer);

	QScriptValue dbFormValue = engine->scriptValueFromQMetaObject<DBFormDlg>();
	engine->globalObject().setProperty("DBForm", dbFormValue);
	qScriptRegisterMetaType(engine, DBFormDlg::toScriptValue, DBFormDlg::fromScriptValue);

	QScriptValue dbFilterTableViewValue = engine->scriptValueFromQMetaObject<DBFilterTableView>();
	engine->globalObject().setProperty("DBFilterTableView", dbFilterTableViewValue);
	qScriptRegisterMetaType(engine, DBFilterTableView::toScriptValue, DBFilterTableView::fromScriptValue);
	engine->setDefaultPrototype(qMetaTypeId<DBFilterTableView*>(), engine->newQObject(DBCommonsScriptPrototypes::instance()->dbFilterTableView()));

	QScriptValue dbListView = engine->scriptValueFromQMetaObject<DBListView>();
	engine->globalObject().setProperty("DBListView", dbListView);
	qScriptRegisterMetaType<DBListView*>(engine, DBListView::toScriptValue, DBListView::fromScriptValue);
	engine->setDefaultPrototype(qMetaTypeId<DBListView*>(), engine->newQObject(DBCommonsScriptPrototypes::instance()->dbListView()));

	QScriptValue dbDetailView = engine->scriptValueFromQMetaObject<DBDetailView>();
	engine->globalObject().setProperty("DBDetailView", dbDetailView);
	qScriptRegisterMetaType<DBDetailView*>(engine, DBDetailView::toScriptValue, DBDetailView::fromScriptValue);
	engine->setDefaultPrototype(qMetaTypeId<DBDetailView*>(), engine->newQObject(DBCommonsScriptPrototypes::instance()->dbDetailView()));

	QScriptValue dbFileUpload = engine->scriptValueFromQMetaObject<DBFileUpload>();
	engine->globalObject().setProperty("DBFileUpload", dbFileUpload);
	qScriptRegisterMetaType<DBFileUpload*>(engine, DBFileUpload::toScriptValue, DBFileUpload::fromScriptValue);
	engine->setDefaultPrototype(qMetaTypeId<DBFileUpload*>(), engine->newQObject(DBCommonsScriptPrototypes::instance()->dbFileUpload()));

	QScriptValue dbHtmlEditor = engine->scriptValueFromQMetaObject<DBHtmlEditor>();
	engine->globalObject().setProperty("DBHtmlEditor", dbHtmlEditor);
	qScriptRegisterMetaType<DBHtmlEditor*>(engine, DBHtmlEditor::toScriptValue, DBHtmlEditor::fromScriptValue);
	engine->setDefaultPrototype(qMetaTypeId<DBHtmlEditor*>(), engine->newQObject(DBCommonsScriptPrototypes::instance()->dbHtmlEditor()));

}

