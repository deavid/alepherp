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
#include "systemdao.h"
#include "dao/database.h"
#include "configuracion.h"

#define SQL_SELECT_SYSTEM_TABLES_BY_NOMBRE "SELECT contenido FROM alepherp_system WHERE nombre = :nombre AND type = 'table'"
#define SQL_SELECT_SYSTEM_TABLES "SELECT nombre, contenido FROM alepherp_system WHERE type = 'table'"
#define SQL_SELECT_SYSTEM_UI "SELECT nombre, contenido FROM alepherp_system WHERE type = 'ui'"
#define SQL_SELECT_SYSTEM_UI_BY_NOMBRE "SELECT contenido FROM alepherp_system WHERE nombre = :nombre AND type = 'ui'"
#define SQL_SELECT_SYSTEM_QS "SELECT nombre, contenido, debug, on_init_debug FROM alepherp_system WHERE type = 'qs'"
#define SQL_SELECT_SYSTEM_REPORT "SELECT nombre, contenido FROM alepherp_system WHERE type = 'report'"

SystemDAO::SystemDAO(QObject *parent) :
    QObject(parent)
{
}

bool SystemDAO::insertSystemObject(const QString &name, const QString &type, const QString &contenido,
								   bool debug, bool debugOnInit, int version)
{
	bool result;
	QScopedPointer<QSqlQuery> qry (new QSqlQuery(Database::getSystemDatabase()));
	QString sql = QString("INSERT INTO alepherp_system(nombre, contenido, type, version, debug, on_init_debug) "
						  "VALUES (:nombre, :contenido, :type, :version, :debug, :on_init_debug)");
	qry->prepare(sql);
	qry->bindValue(":nombre", name);
	qry->bindValue(":contenido", contenido);
	qry->bindValue(":type", type);
	qry->bindValue(":version", version);
	qry->bindValue(":debug", debug);
	qry->bindValue(":on_init_debug", debugOnInit);
	result = qry->exec();
	qDebug() << "SystemDAO::insertSystemObject: [ " << qry->lastQuery() << " ]";
	if ( !result ) {
		qDebug() << "SystemDAO::insertSystemObject: [ " << qry->lastError().text() << " ]";
	}
	return result;
}

ALEPHERP_SYSTEM_OBJECT SystemDAO::systemObject(const QString &name, const QString &type)
{
	QScopedPointer<QSqlQuery> qry (new QSqlQuery(Database::getSystemDatabase()));
	QString sql = QString("SELECT nombre, contenido, type, version, debug, on_init_debug FROM alepherp_system "
						  "WHERE nombre = :nombre and type = :type");
	ALEPHERP_SYSTEM_OBJECT r;
	qry->prepare(sql);
	qry->bindValue(":nombre", name);
	qry->bindValue(":type", type);
	r.fetched = qry->exec() & qry->first();
	qDebug() << "SystemDAO::systemObject: [ " << qry->lastQuery() << " ]";
	if ( !r.fetched ) {
		qDebug() << "SystemDAO::systemObject: [ " << qry->lastError().text() << " ]";
	} else {
		r.name = qry->value(0).toString();
		r.content = qry->value(1).toString();
		r.type = qry->value(2).toString();
		r.version = qry->value(3).toInt();
		r.debug = qry->value(4).toBool();
		r.onInitDebug = qry->value(5).toBool();
	}
	return r;
}

QList<ALEPHERP_SYSTEM_OBJECT> SystemDAO::allSystemObjects()
{
	QScopedPointer<QSqlQuery> qry (new QSqlQuery(Database::getSystemDatabase()));
	QString sql = QString("SELECT nombre, contenido, type, version, debug, on_init_debug FROM alepherp_system");
	QList<ALEPHERP_SYSTEM_OBJECT> list;

	qry->prepare(sql);
	bool result = qry->exec();
	qDebug() << "SystemDAO::allSystemObjects: [ " << qry->lastQuery() << " ]";
	if ( !result ) {
		qDebug() << "SystemDAO::allSystemObjects: [ " << qry->lastError().text() << " ]";
	} else {
		do {
			ALEPHERP_SYSTEM_OBJECT r;
			r.name = qry->value(0).toString();
			r.content = qry->value(1).toString();
			r.type = qry->value(2).toString();
			r.version = qry->value(3).toInt();
			r.debug = qry->value(4).toBool();
			r.onInitDebug = qry->value(5).toBool();
			r.fetched = true;
			list << r;
		} while ( qry->next() );
	}
	return list;
}

bool SystemDAO::deleteSystemObject(const QString &name, const QString &type, int version)
{
	bool result;
	QScopedPointer<QSqlQuery> qry (new QSqlQuery(Database::getSystemDatabase()));
	QString sql = QString("DELETE FROM alepherp_system WHERE nombre = :nombre AND type = :type AND "
						  "version = :version");
	qry->prepare(sql);
	qry->bindValue(":nombre", name);
	qry->bindValue(":type", type);
	qry->bindValue(":version", version);
	result = qry->exec();
	qDebug() << "SystemDAO::deleteSystemObject: [ " << qry->lastQuery() << " ]";
	if ( !result ) {
		qDebug() << "SystemDAO::deleteSystemObject: [ " << qry->lastError().text() << " ]";
	}
	return result;
}

int SystemDAO::versionSystemObject(const QString &name, const QString &type)
{
	QScopedPointer<QSqlQuery> qry (new QSqlQuery(Database::getSystemDatabase()));
	QString sql = QString("SELECT version FROM alepherp_system "
						  "WHERE nombre = :nombre and type = :type");
	int result = -1;

	qry->prepare(sql);
	qry->bindValue(":nombre", name);
	qry->bindValue(":type", type);
	if ( !(qry->exec() & qry->first()) ) {
		qDebug() << "SystemDAO::versionSystemObject: [ " << qry->lastError().text() << " ]";
	} else {
		result = qry->value(0).toInt();
	}
	qDebug() << "SystemDAO::versionSystemObject: [ " << qry->lastQuery() << " ]";
	return result;
}

/*!
  Para una tabla de sistema devuelve su interfaz UI de manera que sea posible
  construir los formularios
  */
bool SystemDAO::getUISystem(const QString &name, QString &ui)
{
	bool result;
	QString sql = QString(SQL_SELECT_SYSTEM_UI_BY_NOMBRE);
	QScopedPointer<QSqlQuery> qry (new QSqlQuery(Database::getSystemDatabase()));
	qry->prepare(sql);
	qry->bindValue(":nombre", name, QSql::In);

	result = qry->exec();
	qDebug() << "SystemDAO: getUISystem: [ " << qry->lastQuery() << " ]";
	if ( result && qry->first() ) {
		ui = qry->value(0).toString();
	} else {
		writeDbMessages(qry.data());
		ui.clear();
		result = false;
	}
	return result;
}

/*!
  Obtiene todas las interfaces UI de manera que sea posible
  construir los formularios
  */
bool SystemDAO::getAllUISystem(QStringList &names, QStringList &uis)
{
	bool result;
	QString sql = QString(SQL_SELECT_SYSTEM_UI);
	QScopedPointer<QSqlQuery> qry (new QSqlQuery(Database::getSystemDatabase()));
	qry->prepare(sql);
	result = qry->exec();
	qDebug() << "SystemDAO: getAllUISystem: [ " << qry->lastQuery() << " ]";
	if ( result ) {
		while ( qry->next() ) {
			names << qry->value(0).toString();
			uis << qry->value(1).toString();
		}
	} else {
		writeDbMessages(qry.data());
		result = false;
	}
	return result;
}

/*!
  Obtiene todas las interfaces de informes de manera que sea posible
  construir los formularios
  */
bool SystemDAO::getAllReportSystem(QStringList &names, QStringList &reports)
{
	bool result;
	QString sql = QString(SQL_SELECT_SYSTEM_REPORT);
	QScopedPointer<QSqlQuery> qry (new QSqlQuery(Database::getSystemDatabase()));
	qry->prepare(sql);
	result = qry->exec();
	qDebug() << "SystemDAO: getAllReportSystem: [ " << qry->lastQuery() << " ]";
	if ( result ) {
		while ( qry->next() ) {
			names << qry->value(0).toString();
			reports << qry->value(1).toString();
		}
	} else {
		writeDbMessages(qry.data());
		result = false;
	}
	return result;
}

/*!
  Obtiene todas las interfaces QS de manera que sea posible
  construir los formularios y tengan ejecución
  */
bool SystemDAO::getAllQSSystem(QStringList &names, QStringList &scripts, QList<bool> &debugFlags, QList<bool> &onInitDebugFlags)
{
	bool result;
	QString sql = QString(SQL_SELECT_SYSTEM_QS);
	QScopedPointer<QSqlQuery> qry (new QSqlQuery(Database::getSystemDatabase()));
	qry->prepare(sql);
	result = qry->exec();
	qDebug() << "SystemDAO: getAllQSSystem: [ " << qry->lastQuery() << " ]";
	if ( result ) {
		while ( qry->next() ) {
			names << qry->value(0).toString();
			scripts << qry->value(1).toString();
			debugFlags << qry->value(2).toBool();
			onInitDebugFlags << qry->value(3).toBool();
		}
	} else {
		writeDbMessages(qry.data());
		result = false;
	}
	return result;
}


/*!
  Para una tabla de sistema devuelve su descripción de manera que sea posible
  construir los beans
  */
void SystemDAO::getXMLSystemTable(const QString &tableName, QString &xml)
{
	QString sql = QString(SQL_SELECT_SYSTEM_TABLES_BY_NOMBRE);
	QScopedPointer<QSqlQuery> qry (new QSqlQuery(Database::getSystemDatabase()));
	bool result;

	qry->prepare(sql);
	qry->bindValue(":nombre", tableName, QSql::In);

	result = qry->exec();
	qDebug() << "SystemDAO: getXMLSystemTable: [ " << tableName << ": " << qry->lastQuery() << " ]";
	if ( result && qry->first() ) {
		xml = qry->value(0).toString();
	} else {
		writeDbMessages(qry.data());
		xml.clear();
	}
}

/*!
  Devuelve todas las descripciones XML de todas las tablas de sistemas. Esta función
  sirve para así poder construir en ria un conjunto de Maestros de beans para la factoria
  de construcción, que permita una construcción rñapida de objetos sin necesidad
  de consultar de nuevo en base de datos. Esta función será llamada al inicio del programa.
  */
bool SystemDAO::getAllXMLSystemTable(QStringList &names, QStringList &xml)
{
	bool r;
	QString sql = QString(SQL_SELECT_SYSTEM_TABLES);
	QScopedPointer<QSqlQuery> qry (new QSqlQuery(Database::getSystemDatabase()));
	qry->prepare(sql);
	r = qry->exec();
	qDebug() << "SystemDAO: getAllXMLSystemTable: [ " << qry->lastQuery() << " ]";
	if ( r ) {
		while ( qry->next() ) {
			names << qry->value(0).toString();
			xml << qry->value(1).toString();
			qDebug() << "SystemDAO: getAllXMLSystemTable: tabla: [ " << qry->value(0).toString() << " ]";
		}
		r = true;
	} else {
		writeDbMessages(qry.data());
		r = false;
	}
	return r;
}

/*!
  Comprueba que la base local de sistema tiene la última versión de los archivos
  de sistema de la base de datos general. Los actualiza si es necesario
  */
bool SystemDAO::checkSystemTable()
{
	QString sql, sqlSystemObject;
	bool result = false;
	QScopedPointer<QSqlQuery> qry (new QSqlQuery(Database::getQDatabase()));
	QScopedPointer<QSqlQuery> qryContent (new QSqlQuery(Database::getQDatabase()));

	sql = QString("SELECT nombre, type, max(version) FROM %1_system GROUP BY nombre, type").arg(configuracion.systemTablePrefix());
	sqlSystemObject = QString("SELECT contenido, debug, on_init_debug FROM %1_system "
							  "WHERE nombre = :nombre AND type = :type AND version = :version").arg(configuracion.systemTablePrefix());
	qry->prepare(sql);
	result = qry->exec() & qry->first();
	qDebug() << "SystemDAO: checkSystemTable: [ " << qry->lastQuery() << " ]";
	if ( result ) {
		do {
			int version = SystemDAO::versionSystemObject(qry->value(0).toString(), qry->value(1).toString());
			// Actuamos si no tenemos la última versión
			if ( version != qry->value(2).toInt() ) {
				if ( version != -1 ) {
					if ( !SystemDAO::deleteSystemObject(qry->value(0).toString(), qry->value(1).toString(), version) ) {
						return false;
					}
				}
				qryContent->prepare(sqlSystemObject);
				qryContent->bindValue(":nombre", qry->value(0));
				qryContent->bindValue(":type", qry->value(1));
				qryContent->bindValue(":version", qry->value(2));
				if ( qryContent->exec() && qryContent->first() ) {
					if ( !SystemDAO::insertSystemObject(qry->value(0).toString(),
														qry->value(1).toString(),
														qryContent->value(0).toString(),
														qryContent->value(1).toBool(),
														qryContent->value(2).toBool(),
														qry->value(2).toInt() ) ) {
						return false;
					}
				} else {
					SystemDAO::writeDbMessages(qryContent.data());
					return false;
				}
			}
		} while (qry->next());
	} else {
		writeDbMessages(qry.data());
	}
	return result;
}

void SystemDAO::writeDbMessages(QSqlQuery *qry)
{
	qDebug() << "SystemDAO::BBDD LastQuery: [ " << qry->lastQuery() << " ]";
	qDebug() << "SystemDAO::BBDD Message(databaseText): [ " << qry->lastError().databaseText() << " ]";
	qDebug() << "SystemDAO::BBDD Message(text): [ " << qry->lastError().text() << " ]";
}
