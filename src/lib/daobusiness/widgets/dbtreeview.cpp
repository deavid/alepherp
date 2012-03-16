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
#include "dbtreeview.h"
#include "models/treebasebeanmodel.h"
#include "models/filterbasebeanmodel.h"
#include "dao/beans/dbfield.h"

#include <QItemSelectionModel>

class DBTreeViewPrivate
{
//	Q_DECLARE_PUBLIC(DBTreeView)
public:

	/** Tablas referenciadas que se utilizarán para mostrar los datos (es
	  decir, deben existir relaciones de padres a hijas definidas en los XML).
	  Se separan por puntos y coma, por ejemplo: familias;subfamilias;articulos
	  */
	QString m_tableNames;
	/** De cada tabla que se muestra de forma jerárquica, este campo indica qué campos
	  se muestran al usuario. Por ejemplo: nombre;nombre;descripcion.
	  Deben tener el mismo número de entradas que m_tableNames y en el mismo orden */
	QString m_visibleColumns;
	/** Por cada nivel jerárquico, indica qué valores proporciona value */
	QString m_keyColumns;
	/** Filtros a aplicar */
	QString m_filter;
	/** Nombre del item raíz. Se muestra al usuario. */
	QString m_rootName;
	/** Modelo que dará soporte a la visualización */
	TreeBaseBeanModel *m_model;
	/** Filtro del modelo */
	FilterBaseBeanModel *m_filterModel;
	/** Indica si se mostrará alguna imagen. La imagen puede ser de un archivo que se indique,
	  o bien de una imagen que se lea de la propia tabla que muestra los datos. El formato sería:
	  file:/home/david/tmp/imagen.png;field:imagen
		  */
	QString m_images;
	/** Tamaño de las imágenes que se mostrarán */
	QSize m_imagesSize;
	/** Modelo de selección */
	QItemSelectionModel *m_selectionModel;
	/** Niveles de los que se puede seleccionar */
	QString m_selectLevels;

	DBTreeViewPrivate();
};

DBTreeViewPrivate::DBTreeViewPrivate()
{
	m_model = NULL;
	m_filterModel = NULL;
	m_selectionModel = NULL;
}

DBTreeView::DBTreeView(QWidget *parent) :
	QTreeView(parent), DBBaseWidget(), d(new DBTreeViewPrivate)
{
}

DBTreeView::~DBTreeView()
{
	delete d;
}

QString DBTreeView::tableNames()
{
	return d->m_tableNames;
}

void DBTreeView::setTableNames(const QString &value)
{
	d->m_tableNames = value;
}

QString DBTreeView::visibleColumns()
{
	return d->m_visibleColumns;
}

void DBTreeView::setVisibleColumns(const QString &value)
{
	d->m_visibleColumns = value;
}

QString DBTreeView::keyColumns()
{
	return d->m_keyColumns;
}

void DBTreeView::setKeyColumns(const QString &value)
{
	d->m_keyColumns = value;
}

QString DBTreeView::filter()
{
	return d->m_filter;
}

void DBTreeView::setFilter(const QString &value)
{
	d->m_filter = value;
	if ( !d->m_filter.isEmpty() ) {
		if ( d->m_model != NULL ) {
			delete d->m_filterModel;
			delete d->m_model;
			delete d->m_selectionModel;
			d->m_model = NULL;
			d->m_filterModel = NULL;
			d->m_selectionModel = NULL;
		}
		init();
	}
}

QString DBTreeView::rootName()
{
	return d->m_rootName;
}

void DBTreeView::setRootName(const QString &value)
{
	d->m_rootName = value;
}

QString DBTreeView::images()
{
	return d->m_images;
}

void DBTreeView::setImages(const QString &value)
{
	d->m_images = value;
}

QSize DBTreeView::imagesSize()
{
	return d->m_imagesSize;
}

void DBTreeView::setImagesSize(const QSize &value)
{
	d->m_imagesSize = value;
}

void DBTreeView::setSelectLevels(const QString &level)
{
	d->m_selectLevels = level;
}

QString DBTreeView::selectLevels()
{
	return d->m_selectLevels;
}

/*!
  Devuelve un listado de los beans correspondiente a los items actualmente seleccionados
  */
BaseBeanPointerList DBTreeView::selectedBeans()
{
	BaseBeanPointerList list;
	QStringList levels = d->m_selectLevels.split(";");

	if ( d->m_model != NULL ) {
		QModelIndexList lIndex = d->m_selectionModel->selectedIndexes();
		foreach ( QModelIndex filterIndex, lIndex ) {
			QModelIndex sourceIndex = d->m_filterModel->mapToSource(filterIndex);
			QSharedPointer<BaseBean> bean = d->m_model->bean(sourceIndex);
			TreeItem *item = d->m_model->item(sourceIndex);
			if ( item != NULL && (levels.contains(QString("%1").arg(item->level())) || d->m_selectLevels.isEmpty()) &&
				 !bean.isNull() && !list.contains(bean) ) {
				list << bean;
			}
		}
	}
	return list;
}

void DBTreeView::applyFieldProperties()
{

}

QVariant DBTreeView::value()
{
	QVariant value;
	if ( d->m_model != NULL ) {
		QModelIndex idx = d->m_selectionModel->currentIndex();
		QModelIndex sourceIdx = d->m_filterModel->mapToSource(idx);
		QSharedPointer<BaseBean> bean = d->m_model->bean(sourceIdx);
		if ( !bean.isNull() ) {
			DBField *fld = bean->field(this->m_fieldName);
			if ( fld != NULL ) {
				value = fld->value();
			}
		}
	}
	return value;
}

/*!
  Este value debe ser una primary key
  */
void DBTreeView::setValue(const QVariant &value)
{
	if ( d->m_model != NULL ) {
		if ( value.type() != QVariant::Map ) {
			return;
		}
		QModelIndex idx = d->m_model->indexByPk(value);
		QModelIndex idxFilter = d->m_filterModel->mapFromSource(idx);
		d->m_selectionModel->select(idxFilter, QItemSelectionModel::Select);
	}
}

void DBTreeView::refresh()
{
	if ( d->m_model != NULL && d->m_filterModel != NULL ) {
		d->m_model->reload();
		d->m_filterModel->invalidate();
	}
}

/*!
  Al mostrarse el control es cuando se crean los modelos y demás
  */
void DBTreeView::showEvent ( QShowEvent * event )
{
	DBBaseWidget::showEvent(event);
	if ( !d->m_tableNames.isEmpty() ) {
		init();
	}
}

void DBTreeView::init()
{
	QStringList tableNames = d->m_tableNames.split(";");
	QStringList fieldsView = d->m_visibleColumns.split(";");
	QStringList images = d->m_images.split(";");
	QStringList filters = d->m_filter.split(";");

	if ( tableNames.size() == fieldsView.size() ) {
		if ( d->m_model == NULL ) {
			d->m_model = new TreeBaseBeanModel(tableNames, d->m_rootName, fieldsView, filters, this);
			d->m_filterModel = new FilterBaseBeanModel(this);
			d->m_filterModel->setSourceModel(d->m_model);
			d->m_model->setImages(images);
			d->m_model->setSize(d->m_imagesSize);
			d->m_selectionModel = new QItemSelectionModel(d->m_filterModel, this);
			QTreeView::setModel(d->m_filterModel);
			QTreeView::setSelectionModel(d->m_selectionModel);
		}
	}
}

QSharedPointer<BaseBean> DBTreeView::selectedBean()
{
	QSharedPointer<BaseBean> bean;
	if ( d->m_model != NULL && d->m_selectionModel != NULL ) {
		if ( d->m_model->property("baseBeanModel").toBool() ) {
			QModelIndex idx = d->m_selectionModel->currentIndex();
			BaseBeanModel *metadataModel = qobject_cast<BaseBeanModel *>(d->m_model);
			if ( d->m_filterModel != NULL && metadataModel != NULL ) {
				QModelIndex sourceIdx = d->m_filterModel->mapToSource(idx);
				bean = metadataModel->bean(sourceIdx);
			} else if ( metadataModel != NULL ) {
				bean = metadataModel->bean(idx);
			}
		}
	}
	return bean;
}

void DBTreeView::observerUnregistered()
{
	DBBaseWidget::observerUnregistered();
	blockSignals(true);
	reset();
	blockSignals(false);
}

/*!
  Tenemos que decirle al motor de scripts, que DBFormDlg se convierte de esta forma a un valor script
  */
QScriptValue DBTreeView::toScriptValue(QScriptEngine *engine, DBTreeView * const &in)
{
	return engine->newQObject(in, QScriptEngine::QtOwnership, QScriptEngine::PreferExistingWrapperObject);
}

void DBTreeView::fromScriptValue(const QScriptValue &object, DBTreeView * &out)
{
	out = qobject_cast<DBTreeView *>(object.toQObject());
}
