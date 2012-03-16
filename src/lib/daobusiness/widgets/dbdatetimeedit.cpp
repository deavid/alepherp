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
#include "dbdatetimeedit.h"
#include "dao/beans/dbfield.h"
#include <QCalendarWidget>
#include "configuracion.h"

#define DBDATETIMEEDIT_SPECIALVALUE	"DD/MM/AAAA"

DBDateTimeEdit::DBDateTimeEdit(QWidget *parent) :
	QDateTimeEdit(parent), DBBaseWidget()
{
	connect(this, SIGNAL(dateChanged(QDate)), this, SLOT(emitValueEdited()));
	this->setCalendarPopup(true);
	if ( this->calendarWidget() != NULL ) {
		if ( configuracion.firstDayOfWeek().toLower() == "monday" ) {
			this->calendarWidget()->setFirstDayOfWeek(Qt::Monday);
		} else if ( configuracion.firstDayOfWeek().toLower() == "tuesday" ) {
			this->calendarWidget()->setFirstDayOfWeek(Qt::Tuesday);
		} else if ( configuracion.firstDayOfWeek().toLower() == "wednesday" ) {
			this->calendarWidget()->setFirstDayOfWeek(Qt::Wednesday);
		} else if ( configuracion.firstDayOfWeek().toLower() == "thursday" ) {
			this->calendarWidget()->setFirstDayOfWeek(Qt::Thursday);
		} else if ( configuracion.firstDayOfWeek().toLower() == "friday" ) {
			this->calendarWidget()->setFirstDayOfWeek(Qt::Friday);
		} else if ( configuracion.firstDayOfWeek().toLower() == "saturday" ) {
			this->calendarWidget()->setFirstDayOfWeek(Qt::Saturday);
		} else if ( configuracion.firstDayOfWeek().toLower() == "sunday" ) {
			this->calendarWidget()->setFirstDayOfWeek(Qt::Sunday);
		}
	}
	QDateTimeEdit::setSpecialValueText(DBDATETIMEEDIT_SPECIALVALUE);
	QDateTimeEdit::setDateTime(QDateTimeEdit::minimumDateTime());
}

DBDateTimeEdit::~DBDateTimeEdit()
{
	emit destroyed(this);
}

void DBDateTimeEdit::setValue(const QVariant &value)
{
	if ( value.isNull() || !value.isValid() ) {
		this->blockSignals(true);
		clear();
		setDateTime(minimumDateTime());
		this->blockSignals(false);
	} else {
		QDateTimeEdit::setDate(value.toDate());
	}
}

void DBDateTimeEdit::emitValueEdited()
{
	QVariant v (QDateTimeEdit::date());
	emit valueEdited(v);
}

void DBDateTimeEdit::showEvent(QShowEvent *event) {
	DBBaseWidget::showEvent(event);
	QWidget::showEvent(event);
}

void DBDateTimeEdit::hideEvent(QHideEvent *event) {
	DBBaseWidget::hideEvent(event);
	QWidget::hideEvent(event);
}

/*!
  Ajusta los parámetros de visualización de este Widget en función de lo definido en DBField m_field
  */
void DBDateTimeEdit::applyFieldProperties()
{
	setReadOnly(!dataEditable());
}

QVariant DBDateTimeEdit::value()
{
	QVariant v;
	if ( date().isNull() ) {
		return v;
	}
	return QVariant(date());
}

void DBDateTimeEdit::observerUnregistered()
{
	DBBaseWidget::observerUnregistered();
	blockSignals(true);
	this->clear();
	blockSignals(false);
}

/*!
  Pide un nuevo observador si es necesario
  */
void DBDateTimeEdit::refresh()
{
	observer();
	if ( m_observer != NULL ) {
		m_observer->sync();
	}
}

void DBDateTimeEdit::focusInEvent(QFocusEvent * event)
{
	if ( dateTime() == minimumDateTime() ) {
		setSpecialValueText("");
		setCurrentSectionIndex(0);
		setSelectedSection(currentSection());
	}
	// Hay que llamar a este evento obligatoriamente.
	QDateTimeEdit::focusInEvent(event);
}

/**
	 Cuando pierde el foco, se da formato.
*/
void DBDateTimeEdit::focusOutEvent(QFocusEvent * event)
{
	if ( dateTime() == minimumDateTime() ) {
		setSpecialValueText(DBDATETIMEEDIT_SPECIALVALUE);
	}
	QDateTimeEdit::focusOutEvent(event);
}

/*!
  Tenemos que decirle al motor de scripts, que DBFormDlg se convierte de esta forma a un valor script
  */
QScriptValue DBDateTimeEdit::toScriptValue(QScriptEngine *engine, DBDateTimeEdit * const &in)
{
	return engine->newQObject(in, QScriptEngine::QtOwnership, QScriptEngine::PreferExistingWrapperObject);
}

void DBDateTimeEdit::fromScriptValue(const QScriptValue &object, DBDateTimeEdit * &out)
{
	out = qobject_cast<DBDateTimeEdit *>(object.toQObject());
}
