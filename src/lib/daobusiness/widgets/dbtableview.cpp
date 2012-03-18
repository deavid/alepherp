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
#include "dbtableview.h"
#include "dao/beans/basebean.h"
#include "dao/beans/dbfield.h"
#include "dao/beans/beansfactory.h"
#include "dao/beans/dbrelation.h"
#include "dao/beans/dbrelationmetadata.h"
#include "dao/beans/dbfieldmetadata.h"
#include "dao/beans/basebeanmetadata.h"
#include "models/perphtmldelegate.h"
#include "models/dbbasebeanmodel.h"
#include "models/relationbasebeanmodel.h"
#include "models/filterbasebeanmodel.h"
#include "models/basebeanmodel.h"
#include "globales.h"
#include "dbnumberedit.h"
#include "configuracion.h"
#include <QHeaderView>

class DBTableViewPrivate
{
	Q_DECLARE_PUBLIC(DBTableView)
public:
	/** Este QAbstractItemDelegate controlará la visualización correcta de campos
	  con código HTML */
	PERPHtmlDelegate *m_htmlDelegate;
	DBTableView *q_ptr;
    QString m_itemCheckBox;
	bool m_automaticName;
	bool m_navigateOnEnter;
	bool m_atRowsEndNewRow;

	DBTableViewPrivate(DBTableView *qq);

	QString configurationName();
};

DBTableViewPrivate::DBTableViewPrivate (DBTableView *qq) : q_ptr(qq)
{
	m_htmlDelegate = NULL;
	m_automaticName = true;
	m_navigateOnEnter = false;
	m_atRowsEndNewRow = false;
}

QString DBTableViewPrivate::configurationName()
{
	QString temp, name;
	if ( !q_ptr->tableName().isEmpty() ) {
		temp = q_ptr->tableName();
	} else if ( !q_ptr->relationName().isEmpty() ) {
		temp = q_ptr->relationName();
	} else if ( q_ptr->metadata() != NULL && !q_ptr->metadata()->tableName().isEmpty() ) {
		temp = q_ptr->metadata()->tableName();
	}
    QDialog *parent = CommonsFunctions::parentDialog(q_ptr);
	if ( parent != NULL ) {
		name = QString("%1-%2").arg(parent->objectName()).arg(temp);
	} else {
		name = temp;
	}
	return name;
}


DBTableView::DBTableView( QWidget * parent ) :
	QTableView(parent),
	DBAbstractViewInterface(),
	d(new DBTableViewPrivate(this))
{
	d->m_htmlDelegate = new PERPHtmlDelegate(this);
	connect(horizontalHeader(), SIGNAL(sectionResized(int, int, int)), this, SLOT(saveHeaderSize(int, int, int)));
	connect(this, SIGNAL(enterPressedOnValidIndex(QModelIndex)), this, SLOT(nextCellOnEnter(QModelIndex)));
}


DBTableView::~DBTableView()
{
	emit destroyed(this);
	delete d;
}

void DBTableView::setAutomaticName(bool value)
{
	d->m_automaticName = value;
}

bool DBTableView::automaticName()
{
	return d->m_automaticName;
}

void DBTableView::setRelationName(const QString &name)
{
	m_relationName = name;
	if ( d->m_automaticName ) {
		setObjectName (d->configurationName());
	}
}

void DBTableView::setTableName(const QString &name)
{
	m_tableName = name;
	if ( d->m_automaticName ) {
		setObjectName (d->configurationName());
	}
}

/*!
  Al mostrarse el control es cuando se crean los modelos y demás
  */
void DBTableView::showEvent ( QShowEvent * event )
{
	if ( !this->m_externalModel ) {
		DBBaseWidget::showEvent(event);
		init(true);
		setFieldCheckBox(d->m_itemCheckBox);
	}
}

void DBTableView::keyPressEvent ( QKeyEvent * event )
{
	QModelIndex index;
	
	if ( event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter ) {
		index = currentIndex();
		if ( index.isValid() ) {
			emit enterPressedOnValidIndex( index );
			event->accept();
			return;
		}
	} else if ( event->key() == Qt::Key_Escape ) {
		index = currentIndex();
		if ( index.isValid() ) {
			int row = index.row();
			if ( model()->rowCount() == 1 ) {
				model()->removeRow(row);
				return;
			} else {
				model()->removeRow(row);
				QModelIndex prev = model()->index(row-1, 0);
				setCurrentIndex(prev);
			}
		}
	}
	QTableView::keyPressEvent(event);
}

void DBTableView::mouseDoubleClickEvent (QMouseEvent * event)
{
	QModelIndex index;

	if ( event->button() == Qt::LeftButton ) {
		index = currentIndex();
		if ( index.isValid() ) {
			emit doubleClickOnValidIndex( index );
			event->accept();
			return;
		}
	}
	QTableView::mouseDoubleClickEvent(event);
}

void DBTableView::closeEvent ( QCloseEvent * event )
{
	DBTableView::closeEvent(event);
}

void DBTableView::setupInternalModel()
{
	DBAbstractViewInterface::setupInternalModel();
	if ( m_metadata != NULL && m_model != NULL ) {
		prepareColumns();
	}
	if ( m_filterModel != NULL ) {
		m_filterModel->setDbStates(BaseBean::INSERT | BaseBean::UPDATE);
	}
}

void DBTableView::setupExternalModel()
{
	DBAbstractViewInterface::setupExternalModel();
	if ( m_metadata != NULL && m_model != NULL ) {
		prepareColumns();
	}
}

void DBTableView::setModel(QAbstractItemModel *model)
{
	QTableView::setModel(model);
	QAbstractItemModel *tmp;
	QAbstractProxyModel *filterModel = qobject_cast<QAbstractProxyModel *>(model);
	if ( filterModel != NULL ) {
		tmp = filterModel->sourceModel();
	} else {
		tmp = model;
	}
	if ( tmp->property("baseBeanModel").toBool() ) {
		BaseBeanModel *metadataModel = qobject_cast<BaseBeanModel *>(tmp);
		if ( metadataModel != NULL ) {
			m_metadata = metadataModel->metadata();
			prepareColumns();
		}
	}
}

/*!
  Ajusta las columnas visibles según la definición de los fields del bean.
  La visibilidad de la columna la da el FilterBaseBeanModel, aquí
  ajustamos parámetros como lo que se visualiza o el orden de las columnas iniciales.
  */
void DBTableView::prepareColumns()
{
	QHeaderView *header = horizontalHeader();
	FilterBaseBeanModel *mdl = qobject_cast<FilterBaseBeanModel *> (model());

	if ( m_metadata == NULL || mdl == NULL ) {
		return;
	}
	disconnect(horizontalHeader(), SIGNAL(sectionResized(int, int, int)), this, SLOT(saveHeaderSize(int, int, int)));
	if ( d->m_automaticName ) {
		setObjectName (d->configurationName());
	}

	QList<DBFieldMetadata *> list = mdl->visibleFields();
	int i = 0;
	foreach ( DBFieldMetadata *fld, list ) {
		if ( fld->html() ) {
			int col = header->visualIndex(i);
			QTableView::setItemDelegateForColumn(col, d->m_htmlDelegate);
		}
		i++;
	}
	configuracion.applyTableViewState(this);
	orderColumns(configuracion.tableViewColumnOrder(this));
	connect(horizontalHeader(), SIGNAL(sectionResized(int, int, int)), this, SLOT(saveHeaderSize(int, int, int)));
}

/*!
  Nos almacenará los anchos que el usuario haya seleccionado
  */
void DBTableView::saveHeaderSize(int logicalSection, int oldSize, int newSize)
{
	Q_UNUSED (logicalSection)
	if ( oldSize != newSize ) {
		configuracion.saveTableViewState(this);
	}
}

/*!
Establece el valor a mostrar en el control
*/
void DBTableView::setValue(const QVariant &value)
{
	Q_UNUSED(value)
}

/*!
Devuelve el valor mostrado o introducido en el control
*/
QVariant DBTableView::value()
{
	return QVariant();
}

/*!
 Ajusta el control y sus propiedades a lo definido en el field
*/
void DBTableView::applyFieldProperties()
{
	prepareColumns();
}

/*!
  Para refrescar los controles: Piden nuevo observador si es necesario
*/
void DBTableView::refresh()
{
	if ( m_model != NULL ) {
		if ( m_filterModel != NULL ) {
			m_filterModel->invalidate();
		}
	}
}

/*!
  Añade un bean al modelo de esta vista. Para ello, se debe estar en modelo datos internos
  y el bean debe ser del mismo tipo que los metadatos que presenta este widget
  */
QSharedPointer<BaseBean> DBTableView::addBean()
{
	QSharedPointer<BaseBean> bean;
	if ( !m_internalData ) {
		qDebug() << "DBTableView::deleteSelectedsBean: LLamada a esta función sin estar configurada para usar datos internos.";
	} else {
		RelationBaseBeanModel *model = qobject_cast<RelationBaseBeanModel *>(m_model);
		if ( model != NULL ) {
			int row = model->rowCount();
			if ( model->insertRow(row) ) {
				bean = model->bean(row);
			}
		}
		if ( m_filterModel != NULL ) {
			m_filterModel->invalidate();
		}
	}
	return bean;
}

/*!
  Marca los beans seleccionados para ser borrados. El widget debe estar en modo datos internos
  */
void DBTableView::deleteSelectedsBean()
{
	QModelIndexList list = m_selectionModel->selectedIndexes();
	QList<int> deleteRows;

	if ( !m_internalData ) {
		qDebug() << "DBTableView::deleteSelectedsBean: LLamada a esta función sin estar configurada para usar datos internos.";
		return;
	}
	if ( m_model != NULL ) {
		foreach ( QModelIndex index, list ) {
			if ( !deleteRows.contains(index.row()) ) {
				if ( m_filterModel != NULL ) {
					m_filterModel->removeRow(index.row());
				} else {
					m_model->removeRow(index.row());
				}
				deleteRows << index.row();
			}
		}
	}
	if ( m_filterModel != NULL ) {
		m_filterModel->invalidate();
	}
}

/*!
  Ordena las columnas de visualización según el hash indicado. El primer elemento
  es el índice del field en los metadatos de BaseBean. El segundo int es el lugar
  en el que debe aparecer
  */
void DBTableView::orderColumns(const QStringList &order)
{
	FilterBaseBeanModel *mdl = qobject_cast<FilterBaseBeanModel *> (model());
	if ( m_metadata == NULL || mdl == NULL ) {
		return;
	}

	QHeaderView *header = this->horizontalHeader();
	QList<DBFieldMetadata *> list = mdl->visibleFields();
	for ( int i = 0 ; i < order.size() ; i++ ) {
		int logicalIndex = -1;
		for ( int iField = 0 ; iField < list.size() ; iField ++ ) {
			if ( list.at(iField)->dbFieldName() == order.at(i) ) {
				logicalIndex = iField;
			}
		}
		int visualIndex = header->visualIndex(logicalIndex);
		header->moveSection(visualIndex, i);
	}
}

/*!
  Almacena en registro el orden y tipo de ordenación de las columnas del DBTableView
  */
void DBTableView::saveTableViewColumnOrder(const QStringList &order, const QStringList &sort)
{
	configuracion.saveTableViewColumnOrder(this, order, sort);
}

void DBTableView::observerUnregistered()
{
	DBBaseWidget::observerUnregistered();
	blockSignals(true);
	reset();
	blockSignals(false);
}

/*!
  Si hay una columna con checkbox, permite marcarla o desmarcarla entera...
  */
void DBTableView::checkAllItems(bool checked)
{
	FilterBaseBeanModel *mdlFilter = qobject_cast<FilterBaseBeanModel *>(model());
	if ( mdlFilter == NULL ) {
		return;
	}
	BaseBeanModel *mdl = qobject_cast<BaseBeanModel *>(mdlFilter->sourceModel());
	if ( mdl == NULL ) {
		return;
	}
	mdl->checkAllItems(checked);
}

/*!
  Devuelve un listado de los beans que han sido checkeados por el usuario
  */
BaseBeanPointerList DBTableView::checkedBeans()
{
    BaseBeanPointerList list;
    FilterBaseBeanModel *mdlFilter = qobject_cast<FilterBaseBeanModel *>(model());
    if ( mdlFilter == NULL ) {
        return list;
    }
    BaseBeanModel *mdl = qobject_cast<BaseBeanModel *>(mdlFilter->sourceModel());
    if ( mdl == NULL ) {
        return list;
    }
    QModelIndexList checkedItems = mdl->checkedItems();
    for ( int i = 0 ; i < checkedItems.size() ; i++ ) {
        QSharedPointer<BaseBean> bean = mdl->bean(checkedItems.at(i));
        if ( !bean.isNull() ) {
            list.append(bean);
        }
    }
    return list;
}

void DBTableView::setCheckedBeansByPk(QVariantList list, bool checked)
{
	// TODO: Hay codigo similar hecho para DBListView. Lo suyo sería hacerlo común en DBAbstractView
	Q_UNUSED(list)
	Q_UNUSED(checked)
}

/*!
  Devuelve un listado de los beans que han sido checkeados por el usuario.
  Esta función puede ser llamada desde Javascript, en el constructor del widget. En ese
  caso los modelos aún no se han creado, por lo que se guardarán en una estructura
  intermedia las primary keys de los beans pasados. No se guardan los beans, porque
  estos pueden haber sido borrados previamente por el motor de javascript
  */
void DBTableView::setCheckedBeans(BaseBeanPointerList list, bool checked)
{
	// TODO: Hay codigo similar hecho para DBListView. Lo suyo sería hacerlo común en DBAbstractView
	Q_UNUSED(list)
	Q_UNUSED(checked)
}

void DBTableView::setFieldCheckBox(const QString &name)
{
    d->m_itemCheckBox = name;
    if ( !d->m_itemCheckBox.isEmpty() ) {
        BaseBeanModel *mdl = qobject_cast<BaseBeanModel *>(m_model);
        if ( mdl != NULL ) {
            QStringList fields;
            fields << d->m_itemCheckBox;
            mdl->setCheckColumns(fields);
        }
    }
}

QString DBTableView::fieldCheckBox()
{
    return d->m_itemCheckBox;
}

bool DBTableView::navigateOnEnter() const
{
	return d->m_navigateOnEnter;
}

void DBTableView::setNavigateOnEnter(bool value)
{
	d->m_navigateOnEnter = value;
}

bool DBTableView::atRowsEndNewRow() const
{
	return d->m_atRowsEndNewRow;
}

void DBTableView::setAtRowsEndNewRow(bool value)
{
	d->m_atRowsEndNewRow = value;
}

void DBTableView::setFilter(const QString &value)
{
    DBAbstractViewInterface::setFilter(value);
    setFieldCheckBox(d->m_itemCheckBox);
}

/*!
  Ordena por el field marcado
  */
void DBTableView::sortByColumn(const QString &field, Qt::SortOrder order)
{
	FilterBaseBeanModel *mdl = qobject_cast<FilterBaseBeanModel*>(model());
	if ( mdl == 0 ) {
		return;
	}
	QList<DBFieldMetadata *> list = mdl->visibleFields();
	for ( int i = 0 ; i < list.size() ; i++ ) {
		if ( list.at(i)->dbFieldName() == field ) {
			QTableView::sortByColumn(i, order);
			return;
		}
	}
}

void DBTableView::nextCellOnEnter(const QModelIndex &actualCell)
{
	QModelIndex nextCell;
	if ( !d->m_navigateOnEnter ) {
		return;
	}
	if ( actualCell.column() == (horizontalHeader()->count()-1) ) {
		if ( actualCell.row() == (this->model()->rowCount()-1) ) {
			if ( d->m_atRowsEndNewRow ) {
				int row = model()->rowCount();
				model()->insertRow(row);
				nextCell = model()->index(row, 0);
				setCurrentIndex(nextCell);
				edit(nextCell);
				return;
			} else {
				nextCell = model()->index(0, 0);
			}
		} else {
			nextCell = model()->index(actualCell.row() + 1, 0);
		}
	} else {
		nextCell = model()->index(actualCell.row(), actualCell.column() + 1);
	}
	setCurrentIndex(nextCell);
	selectionModel()->select(nextCell, QItemSelectionModel::SelectCurrent);
}

/*!
  Tenemos que decirle al motor de scripts, que DBFormDlg se convierte de esta forma a un valor script
  */
QScriptValue DBTableView::toScriptValue(QScriptEngine *engine, DBTableView * const &in)
{
	return engine->newQObject(in, QScriptEngine::QtOwnership, QScriptEngine::PreferExistingWrapperObject);
}

void DBTableView::fromScriptValue(const QScriptValue &object, DBTableView * &out)
{
	out = qobject_cast<DBTableView *>(object.toQObject());
}
