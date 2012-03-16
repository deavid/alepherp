/***************************************************************************
 *   Copyright (C) 2007 by David Pinelo   *
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
#ifndef TREEITEM_H
#define TREEITEM_H

#define ITEM_FAMILIA		0
#define ITEM_SUBFAMILIA		1
#define ITEM_PAPEL			2
#define ITEM_SOBRE			3

#define ITEM_CATEGORIA		4
#define ITEM_ACTIVIDAD		5

#define ITEM_MATERIAL		6
#define ITEM_ALMACEN		7
#define ITEM_SUBTIPO		8

#include <QVariant>
#include <QList>
#include <QPixmap>
#include <QPixmapCache>
#include <QAbstractItemModel>

class BaseBean;
class TreeViewModel;
class TreeItemPrivate;

/**
	@author David Pinelo <david.pinelo@alephsistemas.es>
	Esta clase representa un Item de un TreeView y es parte del modelo. Contendrá 
	varias columnas de datos.

	The data is stored internally in the model using TreeItem objects that are linked
	together in a pointer-based tree structure. Generally, each TreeItem has a parent item,
	and can have a number of child items. However, the root item in the tree structure has
	no parent item and it is never referenced outside the model.
	Each TreeItem contains information about its place in the tree structure; it can return
	its parent item and its row number. Having this information readily available makes 
	implementing the model easier.
	The use of a pointer-based tree structure means that, when passing a model index to a
	view, we can record the address of the corresponding item in the index (see
	QAbstractItemModel::createIndex()) and retrieve it later with
	QModelIndex::internalPointer(). This makes writing the model easier and ensures that
	all model indexes that refer to the same item have the same internal data pointer.
	
	It is used to hold a list of QVariants, containing column data, and information about
	its position in the tree structure.
*/
class TreeItem
{
private:
	TreeItemPrivate *d;

public:
	TreeItem(const QSharedPointer<BaseBean> bean, const QString &fieldView, int level, TreeItem *parent = 0);
	~TreeItem();

	void appendChild(TreeItem *child);

	TreeItem *child(int row);
	int childCount() const;
	void setChildCount(int count);
	int columnCount() const;
	QVariant data(int column) const;
	QVariant data(const QString &dbColumn) const;
	int row() const;
	TreeItem *parent();

	void setFieldView(const QString &value);

	// Cada item tendrá asociado un bean contenedor de datos.
	QSharedPointer<BaseBean> bean();

	void setLevel ( int value );
	int level();
	void setToolTip ( const QString& theValue );
	QString toolTip() const;
	void setImagen ( const QString& theValue );
	QString imagen() const;
	QPixmap pixmap();
	QString fieldView();
	void setModel(TreeViewModel *model);

	void setLoadedChildrens ( bool theValue );
	bool loadedChildrens() const;

};
#endif
