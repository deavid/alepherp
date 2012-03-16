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
#include "dbfieldobserver.h"
#include "widgets/dbbasewidget.h"
#include "widgets/dbdetailview.h"
#include "dao/beans/dbfield.h"
#include "dao/beans/dbfieldmetadata.h"
#include "dao/beans/basebean.h"
#include "dao/beans/dbrelation.h"
#include "dao/beans/basebeanmetadata.h"
#include "dao/dbobject.h"
#include <QCoreApplication>

DBFieldObserver::DBFieldObserver(DBObject *entity) :
	AbstractObserver(entity)
{
	if ( m_entity != NULL ) {
		connect(m_entity, SIGNAL(valueModified(QVariant)), this, SIGNAL(entityValueModified(QVariant)));
	}
}

DBFieldObserver::~DBFieldObserver()
{
	if ( m_entity != NULL ) {
		DBField *fld = qobject_cast<DBField *>(m_entity);
		if ( fld != NULL ) {
			disconnect(fld, SIGNAL(valueModified(const QVariant &)), this, SIGNAL(entityValueModified(const QVariant &)));
		}
	}
	observerToBeDestroyed();
}

bool DBFieldObserver::readOnly()
{
	DBField *fld = qobject_cast<DBField *>(entity());
	if ( fld == NULL ) {
		return true;
	}
	QString tableName = fld->bean()->metadata()->tableName();
	QHash<QString, QVariant> permissions = qApp->property("permissions").toHash();
	QString p = permissions.value(tableName).toString();
	if ( !p.contains("w") ) {
		return true;
	}
	if ( fld != NULL ) {
		return fld->metadata()->readOnly();
	}
	return false;
}

/*!
  Sincroniza el modelo y la vista
  */
void DBFieldObserver::sync()
{
	DBField *fld = qobject_cast<DBField *>(entity());
	if ( fld != NULL ) {
		emit entityValueModified(fld->value());
	} else {
		emit entityValueModified(QVariant());
	}
}

/*!
  Realiza todas las conexiones necesarias entre el widget y el observador
  para permitir la sincronización de datos. Esta función depende de cada observador,
  por ello hay que implementarla en cada observador
  */
void DBFieldObserver::installWidget(QObject *widget)
{
	m_viewWidgets << widget;
	connect(this, SIGNAL(entityValueModified(QVariant)), widget, SLOT(setValue(QVariant)));
	connect(widget, SIGNAL(valueEdited(QVariant)), this, SLOT(widgetEdited(QVariant)));
	connect(widget, SIGNAL(destroyed(QObject*)), this, SLOT(widgetToBeDestroyed(QObject*)));
}

void DBFieldObserver::uninstallWidget(QObject *widget)
{
	int index = m_viewWidgets.indexOf(widget);
	if ( index != -1 ) {
		m_viewWidgets.removeAt(index);
		if ( widget->property("perpControl").toBool() ) {
			DBBaseWidget *bw = dynamic_cast<DBBaseWidget *>(widget);
			bw->observerUnregistered();
		}
		disconnect(this, SIGNAL(entityValueModified(QVariant)), widget, SLOT(setValue(QVariant)));
		disconnect(widget, SIGNAL(valueEdited(QVariant)), this, SLOT(widgetEdited(QVariant)));
		disconnect(widget, SIGNAL(destroyed(QObject*)), this, SLOT(widgetToBeDestroyed(QObject*)));
    }
}

void DBFieldObserver::widgetEdited(const QVariant &value)
{
	DBField *fld = qobject_cast<DBField *>(entity());
	if ( fld != NULL ) {
		fld->setValue(value);
	}
}

/*!
  Longitud máxima de edición del campo de texto
  */
int DBFieldObserver::maxLength()
{
	DBField *fld = qobject_cast<DBField *>(entity());
	if ( fld == NULL ) {
		return 0;
	}
	return fld->metadata()->length();
}

/*!
  Número de decimales
  */
int DBFieldObserver::partD()
{
	DBField *fld = qobject_cast<DBField *>(entity());
	if ( fld == NULL ) {
		return 0;
	}
	return fld->metadata()->partD();
}
