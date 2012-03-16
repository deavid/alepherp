/***************************************************************************
 *   Copyright (C) 2010 by David Pinelo   *
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
#include "relationbasebeanmodel.h"
#include "dao/beans/dbfield.h"
#include "dao/beans/dbrelation.h"
#include "dao/beans/basebean.h"
#include "dao/beans/beansfactory.h"
#include "dao/beans/basebeanmetadata.h"
#include "dao/beans/dbfieldmetadata.h"
#include "dao/beans/dbrelationmetadata.h"
#include "models/fieldbeanitem.h"
#include <QCoreApplication>
#include <QDebug>

class RelationBaseBeanModelPrivate
{
//	Q_DECLARE_PUBLIC(RelationBaseBeanModel)
public:
	/** Muestra del bean que se utilizara para mostrar los datos de este listado */
	BaseBeanMetadata * m_metadata;
	/** Si se pasa un objeto relation, este se almacena en m_relation. En ese caso, el conjunto
	  de registros que se obtienen son los que dependen del padre de m_relation */
	DBRelation * m_relation;
	/** ¿Se pueden editar los datos de este modelo? */
	bool m_readOnly;
	/** Orden con el que se muestran los datos */
	QString m_order;

	RelationBaseBeanModelPrivate() {
		m_metadata = NULL;
		m_relation = NULL;
		m_readOnly = true;
	}
};

/*!
  Este constructor será usado cuando se pretendan visualizar los hijos de una relación
  */
RelationBaseBeanModel::RelationBaseBeanModel(bool readOnly, DBRelation *rel, const QString &order, QObject *parent) :
		BaseBeanModel(parent), d(new RelationBaseBeanModelPrivate)
{
	d->m_metadata = BeansFactory::metadataBean(rel->metadata()->tableName());
	if ( d->m_metadata == NULL ) {
		qWarning() << "RelationBaseBeanModel: No existe la tabla: " << rel->metadata()->tableName();
		return;
	}
	d->m_relation = rel;
	d->m_readOnly = readOnly;
	d->m_order = order;
	// Garantizamos primero que la relación tiene todos sus hijos cargados
	d->m_relation->childs(order);
	// Los cambios que se produzcan en background en los beans, se deben de tener en cuenta
	connect(d->m_relation, SIGNAL(fieldChildModified(BaseBean *,QString,QVariant)), this, SLOT(fieldBeanModified(BaseBean *,QString,QVariant)));
	connect(d->m_relation, SIGNAL(childDbStateModified(BaseBean*,int)), this, SLOT(dbStateBeanModified(BaseBean*,int)));
	connect(d->m_relation, SIGNAL(childInserted(BaseBean*, int)), this, SLOT(refresh()));
}

RelationBaseBeanModel::~RelationBaseBeanModel()
{
	delete d;
}


QString RelationBaseBeanModel::tableName()
{
	return d->m_metadata->tableName();
}

DBRelation * RelationBaseBeanModel::relation()
{
	 return d->m_relation;
}

/*!
  Se ha modificado un hijo en background. Debemos reflejarlo en el modelo
  */
void RelationBaseBeanModel::fieldBeanModified(BaseBean *bean, const QString &fieldName, const QVariant &value)
{
	Q_UNUSED(value)
	DBField *fld = bean->field(fieldName);
	if ( fld != NULL ) {
		QModelIndex index = indexByPk(bean->pkValue());
		// Lo hacemos así por si hay campos calculados, que se refresquen todos
		QModelIndex topLeft = createIndex(index.row(), 0, 0);
		QModelIndex bottomRight = createIndex(index.row(), bean->fieldCount()-1, 0);
		emit dataChanged(topLeft, bottomRight);
	}
}

void RelationBaseBeanModel::dbStateBeanModified(BaseBean *bean, int state)
{
	Q_UNUSED(bean)
	if ( state == BaseBean::TO_BE_DELETED ) {
		refresh();
	}
}

/*!
  Devuelve el bean ubicado en la fila row
*/
QSharedPointer<BaseBean> RelationBaseBeanModel::bean (int row)
{
	QModelIndex idx = createIndex(row, 0, 0);
    QSharedPointer<BaseBean> b = bean(idx);
    b->setCanSaveOnDbDirectly(false);
    return b;
}

/*!
  Devuelve el bean asociado a la fila index.row()
*/
QSharedPointer<BaseBean> RelationBaseBeanModel::bean (const QModelIndex &index)
{
	BaseBeanPointerList list = d->m_relation->childs(d->m_order);
	if ( index.isValid() && (index.row() > -1 && index.row() < list.size()) ) {
        list.at(index.row())->setCanSaveOnDbDirectly(false);
		return list.at(index.row());
	}
	return QSharedPointer<BaseBean>();
}

int RelationBaseBeanModel::columnCount ( const QModelIndex & parent ) const
{
	Q_UNUSED (parent);
	return d->m_metadata->fields().size();
}

int RelationBaseBeanModel::rowCount ( const QModelIndex & parent ) const
{
	Q_UNUSED (parent);
	return d->m_relation->childsCount();
}

/*!
  El modelindex llevará como dato interno, un puntero al DBField del BaseBean que controla
  */
QModelIndex RelationBaseBeanModel::index ( int row, int column, const QModelIndex & parent ) const
{
	Q_UNUSED (parent);
	if ( row > -1 && row < d->m_relation->childsCount() ) {
		QSharedPointer<BaseBean> bean = d->m_relation->child(row);
		if ( bean.isNull() ) {
			return QAbstractItemModel::createIndex(row, column, 0);
		}
		DBField *fld = bean->field(column);
		return QAbstractItemModel::createIndex(row, column, fld);
	} else {
		return QAbstractItemModel::createIndex(row, column, 0);
	}
}

QModelIndex RelationBaseBeanModel::parent ( const QModelIndex & index ) const
{
	Q_UNUSED (index);
	return QModelIndex();
}

QVariant RelationBaseBeanModel::headerData ( int section, Qt::Orientation orientation, int role ) const
{
	QVariant returnData;
	QFont font;
	DBFieldMetadata *field = d->m_metadata->field(section);
	if ( field == NULL  || orientation == Qt::Vertical ) {
		return QAbstractItemModel::headerData(section, orientation, role);
	}
	switch ( role ) {
	case Qt::DisplayRole:
		return QObject::trUtf8(field->fieldName().toUtf8());

	case Qt::UserRole:
		return field->dbFieldName();

	case Qt::FontRole:
		font.setBold(true);
		return font;

	case Qt::DecorationRole:
	case Qt::ToolTipRole:
	case Qt::StatusTipRole:
	case Qt::SizeHintRole:
		// TODO: Estaria guapo esto
		break;
	case Qt::TextAlignmentRole:
		if ( field->type() == QVariant::Int || field->type() == QVariant::Double ) {
			returnData = int (Qt::AlignHCenter | Qt::AlignRight);
		} else {
			returnData = int (Qt::AlignHCenter | Qt::AlignLeft);
		}
		break;
	}
	return returnData;
}

Qt::ItemFlags RelationBaseBeanModel::flags ( const QModelIndex & index ) const
{
	Qt::ItemFlags flags;
	BaseBeanPointerList list = d->m_relation->childs(d->m_order);
	if ( !index.isValid() || index.row() < 0 || index.row() >= list.size() ) {
		return Qt::NoItemFlags;
	}
	DBField *field = static_cast<DBField *> (index.internalPointer());
	flags = Qt::ItemIsSelectable;
	if ( !field->metadata()->serial() ) {
		flags = flags | Qt::ItemIsEnabled;
		if ( !d->m_readOnly && !field->metadata()->calculated() ) {
			flags = flags | Qt::ItemIsEditable;
		}
	}
	if ( field->metadata()->type() == QVariant::Bool ) {
		flags = flags | Qt::ItemIsUserCheckable;
	}
	return flags;
}

QVariant RelationBaseBeanModel::data ( const QModelIndex & item, int role ) const
{
	if ( !item.isValid() || item.row() <= -1 || item.row() >= d->m_relation->childsCount() ) {
		return QVariant();
	}
	DBField *field = static_cast<DBField *> (item.internalPointer());
	if ( field == NULL ) {
		return QVariant();
	}

	if ( role == Qt::TextAlignmentRole ) {
		if ( field->metadata()->type() == QVariant::Int || field->metadata()->type() == QVariant::Double ) {
			return int(Qt::AlignRight | Qt::AlignVCenter);
		} else if ( field->metadata()->type() == QVariant::String ) {
			return int(Qt::AlignLeft | Qt::AlignVCenter);
		} else {
			return int(Qt::AlignRight | Qt::AlignVCenter);
		}
	} else if ( role == Qt::DisplayRole && field->metadata()->type() != QVariant::Bool ) {
		return field->displayValue();
	} else if ( role == Qt::EditRole && field->metadata()->type() != QVariant::Bool ) {
		return field->value();
	} else if ( role == Qt::UserRole ) {
		return field->value();
	} else if ( role == Qt::CheckStateRole ) {
		QStringList chkColumns = BaseBeanModel::checkColumns();
		if ( field->metadata()->type() == QVariant::Bool ) {
			if ( field->value().toBool() ) {
				return Qt::Checked;
			} else {
				return Qt::Unchecked;
			}
		} else if ( chkColumns.contains(field->dbFieldName()) ) {
			if ( m_checkedItems.contains(item.row()) && m_checkedItems[item.row()] ) {
				return Qt::Checked;
			} else {
				return Qt::Unchecked;
			}
		}
	} else if ( role == Qt::FontRole ) {
		return field->metadata()->fontOnGrid();
	} else if ( role == Qt::BackgroundRole && field->metadata()->backgroundColor().isValid() ) {
		return QBrush(field->metadata()->backgroundColor());
	} else if ( role == Qt::ForegroundRole && field->metadata()->color().isValid() ) {
		return QBrush(field->metadata()->color());
	} else if ( role == Qt::DecorationRole ) {
		QMap<QString, QString> optionIcons = field->metadata()->optionsIcons();
		if ( optionIcons.size() != 0 ) {
			return QPixmap(optionIcons[field->value().toString()]);
		}
	}
	return QVariant();
}

bool RelationBaseBeanModel::setData ( const QModelIndex & index, const QVariant & value, int role )
{
	BaseBeanPointerList list = d->m_relation->childs(d->m_order);
	if ( index.row() < 0 || index.row() >= list.size() ) {
		return false;
	}
	DBField *field = static_cast<DBField *> (index.internalPointer());
	if ( role == Qt::EditRole && !d->m_readOnly ) {
		if ( !field->metadata()->calculated() ) {
			field->setValue(value);
		}
		emit dataChanged(index, index);
		return true;
	} else if ( role == Qt::CheckStateRole ) {
		bool ok;
		int v = value.toInt(&ok);
		if ( !ok ) { return false; }
		if ( !d->m_readOnly && field->metadata()->type() == QVariant::Bool ) {
			if ( !field->metadata()->calculated() ) {
				(v == Qt::Checked ? field->setValue(true) : field->setValue(false));
			}
		} else {
			m_checkedItems[index.row()] = ( v == Qt::Checked );
		}
		return false;
	}
	return false;
}

bool RelationBaseBeanModel::removeRows ( int row, int count, const QModelIndex & parent )
{
	Q_UNUSED(parent)
	beginRemoveRows(QModelIndex(), row, row + count - 1);
	for ( int i = 0 ; i < count ; i++ ) {
		QModelIndex idx = index(row + count - 1, 0);
		if ( idx.isValid() ) {
			DBField *fld = static_cast<DBField *>(idx.internalPointer());
			if ( fld != NULL ) {
				BaseBean *bean = fld->bean();
				// Sin estas llamadas de bloqueo, se produce un bucle infinito, por la conexión
				// que existe connect(d->m_relation, SIGNAL(childDbStateModified(BaseBean*,int)), this, SLOT(dbStateBeanModified(BaseBean*,int)));
				this->blockSignals(true);
				if ( bean->dbState() == BaseBean::UPDATE ) {
					bean->setDbState(BaseBean::TO_BE_DELETED);
				} else {
					d->m_relation->removeChildByObjectName(bean->objectName());
				}
				this->blockSignals(false);
			}
		}
	}
	endRemoveRows();
	return true;
}

/*!
  Marcará el bean como para ser borrado.
  */
void RelationBaseBeanModel::removeBaseBean ( const QSharedPointer<BaseBean> &bean )
{
	QModelIndex index = indexByPk(bean->pkValue());
	if ( index.isValid() ) {
		removeRow(index.row());
	}
}

/*!
  Inserta una nueva línea... crea internamente, dentro de la DBRelation * el nuevo bean.
  */
bool RelationBaseBeanModel::insertRows ( int row, int count, const QModelIndex & parent )
{
	Q_UNUSED (parent);
	beginInsertRows (QModelIndex(), row, row + count -1);
	for ( int i = 0 ; i < count ; i ++ ) {
		d->m_relation->blockSignals(true);
		QSharedPointer<BaseBean> bean = d->m_relation->newChild(QSharedPointer<BaseBean>(), row + i);
		d->m_relation->blockSignals(false);
		bean->setCanSaveOnDbDirectly(false);
	}
	endInsertRows ();
	return true;
}

void RelationBaseBeanModel::refresh()
{
	emit QAbstractItemModel::layoutAboutToBeChanged();
	d->m_relation->childs(d->m_order);
	emit QAbstractItemModel::layoutChanged();
}

QList<DBFieldMetadata *> RelationBaseBeanModel::fields()
{
	return d->m_metadata->fields();
}

BaseBeanMetadata * RelationBaseBeanModel::metadata()
{
	return d->m_metadata;
}

/*!
  Devuelve un conjunto de indices, que coindicen exactamente con el valor value de la columna dbColumnName
  del bean
  */
QModelIndexList RelationBaseBeanModel::indexes( const QString &dbColumnName, const QVariant &value )
{
	QModelIndexList list;
	int row = 0;
	BaseBeanPointerList listBean = d->m_relation->childs(d->m_order);

	foreach ( QSharedPointer<BaseBean> bean, listBean ) {
		if ( bean->fieldValue(dbColumnName) == value ) {
			QModelIndex idx = this->index(row, bean->fieldIndex(dbColumnName));
			list << idx;
		}
		row++;
	}
	return list;
}

/*!
  Obtiene el QModelIndex de la fila o Bean que tiene como primary key a value
  */
QModelIndex RelationBaseBeanModel::indexByPk(const QVariant &value)
{
	int row = 0;
	BaseBeanPointerList listBean = d->m_relation->childs(d->m_order);

	foreach ( QSharedPointer<BaseBean> bean, listBean ) {
		if ( bean->pkValue().toMap() == value.toMap() ) {
			QModelIndex idx = this->index(row, 0);
			return idx;
		}
		row++;
	}
	return QModelIndex();
}
