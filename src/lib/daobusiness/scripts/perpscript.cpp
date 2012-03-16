/***************************************************************************
 *   Copyright (C) 2010 by David Pinelo   *
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
#include "perpscript.h"
#include "configuracion.h"
#ifndef QT_NO_SCRIPTTOOLS
 #include <QScriptEngineDebugger>
 #include <QMainWindow>
#endif
#include <QDebug>
#include <QAction>
#include <QListWidget>
#include <QPointer>
#include "dao/database.h"
#include "dao/beans/beansfactory.h"
#include "forms/perpscripteditdlg.h"
#include "scripts/perpscriptengine.h"
#include "scripts/perpscriptcommon.h"

/**
	Esta clase y esta implementación permitirá el que los binarios sigan siendo compatibles
*/
class PERPScriptPrivate
{
//	Q_DECLARE_PUBLIC(PERPScript)
public:
	/** Objetos que este script pondrá a disposición del engine */
	QHash<QString, QObject *> m_availableObjects;
	/** Nombre del script a ejecutar */
	QString m_scriptName;
	/** Código QS a ejecutar */
	QString m_script;
	/** Objeto definido en el script a cuyo constructor se llamará para ejecutar código */
	QString m_scriptObject;
	/** Si este script hace referencia a un widget, aqui podemos definir el widget principal
	  que será pasado al script como ui en el constructor de m_scriptObject */
	QWidget *m_ui;
	/** Si debug se pone a true permitirá que en caso de error, se ejecute el debugger mostrando el error.*/
	bool m_debug;
	/** Si onInitDebug es true, antes de ejecutar el script, se abre el debugger permitiendo depurar el
	script paso a paso.*/
	bool m_onInitDebug;
	/** Si se crea un objeto que controla un widget, se guarda aqui */
	QScriptValue m_createObject;
	/** Tendremos varios objetos QScriptEngine: Uno global para evaluar todas las funciones sencillas
	  de los fields. Otro, específico por cada widget o por cada formulario. Este puntero contiene
	  el propio de cada formulario */
	QScriptEngine *m_engine;
	/** Conjunto de utilidades para los scripts */
	PERPScriptCommon *m_sc;
#ifndef QT_NO_SCRIPTTOOLS
	QScriptEngineDebugger *m_debugger;
#endif

	PERPScriptPrivate();
};

PERPScriptPrivate::PERPScriptPrivate()
{
	m_debug = false;
	m_onInitDebug = false;
	m_ui = NULL;
	m_engine = NULL;
#ifndef QT_NO_SCRIPTTOOLS
	m_debugger = NULL;
#endif
}

PERPScript::PERPScript(QObject *parent) : QObject(parent), d (new PERPScriptPrivate)
{
	d->m_sc = new PERPScriptCommon(this);
}

PERPScript::~PERPScript()
{
	if ( d->m_engine != NULL ) {
		delete d->m_engine;
	}
#ifndef QT_NO_SCRIPTTOOLS
	if ( d->m_debugger != NULL ) {
		delete d->m_debugger;
	}
#endif
	delete d->m_sc;
	delete d;
}

void PERPScript::createEngine()
{
	if ( d->m_engine != NULL ) {
		delete d->m_engine;
	}
#ifndef QT_NO_SCRIPTTOOLS
	if ( d->m_debugger != NULL ) {
		delete d->m_debugger;
	}
	if ( configuracion.debuggerEnabled() ) {
		d->m_debugger = new QScriptEngineDebugger(this);
	}
#endif

	d->m_engine = PERPScriptEngine::newEngine(this, d->m_debugger);
	d->m_engine->setParent(this);
}

void PERPScript::printError(QScriptEngine *engine, const QString &functionName)
{
	if ( engine != NULL ) {
		qDebug() << "PERPScript: " << functionName << "(): " << d->m_scriptName << ". Error en linea: " <<
					engine->uncaughtExceptionLineNumber() <<
				" ERROR: " << engine->uncaughtException().toString();
		QStringList errors = engine->uncaughtExceptionBacktrace();
		foreach ( QString error, errors ) {
			qDebug() << "PERPScript: " << functionName << "(): " << error;
		}
	}
}

/*!
  Pone a disposición del script a ejecutar el objeto obj. El script podrá acceder a él a través
  del nombre scriptName
  */
void PERPScript::addAvailableObject(const QString &scriptName, QObject *obj)
{
	d->m_availableObjects[scriptName] = obj;
}

/*!
  Sustituye un objeto expuesto al motor Javascript bajo el nombre scriptName
  por el nuevo objeto pasado en obj. Para ello se escoge el global object
  */
void PERPScript::replaceAvailabelObject(const QString &scriptName, QObject *obj)
{
	d->m_availableObjects[scriptName] = obj;
	if ( d->m_engine == NULL ) {
		return;
	}
	QScriptValue globalObject = d->m_engine->globalObject();
	if ( globalObject.isUndefined() ) {
		return;
	}
	if ( obj == NULL ) {
		d->m_availableObjects.remove(scriptName);
		globalObject.setProperty(scriptName, QScriptValue());
	} else {
		QScriptValue objectOnEngine = globalObject.property(scriptName);
		if ( !objectOnEngine.isUndefined() ) {
			objectOnEngine.setData(d->m_engine->newQObject(obj));
		} else {
			// Si no existe, creamos la propiedad
			objectOnEngine.setProperty(scriptName, d->m_engine->newQObject(obj));
		}
	}
}

void PERPScript::setScript(const QString &script)
{
	d->m_script = script;
}

QString PERPScript::script() const
{
	return d->m_script;
}

void PERPScript::setScriptName(const QString &scriptName)
{
	d->m_scriptName = scriptName;
	d->m_script = BeansFactory::tableWidgetsScripts.value(scriptName);
}

QString PERPScript::scriptName() const
{
	return d->m_scriptName;
}

void PERPScript::setScriptObject(const QString &script)
{
	d->m_scriptObject = script;
}

QString PERPScript::scriptObject() const
{
	return d->m_scriptObject;
}

void PERPScript::setUi(QWidget *ui)
{
	d->m_ui = ui;
}

QWidget * PERPScript::ui() const
{
	return d->m_ui;
}

bool PERPScript::debug()
{
	return d->m_debug;
}

void PERPScript::setDebug(bool value)
{
	d->m_debug = value;
}

bool PERPScript::onInitDebug()
{
	return d->m_onInitDebug;
}

void PERPScript::setOnInitDebug(bool value)
{
	d->m_onInitDebug = value;
}

void PERPScript::installAvailableObjects(QScriptValue &object)
{
	QHashIterator<QString, QObject *> it (d->m_availableObjects);
	while ( it.hasNext() ) {
		it.next();
		QScriptValue value;
		if ( d->m_engine == NULL ) {
			value = PERPScriptEngine::engine()->newQObject(it.value(), QScriptEngine::QtOwnership);
		} else {
			value = d->m_engine->newQObject(it.value());
		}
		object.setProperty(it.key(), value);
	}
	// Ahora se instala el objeto PERPScriptCommon
	if ( d->m_engine == NULL ) {
		QScriptValue tmp = PERPScriptEngine::engine()->globalObject().property("PERPScriptCommon");
		if ( !tmp.isValid() ) {
			QScriptValue psc = PERPScriptEngine::engine()->newQObject(d->m_sc, QScriptEngine::QtOwnership);
			PERPScriptEngine::engine()->globalObject().setProperty("PERPScriptCommon", psc);
		}
	} else {
		QScriptValue psc = d->m_engine->newQObject(d->m_sc, QScriptEngine::QtOwnership);
		d->m_engine->globalObject().setProperty("PERPScriptCommon", psc);
	}
}

/*!
  Ejecuta el script pasado a PERPScript entiendo este como una clase. Para ello, creará un objeto
  de nombre scriptName y definido en el script como clase. Ese objeto vivirá durante toda la ejecución
  de este PERPScript, y podrá así controla a un widget asociado a través de ui.
  */
bool PERPScript::createQsObject()
{
	bool result ;
	createEngine();

#ifndef QT_NO_SCRIPTTOOLS
	QMainWindow *debugWindow = NULL;
	if ( configuracion.debuggerEnabled() && d->m_debug && d->m_debugger != NULL ) {
		debugWindow = d->m_debugger ->standardWindow();
		debugWindow->setWindowModality(Qt::ApplicationModal);
		if ( d->m_onInitDebug ) {
			d->m_debugger->action(QScriptEngineDebugger::InterruptAction)->trigger();
		}
	}
#endif

	// Aquí se evalúa el código script, La idea de este caso es que se pase la definición de una clase
	// que dará soporte a un objeto widget. Como se le pasa el código de la clase, no se ejecuta nada;
	// Esta clase, y el código, se ejecutará cuando se construya el objeto con construct
	d->m_engine->evaluate(d->m_script, d->m_scriptObject);
	// Obtenemos, dentro del contexto, el activationObject (this) donde instalaremos los objetos que sean necesario ver
	QScriptValue globalObject = d->m_engine->globalObject();
	installAvailableObjects(globalObject);

	if ( d->m_engine->hasUncaughtException() ) {
		result = false;
	} else {
		// Hacemos visible al QS el ui leido
		// La función principal a evaluar tiene el nombre del formulario. Lo que hacen estas
		// instrucciones es construir una clase DBSearchDlg
		if ( !d->m_scriptObject.isEmpty() && d->m_ui != NULL ) {
			QScriptValue constructor = d->m_engine->evaluate(d->m_scriptObject, d->m_scriptObject);
			QScriptValue scriptUi = d->m_engine->newQObject(d->m_ui, QScriptEngine::QtOwnership);
			d->m_createObject = constructor.construct(QScriptValueList() << scriptUi);
		}
		if ( d->m_engine->hasUncaughtException() ) {
            qDebug() << "Line: " << d->m_engine->uncaughtExceptionLineNumber() << ": " <<d->m_engine->uncaughtException().toString();
            qDebug() << d->m_engine->uncaughtExceptionBacktrace();
		}
		result = ! d->m_engine->hasUncaughtException();
	}
#ifndef QT_NO_SCRIPTTOOLS
	if ( configuracion.debuggerEnabled() && d->m_debug && d->m_debugger ) {
		if ( debugWindow ) {
			debugWindow->hide();
		}
	}
#endif
	return result;
}

/*!
	Compruba si existe esta función
	*/
bool PERPScript::existQsFunction(const QString &scriptFunctionName)
{
	if ( d->m_engine == NULL ) {
		return false;
	}
	QScriptValue f = d->m_createObject.property(scriptFunctionName);
	return f.isValid();
}

/*!
  Realiza una llamada a una función miembro del objeto que se creó con createQsObject
  */
bool PERPScript::callQsObjectFunction(QScriptValue &result, const QString scriptFunctionName)
{
	bool r = true;
	if ( d->m_engine == NULL ) {
		return false;
	}
	if ( !d->m_createObject.isNull() && d->m_createObject.isValid() ) {
		QScriptValue f = d->m_createObject.property(scriptFunctionName);
		result = f.call(d->m_createObject, QScriptValueList());
		if ( result.isError() ) {
			printError(d->m_engine, "callQsObjectFunction");
			r = false;
		}
	} else {
		r = false;
	}
	return r;
}


/**
 Evalúa la función scriptFunctionName que se encuentra en script, y devuelve el resultado de esa función
 en value.
*/
QVariant PERPScript::evaluateQsFunction(const QString &scriptFunctionName)
{
	QVariant data;
	QScriptValue result;
#ifndef QT_NO_SCRIPTTOOLS
	QScriptEngineDebugger *debugger = NULL;
	if ( configuracion.debuggerEnabled() && d->m_debug ) {
		debugger = PERPScriptEngine::debugger();
		if ( d->m_onInitDebug ) {
			debugger->action(QScriptEngineDebugger::InterruptAction)->trigger();
		}
	}
#endif

	PERPScriptEngine::engine()->evaluate(d->m_script, d->m_scriptObject);
	// Obtenemos, dentro del contexto, el activationObject (this) donde instalaremos los objetos que sean necesario ver
	QScriptValue activationObject = PERPScriptEngine::engine()->currentContext()->activationObject();
	installAvailableObjects(activationObject);

	if ( PERPScriptEngine::engine()->hasUncaughtException() ) {
		QScriptValue exception = PERPScriptEngine::engine()->uncaughtException ();
		if ( exception.isValid() ) {
			qDebug() << "PERPScript::evaluateQsFunction: ERROR en script: " <<
						PERPScriptEngine::engine()->uncaughtExceptionLineNumber () << ": " << exception.toString();
			qDebug() << "PERPScriot::evaluateQsFunction: Script: " << d->m_script;
		}
	} else {
		if ( scriptFunctionName.isEmpty() ) {
			result = PERPScriptEngine::engine()->evaluate(d->m_scriptObject, d->m_scriptObject);
		} else {
			QScriptValue add = PERPScriptEngine::engine()->currentContext()->activationObject().property(scriptFunctionName);
			result = add.call(d->m_createObject, QScriptValueList());
		}
		if ( result.isValid() && !result.isUndefined() && !result.isNull() && !result.isError() ) {
			data = result.toVariant();
		}
		if ( PERPScriptEngine::engine()->hasUncaughtException() ) {
			printError(PERPScriptEngine::engine(), "evaluateQsFunction");
			data = QVariant();
		}
//		PERPScriptEngine::engine()->collectGarbage();
	}
#ifndef QT_NO_SCRIPTTOOLS
	if ( configuracion.debuggerEnabled() && d->m_debug && debugger ) {
		if ( PERPScriptEngine::debugWindow() ) {
			PERPScriptEngine::debugWindow()->hide();
		}
	}
#endif
	return data;
}

void PERPScript::editScript(QWidget *p)
{
	QWidget *tmp;
	if ( !BeansFactory::tableWidgetsScripts.contains(d->m_scriptName) ) {
		return;
	}
	( p == 0 ? tmp = qobject_cast<QWidget *>(this->parent()) : tmp = p);
	QPointer<PERPScriptEditDlg> dlg = new PERPScriptEditDlg(d->m_scriptName, tmp);
	dlg->setModal(true);
	dlg->setAttribute(Qt::WA_DeleteOnClose);
	dlg->exec();
	delete dlg;
}
