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
#include "dbobject.h"
#include "configuracion.h"
#include "observerfactory.h"
#include "basebeanobserver.h"
#include "dao/beans/dbrelation.h"

DBObject::DBObject(QObject *parent) :
    QObject(parent)
{
	/** Podría asignarse un observador a cada entidad. Pero entonces el consumo de memoria
	  podría ser bastante elevado; además a la hora de localizar el observador adecuado
	  gastaríamos mucho tiempo. Es por ello que inicialmente no se asignan observadores.
	  En cualquier caso, se crearán observadores para todos los beans, donde los formularios
	  podrán inscribirse para visualizar los datos */
	m_observer = NULL;
	setObjectName(QString("%1").arg(configuracion.uniqueId()));
}

DBObject::~DBObject()
{
    if ( m_observer != NULL ) {
        delete m_observer;
    }
}

/*!
  Crea un nuevo observador para este objeto, y se registra como hijo de un observador
  superior si este objeto tuviera padre
  */
AbstractObserver *DBObject::observer ()
{
	if ( m_observer == NULL ) {
		m_observer = ObserverFactory::instance()->newObserver(this);
	}
	return m_observer;
}

/*!
  La destrucción de los observadores se hace al destruir el objeto. Sin embargo, a veces
  es interesante realizar la destrucción a demanda
  */
void DBObject::deleteObserver()
{
	if ( m_observer != NULL ) {
		delete m_observer;
		m_observer = NULL;
	}
}
