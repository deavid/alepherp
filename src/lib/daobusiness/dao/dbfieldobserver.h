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
#ifndef DBFIELDOBSERVER_H
#define DBFIELDOBSERVER_H

#include <QObject>
#include <QWidgetList>
#include <QMultiHash>
#include <QSharedPointer>
#include "observerfactory.h"

class DBObject;
class DBBaseWidget;
class BaseBean;

/**
  Patrón observer para la intercomunicación DBCommonEntity (DBField o DBRelation)
  y los widgets que presentan estos datos
  @author David Pinelo <david.pinelo@alephsistemas.es>
  @see DBField
  @see DBRelation
  @see DBBaseWidget
  */
class DBFieldObserver : public AbstractObserver
{
    Q_OBJECT
	Q_DISABLE_COPY(DBFieldObserver)

	friend class ObserverFactory;

private:
	/** Constructor privado, para que sólo la factoría tenga acceso */
	explicit DBFieldObserver(DBObject *entity);

public:
	virtual ~DBFieldObserver();

	bool readOnly();

	void installWidget(QObject *widget);

	int maxLength();
	int partD();

signals:
	void entityValueModified (const QVariant &value);
	void widgetValueModified (const QVariant &value);

public slots:
	void sync();
	void uninstallWidget(QObject *widget);

private slots:
	void widgetEdited(const QVariant &);
};

#endif // DBFIELDOBSERVER_H
