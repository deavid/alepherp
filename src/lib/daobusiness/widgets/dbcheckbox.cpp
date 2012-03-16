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
#include "dbcheckbox.h"
#include "dao/beans/dbfield.h"

DBCheckBox::DBCheckBox(QWidget *parent) :
	QCheckBox(parent), DBBaseWidget()
{
	connect(this, SIGNAL(stateChanged(int)), this, SLOT(emitValueEdited()));
	connect(this, SIGNAL(clicked()), this, SLOT(userClicked()));
}

DBCheckBox::~DBCheckBox()
{
	emit destroyed(this);
}

void DBCheckBox::setValue(const QVariant &value)
{
	bool ok = value.toBool();
	if ( (ok && checkState() == Qt::Unchecked) || (!ok && checkState() == Qt::Checked) ) {
		if ( ok ) {
			QCheckBox::setCheckState(Qt::Checked);
		} else {
			QCheckBox::setCheckState(Qt::Unchecked);
		}
	}
}

void DBCheckBox::emitValueEdited()
{
	QVariant v (QCheckBox::checkState());
	emit valueEdited(v);
}

/*!
  Ajusta los parámetros de visualización de este Widget en función de lo definido en DBField m_field
  */
void DBCheckBox::applyFieldProperties()
{
	this->setEnabled(dataEditable());
}

QVariant DBCheckBox::value()
{
	return QVariant(checkState());
}

void DBCheckBox::observerUnregistered()
{
	DBBaseWidget::observerUnregistered();
	blockSignals(true);
	setCheckState(Qt::Unchecked);
	blockSignals(false);
}

/*!
  Pide un nuevo observador si es necesario
  */
void DBCheckBox::refresh()
{
	observer();
	if ( m_observer != NULL ) {
		m_observer->sync();
	}
}

void DBCheckBox::userClicked()
{
	m_userModified = true;
}

/*!
  Tenemos que decirle al motor de scripts, que DBFormDlg se convierte de esta forma a un valor script
  */
QScriptValue DBCheckBox::toScriptValue(QScriptEngine *engine, DBCheckBox * const &in)
{
	return engine->newQObject(in, QScriptEngine::QtOwnership, QScriptEngine::PreferExistingWrapperObject);
}

void DBCheckBox::fromScriptValue(const QScriptValue &object, DBCheckBox * &out)
{
	out = qobject_cast<DBCheckBox *>(object.toQObject());
}
