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
#ifndef TREEVIEWMODEL_H
#define TREEVIEWMODEL_H

#include <QAbstractItemModel>
#include <QVariant>
#include <QModelIndex>
#include <QString>
#include <QPixmap>
#include "treeitem.h"
#include "models/basebeanmodel.h"

class BaseBean;
class TreeViewModelPrivate;

/**
	@author David Pinelo <david.pinelo@alephsistemas.es>
*/
class TreeViewModel : public BaseBeanModel
{
	Q_OBJECT

protected:
	/** Item raiz. Borrándolo, se borrarán todos los items que dependen y cuelgan de él */
	TreeItem *m_rootItem;
	/** Lista con las imágenes a mostrar, si las hubiera. Se agrupan como lista. El orden en la lista
	  se corresponde con el nivel del item en el árbol. Además, tienen la siguiente terminología:
	  file:/home/david ... lee la imagen del sistema de ficherso
	  field:imagen ... lee la imagen de la base de datos, de la columna del mismo nombnre
	  */
	QStringList m_images;
	/** Tamaño de las imágenes a mostrar */
	QSize m_size;

public:
	TreeViewModel(QObject *parent = 0);
	~TreeViewModel();

	QVariant data(const QModelIndex &index, int role) const;
	Qt::ItemFlags flags(const QModelIndex &index) const;
	QVariant headerData(int section, Qt::Orientation orientation,
						int role = Qt::DisplayRole) const;
	QModelIndex index(int row, int column,
					const QModelIndex &parent = QModelIndex()) const;
	QModelIndex parent(const QModelIndex &index) const;
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;
	bool hasChildren ( const QModelIndex & parent = QModelIndex() ) const;
	bool loadedChildrens ( const QModelIndex &index );
	virtual void fetchMore(const QModelIndex &parent) = 0;
	virtual bool canFetchMore(const QModelIndex &parent) const = 0;

	void setImages(const QStringList &img);
	void setSize(const QSize &value);

	virtual TreeItem * item (const QModelIndex &index);
	virtual QSharedPointer<BaseBean> bean (const QModelIndex &index);
	virtual BaseBeanMetadata * metadata();
};

#endif
