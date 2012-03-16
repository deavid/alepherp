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
#ifndef SYSTEMDAO_H
#define SYSTEMDAO_H

#include <QObject>
#include <QSqlQuery>

typedef struct {
	QString name;
	QString content;
	QString type;
	int version;
	bool debug;
	bool onInitDebug;
	bool fetched;
} ALEPHERP_SYSTEM_OBJECT;

/*!
  Esta clase contiene toda la lógica para almacenar el código de la lógica de negocio de la aplicación
  */
class SystemDAO : public QObject
{
    Q_OBJECT
public:
    explicit SystemDAO(QObject *parent = 0);

	static bool insertSystemObject(const QString &name, const QString &type, const QString &contenido,
								   bool debug, bool debugOnInit, int version);
	static QList<ALEPHERP_SYSTEM_OBJECT> allSystemObjects();
	static bool deleteSystemObject(const QString &name, const QString &type, int version);
	static int versionSystemObject(const QString &name, const QString &type);
	static ALEPHERP_SYSTEM_OBJECT systemObject(const QString &name, const QString &type);

	static bool checkSystemTable();
	static void getXMLSystemTable(const QString &tableName, QString &xml);
	static bool getAllXMLSystemTable(QStringList &names, QStringList &xml);
	static bool getUISystem(const QString &name, QString &ui);
	static bool getAllUISystem(QStringList &name, QStringList &ui);
	static bool getAllQSSystem(QStringList &names, QStringList &scripts, QList<bool> &debugFlags, QList<bool> &onInitDebugFlags);
	static bool getAllReportSystem(QStringList &names, QStringList &reports);
	static void writeDbMessages(QSqlQuery *qry);

signals:

public slots:

};

#endif // SYSTEMDAO_H
