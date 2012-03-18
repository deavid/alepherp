/***************************************************************************
 *   Copyright (C) 2011 by David Pinelo									*
 *   david.pinelo@alephsistemas.es													*
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
#ifndef DBABSTRACTVIEW_H
#define DBABSTRACTVIEW_H

#include <QString>
#include <QAbstractItemModel>
#include <QItemSelectionModel>
#include "widgets/dbbasewidget.h"
#include "dao/beans/basebean.h"
#include "dao/observerfactory.h"

class FilterBaseBeanModel;

class DBAbstractViewInterface : public DBBaseWidget
{
protected:
	BaseBeanMetadata *m_metadata;
	/** Tabla cuyos resultados se mostrarán */
	QString m_tableName;
	/** Filtro fuerte (en clausula WHERE) que se aplicará al crear el modelo */
	QString m_filter;
	/** Orden en el que se muestran los datos */
	QString m_order;
	/** Chivato que indicará si el modelo se ha pasado ya creado y externo (por setModel)
	  o es interno */
	bool m_externalModel;
	/** Puntero al modelo */
	QAbstractItemModel *m_model;
	/** Filtro del modelo, si es interno */
	FilterBaseBeanModel *m_filterModel;
	/** Los datos se leen de base de datos o bien del bean del formulario que contiene a este control */
	bool m_internalData;
	/** Modelo de selección */
	QItemSelectionModel *m_selectionModel;

	virtual bool internalDataPropertyVisible();
	virtual bool externalDataPropertyVisible();
	virtual void setupExternalModel();
	virtual void setupInternalModel();

	void init(bool onShowEvent = false);

public:
	DBAbstractViewInterface();
    virtual ~DBAbstractViewInterface();

	virtual bool externalModel();
	virtual void setExternalModel(bool value);
	virtual QString tableName();
	virtual void setTableName(const QString &value);
	virtual QString filter();
	virtual void setFilter(const QString &value);
	virtual QString order();
	void virtual setOrder(const QString &value);
	virtual bool internalData();
	virtual void setInternalData(bool value);
	virtual void setRelationFilter(const QString &name);

	virtual BaseBeanPointerList checkedBeans() = 0;
	virtual void setCheckedBeans(BaseBeanPointerList list, bool checked = true) = 0;
	virtual void setCheckedBeansByPk(QVariantList list, bool checked = true) = 0;

	BaseBeanMetadata * metadata();

	BaseBeanPointerList selectedBeans();

	int observerType() { return OBSERVER_DBRELATION; }

	/** Establece el valor a mostrar en el control */
	virtual void setValue(const QVariant &value) = 0;
	/** Devuelve el valor mostrado o introducido en el control */
	virtual QVariant value() = 0;
	/** Ajusta el control y sus propiedades a lo definido en el field */
	virtual void applyFieldProperties() = 0;

	virtual void valueEdited(const QVariant &value) = 0;

	/** Esta señal indicará cuándo se borra un widget. No se puede usar destroyed(QObject *)
	  ya que cuando ésta se emite, se ha ejecutado ya el destructor de QWidget */
	virtual void destroyed(QWidget *widget) = 0;

	/** Para refrescar los controles: Piden nuevo observador si es necesario */
	virtual void refresh() = 0;

};

#endif // DBABSTRACTVIEW_H
