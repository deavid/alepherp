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
#ifndef TREEBASEBEANMODEL_H
#define TREEBASEBEANMODEL_H

#include <QString>
#include <QStringList>
#include <QHash>
#include "models/treeviewmodel.h"

class BaseBean;
class DBRelation;
class TreeBaseBeanModelPrivate;

class TreeBaseBeanModel : public TreeViewModel
{
	Q_OBJECT
	Q_PROPERTY (bool baseBeanModel READ baseBeanModel)

protected:
	TreeBaseBeanModelPrivate *d;
	Q_DECLARE_PRIVATE(TreeBaseBeanModel)

	void setupInitialData();
	void addChilds(TreeItem *parent);
	void searchForItem(const QSharedPointer<BaseBean> &bean, TreeItem *parent, TreeItem **item, bool &found);
	void searchForItem(const QVariant &pk, TreeItem *parent, TreeItem **item, bool &found);

public:
	explicit TreeBaseBeanModel(QObject *parent = 0);
	explicit TreeBaseBeanModel(const QStringList &tableNames, const QString &rootName,
							   const QStringList &fieldsView,
							   const QStringList &filterLevels, QObject *parent = 0);
	~TreeBaseBeanModel();

	bool baseBeanModel() { return true; }
	void setTableNames(const QStringList &tableNames);
	void setFieldsView(const QStringList &fieldsView);
	void setFilterLevels(const QStringList &filterLevels);
	void setViewIntermediateNodesWithoutChilds(bool value);

	void fetchMore(const QModelIndex &parent);
	bool canFetchMore(const QModelIndex &parent) const;
	bool hasChildren ( const QModelIndex & parent ) const;

	QModelIndex index(const QSharedPointer<BaseBean> &bean);
	QModelIndex indexByPk(const QVariant &pk);

	QList<QModelIndex> firstLevelIndexes();
	void reload();

	virtual QString toolTip(const QSharedPointer<BaseBean> &bean);
};

#endif // TREEBASEBEANMODEL_H
