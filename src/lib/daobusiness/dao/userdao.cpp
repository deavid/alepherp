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
#include "userdao.h"
#include "configuracion.h"
#include "dao/database.h"
#include "dao/basedao.h"
#include "dao/beans/beansfactory.h"
#include "dao/beans/basebeanmetadata.h"
#include "models/envvars.h"
#include <QCryptographicHash>
#include <QCoreApplication>


#define SQL_SELECT_USER "SELECT username, password FROM %1_users WHERE username=:username"
#define SQL_SELECT_USER_ROLES "SELECT id_rol FROM %1_users_roles WHERE username=:username"
#define SQL_SELECT_PERMISSIONS_BY_USER "SELECT tablename, permissions FROM %1_permissions WHERE username=:username"
#define SQL_SELECT_PERMISSIONS_BY_ROL "SELECT tablename, permissions FROM %1_permissions WHERE id_rol=:id_rol"
#define SQL_CHANGE_PASSWORD "UPDATE %1_users SET password=:password WHERE username=:username"

#define SQL_SELECT_USER_CI "SELECT username, password FROM %1_users WHERE upper(username)=upper(:username)"
#define SQL_SELECT_USER_ROLES_CI "SELECT id_rol FROM %1_users_roles WHERE upper(username)=upper(:username)"
#define SQL_SELECT_PERMISSIONS_BY_USER_CI "SELECT tablename, permissions FROM %1_permissions WHERE upper(username)=upper(:username)"
#define SQL_CHANGE_PASSWORD_CI "UPDATE %1_users SET password=:password WHERE upper(username)=upper(:username)"

UserDAO::UserDAO(QObject *parent) :
    QObject(parent)
{
}

/*!
  Realiza un login a base de datos. Devuelve true o false si el usuario existe en base de datos
 */
bool UserDAO::login (const QString &userName, const QString &password, bool &passwordIsEmpty, QString &errorMessage)
{
	bool result = false;
	QString sql;
	if ( EnvVars::var("userNameCaseInsensitive") == "true" ) {
		sql = QString(SQL_SELECT_USER_CI).arg(configuracion.systemTablePrefix());
	} else {
		sql = QString(SQL_SELECT_USER).arg(configuracion.systemTablePrefix());
	}
	QScopedPointer<QSqlQuery> qry (new QSqlQuery(Database::getQDatabase()));
	qry->prepare(sql);
	qry->bindValue(":username", userName, QSql::In);

	result = qry->exec() && qry->first();
	qDebug() << "UserDAO::login: [" << qry->lastQuery() << "]";
	if ( result ) {
		if ( qry->value(1).toString().isEmpty() ) {
			passwordIsEmpty = true;
			result = true;
		} else {
			passwordIsEmpty = false;
			QString passwordMd5 = qry->value(1).toString();
			if ( !passwordMd5.isEmpty() ) {
				QByteArray hashMd5 = QCryptographicHash::hash(password.toLatin1(), QCryptographicHash::Md5).toHex();
				if ( hashMd5 == passwordMd5 ) {
					result = true;
				} else {
					result = false;
				}
			} else {
				result = false;
			}
		}
	} else {
		writeDbMessages(qry.data());
	}
	return result;
}

/*!
  Devuelve la tabla de permisos de un usuario \a userName. Los permisos se aplican por tabla y por usuario y/o rol.
  El permiso de usuario prevalece sobre el de rol.
  En la tabla permisos se puede además introducir un wildcard, * por usuario que le da acceso a todo el sistema
  */
QHash<QString, QVariant> UserDAO::permission(const QString &userName)
{
	QString sql = QString(SQL_SELECT_PERMISSIONS_BY_ROL).arg(configuracion.systemTablePrefix());
	QScopedPointer<QSqlQuery> qry (new QSqlQuery(Database::getQDatabase()));
	QHash<QString, QVariant> hash;
	QList<int> idRoles = UserDAO::roles(userName);

	foreach ( int idRol, idRoles ) {
		qry->prepare(sql);
		qry->bindValue(":id_rol", idRol, QSql::In);
		if ( qry->exec() ) {
			while ( qry->next() ) {
				if ( qry->value(0).toString() == "*" ) {
					return grantAllAccess();
				}
				hash[qry->value(0).toString()] = qry->value(1).toString();
			}
		}
		qDebug() << "UserDAO::permission: [" << qry->lastQuery() << "]";
	}
	if ( EnvVars::var("userNameCaseInsensitive") == "true" ) {
		sql = QString(SQL_SELECT_PERMISSIONS_BY_USER_CI).arg(configuracion.systemTablePrefix());
	} else {
		sql = QString(SQL_SELECT_PERMISSIONS_BY_USER).arg(configuracion.systemTablePrefix());
	}
	qry->prepare(sql);
	qry->bindValue(":userName", userName, QSql::In);
	if ( qry->exec() ) {
		while ( qry->next() ) {
			if ( qry->value(0).toString() == "*" ) {
				return grantAllAccess();
			}
			hash[qry->value(0).toString()] = qry->value(1).toString();
		}
	} else {
		writeDbMessages(qry.data());
	}
	return hash;
}

/*!
  Otorga permisos a todas las tablas de sistema. Esto, desde luego, es un punto flaco en seguridad, pero teniendo
  en cuenta el target de usuarios al que va dirigido esta aplicación, es suficiente, y es una solución de
  compromiso aceptable.
  */
QHash<QString, QVariant> UserDAO::grantAllAccess()
{
	QHash<QString, QVariant> hash;
	foreach ( BaseBeanMetadata *bean, BeansFactory::metadatasBeans ) {
		hash[bean->tableName()] = "rw";
	}
	return hash;
}

bool UserDAO::changePassword (const QString &oldPassword, const QString &newPassword)
{
	QString sql;
	if ( EnvVars::var("userNameCaseInsensitive") == "true" ) {
		sql = QString(SQL_CHANGE_PASSWORD_CI).arg(configuracion.systemTablePrefix());
	} else {
		sql = QString(SQL_CHANGE_PASSWORD).arg(configuracion.systemTablePrefix());
	}
	QString userName = qApp->property("userName").toString();
	QString errorMessage;
	bool result = false, passwordIsEmpty;

	if ( !userName.isEmpty() && UserDAO::login(userName, oldPassword, passwordIsEmpty, errorMessage) ) {
		QByteArray hashMd5 = QCryptographicHash::hash(newPassword.toLatin1(), QCryptographicHash::Md5).toHex();
		QScopedPointer<QSqlQuery> qry (new QSqlQuery(Database::getQDatabase()));
		qry->prepare(sql);
		qry->bindValue(":username", userName, QSql::In);
		qry->bindValue(":password", QString(hashMd5));

		if ( qry->exec() ) {
			result = true;
		} else {
			writeDbMessages(qry.data());
		}
		qDebug() << "UserDAO::changePassword: [" << qry->lastQuery() << "]";
	}
	return result;
}

QList<int> UserDAO::roles(const QString &userName)
{
	QString sql;
	if ( EnvVars::var("userNameCaseInsensitive") == "true" ) {
		sql = QString(SQL_SELECT_USER_ROLES_CI).arg(configuracion.systemTablePrefix());
	} else {
		sql = QString(SQL_SELECT_USER_ROLES).arg(configuracion.systemTablePrefix());
	}
	QScopedPointer<QSqlQuery> qry (new QSqlQuery(Database::getQDatabase()));
	QList<int> idRoles;

	qry->prepare(sql);
	qry->bindValue(":username", userName, QSql::In);

	if ( qry->exec() ) {
		while ( qry->next() ) {
			idRoles << qry->value(0).toInt();
		}
	} else {
		writeDbMessages(qry.data());
	}
	qDebug() << "UserDAO::roles: [" << qry->lastQuery() << "]";
	return idRoles;
}

/**
  Permite crear un usuario de sistema. Sera útil para ponerlo a disposición de la funciones javascript
  */
bool UserDAO::createUser(const QString &userName, const QString &password)
{
	QString sql = QString("INSERT INTO %1_users (username, password) VALUES ('%2', md5('%3'))").
			arg(configuracion.systemTablePrefix()).arg(userName).arg(password);
	QScopedPointer<QSqlQuery> qry (new QSqlQuery(Database::getQDatabase()));
	qry->prepare(sql);
	bool result = qry->exec();
	qDebug() << "UserDAO::createUser: [" << qry->lastQuery() << "]";
	if ( !result ) {
		writeDbMessages(qry.data());
	}
	return result;
}

void UserDAO::writeDbMessages(QSqlQuery *qry)
{
	qDebug() << "UserDAO: BBDD LastQuery: " << qry->lastQuery();
	qDebug() << "UserDAO: BBDD Message(databaseText): " << qry->lastError().databaseText();
	qDebug() << "UserDAO: BBDD Message(text): " << qry->lastError().text();
}
