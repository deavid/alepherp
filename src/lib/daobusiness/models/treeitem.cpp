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
#include "models/treeitem.h"
#include "models/treeviewmodel.h"
#include "dao/beans/basebean.h"
#include "dao/beans/beansfactory.h"
#include "dao/beans/dbfield.h"

class TreeItemPrivate
{
public:
	/** Columna del bean que mostrará este item, cuando se llame desde el model con el role DisplayRole */
	QString m_fieldView;
	QList<TreeItem*> m_childItems;
	QList<QVariant> m_itemData;
	QSharedPointer<BaseBean> m_bean;
	TreeItem *m_parentItem;
	/** Indica si los hijos de este nodo están cargados en memoria o no.
	NO indica si este nodo no tiene hijos */
	bool m_loadedChildrens;
	/** Lleva el número de hijos de este item. Es muy útil si no están cargados los hijos, pero se han
	  contado de base de datos */
	int m_childCount;
	QString m_toolTip;
	QString m_imagen;
	/** Nivel de anidación en el árbol */
	int m_level;
	/** Enlace al modelo */
	TreeViewModel *m_model;

	TreeItemPrivate() {}
};

/*!
	The constructor is only used to record the item's parent and the data associated 
	with each column.
	It is up to the constructor to create a root item for the model. This item only 
	contains vertical header data for convenience. We also use it to reference the 
	internal data structure that contains the model data, and it is used to represent 
	an imaginary parent of top-level items in the model.
 */
TreeItem::TreeItem(const QSharedPointer<BaseBean> bean, const QString &fieldView, int level, TreeItem *parent) : d(new TreeItemPrivate)
{
	d->m_childCount = 0;
	d->m_parentItem = parent;
	d->m_fieldView = fieldView;
	d->m_toolTip = "";
	d->m_imagen = "";
	d->m_loadedChildrens = false;
	d->m_level = level;
	d->m_model = NULL;
	if ( !bean.isNull() ) {
		d->m_bean = bean;
	}
}

/*!
	A pointer to each of the child items belonging to this item will be stored in the 
	childItems private member variable. When the class's destructor is called, 
	it must delete each of these to ensure that their memory is reused:
*/
TreeItem::~TreeItem()
{
	qDeleteAll(d->m_childItems);
	delete d;
}

/*!
	Este método se utiliza sólo cuando se añaden datos al modelo, es decir, cuando se está construyendo
	o consultando nodos por primera vez. No es usado normalmente.
*/
void TreeItem::appendChild(TreeItem *item)
{
	d->m_childItems.append(item);
}

/*!
	The child() and childCount() functions allow the model to obtain information about any child items.
*/
TreeItem *TreeItem::child(int row)
{
	return d->m_childItems.value(row);
}

void TreeItem::setFieldView(const QString &value)
{
	d->m_fieldView = value;
}

int TreeItem::childCount() const
{
	if ( d->m_loadedChildrens ) {
		return d->m_childItems.count();
	}
	return d->m_childCount;
}

void TreeItem::setChildCount(int count)
{
	d->m_childCount = count;
}

/*!
	The row() and parent() functions are used to obtain the item's row number and parent item.
*/
int TreeItem::row() const
{
	if ( d->m_parentItem != NULL ) {
		return ( d->m_parentItem->d->m_childItems.indexOf(const_cast<TreeItem*>(this)) );
	}

	return 0;
}

/*!
	Information about the number of columns associated with the item is provided by 
	columnCount(), and the data in each column can be obtained with the data() function.
*/
int TreeItem::columnCount() const
{
	if ( !d->m_bean.isNull() ) {
		return d->m_bean->fields().count();
	} else {
		return 1;
	}
}

/*!
  El dato que se devuelve es el del field correspondiente al bean
  */
QVariant TreeItem::data(int dbColumn) const
{
	if ( d->m_bean != NULL ) {
		return d->m_bean->fieldValue(dbColumn);
	} else {
		return d->m_fieldView;
	}
}

/*!
  El dato que se devuelve es el del field correspondiente al bean
  */
QVariant TreeItem::data(const QString &dbColumn) const
{
	if ( d->m_bean != NULL ) {
		return d->m_bean->fieldValue(dbColumn);
	} else {
		return d->m_fieldView;
	}
}

/*!
  Devuelve el campo que se mostrará cuando se llame a este item con DisplayRole
  */
QString TreeItem::fieldView()
{
	return d->m_fieldView;
}

TreeItem *TreeItem::parent()
{
	return d->m_parentItem;
}

QSharedPointer<BaseBean>  TreeItem::bean()
{
	return d->m_bean;
}

QString TreeItem::toolTip() const
{
	return d->m_toolTip;
}


void TreeItem::setToolTip ( const QString& theValue )
{
	d->m_toolTip = theValue;
}

void TreeItem::setLevel ( int value )
{
	d->m_level = value;
}

int TreeItem::level()
{
	return d->m_level;
}


QString TreeItem::imagen() const
{
	return d->m_imagen;
}


void TreeItem::setImagen ( const QString& theValue )
{
	d->m_imagen = theValue;
}

QPixmap TreeItem::pixmap()
{
	QPixmap pix;
	if ( !d->m_imagen.isEmpty() &&
			 !QPixmapCache::find( d->m_imagen.left( 100 ), pix ) ) {
		pix.loadFromData( d->m_imagen.toUtf8() );
		QPixmapCache::insert( d->m_imagen.left( 100 ), pix );
	}
	return pix;
}


bool TreeItem::loadedChildrens() const
{
	return d->m_loadedChildrens;
}


void TreeItem::setLoadedChildrens ( bool theValue )
{
	d->m_loadedChildrens = theValue;
}

void TreeItem::setModel(TreeViewModel *model)
{
	d->m_model = model;
}
