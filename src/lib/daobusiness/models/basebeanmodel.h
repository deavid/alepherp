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
#ifndef BASEBEANMODEL_H
#define BASEBEANMODEL_H

#include <QAbstractItemModel>
#include <QSharedPointer>
#include <QModelIndex>
#include "dao/beans/basebean.h"

class BaseBean;
class BaseBeanMetadata;
class BaseBeanModelPrivate;

class BaseBeanModel : public QAbstractItemModel
{
    Q_OBJECT
private:
	BaseBeanModelPrivate *d;
	Q_DECLARE_PRIVATE(BaseBeanModel)

protected:
	QHash<int, bool> m_checkedItems;

public:
    explicit BaseBeanModel(QObject *parent = 0);
	virtual ~BaseBeanModel();

	virtual QSharedPointer<BaseBean> bean(int row);
	virtual QSharedPointer<BaseBean> bean(const QModelIndex &index) = 0;
	virtual QSharedPointer<BaseBean> beanToBeEdited(int row);
	virtual QSharedPointer<BaseBean> beanToBeEdited(const QModelIndex &index);

	virtual BaseBeanMetadata * metadata() = 0;

	virtual void setCheckColumns(const QStringList &fieldNames);
	virtual QStringList checkColumns() const;

	QModelIndexList checkedItems();
	void setCheckedItems(QModelIndexList list, bool checked = true);
	void setCheckedItem(QModelIndex index, bool checked = true);
	void setCheckedItem(int row, bool checked = true);
	void checkAllItems(bool checked = true);

signals:

public slots:

};

#endif // BASEBEANMODEL_H
