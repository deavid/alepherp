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
#include "menutreewidget.h"
#include <QMainWindow>
#include <QAction>
#include <QPushButton>

MenuTreeWidget::MenuTreeWidget(QWidget *parent) :
    QTreeWidget(parent)
{
	QFont font;

	font.setBold(true);
	// Buscamos la barra de menú si existiera
	QMenuBar *menu = menuBar();
	if ( menu == NULL ) {
		return;
	}
	QList<QMenu *> list = menu->findChildren<QMenu *> ();
	setColumnCount(1);
	setAnimated(true);
	foreach ( QMenu *m, list ) {
		if ( m->isVisible() ) {
			QTreeWidgetItem *item = new QTreeWidgetItem( (QTreeWidget*)0,
													   QStringList(m->title()));
			item->setBackgroundColor(0, Qt::lightGray);
			item->setTextAlignment(0, Qt::AlignCenter);
			item->setFont(0, font);
			insertTopLevelItem(0, item);
			// Ahora vamos recorriendo uno a uno los menus hijos
			addMenuChilds(m, item);
		}
	}
	connect(this, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(itemHasBeenClicked(QTreeWidgetItem*,int)));
}

QMenuBar * MenuTreeWidget::menuBar()
{
	QObject *temp = this;
	while ( temp != 0 ) {
		QMainWindow *dlg = qobject_cast<QMainWindow *>(temp);
		if ( dlg != 0 ) {
			return qobject_cast<QMenuBar *>(dlg->menuWidget());
		}
		temp = temp->parent();
	}
	return NULL;
}

QList<QTreeWidgetItem *> MenuTreeWidget::addMenuChilds(QMenu *menu, QTreeWidgetItem *parent)
{
	QList<QMenu *> listChilds = menu->findChildren<QMenu *>();
	QList<QTreeWidgetItem *> items;

	foreach ( QMenu *m, listChilds ) {
		QTreeWidgetItem * item = new QTreeWidgetItem (parent, QStringList(m->title()));
		items.append(item);
		QList<QMenu *> menuChilds = menu->findChildren<QMenu *> ();
		foreach ( QMenu *menuChild, menuChilds ) {
			addMenuChilds(menuChild, item);
		}
	}
	QList<QAction *> actions = menu->actions();
	foreach ( QAction *action, actions ) {
		if ( !action->isSeparator() && action->isVisible() ) {
			QVariant actionPtr = (qulonglong) action;
			QTreeWidgetItem  *item = new QTreeWidgetItem(parent, QStringList(action->text()));
			item->setIcon(0, action->icon());
			item->setDisabled(!action->isEnabled());
			item->setData(0, Qt::UserRole, actionPtr);
			items.append(item);
		}
	}
	return items;
}

void MenuTreeWidget::itemHasBeenClicked(QTreeWidgetItem *item, int column)
{
	Q_UNUSED(column)
	if ( item != NULL ) {
		QAction *action = (QAction *) (item->data(0, Qt::UserRole).toULongLong());
		if ( action != NULL ) {
			action->trigger();
		}
	}
}
