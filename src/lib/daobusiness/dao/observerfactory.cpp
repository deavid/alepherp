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
#include "observerfactory.h"
#include "configuracion.h"
#include "dao/basebeanobserver.h"
#include "dao/dbfieldobserver.h"
#include "dao/dbrelationobserver.h"
#include "dao/dbobject.h"
#include "dao/beans/basebean.h"
#include "dao/beans/dbfield.h"
#include "dao/beans/dbrelation.h"
#include "dao/beans/dbrelationmetadata.h"
#include "dao/beans/basebeanmetadata.h"
#include "widgets/dbbasewidget.h"
#include "forms/perpbasedialog.h"
#include <QCoreApplication>

QList<DBObject *> ObserverFactory::registeredObjects;

ObserverFactory::ObserverFactory(QObject *parent) :
    QObject(parent)
{
}

ObserverFactory * ObserverFactory::instance()
{
	static ObserverFactory* singleton = 0;
	if ( singleton == 0 ) {
		singleton = new ObserverFactory();
		// Esto garantiza que el objeto se borra al cerrar la aplicación
		singleton->setParent(qApp);
	}
	return singleton;
}

/*!
  Devuelve el observador adecuado para una entidad. Este observador le permitirá
  visualizar los datos en la UI si se ha definido así. Para devolver el observador, debe
  examinar las propiedades del entity y crearlo teniendo en cuenta esas propuedades.
  Un observador sólo tendrá un único entity y cero o varios widgets.
  */
AbstractObserver *ObserverFactory::newObserver(DBObject *entity)
{
	AbstractObserver *obj = NULL;
	QString className (entity->metaObject()->className());
	if ( className == "BaseBean" ) {
		obj = new BaseBeanObserver(entity);
	} else if ( className == "DBField" ) {
		obj = new DBFieldObserver(entity);
	} else if ( className == "DBRelation" ) {
		obj = new DBRelationObserver(entity);
	}
	return obj;
}

/*!
  Esta función será llamado por los widgets para saber qué observador le corresponde.
  Para ello se creará o buscará el observer adecuado dentro de bean
  */
AbstractObserver * ObserverFactory::registerBaseWidget(DBBaseWidget *baseWidget, BaseBean *bean)
{
	switch ( baseWidget->observerType() ) {
	case OBSERVER_DBFIELD:
		return installDBFieldObserver(baseWidget, bean);
	case OBSERVER_DBRELATION:
		return installDBRelationObserver(baseWidget, bean);
    case OBSERVER_BASEBEAN:
        return installBaseBeanObserver(baseWidget, bean);
    }
    return NULL;
}

AbstractObserver *ObserverFactory::installBaseBeanObserver(DBBaseWidget *baseWidget, BaseBean *bean)
{
    AbstractObserver *observer = NULL;
    QWidget *widget = dynamic_cast<QWidget *>(baseWidget);
    if ( widget == 0 ) {
        return NULL;
    }

    if ( bean != NULL && bean->dbState() != BaseBean::TO_BE_DELETED ) {
        observer = bean->observer();
        observer->installWidget(widget);
        return observer;
    }
    return NULL;
}

AbstractObserver *ObserverFactory::installDBFieldObserver(DBBaseWidget *baseWidget, BaseBean *bean)
{
    AbstractObserver *observer = NULL;
	DBField *fld;
	QWidget *widget = dynamic_cast<QWidget *>(baseWidget);
	if ( widget == 0 ) {
		return NULL;
	}

	if ( bean != NULL && bean->dbState() != BaseBean::TO_BE_DELETED ) {
		fld = NULL;
		QStringList path = baseWidget->fieldName().split(".");
		if ( path.size() == 1 ) {
			fld = bean->field(baseWidget->fieldName());
		} else {
			fld = qobject_cast<DBField *>( bean->navigateThrough (baseWidget->fieldName(), baseWidget->relationFilter()) );
		}
		if ( fld != NULL ) {
			observer = fld->observer();
			observer->installWidget(widget);
			return observer;
		}
	}
	return NULL;
}

/*!
  Determina la relación que se devuelve según la configuración de un base widget.
  Se mira para determinarlo lo que aparezca en relationName y relationFilter. Así es posible
  tener:
  relationName = presupuestos_cabecera.presupuestos_ejemplares.presupuestos_actividades
  relationFilter = id_presupuesto = primaryKeyIdPresupuesto;id=pkNumEjemplares;id_categoria=2
  Se debe por tanto pasar las primary key de las primeras tablas.
  */
AbstractObserver *ObserverFactory::installDBRelationObserver(DBBaseWidget *baseWidget, BaseBean *bean)
{
	AbstractObserver *observer = NULL;
	DBRelation *rel;
	QWidget *widget = dynamic_cast<QWidget *>(baseWidget);
	if ( widget == 0 ) {
		return NULL;
	}

	if ( bean != NULL && bean->dbState() != BaseBean::TO_BE_DELETED ) {
		// El relationName puede venir en la forma: presupuestos_ejemplares.presupuestos_actividades ...
		// En ese caso, debemos iterar hasta dar con la relación adecuada. Esa iteración se realiza en
		// combinación con el filtro establecido. El filtro en ese caso debe mostrar primero
		// la primary key de presupuestos_ejemplares, y después el filtro que determina presupuestos_actividades
		QStringList relations = baseWidget->relationName().split(".");
		if ( relations.size() == 1 ) {
			rel = bean->relation(baseWidget->relationName());
		} else {
			rel = qobject_cast<DBRelation *> (bean->navigateThrough(baseWidget->relationName(), baseWidget->relationFilter()));
		}
		if ( rel != NULL ) {
			observer = rel->observer();
		}
	}
	if ( observer != NULL ) {
		observer->installWidget(widget);
	}
	return observer;
}

AbstractObserver::AbstractObserver(DBObject *entity) : QObject(entity)
{
	m_entity = entity;
	setObjectName(QString("%1").arg(configuracion.uniqueId()));
}

AbstractObserver::~AbstractObserver()
{
	foreach(QObject *widget, m_viewWidgets) {
		if ( widget->property("perpControl").toBool() ) {
			DBBaseWidget *baseWidget = dynamic_cast<DBBaseWidget *>(widget);
			baseWidget->observerUnregistered();
		}
	}
}

QObject * AbstractObserver::entity()
{
	return m_entity;
}

QObjectList AbstractObserver::viewWidgets()
{
	return m_viewWidgets;
}

/*!
  Este slot se conecta automáticamente a la destrucción de un widget. Elimina el widget
  de la lista de objetos observados por el observador, y procede al unregister del mismo (esto
  es, las desconexiones)
  */
void AbstractObserver::widgetToBeDestroyed(QObject *widget)
{
	int index = m_viewWidgets.indexOf(widget);
	if ( index != -1 ) {
		m_viewWidgets.removeAt(index);
	}
	if ( widget != NULL ) {
		uninstallWidget(widget);
	}
}

/*!
  Este slot se conecta automáticamente a la destrucción de una entidad. Forzara por tanto
  a los widgets hijos a solicitar una nueva entidad a la que conectarse
  */
void AbstractObserver::observerToBeDestroyed()
{
    foreach ( QObject *obj, m_viewWidgets ) {
        if ( obj->property("perpControl").toBool() ) {
            DBBaseWidget *baseWidget = dynamic_cast<DBBaseWidget *>(obj);
            baseWidget->observerUnregistered();
        }
    }
}
