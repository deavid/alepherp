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
#include "dbbasewidget.h"
#include "forms/dbrecorddlg.h"
#include "dao/dbfieldobserver.h"
#include "dao/observerfactory.h"
#include "scripts/perpscriptwidget.h"
#include <QObject>
#include <QWidget>
#include <QDateTime>
#include <QDebug>

DBBaseWidget::DBBaseWidget()
{
	m_relation = NULL;
	m_observer = NULL;
	m_dataEditable = false;
	m_userModified = false;
	m_dataFromParentDialog = true;
}

/*!
  Preguntaremos a la factoría de observadores cuál es mi observador, según
  la configuración que se le haya dado a este widget en el ui. Así sabremos siempre
  que estamos presentando la información adecuado
*/
AbstractObserver * DBBaseWidget::observer()
{
	if ( m_observer == NULL ) {
		m_observer = ObserverFactory::instance()->registerBaseWidget(this, getBeanFromContainer());
		if ( m_observer != NULL ) {
			applyFieldProperties();
			m_observer->sync();
		}
	}
	return m_observer;
}

void DBBaseWidget::setFieldName(const QString &name)
{
	m_fieldName = name;
}

QString DBBaseWidget::fieldName()
{
	return m_fieldName;
}

void DBBaseWidget::setRelationName(const QString &name)
{
	m_relationName = name;
}

QString DBBaseWidget::relationName()
{
	return m_relationName;
}

void DBBaseWidget::setRelationFilter(const QString &name)
{
	if ( m_relationFilter != name ) {
		m_relationFilter = name;
		if ( m_observer != NULL ) {
			m_observer->uninstallWidget(dynamic_cast<QObject *>(this));
			m_observer = NULL;
		}
		observer();
	}
}

QString DBBaseWidget::relationFilter()
{
	return m_relationFilter;
}

bool DBBaseWidget::dataEditable()
{
	if ( observer() == NULL ) {
		return m_dataEditable;
	}
	if ( observer()->readOnly() ) {
		return false;
	}
	return m_dataEditable;
}

void DBBaseWidget::setDataEditable(bool value)
{
	m_dataEditable = value;
}

bool DBBaseWidget::userModified()
{
	return m_userModified;
}

void DBBaseWidget::setUserModified(bool value)
{
	m_userModified = value;
}

bool DBBaseWidget::dataFromParentDialog()
{
	return m_dataFromParentDialog;
}

void DBBaseWidget::setDataFromParentDialog(bool value)
{
	m_dataFromParentDialog = value;
}

DBRelation *DBBaseWidget::relation ()
{
	return m_relation;
}

void DBBaseWidget::setDBRelation(DBRelation *rel)
{
	m_relation = rel;
}

void DBBaseWidget::observerUnregistered()
{
	m_observer = NULL;
}

/*!
  Cuando el objeto pasa de visible a no visible, pregunta cuál es su observador, según
  sus datos de UI para mostrar los datos. Este es el origen por el que se empieza a producir
  la sincronía entre los datos de la capa de negocio y la de visualización
  */
void DBBaseWidget::showEvent ( QShowEvent * event )
{
	Q_UNUSED (event)
	observer();
}

void DBBaseWidget::hideEvent ( QHideEvent * event )
{
	Q_UNUSED (event)
}

/*!
  Los widgets que muestran datos pueden estar en dos tipos de contenedores lógicos: Uno, el DBRecord,
  otro, un PERPScriptWidget. Esta función busca el contenedor más cercano en la jerarquía
  para de ahí obtener el bean que controla
  */
BaseBean * DBBaseWidget::getBeanFromContainer()
{
	QObject *temp = (dynamic_cast<QObject *>(this))->parent();
	if ( temp == NULL ) {
        return NULL;
	}
	if ( m_dataFromParentDialog ) {
		while ( temp->parent() != 0 ) {
			DBRecordDlg *dlg = qobject_cast<DBRecordDlg *>(temp);
			if ( dlg != 0 ) {
				QSharedPointer<BaseBean> b = dlg->bean();
				if ( b.isNull() ) {
					return NULL;
				} else {
					return b.data();
				}
			}
			temp = temp->parent();
		}
	} else {
		while ( temp->parent() != 0 ) {
			PERPScriptWidget *wid = qobject_cast<PERPScriptWidget *>(temp);
			if ( wid != 0 ) {
                return wid->bean();
			}
			temp = temp->parent();
		}
		temp = (dynamic_cast<QObject *>(this))->parent();
		while ( temp->parent() != 0 ) {
			DBRecordDlg *dlg = qobject_cast<DBRecordDlg *>(temp);
			if ( dlg != 0 ) {
				QSharedPointer<BaseBean> b = dlg->bean();
				if ( b.isNull() ) {
					return NULL;
				} else {
					return b.data();
				}
			}
			temp = temp->parent();
		}
	}
    return NULL;
}
