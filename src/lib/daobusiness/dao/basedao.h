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
#ifndef BASEDAO_H
#define BASEDAO_H

#include <QSqlDatabase>
#include <QList>
#include <QObject>
#include <QSharedPointer>
#include <QHash>
#include "dao/beans/basebean.h"

class BaseBean;
class BaseBeanMetadata;
class DBField;
class DBRelation;
class DBFieldMetadata;
class DBRelationMetadata;
class CachedContent;
class CachedBean;

/**
  Clase principal de acceso a base de datos para los BaseBean genéricos.
	@author David Pinelo <david.pinelo@alephsistemas.es>
*/
class BaseDAO : public QObject {
	Q_OBJECT

private:
	/** Contendrá información de base de datos de los últimos mensajes de error, generados
	en la última operación */
	static QString m_lastMessage;
	static QList<BaseBean *> m_beansRemoved;
	static QHash<QString, CachedContent *> m_cachedQuerys;
	static QHash<QString, CachedBean *> m_cachedBeans;
	static QHash<QString, QVariant> m_cachedSimpleQuerys;

	static bool isQueryCached(const QString &tableName, const QString &sql);
	static bool isBeanCached(const QString &tableName, const QString &sql);
	static void appendToCachedQuerys(const QString &tableName, const QString &sql, const BaseBeanPointerList &list);
	static void appendToCachedBeans(const QString &tableName, const QString &sql, const QSharedPointer<BaseBean> &bean);
	static BaseBeanPointerList getContentCachedQuery(const QString &tableName, const QString &sql);
	static QSharedPointer<BaseBean> getContentCachedBean(const QString &tableName, const QString &sql);

protected:
	static void clearDbMessages();
	static void buildSqlSelect(const QList<DBFieldMetadata *> &fields, const QHash<QString, QString> &xmlSql, const QString &where, const QString &order,
								QString &sql);
	static void buildSqlSelect(const QString &tableName, const QString &where, const QString &order,
								 const QList<DBFieldMetadata *> &fields, QString &sql);
	static bool buildSqlSelectWithLimits(QString &sql, const QString &tableName, 
					const QString &where = "", const QString &order = "", 
					int numRows = -1, int offset = -1);
	static QString sqlSelectFieldsClausule(const QList<DBFieldMetadata *> &fields, 
					const QHash<QString, QString> &alias = QHash<QString, QString>() );

	static void readSerialValuesAfterInsert(BaseBean *bean, int oid = -1, const QString &connection = "");

	static bool insertFieldOnUpdateSql(DBField *field, BaseBean::DbBeanStates state);
	static void cleanCachedDataIfRequired(const QString &tableName);

public:
	BaseDAO(QObject *parent = 0);

	~BaseDAO();

	static BaseDAO *instance();

	static bool transaction(const QString &connection = "");
	static bool rollback(const QString &connection = "");
	static bool commit(const QString &connection = "");

    static bool loadEnvVars();

	QString getLastDbMessage();

	static QSharedPointer<BaseBean> selectByPk(QVariant id, const QString &tableName, const QString &connection = "");
	static bool selectByPk(QVariant id, BaseBean *bean, const QString &connection = "");
	static bool select(BaseBeanPointerList &beans, const QString &tableName,
				const QString &where = "", const QString &order = "", int numRows = -1, int offset = -1,
				const QString &connection = "");
	static bool select(QList<BaseBean *> &beans, const QString &tableName,
				const QString &where = "", const QString &order = "", int numRows = -1, int offset = -1,
				const QString &connection = "");
	static bool selectFirst(BaseBean *bean, const QString &where, const QString &order = "",
				const QString &connection = "");
	static bool selectFirst(const QSharedPointer<BaseBean> &bean, const QString &where, const QString &order = "",
				const QString &connection = "");
	static int selectCount(const QString &tableName, const QString &where = "", const QString &connection = "");
	static int selectCountWithFrom(const QString &sql, const QString &connection = "");
	static bool insert(BaseBean *bean, bool saveChilds = true, const QString &connection = "");
	static bool insert(const QSharedPointer<BaseBean> &bean, bool saveChilds = true, const QString &connection = "");
	static bool update(BaseBean *bean, bool saveChilds = true,  const QString &connection = "");
	static bool update(const QSharedPointer<BaseBean> &bean, bool saveChilds = true, const QString &connection = "");
	static bool remove(BaseBean *bean, bool firstCall = true, const QString &connection = "");
	static bool remove(const QSharedPointer<BaseBean> &bean, bool firstCall = true, const QString &connection = "");

	static bool selectField(DBField *fld, const QString &connection = "");

	static bool execute(const QString &sql, const QString &connection = "");
	static bool execute(const QString &sql, QVariant &result, const QString &connection = "");
	static bool executeCached(const QString &sql, QVariant &result, const QString &connection = "");

	static int newLock (const QString &tableName, const QString &userName, const QVariant &pkRecord, const QString &connection = "");
	static bool unlock (int id, const QString &connection = "");
	static bool lockInformation(const QString &tableName, const QVariant &pk, QHash<QString, QVariant> &information, const QString &connection = "");
	static bool isLockValid(int id, const QString &tableName, const QString &userName, const QVariant &pk, const QString &connection = "");

	static void copyBaseBean(const QSharedPointer<BaseBean> &orig, const QSharedPointer<BaseBean> &dest, bool copy1MRelationChils = true);
	static void copyBaseBean(const QSharedPointer<BaseBean> &orig, const QSharedPointer<BaseBean> &dest, const QStringList &relationsChildsToCopy);

	static QString serializePk(const QVariant &pk);

	static bool reloadBeanFromDB(const QSharedPointer<BaseBean> &bean);
	static bool reloadFieldChangedAfterSave(BaseBean *bean);

	static bool canExecute();
	static void writeDbMessages(QSqlQuery *qry);
};

#endif

