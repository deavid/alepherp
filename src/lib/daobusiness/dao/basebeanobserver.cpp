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
#include "basebeanobserver.h"
#include "dao/beans/basebean.h"
#include "dao/beans/dbfield.h"
#include "dao/beans/dbrelation.h"
#include "dao/dbfieldobserver.h"
#include "dao/beans/basebeanvalidator.h"
#include "dao/beans/basebeanmetadata.h"
#include "widgets/dbbasewidget.h"
#include <QCoreApplication>

class BaseBeanObserverPrivate
{
public:
	BaseBeanValidator *m_validator;

	BaseBeanObserverPrivate() {
		m_validator = NULL;
	}
};

BaseBeanObserver::BaseBeanObserver(DBObject *entity) :
	AbstractObserver(entity), d(new BaseBeanObserverPrivate)
{
	BaseBean *bean = qobject_cast<BaseBean *>(entity);
	if ( bean != 0 ) {
		d->m_validator = new BaseBeanValidator(this);
		d->m_validator->setBean(bean);
		connect(bean, SIGNAL(beanModified(bool)), this, SLOT(setWindowModified(bool)));
	}
}

BaseBeanObserver::~BaseBeanObserver()
{
	observerToBeDestroyed();
	delete d;
}

/*!
  Se informa a los widgets que muestran los datos de este bean que los datos han sido modificados
  */
void BaseBeanObserver::setWindowModified(bool value)
{
	Q_UNUSED(value)
	foreach (QObject *widget, m_viewWidgets) {
		// No se llama a setWindowModified, ya que esta función no es virtual, y no estaríamos
		// llamando al setWindowModified del hijo del widget (y no ejecutaría nuestro código propio en setWindowModified).
		// Llamamos a la propiedad y lo solucionamos
		if ( widget->property("windowModified").isValid() ) {
			widget->setProperty("windowModified", value);
		}
	}
}

bool BaseBeanObserver::readOnly()
{
	BaseBean *bean = qobject_cast<BaseBean *> (entity());
	if ( bean == NULL ) {
		return true;
	}
	QString tableName = bean->metadata()->tableName();
	QHash<QString, QVariant> permissions = qApp->property("permissions").toHash();
	QString p = permissions.value(tableName).toString();
	if ( !p.contains("w") ) {
		return true;
	}
	return false;
}

/*!
  Realiza todas las conexiones necesarias entre el widget y el observador
  para permitir la sincronización de datos. Esta función depende de cada observador,
  por ello hay que implementarla en cada observador
  */
void BaseBeanObserver::installWidget(QObject *widget)
{
	m_viewWidgets << widget;
	connect (widget, SIGNAL(destroyed(QObject*)), this, SLOT(widgetToBeDestroyed(QObject*)));
}

/*!
  Desinstala un widget de este observador. Como este observador es el de un base bean
  se encargará de desinstalar a su vez los observadores hijos de este observador
  (los observadores de los DBField, DBRelation...)
  */
void BaseBeanObserver::uninstallWidget(QObject *widget)
{
	int index = m_viewWidgets.indexOf(widget);
	BaseBean *bean = qobject_cast<BaseBean *> (entity());
	if ( index != -1 ) {
		m_viewWidgets.removeAt(index);
		disconnect (widget, SIGNAL(destroyed(QObject*)), this, SLOT(widgetToBeDestroyed(QObject*)));
    }
	// Al desinstalar el formulario, debemos desinstalar todos los controles de ese form que pudieran estar asociados
	// TODO También habría que hacerlo para todos los hijos de las relaciones
	QList<QWidget *> childs = widget->findChildren<QWidget *>();
	foreach ( QWidget *childWidget, childs ) {
		if ( childWidget->property("perpControl").toBool() ) {
			DBBaseWidget *baseWidget = dynamic_cast<DBBaseWidget *> (childWidget);
			DBObject *obj = bean->navigateThrough(baseWidget->fieldName(), baseWidget->relationFilter());
			if ( obj != NULL ) {
				obj->observer()->uninstallWidget(childWidget);
			}
		}
	}
}

/*!
  Realiza la sincronización entre los datos del objeto de negocio y el de visualización
  */
void BaseBeanObserver::sync()
{
	BaseBean *bean = qobject_cast<BaseBean *>(m_entity);
	setWindowModified(bean->modified());
	foreach (QObject *form, m_viewWidgets) {
		QList<QWidget *> widgets = form->findChildren<QWidget *>();
		if ( form->property("perpControl").toBool() ) {
			DBBaseWidget *baseWidget = dynamic_cast<DBBaseWidget *>(form);
			baseWidget->refresh();
		}
		foreach ( QWidget *w, widgets ) {
			if ( w->property("perpControl").toBool() ) {
				DBBaseWidget *baseWidget = dynamic_cast<DBBaseWidget *>(w);
				baseWidget->refresh();
			}
		}
	}
}

/*!
  Valida los datos contenidos en este bean, antes de almacenar en BBDD
  */
bool BaseBeanObserver::validate()
{
	return d->m_validator->validate();
}

/*!
  Esta función se llama después de llamar a validate, y devuelve una cadena de texto
  con el resultado de la validación
  */
QString BaseBeanObserver::validateMessages()
{
	return d->m_validator->validateMessages();
}

/*!
  Esta función se llama después de llamar a validate, y devuelve una cadena de texto
  con el resultado de la validación, en html
  */
QString BaseBeanObserver::validateHtmlMessages()
{
	return d->m_validator->validateHtmlMessages();
}

/*!
  Caso de una validación incorrecta, devuelve el primer widget que generó un problema
  */
QWidget *BaseBeanObserver::focusWidgetOnBadValidate()
{
	return d->m_validator->widgetOnBadValidate();
}

