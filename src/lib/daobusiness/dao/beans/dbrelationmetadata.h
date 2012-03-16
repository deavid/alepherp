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
#ifndef DBRELATIONMETADATA_H
#define DBRELATIONMETADATA_H

#include <QObject>
#include <QString>

class DBRelationMetadataPrivate;
class BaseBeanMetadata;

class DBRelationMetadata : public QObject
{
	Q_OBJECT
	Q_ENUMS(DBRelationType)

private:
	Q_DISABLE_COPY(DBRelationMetadata)
	DBRelationMetadataPrivate *d;
	Q_DECLARE_PRIVATE(DBRelationMetadata)

public:
	DBRelationMetadata(QObject *parent = NULL);
	~DBRelationMetadata();

	enum DBRelationType {ONE_TO_ONE, ONE_TO_MANY, MANY_TO_ONE};

	DBRelationType type();
	void setType(DBRelationType type);
	bool fetchAutomatic();
	void setFetchAutomatic(bool value);
	QString name();
	void setName(const QString &name);
	QString rootFieldName();
	void setRootFieldName(const QString &value);
	QString childFieldName();
	void setChildFieldName(const QString &value);
	QString tableName();
	void setTableName(const QString &value);
	QString order();
	void setOrder(const QString &value);
	bool editable();
	void setEditable(bool value);
	bool deleteCascade();
	void setDeleteCascade(bool value);
	void setAvoidDeleteIfIsReferenced (bool value);
	bool avoidDeleteIfIsReferenced ();
	bool includeOnCopy();
	void setIncludeOnCopy(bool value);

	BaseBeanMetadata * rootMetadata();
};

#endif // DBRELATIONMETADATA_H
