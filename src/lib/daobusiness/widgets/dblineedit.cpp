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
#include "dblineedit.h"
#include "dao/beans/dbfield.h"
#include "dao/dbfieldobserver.h"

DBLineEdit::DBLineEdit(QWidget *parent) :
	QLineEdit(parent), DBBaseWidget()
{
	//connect(this, SIGNAL(textEdited(QString)), this, SLOT(emitValueEdited()));
	connect(this, SIGNAL(textChanged(QString)), this, SLOT(emitValueEdited()));
}

DBLineEdit::~DBLineEdit()
{
	emit destroyed(this);
}

void DBLineEdit::setValue(const QVariant &value)
{
	if ( text() != value.toString() ) {
		QLineEdit::setText(value.toString());
	}
}

void DBLineEdit::emitValueEdited()
{
	QVariant v (QLineEdit::text());
	emit valueEdited(v);
}

/*!
  Ajusta los parámetros de visualización de este Widget en función de lo definido en DBField m_field
  */
void DBLineEdit::applyFieldProperties()
{
	DBFieldObserver *obs = qobject_cast<DBFieldObserver *>(observer());
	if ( obs != NULL ) {
		setMaxLength(obs->maxLength());
		setReadOnly(!dataEditable());
	}
}

QVariant DBLineEdit::value()
{
	QVariant v;
	if ( text().isEmpty() ) {
		return v;
	}
	return QVariant(text());
}

void DBLineEdit::observerUnregistered()
{
	DBBaseWidget::observerUnregistered();
	blockSignals(true);
	this->clear();
	blockSignals(false);
}

/*!
  Pide un nuevo observador si es necesario
  */
void DBLineEdit::refresh()
{
	observer();
	if ( m_observer != NULL ) {
		m_observer->sync();
	}

}

/*!
  Tenemos que decirle al motor de scripts, que DBFormDlg se convierte de esta forma a un valor script
  */
QScriptValue DBLineEdit::toScriptValue(QScriptEngine *engine, DBLineEdit * const &in)
{
	return engine->newQObject(in, QScriptEngine::QtOwnership, QScriptEngine::PreferExistingWrapperObject);
}

void DBLineEdit::fromScriptValue(const QScriptValue &object, DBLineEdit * &out)
{
	out = qobject_cast<DBLineEdit *>(object.toQObject());
}
