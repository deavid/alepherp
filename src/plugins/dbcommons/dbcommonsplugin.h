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
#ifndef DBCOMMONSPLUGIN_H
#define DBCOMMONSPLUGIN_H

#include <QObject>
#include <QDesignerCustomWidgetCollectionInterface>

class DBCommonsPlugin : public QObject, public QDesignerCustomWidgetCollectionInterface
{
    Q_OBJECT
	Q_INTERFACES(QDesignerCustomWidgetCollectionInterface)

public:
    explicit DBCommonsPlugin(QObject *parent = 0);

	virtual QList<QDesignerCustomWidgetInterface*> customWidgets() const;

signals:

public slots:

private:
	 QList<QDesignerCustomWidgetInterface*> m_widgets;
};

#endif // DBCOMMONSPLUGIN_H
