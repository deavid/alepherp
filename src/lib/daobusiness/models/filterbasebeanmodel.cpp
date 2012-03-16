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
#include "filterbasebeanmodel.h"
#include "configuracion.h"
#include "models/basebeanmodel.h"
#include "models/treebasebeanmodel.h"
#include "models/treeitem.h"
#include "models/dbbasebeanmodel.h"
#include "dao/beans/basebean.h"
#include "dao/beans/basebeanmetadata.h"
#include "dao/beans/dbfield.h"
#include "dao/beans/dbfieldmetadata.h"
#include <QDate>
#include <QDebug>

class FilterBaseBeanModelPrivate
{
//	Q_DECLARE_PUBLIC(FilterBaseBeanModel)
public:
	QHash<QString, QPair<QString, QVariant> > m_filter;
	int m_states;
    bool m_includeMemoFieldsOnFilter;
	int m_columnCount;

    FilterBaseBeanModelPrivate() {
        m_includeMemoFieldsOnFilter = false;
		m_columnCount = -1;
	}
};

FilterBaseBeanModel::FilterBaseBeanModel(QObject *parent) :
	QSortFilterProxyModel(parent), d(new FilterBaseBeanModelPrivate)
{
	d->m_states = BaseBean::INSERT | BaseBean::UPDATE | BaseBean::TO_BE_DELETED;
}

FilterBaseBeanModel::~FilterBaseBeanModel()
{
	delete d;
}

void FilterBaseBeanModel::setDbStates(int state)
{
	d->m_states = state;
}

int FilterBaseBeanModel::dbStates()
{
	return d->m_states;
}

bool FilterBaseBeanModel::includeMemoFieldsOnFilter()
{
    return d->m_includeMemoFieldsOnFilter;
}

void FilterBaseBeanModel::setIncludeMemoFieldsOnFilter(bool value)
{
    d->m_includeMemoFieldsOnFilter = value;
}

void FilterBaseBeanModel::resetFilter()
{
	d->m_filter.clear();
	invalidateFilter();
}

QHash<QString, QPair<QString, QVariant> > FilterBaseBeanModel::filter()
{
	return d->m_filter;
}

void FilterBaseBeanModel::removeFilterKeyColumn (const QString &dbFieldName)
{
	d->m_filter.remove(dbFieldName);
}

void FilterBaseBeanModel::setFilterKeyColumn (const QString &dbFieldName, const QVariant &value, const QString &op)
{
	QPair<QString, QVariant> pair;
	pair.first = op;
	pair.second = value;
	d->m_filter[dbFieldName] = pair;
}

void FilterBaseBeanModel::setFilterKeyColumn (const QString &dbFieldName, const QRegExp &regExp)
{
	QPair<QString, QVariant> pair;
	pair.first = "=";
	pair.second = regExp.pattern();
	d->m_filter[dbFieldName] = pair;
}

void FilterBaseBeanModel::setFilterKeyColumnBetween (const QString &field, const QVariant &value1, const QVariant &value2)
{
	QPair<QString, QVariant> pair;
	QVariantList varList;
	pair.first = "=";
	varList << value1;
	varList << value2;
	pair.second = varList;
	d->m_filter[field] = pair;
}

/*!
 Establece un filtro para la visualización del bean asignado a un index. Este filtro se aplica
 a todos los beans del modelo
 */
void FilterBaseBeanModel::setFilter (const QString &filter)
{
	QPair<QString, QVariant> pair;
	pair.first = "=";
	pair.second = filter;
	d->m_filter.insertMulti("aloneFilter", pair);
}

/*!
 Establece un filtro para la visualización del bean asignado a un index. Este filtro se aplica
 cuando todos los beans están en un mismo nivel (no en un árbol).
 El nivel 0 es la raíz del árbol
 */
void FilterBaseBeanModel::setFilterByLevel (int level, const QString &filter)
{
	QPair<QString, QVariant> pair;
	pair.first = "=";
	pair.second = filter;
	QString key = QString("Level%1").arg(level);
	d->m_filter[key] = pair;
}

void FilterBaseBeanModel::setFilterPkColumn ( const QVariant &pk )
{
	QVariantMap values = pk.toMap();
	QMapIterator<QString, QVariant> it(values);
	BaseBeanModel *model = static_cast<BaseBeanModel *>(sourceModel());
	BaseBeanMetadata *metadata = model->metadata();
	QDate v;
	QPair<QString, QVariant> pair;
	pair.first = "=";
	while ( it.hasNext() ) {
		it.next();
		DBFieldMetadata *fld = metadata->field( it.key() );
		if ( fld != NULL ) {
			switch ( fld->type() ) {
			case QVariant::Int:
				pair.second = QString("%1").arg(it.value().toInt());
				break;
			case QVariant::Double:
				pair.second = QString("%1").arg(it.value().toDouble());
				break;
			case QVariant::Date:
				v = it.value().toDate();
				pair.second = v.toString("dd/MM/yyyy");
				break;
			case QVariant::String:
				pair.second = it.value().toString();
				break;
			default:
				pair.second = it.value().toString();
				break;
			}
			d->m_filter[fld->dbFieldName()] = pair;
		}
	}
}

bool FilterBaseBeanModel::filterAcceptsColumn (int source_column, const QModelIndex & sourceParent) const
{
	Q_UNUSED(sourceParent)
	BaseBeanModel *model = static_cast<BaseBeanModel *>(sourceModel());
	TreeBaseBeanModel *modelTree = qobject_cast<TreeBaseBeanModel *>(sourceModel());
	if ( model == NULL ) {
		return false;
	}
	if ( modelTree != NULL ) {
		return true;
	}
	BaseBeanMetadata *metadata = model->metadata();
	if ( metadata == NULL ) {
		return false;
	}
	DBFieldMetadata *fld = metadata->field(source_column);
	if ( fld == NULL ) {
		return false;
	}
	return fld->visibleGrid();
}

bool FilterBaseBeanModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
	Q_UNUSED(sourceParent)
	BaseBeanModel *model = static_cast<BaseBeanModel *>(sourceModel());
	QModelIndex index = model->index(sourceRow, 0);
	QHashIterator<QString, QPair<QString, QVariant> > it (d->m_filter);
	TreeBaseBeanModel *treeSource = qobject_cast<TreeBaseBeanModel *>(sourceModel());
	QSharedPointer<BaseBean> bean;

	if ( d->m_filter.isEmpty() && d->m_states == (BaseBean::INSERT | BaseBean::UPDATE | BaseBean::TO_BE_DELETED) ) {
		return true;
	}
	// Si el bean no se ha obtenido de base de datos, es porque seguramente no haya sido visualizado en pantalla,
	// ya que todas las demás posibilidades estan cubiertas. En ese caso se acepta directamente
	DBBaseBeanModel *dbmodel = qobject_cast<DBBaseBeanModel *>(sourceModel());
	if ( dbmodel != NULL ) {
		dbmodel->setStaticModel(true);
	}

	// Si obtenemos el bean antes, al ser esta función llamada para TODAS las filas, se obtendrían todos los beans del tirón
	bean = model->bean(index);
	if ( dbmodel != NULL ) {
		dbmodel->setStaticModel(false);
		// Un pequeño detalle estético. Si se está agregando un bean a un modelo de base de datos, se permite
		// su visualización
		if ( bean->dbState() == BaseBean::INSERT ) {
			return true;
		}
	}
	if ( bean.isNull() ) {
		return false;
	}
	if ( ! (d->m_states & bean->dbState()) ) {
		return false;
	}
	if ( bean->metadata()->logicalDelete() ) {
		DBField *fld = bean->field("is_deleted");
		if ( fld != NULL && fld->value().toBool() ) {
			return false;
		}
	}
	if ( treeSource != NULL ) {
		TreeItem *item = treeSource->item(index);
		if ( item != NULL ) {
			QString key = QString("Level%1").arg(item->level());
			if ( d->m_filter.contains(key) ) {
				return bean->checkFilter(d->m_filter.value(key).second.toString());
			}
		}
		return true;
	}

	while ( it.hasNext() ) {
		it.next();
		if ( it.key() == "aloneFilter" ) {
			if ( !bean->checkFilter(it.value().second.toString()) ) {
				return false;
			}
		} else {
            DBField *fld = bean->field(it.key());
            if ( fld != NULL ) {
                if ( !fld->metadata()->memo() || d->m_includeMemoFieldsOnFilter ) {
                    if ( it.value().second.type() == QVariant::List ) {
                        QVariantList list = it.value().second.toList();
                        if ( list.size() == 2 && !fld->checkValue(list.at(0), list.at(1)) ) {
                            return false;
                        }
                    } else {
						QString v = it.value().second.toString();
						if ( v.isEmpty() ) {
							return true;
						} else {
							if ( !fld->checkValue(it.value().second, it.value().first) ) {
								return false;
							}
						}
                    }
                }
            }
		}
	}
	return true;
}

bool FilterBaseBeanModel::lessThan ( const QModelIndex & left, const QModelIndex & right ) const
{
	bool result;
	BaseBeanModel *model = static_cast<BaseBeanModel *>(sourceModel());
	BaseBeanMetadata *bean = model->metadata();

	QModelIndex vLeftCol = sourceModel()->index(left.row(), left.column());
	QModelIndex vRightCol = sourceModel()->index(right.row(), right.column());
	QVariant vLeft = sourceModel()->data(vLeftCol, Qt::UserRole);
	QVariant vRight = sourceModel()->data(vRightCol, Qt::UserRole);
	DBFieldMetadata *fld = bean->field(left.column());
	if ( fld != NULL ) {
		switch ( fld->type() ) {
		case QVariant::Int:
			result = ( vLeft.toInt() < vRight.toInt() );
			break;

		case QVariant::Double:
			result = ( vLeft.toDouble() < vRight.toDouble() );
			break;

		case QVariant::Date:
			result = ( vLeft.toDate() < vRight.toDate() );
			break;

		case QVariant::String:
			result = ( vLeft.toString().toUpper() < vRight.toString().toUpper() );
			break;

		default:
			result = QSortFilterProxyModel::lessThan(left, right);
			break;
		}
	} else {
		result = QSortFilterProxyModel::lessThan(left, right);
	}
	if ( !result ) {
		return false;
	}
	return true;
}

QSharedPointer<BaseBean> FilterBaseBeanModel::bean (const QModelIndex &index)
{
	if ( index.isValid() ) {
		QModelIndex source = mapToSource(index);
		BaseBeanModel *model = static_cast<BaseBeanModel *>(sourceModel());
		if ( model != NULL ) {
			return model->bean(source);
		}
	}
	return QSharedPointer<BaseBean>();
}

QSharedPointer<BaseBean> FilterBaseBeanModel::bean (int row)
{
	QModelIndex idx = index(row, 0, QModelIndex());
	return bean(idx);
}

QSharedPointer<BaseBean> FilterBaseBeanModel::beanToBeEdited (const QModelIndex &index)
{
	if ( index.isValid() ) {
		QModelIndex source = mapToSource(index);
		BaseBeanModel *model = static_cast<BaseBeanModel *>(sourceModel());
		if ( model != NULL ) {
			return model->beanToBeEdited(source);
		}
	}
	return QSharedPointer<BaseBean>();
}

QSharedPointer<BaseBean> FilterBaseBeanModel::beanToBeEdited (int row)
{
	QModelIndex idx = index(row, 0, QModelIndex());
	return beanToBeEdited(idx);
}

/*!
  Devuelve la lista de definiciones de campo que son visibles según la propiedad
  visibleGrid que se ajusta desde el XML
  */
QList<DBFieldMetadata *> FilterBaseBeanModel::visibleFields() const
{
	QList<DBFieldMetadata *> list;
	BaseBeanModel *model = static_cast<BaseBeanModel *>(sourceModel());
	if ( model != NULL ) {
		BaseBeanMetadata *metadata = model->metadata();
		if ( metadata != NULL ) {
			foreach ( DBFieldMetadata *fld, metadata->fields() ) {
				if ( fld->visibleGrid() ) {
					list << fld;
				}
			}
		}
	}
	return list;
}

/**
  Debe tenerse en cuenta el filtro de las columnas
  */
QModelIndex FilterBaseBeanModel::mapFromSource ( const QModelIndex & sourceIndex ) const
{
	return QSortFilterProxyModel::mapFromSource(sourceIndex);
}

QModelIndex FilterBaseBeanModel::mapToSource ( const QModelIndex & proxyIndex ) const
{
	return QSortFilterProxyModel::mapToSource(proxyIndex);
}

/*!
  Necesario implementarlo para modelos muy grandes de datos
  */
int FilterBaseBeanModel::columnCount (const QModelIndex & parent) const
{
	if ( d->m_columnCount != -1 ) {
		return d->m_columnCount;
	}
	BaseBeanModel *model = static_cast<BaseBeanModel *>(sourceModel());
	TreeBaseBeanModel *modelTree = qobject_cast<TreeBaseBeanModel *>(sourceModel());
	if ( model == NULL ) {
		d->m_columnCount = QSortFilterProxyModel::columnCount(parent);
		return d->m_columnCount;
	}
	if ( modelTree != NULL ) {
		d->m_columnCount = QSortFilterProxyModel::columnCount(parent);
		return d->m_columnCount;
	}
	BaseBeanMetadata *metadata = model->metadata();
	if ( metadata == NULL ) {
		d->m_columnCount = QSortFilterProxyModel::columnCount(parent);
		return d->m_columnCount;
	}
	d->m_columnCount = 0;
	foreach (DBFieldMetadata *fld, metadata->fields()) {
		if ( fld->visibleGrid() ) {
			d->m_columnCount++;
		}
	}
	return d->m_columnCount;
}

/**
  Si el número de registros a ordenar es muy elevado, este proxy obtendrá todos los registros
  de base de datos, lo que puede ser muy pesado. Mejor, solicitamos una nueva consulta con offset
  a la base de datos
  */
void FilterBaseBeanModel::sort ( int column, Qt::SortOrder order )
{
	DBBaseBeanModel *model = qobject_cast<DBBaseBeanModel *>(sourceModel());
	if ( model == NULL ) {
		QSortFilterProxyModel::sort(column, order);
	} else {
		if ( FilterBaseBeanModel::rowCount() > model->offset() ) {
			QString orderClausule;
			QList<DBFieldMetadata *> visibleFlds = visibleFields();
			if ( visibleFlds.size() == 0 || column >= visibleFlds.size() ) {
				QSortFilterProxyModel::sort(column, order);
				return;
			}
			DBFieldMetadata *fld = visibleFlds.at(column);
			if ( fld == NULL ) {
				QSortFilterProxyModel::sort(column, order);
				return;
			}
			orderClausule = QString("%1 %2").arg(visibleFlds.at(column)->dbFieldName()).
					arg(order == Qt::AscendingOrder ? "ASC" : "DESC");
			if ( orderClausule != model->orderClausule() ) {
				model->setOrder(orderClausule);
			}
		} else {
			QSortFilterProxyModel::sort(column, order);
		}
	}
}

/*!
  Se implementa para mejorar la eficiencia en modelos grandes
  */
QVariant FilterBaseBeanModel::headerData (int section, Qt::Orientation orientation, int role) const
{
	BaseBeanModel *model = qobject_cast<DBBaseBeanModel *>(sourceModel());
	if ( model == NULL ) {
		return QSortFilterProxyModel::headerData(section, orientation, role);
	}
	QList<DBFieldMetadata *> visibleFlds = visibleFields();
	if ( visibleFlds.size() == 0 || section >= visibleFlds.size() ) {
		return QSortFilterProxyModel::headerData(section, orientation, role);
	}
	DBFieldMetadata *fld = visibleFlds.at(section);
	if ( fld == NULL ) {
		return QSortFilterProxyModel::headerData(section, orientation, role);
	}
	int indexSection = model->metadata()->fieldIndex(fld->dbFieldName());
	return model->headerData(indexSection, orientation, role);
}

