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
#include "dblabel.h"

DBLabel::DBLabel(QWidget *parent) :
	QLabel(parent), DBBaseWidget()
{
}

DBLabel::~DBLabel()
{
	emit destroyed(this);
}

void DBLabel::setValue(const QVariant &value)
{
	QLabel::setText(value.toString());
}

/*!
  Ajusta los parámetros de visualización de este Widget en función de lo definido en DBField m_field
  */
void DBLabel::applyFieldProperties()
{

}

QVariant DBLabel::value()
{
	QVariant v;
	return v;
}

void DBLabel::observerUnregistered()
{
	DBBaseWidget::observerUnregistered();
	blockSignals(true);
	this->clear();
	blockSignals(false);
}

/*!
  Pide un nuevo observador si es necesario
  */
void DBLabel::refresh()
{
	observer();
	if ( m_observer != NULL ) {
		m_observer->sync();
	}
}

/*!
  Tenemos que decirle al motor de scripts, que DBFormDlg se convierte de esta forma a un valor script
  */
QScriptValue DBLabel::toScriptValue(QScriptEngine *engine, DBLabel * const &in)
{
	return engine->newQObject(in, QScriptEngine::QtOwnership, QScriptEngine::PreferExistingWrapperObject);
}

void DBLabel::fromScriptValue(const QScriptValue &object, DBLabel * &out)
{
	out = qobject_cast<DBLabel *>(object.toQObject());
}
