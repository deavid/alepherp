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
#include "treebasebeanmodel.h"
#include "models/treeitem.h"
#include "models/treeviewmodel.h"
#include "models/treebasebeanmodel_p.h"
#include "dao/beans/basebean.h"
#include "dao/beans/dbrelation.h"
#include "dao/beans/dbfield.h"
#include "dao/basedao.h"


TreeBaseBeanModel::TreeBaseBeanModel(QObject *parent) : TreeViewModel(parent), d(new TreeBaseBeanModelPrivate)
{
	m_rootItem = new TreeItem(QSharedPointer<BaseBean>(), "", 0);
}

TreeBaseBeanModel::TreeBaseBeanModel(const QStringList &tableNames, const QString &rootName, const QStringList &fieldsView,
									 const QStringList &filterLevels, QObject *parent)
										 : TreeViewModel(parent), d(new TreeBaseBeanModelPrivate)
{
	d->m_tableNames = tableNames;
	d->m_fieldsView = fieldsView;
	d->m_filterLevels = filterLevels;
	d->m_rootName = rootName;
	m_rootItem = new TreeItem(QSharedPointer<BaseBean>(), d->m_rootName, 0);
	setupInitialData();
}

TreeBaseBeanModel::~TreeBaseBeanModel()
{
	delete m_rootItem;
	delete d;
}

void TreeBaseBeanModel::setTableNames(const QStringList &tableNames)
{
	d->m_tableNames = tableNames;
}

void TreeBaseBeanModel::setFieldsView(const QStringList &fieldsView)
{
	d->m_fieldsView = fieldsView;
}

void TreeBaseBeanModel::setFilterLevels(const QStringList &filterLevels)
{
	d->m_filterLevels = filterLevels;
}

void TreeBaseBeanModel::setViewIntermediateNodesWithoutChilds(bool value)
{
	d->m_viewIntermediateNodesWithoutChilds = value;
}

/*!
  Introduce datos iniciales en el tree: Estos datos iniciales son el primer y segundo nivel
  */
void TreeBaseBeanModel::setupInitialData()
{
	TreeItem *item;
	BaseBeanPointerList list;
	bool result;
	QString firstFilter;

	if ( d->m_filterLevels.size() > 0 ) {
		firstFilter = d->m_filterLevels.at(0);
	}
	result = BaseDAO::select(list, d->m_tableNames.at(0), firstFilter);
	if ( result ) {
		m_rootItem->setChildCount(list.size());
		m_rootItem->setLoadedChildrens(true);
		foreach ( QSharedPointer<BaseBean> bean, list ) {
			// Los hijos se obtienen a través de la relación que exista entre m_tableNames.at(0) u m_tableNames.at(1)
			DBRelation *rel = bean->relation(d->m_tableNames.at(1));
			QString filter;
			if ( d->m_filterLevels.size() > 1 ) {
				filter = d->m_filterLevels.at(1);
			}
			if ( rel != NULL ) {
				rel->setFilter(filter);
				int count = rel->childsCount();
				if ( count > 0 ) {
					// Poniendo como padre de este item a m_rootItem, garantizamos que cuando se borre
					// m_rootItem se borran sus hijos (en cascada)
					// Root item tiene nivel 0. Estos beans ya tienen nivel 1
					item = new TreeItem(bean, d->m_fieldsView.at(0), 1, m_rootItem);
					item->setLoadedChildrens(false);
					item->setChildCount(count);
					m_rootItem->appendChild(item);
				}
			}
		}
		m_rootItem->setLoadedChildrens(true);
	}
}

void TreeBaseBeanModel::addChilds(TreeItem *parent)
{
	TreeItem *item;
	int newLevel = parent->level() + 1;

	QSharedPointer<BaseBean> beanParent = parent->bean();
	if ( beanParent != NULL && !parent->loadedChildrens() ) {
		DBRelation *rel = beanParent->relation(d->m_tableNames.at(parent->level()));
		if ( rel != NULL ) {
			QString filter;
			if ( d->m_filterLevels.size() > parent->level() ) {
				filter = d->m_filterLevels.at(parent->level());
			}
			rel->setFilter(filter);
			BaseBeanPointerList hijos = rel->childs();
			foreach ( QSharedPointer<BaseBean> beanChild, hijos ) {
				// Obtenemos el número de hijos de este hijo
				int childCount = 0;
				if ( d->m_tableNames.size() > newLevel ) {
					DBRelation *childRel = beanChild->relation(d->m_tableNames.at(newLevel));
					if ( childRel != NULL ) {
						childCount = childRel->childsCount();
					}
				}
				if ( !d->m_viewIntermediateNodesWithoutChilds && childCount == 0 && newLevel < d->m_tableNames.size() ) {
				} else {
					item = new TreeItem(beanChild, d->m_fieldsView.at(parent->level()), newLevel, parent);
					if ( childCount == 0 ) {
						item->setLoadedChildrens(true);
					} else {
						item->setLoadedChildrens(false);
					}
					item->setChildCount(childCount);
					item->setToolTip(toolTip(beanChild));
					parent->appendChild(item);
				}
			}
		}
		parent->setLoadedChildrens(true);
	}
}

QString TreeBaseBeanModel::toolTip(const QSharedPointer<BaseBean> &bean)
{
	Q_UNUSED(bean)
	return QString("");
}

void TreeBaseBeanModel::fetchMore(const QModelIndex &parent)
{
	// En función del tipo de padre, haremos una cosa u otra
	if ( parent.isValid() ) {
		TreeItem *item = static_cast<TreeItem*>(parent.internalPointer());
		if ( item == NULL ) {
			return;
		}
		// Estamos en la hoja del árbol? Si no lo estamos podemos agregar hijos
		if ( item->level() < d->m_tableNames.size() ) {
			addChilds(item);
		}
	}
}

bool TreeBaseBeanModel::canFetchMore(const QModelIndex &parent) const
{
	bool resultado = false;
	QString valor;

	if ( parent.isValid() ) {
		TreeItem *item = static_cast<TreeItem*>(parent.internalPointer());
		if ( item == NULL ) {
			return false;
		}
		// Estamos en la hoja del árbol? Si no lo estamos podemos agregar hijos
		if ( item->level() < d->m_tableNames.size() ) {
			resultado = true;
		} else {
			resultado = false;
		}
	}

	return resultado;
}

/*!
	Está implementado en TreeViewModel, así que lo implementamos aquí al tener el fetchMore.
 */
bool TreeBaseBeanModel::hasChildren ( const QModelIndex & parent ) const
{
	bool resultado = false;

	if ( !parent.isValid() ) {
		return TreeViewModel::hasChildren( parent );
	}

	if ( parent.isValid() ) {
		TreeItem *item = static_cast<TreeItem*>(parent.internalPointer());
		if ( item == NULL ) {
			return false;
		}

		// Estamos en la hoja del árbol? Si no lo estamos podemos agregar hijos
		if ( item->level() == d->m_tableNames.size() ) {
			return false;
		}

		if ( item->childCount() > 0 ) {
			resultado = true;
		} else {
			resultado = false;
		}
	}

	return resultado;
}

/*!
	Recarga todos los datos.
 */
void TreeBaseBeanModel::reload()
{
	emit QAbstractItemModel::layoutAboutToBeChanged();
	delete m_rootItem;
	m_rootItem = new TreeItem(QSharedPointer<BaseBean>(), d->m_rootName, 0);
	setupInitialData();
	emit QAbstractItemModel::layoutChanged();
}

/*!
	Busca recursivamente en el arbol por un item que contenga el bean dado. Si se encuentra
	pone found a true
 */
void TreeBaseBeanModel::searchForItem(const QSharedPointer<BaseBean> &bean, TreeItem *parent, TreeItem **item, bool &found)
{
	if ( found ) {
		return;
	}
	for ( int i = 0 ; i < parent->childCount() ; i++ ) {
		if ( parent->child(i)->bean()->pkValue().toMap() != bean->pkValue().toMap() ) {
			if ( !parent->child(i)->loadedChildrens() ) {
				QModelIndex index = createIndex(parent->child(i)->row(), 0, parent->child(i));
				if ( canFetchMore(index) ) {
					fetchMore(index);
				}
			}
			searchForItem(bean, parent->child(i), item, found);
		} else {
			found = true;
			*item = parent->child(i);
		}
	}
}

void TreeBaseBeanModel::searchForItem(const QVariant &pk, TreeItem *parent, TreeItem **item, bool &found)
{
	if ( found ) {
		return;
	}
	for ( int i = 0 ; i < parent->childCount() ; i++ ) {
		if ( parent->child(i)->bean()->pkValue().toMap() != pk.toMap() ) {
			if ( !parent->child(i)->loadedChildrens() ) {
				QModelIndex index = createIndex(parent->child(i)->row(), 0, parent->child(i));
				if ( canFetchMore(index) ) {
					fetchMore(index);
				}
			}
			searchForItem(pk, parent->child(i), item, found);
		} else {
			found = true;
			*item = parent->child(i);
		}
	}
}

/*!
	Devuelve el ndice del modelo para el bean dado.
 */
QModelIndex TreeBaseBeanModel::index(const QSharedPointer<BaseBean> &bean)
{
	// Primero vamos a determinar en qu nivel se encuentra el objeto
	QModelIndex indice;
	bool found = false;
	TreeItem *item;

	searchForItem(bean, m_rootItem, &item, found);

	if ( found ) {
		indice = createIndex( item->row(), 0, item->parent() );
	}

	return indice;
}

QModelIndex TreeBaseBeanModel::indexByPk(const QVariant &pk)
{
	// Primero vamos a determinar en qu nivel se encuentra el objeto
	QModelIndex indice;
	bool found = false;
	TreeItem *item;

	searchForItem(pk, m_rootItem, &item, found);

	if ( found ) {
		indice = createIndex( item->row(), 0, item->parent() );
	}

	return indice;
}

/**
	Devuelve indices de todas el pimer nivel del model. Util cuando se quiere solo expandir
	ese primer nivel
 */
QList<QModelIndex> TreeBaseBeanModel::firstLevelIndexes()
{
	TreeItem *item;
	QList<QModelIndex> list;
	QModelIndex indice;

	for ( int i = 0 ; i < m_rootItem->childCount() ; i++ ) {
		item = m_rootItem->child(i);
		if ( item != NULL ) {
			indice = createIndex(item->row(), 0, item);
			list << indice;
		}
	}
	return list;
}

