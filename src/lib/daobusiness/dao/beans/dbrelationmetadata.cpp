/***************************************************************************
 *   Copyright (C) 2007 by David Pinelo   *
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
#include "dbrelationmetadata.h"
#include "dao/beans/basebeanmetadata.h"

class DBRelationMetadataPrivate
{
//	Q_DECLARE_PUBLIC(DBRelationMetadata)
public:
	/** Nombre que se le da a la relacion */
	QString m_name;
	/** Tipo de relacion que se establece */
	DBRelationMetadata::DBRelationType m_type;
	/** Indica si se hace un select automatico de los childs al obtener el padre */
	bool m_fetchAutomatic;
	/** Nombre de la tabla que contiene los hijos */
	QString m_tableName;
	/** Campo de relacion en la tabla raiz */
	QString m_rootFieldName;
	/** Campo de relacion en la tabla hija */
	QString m_childFieldName;
	/** Orden con el que se obtienen de base de datos los hijos */
	QString m_order;
	/** ¿Los hijos de esta relación son editables? */
	bool m_editable;
	/** ¿Se borran en cascada los datos, cuando se borra el bean padre? */
	bool m_deleteCascade;
	/** En otras relaciones, esta variable establece lo fuerte que es, de modo que el bean
	  puede no ser borrado si está relacionado */
	bool m_avoidDeleteIfIsReferenced;
	/** Indica si al copiarse un bean (Desde DBForm, por ejemplo, y utilizando la función BaseDAO:copyBean)
	  los childs de esta relación se incluyen en la copia */
	bool m_includeOnCopy;

	DBRelationMetadataPrivate () {}
};

DBRelationMetadata::DBRelationMetadata(QObject *parent) : QObject(parent), d(new DBRelationMetadataPrivate)
{
	d->m_fetchAutomatic = false;
	d->m_type = DBRelationMetadata::ONE_TO_ONE;
	d->m_editable = true;
	d->m_deleteCascade = false;
    d->m_avoidDeleteIfIsReferenced = false;
	d->m_includeOnCopy = false;
}

DBRelationMetadata::~DBRelationMetadata()
{
	delete d;
}

bool DBRelationMetadata::fetchAutomatic()
{
	return d->m_fetchAutomatic;
}

void DBRelationMetadata::setFetchAutomatic(bool value)
{
	d->m_fetchAutomatic = value;
}

QString DBRelationMetadata::name()
{
	return d->m_name;
}

void DBRelationMetadata::setName(const QString &name)
{
	d->m_name = name;
}

QString DBRelationMetadata::rootFieldName()
{
	return d->m_rootFieldName;
}

void DBRelationMetadata::setRootFieldName(const QString &value)
{
	d->m_rootFieldName = value;
}

QString DBRelationMetadata::childFieldName()
{
	return d->m_childFieldName;
}

void DBRelationMetadata::setChildFieldName(const QString &value)
{
	d->m_childFieldName = value;
}

QString DBRelationMetadata::tableName()
{
	return d->m_tableName;
}

void DBRelationMetadata::setTableName(const QString &value)
{
	d->m_tableName = value;
}

DBRelationMetadata::DBRelationType DBRelationMetadata::type()
{
	return d->m_type;
}

void DBRelationMetadata::setType(DBRelationMetadata::DBRelationType type)
{
	d->m_type = type;
}

QString DBRelationMetadata::order()
{
	return d->m_order;
}

void DBRelationMetadata::setOrder(const QString &value)
{
	d->m_order = value;
}

bool DBRelationMetadata::editable()
{
	return d->m_editable;
}

void DBRelationMetadata::setEditable(bool value)
{
	d->m_editable = value;
}

BaseBeanMetadata * DBRelationMetadata::rootMetadata()
{
	return qobject_cast<BaseBeanMetadata *>(parent());
}

bool DBRelationMetadata::deleteCascade()
{
	return d->m_deleteCascade;
}

void DBRelationMetadata::setDeleteCascade(bool value)
{
	d->m_deleteCascade = value;
}

void DBRelationMetadata::setAvoidDeleteIfIsReferenced (bool value)
{
	d->m_avoidDeleteIfIsReferenced = value;
}

bool DBRelationMetadata::avoidDeleteIfIsReferenced ()
{
	return d->m_avoidDeleteIfIsReferenced;
}

bool DBRelationMetadata::includeOnCopy()
{
	return d->m_includeOnCopy;
}

void DBRelationMetadata::setIncludeOnCopy(bool value)
{
	d->m_includeOnCopy = value;
}
