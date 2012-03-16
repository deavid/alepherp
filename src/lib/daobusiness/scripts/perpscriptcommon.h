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
#ifndef PERPSCRIPTCOMMON_H
#define PERPSCRIPTCOMMON_H

#include <QObject>
#include <QScriptValue>
#include <QScriptable>
#include "dao/beans/basebean.h"

class PERPScriptCommonPrivate;

/*!
  Conjunto de funciones habituales y comunes disponibles en los scripts a partir
  del objeto PERPScriptCommon
  */
class PERPScriptCommon : public QObject, public QScriptable
{
    Q_OBJECT
private:
	PERPScriptCommonPrivate *d_ptr;
	Q_DECLARE_PRIVATE(PERPScriptCommon)

public:
    explicit PERPScriptCommon(QObject *parent = 0);
	~PERPScriptCommon();

	static QScriptValue toScriptValue(QScriptEngine *engine, PERPScriptCommon* &in);
	static void fromScriptValue(const QScriptValue &object, PERPScriptCommon* &out);

	Q_INVOKABLE bool saveToFile(const QString &fileName, const QString &content, bool overwrite = false);
	Q_INVOKABLE QString readFromFile(const QString &fileName);
	Q_INVOKABLE QString saveToTempFile(const QString &content);
	Q_INVOKABLE QString readFromTempFile(const QString &fileName, bool remove = true);

	Q_INVOKABLE QScriptValue createBean(const QString &tableName);
	Q_INVOKABLE QScriptValue bean(const QString &tableName, const QString &where);
	Q_INVOKABLE QScriptValue beans(const QString &tableName, const QString &where, const QString &order);
	Q_INVOKABLE QVariant sqlSelect(const QString &fields, const QString &from, const QString &where, const QString &sort = "");
	Q_INVOKABLE bool sqlExecute(const QString &sql);

	Q_INVOKABLE QString xmlQuery(const QString &xml, const QString &query);

	Q_INVOKABLE bool updateQsSystemObject(const QString &objectName, const QString &content);

	Q_INVOKABLE QString formatNumber(const QVariant &number, int numDecimals = 2);
	Q_INVOKABLE double parseDouble(const QString &number);

	Q_INVOKABLE void setEnvVar(const QString &varName, const QVariant &v);
	Q_INVOKABLE QVariant envVar(const QString &name);

	Q_INVOKABLE QString username();

	Q_INVOKABLE QVariant registryValue(const QString &key);
	Q_INVOKABLE void setRegistryValue(const QString &key, const QString &value);

	Q_INVOKABLE QScriptValue getOpenForm(const QString &name);

	Q_INVOKABLE QScriptValue login();
	Q_INVOKABLE QScriptValue newLoginUser();
	Q_INVOKABLE bool changeUserPassword();

	Q_INVOKABLE bool checkMd5(const QString &valueToCheck, const QString &md5Hash);

    Q_INVOKABLE bool validateCIF(const QString &cif);
    Q_INVOKABLE bool validateNIF(const QString &nif);

	Q_INVOKABLE bool createSystemUser(const QString &userName, const QString &password);

signals:

public slots:

};

Q_DECLARE_METATYPE(PERPScriptCommon*)

#endif // PERPSCRIPTCOMMON_H
