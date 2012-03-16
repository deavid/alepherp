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
#ifndef DBOBJECT_H
#define DBOBJECT_H

#include <QObject>

class AbstractObserver;

/**
  Esta clase es la clase abstracta base para todos los objetos que interaccionan
  con base de datos, que son principalmente BaseBean, DBField y DBRelation
  @author David Pinelo <david.pinelo@alephsistemas.es>
  @see BaseBean
  @see DBField
  @see DBRelation
  */
class DBObject : public QObject
{
    Q_OBJECT

private:
	Q_DISABLE_COPY(DBObject)

protected:
	/** Cada entidad de base de datos tendrá su observador único. A ese observador, y a través
	  de la factoría ObserverFactory, podrán agregarse todos aquellos controles o widgets
	  que quieran presentar o modificar datos de este objeto de base de datos */
	AbstractObserver *m_observer;

public:
    explicit DBObject(QObject *parent = 0);
	virtual ~DBObject();

	virtual AbstractObserver *observer ();
	virtual void deleteObserver();

signals:

public slots:

};

#endif // DBOBJECT_H
