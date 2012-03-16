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
#include "historydao.h"
#include "configuracion.h"
#include "dao/beans/basebean.h"
#include "dao/beans/basebeanmetadata.h"
#include "dao/beans/dbfield.h"
#include "dao/beans/dbfieldmetadata.h"
#include "dao/database.h"
#include <QSqlQuery>
#include <QCoreApplication>

#define SQL_NEW_ENTRY "INSERT INTO %1_history (username, action, tablename, pkey, changed_data) VALUES (:username, :action, :tablename, :pkey, :changed_data)"


HistoryDAO::HistoryDAO(QObject *parent) :
    QObject(parent)
{
}

QString HistoryDAO::createData(BaseBean *bean)
{
	QString result;

	result = QString("<data>");
	QList<DBField *> flds = bean->fields();
	foreach ( DBField *fld, flds ) {
		if ( fld->modified() && !fld->metadata()->calculated() )  {
			result = QString("%1\n<field name=\"%2\">%3</field>").arg(result).
				arg(fld->metadata()->dbFieldName()).arg(fld->sqlValue(false));
		}
	}
	result = QString("%1\n</data>").arg(result);
	return result;
}

bool HistoryDAO::insertEntry(BaseBean *bean, const QString connection)
{
	QString sql = QString(SQL_NEW_ENTRY).arg(configuracion.systemTablePrefix());
	QScopedPointer<QSqlQuery> qry (new QSqlQuery(Database::getQDatabase(connection)));
	QString userName = qApp->property("userName").toString();

	qry->prepare(sql);
	qry->bindValue(":username", userName, QSql::In);
	qry->bindValue(":action", QString("INSERT"), QSql::In);
	qry->bindValue(":tablename", bean->metadata()->tableName(), QSql::In);
	qry->bindValue(":pkey", bean->pkSerializedValue(), QSql::In);
	qry->bindValue(":changed_data", HistoryDAO::createData(bean), QSql::In);
	bool result = qry->exec();
	qDebug() << "HistoryDAO: insertEntry: [ " << qry->lastQuery() << " ]";
	if ( !result ) {
		BaseDAO::writeDbMessages(qry.data());
	}
	return result;
}

bool HistoryDAO::updateEntry(BaseBean *bean, const QString connection)
{
	QString sql = QString(SQL_NEW_ENTRY).arg(configuracion.systemTablePrefix());
	QScopedPointer<QSqlQuery> qry (new QSqlQuery(Database::getQDatabase(connection)));
	QString userName = qApp->property("userName").toString();

	qry->prepare(sql);
	qry->bindValue(":username", userName, QSql::In);
	qry->bindValue(":action", QString("UPDATE"), QSql::In);
	qry->bindValue(":tablename", bean->metadata()->tableName(), QSql::In);
	qry->bindValue(":pkey", bean->pkSerializedValue(), QSql::In);
	qry->bindValue(":changed_data", HistoryDAO::createData(bean), QSql::In);
	bool result = qry->exec();
	qDebug() << "HistoryDAO: updateEntry: [ " << qry->lastQuery() << " ]";
	if ( !result ) {
		BaseDAO::writeDbMessages(qry.data());
	}
	return result;
}

bool HistoryDAO::removeEntry(BaseBean *bean, const QString connection)
{
	QString sql = QString(SQL_NEW_ENTRY).arg(configuracion.systemTablePrefix());
	QScopedPointer<QSqlQuery> qry (new QSqlQuery(Database::getQDatabase(connection)));
	QString userName = qApp->property("userName").toString();

	qry->prepare(sql);
	qry->bindValue(":username", userName, QSql::In);
	qry->bindValue(":action", QString("DELETE"), QSql::In);
	qry->bindValue(":tablename", bean->metadata()->tableName(), QSql::In);
	qry->bindValue(":pkey", bean->pkSerializedValue(), QSql::In);
	qry->bindValue(":changed_data", HistoryDAO::createData(bean), QSql::In);
	bool result = qry->exec();
	qDebug() << "HistoryDAO: removeEntry: [ " << qry->lastQuery() << " ]";
	if ( !result ) {
		BaseDAO::writeDbMessages(qry.data());
	}
	return result;
}
