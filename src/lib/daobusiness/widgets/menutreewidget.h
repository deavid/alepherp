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
#ifndef MENUTREEWIDGET_H
#define MENUTREEWIDGET_H

#include <QTreeWidget>
#include <QMenuBar>
#include <alepherpglobal.h>

/*!
  Esta clase mostrará de forma jerárquica los menús de la aplicación en un QTreeWidget
  @author David Pinelo david.pinelo@alephsistemas.es
  */
class Q_ALEPHERP_EXPORT MenuTreeWidget : public QTreeWidget
{
    Q_OBJECT
private:
	QMenuBar *menuBar();
	QList<QTreeWidgetItem *> addMenuChilds(QMenu *menu, QTreeWidgetItem *parent);

public:
	explicit MenuTreeWidget(QWidget *parent = 0);

signals:

private slots:
	void itemHasBeenClicked(QTreeWidgetItem *item, int column);

public slots:

};

#endif // MENUTREEWIDGET_H
