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
#ifndef DATABASE_H
#define DATABASE_H

#define BASE_CONNECTION		"BaseConnection"

#include <QSqlDatabase>
#include <QtDebug>
#include <QSqlError>
#include <QMessageBox>
#include <QSqlQuery>
#include <QMutex>
#include <QMutexLocker>
#include <QObject>
#include <alepherpglobal.h>

class DatabasePrivate;

/**
	Clase abstracta que envuelve los métodos para la apertura de conexiones a base de datos.
	Se utiliza el pool propio que implementa QSqlDatabase.
	@author David Pinelo <david.pinelo@alephsistemas.es>
*/
class Q_ALEPHERP_EXPORT Database : public QObject {
	Q_OBJECT

private:
	DatabasePrivate *d;
	Q_DECLARE_PRIVATE(Database)

    static bool openPostgreSQL(const QString &name);
    static bool openODBC(const QString &name);
	static bool openSQLite(const QString &name, bool &emptyDatabase);
	static bool createTablesSQLite();
	static bool createSystemTablesSQLite();
	static QString systemDatabaseName();

public:
	static QMutex mutex;
    static QString openMessage;

	Database(QObject *parent = 0);
	~Database();

	static bool initSystemsBeans();

	static Database *instance();
	static const QSqlDatabase getQDatabase(const QString &connection = "");
	static const QSqlDatabase getSystemDatabase();

	Q_INVOKABLE static bool createConnection(const QString &name);
	static bool createSystemConnection();

signals:
	void newDbNotification(const QString &name);
};

#endif
