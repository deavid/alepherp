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
#include "basebeanmodel.h"
#include <QStringList>
#include <limits>

class BaseBeanModelPrivate
{
public:
	QStringList m_checkFields;

    BaseBeanModelPrivate() {}
};

BaseBeanModel::BaseBeanModel(QObject *parent) :
	QAbstractItemModel(parent), d(new BaseBeanModelPrivate)
{
}

BaseBeanModel::~BaseBeanModel()
{
	delete d;
}

void BaseBeanModel::setCheckColumns(const QStringList &fieldNames)
{
	emit QAbstractItemModel::layoutAboutToBeChanged();
	d->m_checkFields = fieldNames;
	emit QAbstractItemModel::layoutChanged();
}

QStringList BaseBeanModel::checkColumns() const
{
	return d->m_checkFields;
}

QModelIndexList BaseBeanModel::checkedItems()
{
	QModelIndexList list;
	QHashIterator<int, bool> it(m_checkedItems);
	while ( it.hasNext() ) {
		it.next();
		if ( it.value() ) {
			QModelIndex idx = createIndex(it.key(), 0, 0);
			list.append(idx);
		}
	}
	return list;
}

/*!
  Marca o desmarca todos los items (segun el valor de checked)
  */
void BaseBeanModel::checkAllItems(bool checked)
{
	QModelIndexList list;
	for ( int i = 0 ; i < rowCount() ; i++ ) {
		list.append(createIndex(i, 0, 0));
	}
	setCheckedItems(list, checked);
}

void BaseBeanModel::setCheckedItems(QModelIndexList list, bool checked)
{
	int lessRow=ULONG_MAX, lessCol=ULONG_MAX, maxRow=0, maxCol=0;
	if ( list.size() == 0 ) {
		return;
	}
	foreach (QModelIndex index, list) {
		if ( checked ) {
			m_checkedItems[index.row()] = Qt::Checked;
		} else {
			m_checkedItems[index.row()] = Qt::Unchecked;
		}
		if ( lessRow > index.row() ) {
			lessRow = index.row();
		}
		if ( lessCol > index.column() ) {
			lessCol = index.column();
		}
		if ( maxRow < index.row() ) {
			maxRow = index.row();
		}
		if ( maxCol < index.column() ) {
			maxCol = index.column();
		}
	}
	QModelIndex topLeft = index(lessRow, lessCol);
	QModelIndex bottomRight = index(maxRow, maxCol);
	emit dataChanged(topLeft, bottomRight);
}

void BaseBeanModel::setCheckedItem(QModelIndex index, bool checked)
{
	if ( index.isValid() ) {
		if ( checked ) {
			m_checkedItems[index.row()] = Qt::Checked;
		} else {
			m_checkedItems[index.row()] = Qt::Unchecked;
		}
		emit dataChanged(index, index);
	}
}

void BaseBeanModel::setCheckedItem(int row, bool checked)
{
	QModelIndex idx = index(row, 0);
	setCheckedItem(idx, checked);
}

QSharedPointer<BaseBean> BaseBeanModel::bean(int row)
{
	QModelIndex idx = index(row, 0);
	return bean(idx);
}

QSharedPointer<BaseBean> BaseBeanModel::beanToBeEdited(int row)
{
	return bean(row);
}

/*!
  Las definiciones de tablas XML permiten que el contenido que se visualiza
  de una tabla sea el resultado de una vista, pero el bean a editar no es el de la vista
  sino el de la tabla original. Esta función devuelve ese bean original.
  */
QSharedPointer<BaseBean> BaseBeanModel::beanToBeEdited(const QModelIndex &index)
{
	return bean(index);
}
