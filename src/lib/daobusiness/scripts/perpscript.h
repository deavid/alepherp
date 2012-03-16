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
#ifndef PERPSCRIPT_H
#define PERPSCRIPT_H

#include <QObject>
#include <QScriptEngine>
#include <alepherpglobal.h>

class QScriptEngineDebugger;
class QMainWindow;

class PERPScriptPrivate;

/**
  Esta clase ejecutará scripts en ECMAScript almacenados en base de datos
  @autho David Pinelo <david.pinelo@alephsistemas.es>
  */
class Q_ALEPHERP_EXPORT PERPScript : public QObject
{
    Q_OBJECT
	/** Contiene el código Script a ejecutar */
	Q_PROPERTY(QString script READ script WRITE setScript)
	/** Contiene el nombre del script. Este nombre podrá ser usado por ejemplo para que el Debugger
	o la salida de error, muestren este scriptName y se pueda identificar rápido código erróneo.
	También sirve, para si es un script de base de datos, poder editarlo y corregirlo. */
	Q_PROPERTY(QString scriptName READ scriptName WRITE setScriptName)
	/** Los scripts a ejecutar pueden ser una clase, que dan soporte a todo un QDialog o un QWidget. Esta
	  propiedad almacena el nombre de esa clase que se construirá y asignará al objeto ui, permitiendo
	  así que el script pueda controlar al widget */
	Q_PROPERTY(QString scriptObject READ scriptObject WRITE setScriptObject)
	Q_PROPERTY(QWidget * ui READ ui WRITE setUi)
	/** Indica si depuramos el script: Esto es, cuando se produce un error, se abre el debugger
	y lo muestra */
	Q_PROPERTY(bool debug READ debug WRITE setDebug)
	/** Indica si el script se ejecuta desde el debugger, por ejemplo paso a paso */
	Q_PROPERTY(bool onInitDebug READ onInitDebug WRITE setOnInitDebug)

private:
	PERPScriptPrivate *d;
	Q_DECLARE_PRIVATE(PERPScript)

	void installAvailableObjects(QScriptValue &object);
	void createEngine();
	void printError(QScriptEngine *engine, const QString &functionName);

public:
    explicit PERPScript(QObject *parent = 0);
	~PERPScript();

	bool existQsFunction(const QString &scriptFunctionName);
	bool createQsObject();
	bool callQsObjectFunction(QScriptValue &result, const QString scriptFunctionName);
	QVariant evaluateQsFunction(const QString &scriptFunctionName);

	void addAvailableObject(const QString &scriptName, QObject *obj);
	void replaceAvailabelObject(const QString &scriptName, QObject *obj);

	void setScript(const QString &script);
	QString script() const;
	void setScriptName(const QString &script);
	QString scriptName() const;
	void setScriptObject(const QString &script);
	QString scriptObject() const;
	void setUi(QWidget *ui);
	QWidget *ui() const;
	bool debug();
	void setDebug(bool value);
	bool onInitDebug();
	void setOnInitDebug(bool value);

signals:

public slots:
	void editScript(QWidget *parent = 0);

};

#endif // PERPSCRIPT_H
