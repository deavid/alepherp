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
#include "dbabstractview.h"
#include "models/filterbasebeanmodel.h"
#include "models/dbbasebeanmodel.h"
#include "models/relationbasebeanmodel.h"
#include "dao/beans/basebeanmetadata.h"
#include "dao/beans/beansfactory.h"
#include "dao/beans/dbrelation.h"
#include <QtGui>

DBAbstractViewInterface::DBAbstractViewInterface()
{
	m_externalModel = false;
	m_internalData = false;
	m_model = NULL;
	m_filterModel = NULL;
	m_selectionModel = NULL;
	m_metadata = NULL;
}

DBAbstractViewInterface::~DBAbstractViewInterface()
{
	if ( m_model != NULL ) {
		delete m_model;
	}
	if ( m_filterModel != NULL ) {
		delete m_filterModel;
	}
}

bool DBAbstractViewInterface::externalModel()
{
	return m_externalModel;
}

void DBAbstractViewInterface::setExternalModel(bool value)
{
	m_externalModel = value;
}

QString DBAbstractViewInterface::tableName()
{
	return m_tableName;
}

void DBAbstractViewInterface::setTableName(const QString &value)
{
	m_tableName = value;
}

QString DBAbstractViewInterface::filter()
{
	return m_filter;
}

void DBAbstractViewInterface::setFilter(const QString &value)
{
	if ( m_filter != value ) {
		m_filter = value;
		init();
	}
}

QString DBAbstractViewInterface::order()
{
	return m_order;
}

void DBAbstractViewInterface::setOrder(const QString &value)
{
	m_order = value;
}

BaseBeanMetadata * DBAbstractViewInterface::metadata()
{
	return m_metadata;
}

void DBAbstractViewInterface::setRelationFilter(const QString &name)
{
	if ( m_relationFilter != name ) {
		DBBaseWidget::setRelationFilter(name);
		init();
	}
}

/*!
  Inicia el modelo de datos si es interno y el control a partir de los datos pasados en tableName y filter.
  Sólo se inicia si el widget está visible
  */
void DBAbstractViewInterface::init(bool onShowEvent)
{
	QWidget *widget = dynamic_cast<QWidget *> (this);
	if ( onShowEvent || widget->isVisible() ) {
		if ( m_externalModel ) {
			setupExternalModel();
		} else {
			setupInternalModel();
		}
	}
}

void DBAbstractViewInterface::setupExternalModel()
{
	QAbstractItemView *widget = dynamic_cast<QAbstractItemView *> (this);
	QString tableName;
	if ( m_model->property("baseBeanModel").toBool() ) {
		tableName = m_model->property("tableName").toString();
		m_metadata = BeansFactory::metadataBean(tableName);
		if ( m_metadata == NULL ) {
			return;
		}
		m_tableName = tableName;
	}
	if ( m_metadata != NULL && m_model != NULL ) {
		m_selectionModel = new QItemSelectionModel(m_model);
		widget->setModel(m_model);
		widget->setSelectionModel(m_selectionModel);
	}
}

void DBAbstractViewInterface::setupInternalModel()
{
	QAbstractItemView *widget = dynamic_cast<QAbstractItemView *> (this);
	if ( m_tableName.isEmpty() && m_relationName.isEmpty() ) {
		return;
	}
	if ( m_selectionModel != NULL ) {
		delete m_selectionModel;
		m_selectionModel = NULL;
	}
	if ( m_model != NULL ) {
		delete m_model;
		m_model = NULL;
	}
	if ( m_filterModel != NULL ) {
		delete m_filterModel;
		m_filterModel = NULL;
	}
	// Internal data indica si los datos se leen de base de datos o si se leen desde beans en memoria
	if ( !m_internalData ) {
		m_metadata = BeansFactory::metadataBean(m_tableName);
		// El filtro aquí pasado es fuerte (se traduce en la SQL)
		if ( m_metadata != NULL ) {
			m_model = new DBBaseBeanModel(m_tableName, m_filter, m_order);
		}
	} else {
		QStringList relations = m_relationName.split(".");
		m_metadata = BeansFactory::metadataBean(relations.last());
		AbstractObserver *obs = observer();
		if ( obs != NULL && m_metadata != NULL ) {
			DBRelation *rel = qobject_cast<DBRelation *>(obs->entity());
			if ( rel != 0 ) {
				m_model = new RelationBaseBeanModel(!m_dataEditable, rel, m_order);
			}
		}
	}
	if ( m_model != NULL ) {
		m_filterModel = new FilterBaseBeanModel();
		m_filterModel->setSourceModel(m_model);
	}
	if ( m_metadata != NULL && m_model != NULL && m_filterModel != NULL ) {
		m_selectionModel = new QItemSelectionModel(m_filterModel);
		if ( widget != NULL ) {
			widget->setModel(m_filterModel);
		}
		if ( widget != NULL ) {
			widget->setSelectionModel(m_selectionModel);
		}
	}
	if ( m_filterModel != NULL ) {
		// Del filtro, sólo hay que coger la parte que corresponde ya a al observador
		QStringList fieldsFilter = m_relationFilter.split(";");
		QStringList relations = m_relationName.split(".");
		for ( int i = (relations.size() - 1) ; i < fieldsFilter.size() ; i++ ) {
			QString filter = fieldsFilter.at(i);
			if ( !filter.isEmpty() ) {
				m_filterModel->setFilter(filter.trimmed());
			}
		}
		m_filterModel->invalidate();
	}
}

bool DBAbstractViewInterface::internalData()
{
	return m_internalData;
}

void DBAbstractViewInterface::setInternalData(bool value)
{
	m_internalData = value;
}

/*!
  Algunas propiedades serán visibles dependiendo de si internalData es true o no
  */
bool DBAbstractViewInterface::internalDataPropertyVisible()
{
	if ( m_internalData ) {
		return true;
	}
	return false;
}

bool DBAbstractViewInterface::externalDataPropertyVisible()
{
	if ( !m_internalData ) {
		return true;
	}
	return false;
}

/*!
  Devuelve un listado de los beans correspondiente a los items actualmente seleccionados
  */
BaseBeanPointerList DBAbstractViewInterface::selectedBeans()
{
	BaseBeanPointerList list;
	if ( m_model != NULL && m_model->property("baseBeanModel").toBool() ) {
		BaseBeanModel *tmp = qobject_cast<BaseBeanModel *>(m_model);
		QModelIndexList lIndex = m_selectionModel->selectedIndexes();
		foreach ( QModelIndex index, lIndex ) {
			QSharedPointer<BaseBean> bean = tmp->bean(index);
			if ( !bean.isNull() && !list.contains(bean) ) {
				list << bean;
			}
		}
	}
	return list;
}

