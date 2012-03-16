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
#ifndef DBNumberEdit_H
#define DBNumberEdit_H

#include <QLineEdit>
#include <QWidget>
#include <QLocale>
#include <QString>
#include <QFocusEvent>
#include <QTextStream>
#include <QKeyEvent>
#include <QScriptValue>
#include <alepherpglobal.h>
#include "configuracion.h"
#include "widgets/dbbasewidget.h"
#include "dao/observerfactory.h"

class DBNumberEditPrivate;

/**
	Este texto será utilizado para presentar números en la aplicación.
	El funcionamiento será el siguiente:
	Cuando el objeto recibe el foco, eliminará el formato del número,
	dejándolo solo como una sucesión de números en la parte entera y una
	coma para separar la decimal. Cuando lo pierde, formatea el número
	
	@author David Pinelo <david.pinelo@alephsistemas.es>
	@see QLineEdit
	@see DBBaseWidget
*/

class Q_ALEPHERP_EXPORT DBNumberEdit : public QLineEdit, public DBBaseWidget
{
	Q_OBJECT
	Q_PROPERTY (QString fieldName READ fieldName WRITE setFieldName)
	Q_PROPERTY (QString relationFilter READ relationFilter WRITE setRelationFilter)
	Q_PROPERTY (bool dataEditable READ dataEditable WRITE setDataEditable)
	Q_PROPERTY (bool perpControl READ perpControl)
	Q_PROPERTY (bool userModified READ userModified WRITE setUserModified)
	Q_PROPERTY (QVariant value READ value WRITE setValue)
	/** Un control puede estar dentro de un PERPScriptWidget. ¿De dónde lee los datos? Los puede leer
	  del bean asignado al propio PERPScriptWidget, o bien, leerlos del bean del formulario base
	  que lo contiene. Esta propiedad marca esto */
	Q_PROPERTY (bool dataFromParentDialog READ dataFromParentDialog WRITE setDataFromParentDialog)

private:
	DBNumberEditPrivate *d;
	Q_DECLARE_PRIVATE(DBNumberEdit)

protected:
	void showEvent(QShowEvent *event) { DBBaseWidget::showEvent(event); QWidget::showEvent(event); }
	void hideEvent(QHideEvent *event) { DBBaseWidget::hideEvent(event); QWidget::hideEvent(event); }

public:
	DBNumberEdit(QWidget * parent = 0);

	~DBNumberEdit();

	int observerType() { return OBSERVER_DBFIELD; }
	double numericValue();
	QVariant value();

	void setDecimalNumbers ( int theValue );
	int decimalNumbers() const;

	void focusInEvent ( QFocusEvent * event );
	void focusOutEvent ( QFocusEvent * event );
	void applyFieldProperties();

	static QScriptValue toScriptValue(QScriptEngine *engine, DBNumberEdit * const &in);
	static void fromScriptValue(const QScriptValue &object, DBNumberEdit * &out);

public slots:
	void setNumericValue(double valor);
	void setNumericValue(int valor) { setNumericValue((double) valor); }
	void clear();
	void setValue(const QVariant &value);
	void refresh();
	void observerUnregistered();

private slots:
	void storeNumber(const QString &texto);

signals:
	void valueEdited(const QVariant &value);
	void destroyed(QWidget *widget);
};

/**
  Esta clase leerá todos los keypress de los number edit, para solventar
  un error en los layouts de los teclados españoles: El separador de decimales
  en España es la coma. En el keypad (teclado numérico), sin embargo, aparece un "."
  Lo que lleva a confusión al usuario.
  */
class DBNumberEditKeyPressEater : public QObject
{
	Q_OBJECT
public:
	DBNumberEditKeyPressEater(QObject *parent = 0);

protected:
	bool eventFilter(QObject *obj, QEvent *event);
};

Q_DECLARE_METATYPE(DBNumberEdit*)
Q_SCRIPT_DECLARE_QMETAOBJECT(DBNumberEdit, DBNumberEdit*)

#endif
