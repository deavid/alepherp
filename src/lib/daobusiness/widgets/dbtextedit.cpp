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
#include "dbtextedit.h"
#include "dao/beans/dbfield.h"

DBTextEdit::DBTextEdit(QWidget *parent) :
	QPlainTextEdit(parent), DBBaseWidget()
{
	connect(this, SIGNAL(textChanged()), this, SLOT(emitValueEdited()));
}

DBTextEdit::~DBTextEdit()
{
	emit destroyed(this);
}

void DBTextEdit::setValue(const QVariant &value)
{
	if ( toPlainText() != value.toString() ) {
		QPlainTextEdit::setPlainText(value.toString());
	}
}

void DBTextEdit::emitValueEdited()
{
	QVariant v (this->toPlainText());
	emit valueEdited(v);
}

/*!
  Ajusta los parámetros de visualización de este Widget en función de lo definido en DBField m_field
  */
void DBTextEdit::applyFieldProperties()
{
	setReadOnly(!dataEditable());
}

QVariant DBTextEdit::value()
{
	QVariant v;
	if ( toPlainText().isEmpty() ) {
		return v;
	}
	return QVariant(toPlainText());
}

void DBTextEdit::observerUnregistered()
{
	DBBaseWidget::observerUnregistered();
	blockSignals(true);
	this->clear();
	blockSignals(false);
}

/*!
  Pide un nuevo observador si es necesario
  */
void DBTextEdit::refresh()
{
	observer();
	if ( m_observer != NULL ) {
		m_observer->sync();
	}
}

/*!
  Tenemos que decirle al motor de scripts, que DBFormDlg se convierte de esta forma a un valor script
  */
QScriptValue DBTextEdit::toScriptValue(QScriptEngine *engine, DBTextEdit * const &in)
{
	return engine->newQObject(in, QScriptEngine::QtOwnership, QScriptEngine::PreferExistingWrapperObject);
}

void DBTextEdit::fromScriptValue(const QScriptValue &object, DBTextEdit * &out)
{
	out = qobject_cast<DBTextEdit *>(object.toQObject());
}
