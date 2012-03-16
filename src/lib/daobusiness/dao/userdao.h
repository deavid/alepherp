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
#ifndef USERDAO_H
#define USERDAO_H

#include <QObject>
#include <QHash>
#include <QSqlQuery>
#include <alepherpglobal.h>

class Q_ALEPHERP_EXPORT UserDAO : public QObject
{
    Q_OBJECT
	Q_FLAGS(Permissions)
public:
    explicit UserDAO(QObject *parent = 0);

	enum Permission {
		READ = 0x01,
		WRITE = 0x02
	};
	Q_DECLARE_FLAGS(Permissions, Permission)

	static bool login (const QString &userName, const QString &password, bool &passwordIsEmpty, QString &errorMessage);
	static QHash<QString, QVariant> permission(const QString &userName);
	static bool changePassword (const QString &oldPassword, const QString &newPassword);
	static QList<int> roles(const QString &userName);
	static QHash<QString, QVariant> grantAllAccess();
	static bool createUser(const QString &userName, const QString &password);
	static void writeDbMessages(QSqlQuery *qry);

signals:

public slots:

};

Q_DECLARE_OPERATORS_FOR_FLAGS(UserDAO::Permissions)

#endif // USERDAO_H
