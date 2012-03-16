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
#include "models/treeviewmodel.h"
#include "dao/beans/basebean.h"
#include "dao/beans/beansfactory.h"
#include "dao/beans/basebeanmetadata.h"
#include <QDebug>

/*!
	The constructor is only used to record the item's parent and the data associated with each column.
	For simplicity, the model does not allow its data to be edited. As a result, 
	the constructor takes an argument containing the data that the model will share 
	with views and delegates 
*/
TreeViewModel::TreeViewModel(QObject *parent) : BaseBeanModel(parent)
{
}

/*!
	A pointer to each of the child items belonging to this item will be stored in the 
	childItems private member variable. When the class's destructor is called, 
	it must delete each of these to ensure that their memory is reused
 */
TreeViewModel::~TreeViewModel()
{
}

/*!
    Los modelos deben implementar una funcion index() que ser la encargada de proveer los 
    index (que son los elementos utilizados para la comunicacin con las vistas) para
    que las vistas y los objetos delegates puedan acceder a los datos.
    
	Models must implement an index() function to provide indexes for views and 
	delegates to use when accessing data. Indexes are created for other components 
	when they are referenced by their row and column numbers, and their parent 
	model index. If an invalid model index is specified as the parent, it is up 
	to the model to return an index that corresponds to a top-level item in the model.
	When supplied with a model index, we first check whether it is valid. If it is not, 
	we assume that a top-level item is being referred to; otherwise, we obtain the 
	data pointer from the model index with its internalPointer() function and use it 
	to reference a TreeItem object. Note that all the model indexes that we construct 
	will contain a pointer to an existing TreeItem, so we can guarantee that any valid 
	model indexes that we receive will contain a valid data pointer.
*/
QModelIndex TreeViewModel::index(int row, int column, const QModelIndex &parent)
		 const
{
	TreeItem *parentItem;
	TreeItem *childItem;
	
	if (!hasIndex(row, column, parent))
		return QModelIndex();

	if (!parent.isValid())
		parentItem = m_rootItem;
	else
		parentItem = static_cast<TreeItem*>(parent.internalPointer());

	childItem = parentItem->child(row);
	
	if ( childItem )
		return createIndex(row, column, childItem);
	else
		return QModelIndex();
}


QModelIndex TreeViewModel::parent(const QModelIndex &index) const
{
	if (!index.isValid())
		return QModelIndex();

	TreeItem *childItem = static_cast<TreeItem*>(index.internalPointer());
	TreeItem *parentItem = childItem->parent();

	if ( parentItem == m_rootItem )
		return QModelIndex();

	if ( parentItem != NULL ) {
		return createIndex(parentItem->row(), 0, parentItem);
	} else {
		return QModelIndex();
	}
}

int TreeViewModel::rowCount(const QModelIndex &parent) const
{
	TreeItem *parentItem;
	if (parent.column() > 0)
		return 0;

	if (!parent.isValid())
		parentItem = m_rootItem;
	else
		parentItem = static_cast<TreeItem*>(parent.internalPointer());

	return parentItem->childCount();
}

int TreeViewModel::columnCount(const QModelIndex &parent) const
{
	if (parent.isValid())
		return static_cast<TreeItem*>(parent.internalPointer())->columnCount();
	else
		return m_rootItem->columnCount();
}

/*!
	Data is obtained from the model via data(). Since the item manages its own columns, 
	we need to use the column number to retrieve the data with the 
	TreeItem::data() function:
*/
QVariant TreeViewModel::data(const QModelIndex &index, int role) const
{
	QVariant devolver;
	
	if ( !index.isValid() )
		return QVariant();
	
	TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
	QSharedPointer<BaseBean> bean = item->bean();
	if ( item != NULL ) {
		switch ( role ) {
			case Qt::DisplayRole:
				devolver = item->data(item->fieldView());
				break;
			
			case Qt::DecorationRole:
				if ( m_images.size() != 0 && m_images.size() > item->level() ) {
					QString temp = m_images.at(item->level());
					QStringList img = temp.split(":");
					QPixmap pixmap;
					if ( img.at(0) == "field" ) {
						QByteArray imgData = bean->fieldValue(img.at(1)).toByteArray();
						pixmap = QPixmap(imgData.constData());
					} else if ( img.at(0) == "file" ) {
						pixmap = QPixmap(QString(":%1").arg(img.at(1)));
					} else if ( !item->pixmap().isNull() ) {
						pixmap = item->pixmap();
					}
					if ( pixmap.isNull() ) {
						devolver = QVariant();
					} else if ( !m_size.isEmpty() ){
						pixmap = pixmap.scaled(m_size);
						devolver = pixmap;
					}
				}
				break;
			
			case Qt::ToolTipRole:
				devolver = item->toolTip();
				break;
			
			case Qt::UserRole:
				devolver = item->data(index.column());
				break;

			default:
				devolver = QVariant();
				break;
		}
	} else {
		devolver = QVariant();
	}
	
	return devolver;
}

Qt::ItemFlags TreeViewModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return 0;

	return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}


QVariant TreeViewModel::headerData(int section, Qt::Orientation orientation,
							   int role) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
		return m_rootItem->data(section);

	return QVariant();
}


QSharedPointer<BaseBean> TreeViewModel::bean(const QModelIndex & index)
{
	if (!index.isValid())
		return QSharedPointer<BaseBean>();

	TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
	return item->bean();
}

/** Aquí no hay un masterBean definido... hay un montón */
BaseBeanMetadata * TreeViewModel::metadata()
{
	return NULL;
}

bool TreeViewModel::hasChildren(const QModelIndex & parent) const
{
	TreeItem *parentItem;

	if (!parent.isValid())
		parentItem = m_rootItem;
	else
		parentItem = static_cast<TreeItem*>(parent.internalPointer());

	if ( parentItem->childCount() == 0 && parentItem->loadedChildrens() )
		return false;
	else
		return true;
}

bool TreeViewModel::loadedChildrens ( const QModelIndex &index )
{
	TreeItem *item;

	if (!index.isValid()) {
		return false;
	}
	
	item = static_cast<TreeItem *>(index.internalPointer());
	return item->loadedChildrens();
}

void TreeViewModel::setImages(const QStringList &img)
{
	m_images = img;
}

void TreeViewModel::setSize(const QSize &value)
{
	m_size = value;
}

TreeItem * TreeViewModel::item (const QModelIndex &index)
{
	TreeItem *item = static_cast<TreeItem *>(index.internalPointer());
	return item;
}

