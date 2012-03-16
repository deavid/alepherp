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
#include "dbnumberedit.h"
#include "configuracion.h"
#include "dao/beans/dbfield.h"
#include "dao/dbfieldobserver.h"
#include <QApplication>

DBNumberEditKeyPressEater::DBNumberEditKeyPressEater(QObject *parent) : QObject(parent)
{
}

class DBNumberEditPrivate {
public:
	/** Es posible establecer el número de decimales que deseamos visualizar en el control.
	Lo hacemos con esta variable */
	int m_decimalNumbers;
	/** Esta variable, almacenará internamente el valor numérico del control */
	double m_numericValue;
	/** Variable chivato para evitar interacciones desagradables entre entrada de datos del
	usuario y procesamiento interno */
	bool m_doingChanges;
	/** El elemento que validará los datos que se introducen */
	QDoubleValidator *m_validator;
	/** Trataremos las pulsaciones de las teclas */
	DBNumberEditKeyPressEater *m_filterObject;
};

DBNumberEdit::DBNumberEdit(QWidget * parent)
: QLineEdit(parent), DBBaseWidget(), d(new DBNumberEditPrivate)
{
	// Capturamos la señal de texto cambiado, para almacenar siempre el valor numérico del editor.
	connect (this, SIGNAL(textChanged(QString)), this, SLOT(storeNumber(QString)));
	// Número de decimales que por defecto mostrará el control
	d->m_decimalNumbers = 2;
	// Inicialmente tendrá cero
	d->m_numericValue = 0;
	d->m_doingChanges = false;
	d->m_filterObject = new DBNumberEditKeyPressEater(this);
	
	setCursor(Qt::IBeamCursor);
	setFocusPolicy(Qt::StrongFocus);
	setAttribute(Qt::WA_InputMethodEnabled);
	setAttribute(Qt::WA_KeyCompression);
	setMouseTracking(true);
	setAcceptDrops(true);
	setAttribute(Qt::WA_MacShowFocusRect);
	setAlignment (Qt::AlignRight);
	d->m_validator = new QDoubleValidator(this);
	d->m_validator->setDecimals(this->d->m_decimalNumbers);
	d->m_validator->setLocale(*(configuracion.getLocale()));
	this->installEventFilter(d->m_filterObject);
	QLineEdit::setValidator(d->m_validator);
}


DBNumberEdit::~DBNumberEdit()
{
	delete d;
	emit destroyed(this);
}

/**
	Si en esta función accedemos al texto, utilizando text(), se pierde la posición del cursor
	y la edición del número se hace imposible. Por eso, se ha creado una variable interna
	que almacena el valor numérico, cada vez que el texto cambia. Aquí se devuelve ese valor
 */
double DBNumberEdit::numericValue()
{
	return d->m_numericValue;
}


int DBNumberEdit::decimalNumbers() const
{
	return d->m_decimalNumbers;
}


void DBNumberEdit::setDecimalNumbers ( int theValue )
{
	d->m_decimalNumbers = theValue;
	d->m_validator->setDecimals(d->m_decimalNumbers);
}

void DBNumberEdit::setValue(const QVariant &value)
{
	bool ok;
	double v = value.toDouble(&ok);
	double orig = numericValue();
	if ( ok && v != orig ) {
		setNumericValue(value.toDouble());
	}
}

/** 
	Esta función presentará en el control, el valor pasado en el parámetro, para su edición
*/
void DBNumberEdit::setNumericValue(double valor)
{
	d->m_doingChanges = true;
	d->m_numericValue = valor;
	if ( !hasFocus() ) {
		setText (configuracion.getLocale()->toString(valor, 'f', d->m_decimalNumbers));
	}
	update();
	d->m_doingChanges = false;
}

/**
	Si en cualquier función accedemos al texto, utilizando text(), se pierde la posición del cursor
	y la edición del número se hace imposible. Por eso, se ha creado una variable interna
	que almacena el valor numérico, cada vez que el texto cambia. Aquí se almacena ese valor
	cada vez que se modifca el texto.
*/
void DBNumberEdit::storeNumber(const QString & texto)
{
	bool ok;

	if ( !d->m_doingChanges ) {
		if ( !texto.isNull() && !texto.isEmpty() ) {
			d->m_numericValue = configuracion.getLocale()->toDouble(texto, &ok);
			if ( !ok ) {
				d->m_numericValue = 0;
			}
		} else {
			d->m_numericValue = 0;
		}
	}
	emit valueEdited(QVariant(d->m_numericValue));
}

/**
	Cuando el texto recibe el foco, se le eliminan todos los caracteres menos números y punto.
	Es decir, mostramos en el texto, el numerito almacenado internamente, pero sin locale.
 */
void DBNumberEdit::focusInEvent(QFocusEvent * event)
{
	QString text;
	
	d->m_doingChanges = true;
	if ( d->m_numericValue == 0 ) {
		text = "";
	} else {
		QTextStream (&text) << d->m_numericValue;
		// Sustituimos el punto en d->m_numericValue por el caracter separador de decimales del local.
		QChar separadorDecimales = configuracion.getLocale()->decimalPoint ();
		text = text.replace('.', separadorDecimales);
	}
	
	blockSignals(true);
	setAlignment (Qt::AlignLeft);
	setText (text);
	blockSignals(false);
	
	// Además, seleccionamos todo el texto
	
	d->m_doingChanges = false;
	// Hay que llamar a este evento obligatoriamente.
	QLineEdit::focusInEvent(event);
	selectAll ();
}

/**
	 Cuando pierde el foco, se da formato.
*/
void DBNumberEdit::focusOutEvent(QFocusEvent * event)
{
	d->m_doingChanges = true;
	if ( event->lostFocus() ) {
		blockSignals(true);
		setText (configuracion.getLocale()->toString(d->m_numericValue, 'f', d->m_decimalNumbers));
		setAlignment (Qt::AlignRight);
		blockSignals(false);
	}
	d->m_doingChanges = false;
	// Hay que llamar a este evento obligatoriamente.
	QLineEdit::focusOutEvent(event);
}

/**
	Limpieza absoluta del control
*/
void DBNumberEdit::clear()
{
	d->m_numericValue = 0;
	QLineEdit::clear();
}

/*!
  Ajusta los parámetros de visualización de este Widget en función de lo definido en DBField d->m_field
  */
void DBNumberEdit::applyFieldProperties()
{
	DBFieldObserver *obs= qobject_cast<DBFieldObserver *>(observer());
	if ( obs != NULL ) {
		setReadOnly(!dataEditable());
		setDecimalNumbers(obs->partD());
	}
}

QVariant DBNumberEdit::value()
{
	QVariant v(numericValue());
	return v;
}

void DBNumberEdit::observerUnregistered()
{
	DBBaseWidget::observerUnregistered();
	blockSignals(true);
	this->clear();
	blockSignals(false);
}

/*!
  Pide un nuevo observador si es necesario
  */
void DBNumberEdit::refresh()
{
	observer();
	if ( m_observer != NULL ) {
		m_observer->sync();
	}
}

/*!
  Se capturará un caso particular: El botón "." del teclado numérico, trasladando ese teclado,
  si el sistema está asi configurado al separador de decimales por defecto del sistema
  */
bool DBNumberEditKeyPressEater::eventFilter(QObject *obj, QEvent *event)
{
	if (event->type() == QEvent::KeyPress) {
		QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
		if ( ( keyEvent->modifiers() & Qt::KeypadModifier && keyEvent->text() == "." &&
			   configuracion.getLocale()->decimalPoint() != '.' ) ) {
			QKeyEvent *ev = new QKeyEvent(QEvent::KeyPress, Qt::Key_Comma, 0, configuracion.getLocale()->decimalPoint());
			QApplication::sendEvent(parent(), ev);
			return true;
		}
	}
	// standard event processing
	return QObject::eventFilter(obj, event);
}

/*!
  Tenemos que decirle al motor de scripts, que DBFormDlg se convierte de esta forma a un valor script
  */
QScriptValue DBNumberEdit::toScriptValue(QScriptEngine *engine, DBNumberEdit * const &in)
{
	return engine->newQObject(in, QScriptEngine::QtOwnership, QScriptEngine::PreferExistingWrapperObject);
}

void DBNumberEdit::fromScriptValue(const QScriptValue &object, DBNumberEdit * &out)
{
	out = qobject_cast<DBNumberEdit *>(object.toQObject());
}
