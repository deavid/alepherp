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
#ifndef OBSERVERFACTORY_H
#define OBSERVERFACTORY_H

#include <QObject>
#include <QWidgetList>
#include <QSharedPointer>

class DBBaseWidget;
class DBObject;
class AbstractObserver;
class PERPBaseDialog;
class PERPScriptWidget;
class BaseBean;
class DBRelation;

#define OBSERVER_BASEBEAN	0
#define OBSERVER_DBFIELD	1
#define OBSERVER_DBRELATION 2

class ObserverFactory : public QObject
{
    Q_OBJECT

private:
	static QList<DBObject *> registeredObjects;

    AbstractObserver *installBaseBeanObserver(DBBaseWidget *widget, BaseBean *bean);
    AbstractObserver *installDBFieldObserver(DBBaseWidget *widget, BaseBean *bean);
    AbstractObserver *installDBRelationObserver(DBBaseWidget *widget, BaseBean *bean);

public:
    explicit ObserverFactory(QObject *parent = 0);

	static ObserverFactory *instance();

	AbstractObserver *newObserver(DBObject *entity);
    AbstractObserver *registerBaseWidget(DBBaseWidget *widget, BaseBean *bean);


signals:

public slots:

private slots:
};

/**
  Define un observador abstracto. El observador está entre la entidad que almacena el dato e interacciona
  con la base de datos (DBObject) y el objeto de visualización (QWidget). Esta entidad recibe las notificaciones
  de ambas y las intercambia.
  El observador debe saber qué DBObject puede interrelacionar con un widget. Para ello, debe almacenar
  una serie de datos que permita saber cuándo hay visualización. Esos campos son, tableName, dbFieldName,
  relationName, relationFieldName
  @author David Pinelo <david.pinelo@alephsistemas.es>
  @see DBObject
  @see ObserverFactory
  @see QWidget
  */
class AbstractObserver : public QObject
{
	Q_OBJECT
	Q_DISABLE_COPY(AbstractObserver)
protected:
	/** Lista de widgets que tiene que notificar */
	QObjectList m_viewWidgets;

protected:
	/** Entidad a la que se observa */
	QObject *m_entity;

public:
	explicit AbstractObserver(DBObject *m_entity);
	virtual ~AbstractObserver();

	QObject *entity();

	/*!
	  Realiza todas las conexiones necesarias entre el widget y el observador
	  para permitir la sincronización de datos. Esta función depende de cada observador,
	  por ello hay que implementarla en cada observador
	  */
	virtual void installWidget(QObject *widget) = 0;
	virtual void uninstallWidget(QObject *widget) = 0;

	QObjectList viewWidgets();

	virtual	bool readOnly() = 0;

protected slots:
	virtual void widgetToBeDestroyed(QObject *widget);
	virtual void observerToBeDestroyed();

public slots:
	virtual void sync() = 0;

signals:
};

#endif // OBSERVERFACTORY_H
