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
#include <QCoreApplication>
#include <QDebug>
#include <QSqlRecord>
#include <QSqlQuery>
#include <QSqlDriver>
#include <QStringBuilder>
#include "dao/basedao.h"
#include "dao/historydao.h"
#include "dao/database.h"
#include "dao/beans/basebean.h"
#include "dao/beans/beansfactory.h"
#include "dao/beans/dbfield.h"
#include "dao/beans/dbrelation.h"
#include "dao/beans/dbfieldmetadata.h"
#include "dao/beans/basebeanmetadata.h"
#include "dao/beans/dbrelationmetadata.h"
#include "models/envvars.h"
#include "configuracion.h"

#define SQL_SELECT_SYSTEM_ENVVARS "SELECT variable, value FROM %1_envvars WHERE username='%2' or username is null or username=''"

QString BaseDAO::m_lastMessage;
/** Guardamos la lista de beans que se borran en una iteración para después poder ejecutar sobre ellos
  la acción de borrado */
QList<BaseBean *> BaseDAO::m_beansRemoved;
/** Querys que han sido cacheadas. Se almacena primero el nombre de la tabla a cachear, y después
la query referente a esa tabla cacheada y los datos. Se optimiza rendimiento */
QHash<QString, CachedContent *> BaseDAO::m_cachedQuerys;
QHash<QString, CachedBean *> BaseDAO::m_cachedBeans;
QHash<QString, QVariant> BaseDAO::m_cachedSimpleQuerys;

class CachedContent : public QObject {
public:
	CachedContent(QObject *parent) : QObject(parent) {}
	QHash<QString, BaseBeanPointerList> hash;
};

class CachedBean : public QObject {
public:
	CachedBean(QObject *parent) : QObject(parent) {}
	QHash<QString, QSharedPointer<BaseBean> > hash;
};

BaseDAO::BaseDAO(QObject *parent) : QObject (parent)
{
}

BaseDAO::~BaseDAO()
{
}

/*!
  Singleton
  */
BaseDAO * BaseDAO::instance()
{
	static BaseDAO *singleton = 0;
	if ( singleton == 0 ) {
		singleton = new BaseDAO();
		singleton->setParent(qApp);
	}
	return singleton;
}

bool BaseDAO::transaction(const QString &connection)
{
	QSqlDatabase db = QSqlDatabase::database((connection.isEmpty() ? BASE_CONNECTION : connection));
	return db.transaction();
}

bool BaseDAO::rollback(const QString &connection)
{
	QSqlDatabase db = QSqlDatabase::database((connection.isEmpty() ? BASE_CONNECTION : connection));
	return db.rollback();
}

bool BaseDAO::commit(const QString &connection)
{
	QSqlDatabase db = QSqlDatabase::database((connection.isEmpty() ? BASE_CONNECTION : connection));
	return db.commit();
}

bool BaseDAO::isQueryCached(const QString &tableName, const QString &sql)
{
	if ( BaseDAO::m_cachedQuerys.contains(tableName) ) {
		CachedContent *content = BaseDAO::m_cachedQuerys[tableName];
		return content->hash.contains(sql);
	}
	return false;
}

bool BaseDAO::isBeanCached(const QString &tableName, const QString &sql)
{
	if ( BaseDAO::m_cachedBeans.contains(tableName) ) {
		CachedBean *content = BaseDAO::m_cachedBeans[tableName];
		return content->hash.contains(sql);
	}
	return false;
}

void BaseDAO::appendToCachedQuerys(const QString &tableName, const QString &sql, const BaseBeanPointerList &list)
{
	CachedContent *content;
	if ( !BaseDAO::m_cachedQuerys.contains(tableName) ) {
		content = new CachedContent(qApp);
		BaseDAO::m_cachedQuerys[tableName] = content;
	} else {
		content = BaseDAO::m_cachedQuerys[tableName];
	}
	// Ojo: Guardamos un clon del bean creado, ya que si no, éste podría destruirse previamente
	BaseBeanPointerList temp;
	foreach (QSharedPointer<BaseBean> bean, list) {
		temp.append(bean->clone());
	}
	content->hash[sql] = temp;
}

void BaseDAO::appendToCachedBeans(const QString &tableName, const QString &sql, const QSharedPointer<BaseBean> &bean)
{
	CachedBean *content;
	if ( !BaseDAO::m_cachedBeans.contains(tableName) ) {
		content = new CachedBean(qApp);
		BaseDAO::m_cachedBeans[tableName] = content;
	} else {
		content = BaseDAO::m_cachedBeans[tableName];
	}
	content->hash[sql] = bean->clone();
}

BaseBeanPointerList BaseDAO::getContentCachedQuery(const QString &tableName, const QString &sql)
{
	BaseBeanPointerList empty;
	if ( BaseDAO::isQueryCached(tableName, sql) ) {
		if ( BaseDAO::m_cachedQuerys.contains(tableName) ) {
			CachedContent *content = BaseDAO::m_cachedQuerys[tableName];
			BaseBeanPointerList temp;
			foreach (QSharedPointer<BaseBean> bean, content->hash[sql]) {
				temp.append(bean->clone());
			}
			return temp;
		}
	}
	return empty;
}

QSharedPointer<BaseBean> BaseDAO::getContentCachedBean(const QString &tableName, const QString &sql)
{
	QSharedPointer<BaseBean> empty;
	if ( BaseDAO::isBeanCached(tableName, sql) ) {
		if ( BaseDAO::m_cachedBeans.contains(tableName) ) {
			CachedBean *content = BaseDAO::m_cachedBeans[tableName];
			return content->hash[sql]->clone();
		}
	}
	return empty;
}

/*!
  Tras actualizar los datos de algún bean marcado como cacheado, y ser guardados estos datos en base
  de datos, limpiamos la caché. Este limpiado es ahora poco selectivo (limpiará todos los beans que
  corresponden a una tabla...), pero por el momento no compensa granularizar a por ejemplo, primary key
  */
void BaseDAO::cleanCachedDataIfRequired(const QString &tableName)
{
	if ( BaseDAO::m_cachedQuerys.contains(tableName) ) {
		delete BaseDAO::m_cachedQuerys[tableName];
		BaseDAO::m_cachedQuerys.remove(tableName);
	}
	if ( BaseDAO::m_cachedBeans.contains(tableName) ) {
		delete BaseDAO::m_cachedBeans[tableName];
		BaseDAO::m_cachedBeans.remove(tableName);
	}
	QHashIterator<QString, QVariant> it(BaseDAO::m_cachedSimpleQuerys);
	while (it.hasNext()) {
		it.next();
		if ( it.key().contains(tableName) ) {
			BaseDAO::m_cachedSimpleQuerys.remove(it.key());
		}
	}
}

bool BaseDAO::loadEnvVars()
{
    bool result;
    QString sql = QString(SQL_SELECT_SYSTEM_ENVVARS).arg(configuracion.systemTablePrefix()).
            arg(qApp->property("userName").toString());
    QScopedPointer<QSqlQuery> qry (new QSqlQuery(Database::getQDatabase()));
    result = qry->prepare(sql);
    if (result) result = qry->exec();
	qDebug() << "BaseDAO: loadEnvVars: [" << qry->lastQuery() << "]";
    if ( result ) {
        while ( qry->next() ) {
			QString ba = qry->value(1).toString();
			EnvVars::setVar(qry->value(0).toString(), ba);
        }
    } else {
        writeDbMessages(qry.data());
        result = false;
    }
    return result;

}

bool BaseDAO::buildSqlSelectWithLimits(QString &sql, const QString &tableName, const QString &where, const QString &order, int numRows, int offset)
{
	BaseBeanMetadata *metadata = BeansFactory::metadataBean(tableName);
	if ( metadata == NULL ) {
		qDebug() << "BaseDAO: select: buildSqlSelectWithLimits: [" << tableName << "] no existe en las definiciones";
		return false;
	}
	QList<DBFieldMetadata *> fields = metadata->fields();
	if ( metadata->sql().isEmpty() ) {
		buildSqlSelect(tableName, where, order, fields, sql);
	} else {
		QHash<QString, QString> xmlSql = metadata->sql();
		buildSqlSelect(fields, xmlSql, where, order, sql);
	}
	if ( sql.isEmpty() ) {
		return false;
	}
	if ( numRows != -1 && offset != -1 ) {
		sql = sql % QString(" LIMIT %2 OFFSET %3").arg(numRows).arg(offset);
	}
	return true;
}

/*!
  Esta función construye un select que ejecuta en base de datos en función de la definición
  en xml que se tenga de la tabla
  */
bool BaseDAO::select(BaseBeanPointerList &beans, const QString &tableName,
			const QString &where, const QString &order, int numRows, int offset, const QString &connection)
{
	QString sql;
	QScopedPointer<QSqlQuery> qry (new QSqlQuery(Database::getQDatabase(connection)));
    bool result;
	BaseBeanMetadata *metadata = BeansFactory::metadataBean(tableName);

	if ( !buildSqlSelectWithLimits(sql, tableName, where, order, numRows, offset) ) {
		return false;
	}

	// ¿Es cacheada la consulta o el bean?
	if ( metadata->isCached() && BaseDAO::isQueryCached(metadata->tableName(), sql) ) {
		beans = BaseDAO::getContentCachedQuery(metadata->tableName(), sql);
		return true;
	} else {
		result = qry->prepare(sql);
		if (result) result = qry->exec();
		qDebug() << "BaseDAO: select: [" << qry->lastQuery() << "]";
		if ( result ) {
			while ( qry->next() ) {
				// Si no se crea el bean, indicando que debe tener el padre por defecto para beans cacheados,
				// éstos seran borrados, por eso se pasa metadata->isCached
				beans.append(BeansFactory::instance()->newQBaseBean(tableName, false));
				beans.last()->blockSignals(true);
				beans.last()->setDbState(BaseBean::UPDATE);
				int j = 0;
				for ( int i = 0 ; i < metadata->fieldCount() ; i++ ) {
					// Ojo: Nos saltamos campos calculados
					DBFieldMetadata *fld = metadata->field(i);
					if ( ! fld->calculated() && !fld->memo() ) {
						beans.last()->setFieldValueFromInternal(i, qry->value(j));
						j++;
					}
				}
				beans.last()->recalculateCalculatedFields();
				beans.last()->blockSignals(false);
				beans.last()->setLoadTime(QDateTime::currentDateTime());
			}
			result = true;
			if ( metadata->isCached() ) {
				BaseDAO::appendToCachedQuerys(metadata->tableName(), sql, beans);
			}
		} else {
			writeDbMessages(qry.data());
		}
	}
	return result;
}

/*!
  Función que realiza un select. Crea con new los beans a contener en QList. Es responsabilidad
  del programador su destrucción
  */
bool BaseDAO::select(QList<BaseBean *> &beans, const QString &tableName,
			const QString &where, const QString &order, int numRows, int offset,
			const QString &connection)
{
	QString sql;
	QScopedPointer<QSqlQuery> qry (new QSqlQuery(Database::getQDatabase(connection)));
    bool result;
	BaseBeanMetadata *metadata = BeansFactory::metadataBean(tableName);

	if ( !buildSqlSelectWithLimits(sql, tableName, where, order, numRows, offset) ) {
		return false;
	}

	result = qry->prepare(sql);
    if ( result ) result = qry->exec();
	qDebug() << "BaseDAO: select: [" << qry->lastQuery() << "]";
    if ( result ) {
		while ( qry->next() ) {
			beans.append(BeansFactory::instance()->newBaseBean(tableName));
			beans.last()->blockSignals(true);
			beans.last()->setDbState(BaseBean::UPDATE);
			int j = 0;
			for ( int i = 0 ; i < metadata->fieldCount() ; i++ ) {
				// Ojo: Nos saltamos campos calculados
				DBFieldMetadata *fld = metadata->field(i);
                if ( ! fld->calculated() && !fld->memo() ) {
					beans.last()->setFieldValueFromInternal(i, qry->value(j));
					j++;
				}
			}
			beans.last()->recalculateCalculatedFields();
			beans.last()->blockSignals(false);
			beans.last()->setLoadTime(QDateTime::currentDateTime());
		}
		result = true;
	} else {
		writeDbMessages(qry.data());
	}
	return result;
}

/*!
  Cuenta el número de registros que proporcionaría una SELECT sobre tableName con el WHERE where. Ojo,
  puede haber tablas que no correspondan físicamente a ninguna tabla en base de datos (se definen con sql
  en el XML). En ese caso, esas tablas deben especificar en su definición la consulta para obtener
  la cuenta del número de registros. Aquí se tiene en cuenta esto.
  */
int BaseDAO::selectCount(const QString &tableName, const QString &where, const QString &connection)
{
	QScopedPointer<QSqlQuery> qry (new QSqlQuery(Database::getQDatabase(connection)));
	BaseBeanMetadata *metadata = BeansFactory::metadataBean(tableName);
	QString sql;
	if ( metadata == NULL || metadata->sql().isEmpty() ) {
		sql = QString("SELECT count(*) FROM %1 WHERE %2").arg(tableName).arg(where);
	} else {
		sql = metadata->sql().value("SELECTCOUNT");
		sql = sql.replace(":whereClausule", where);
	}

	bool result = qry->prepare(sql);
    if ( result ) result = qry->exec();
	qDebug() << "BaseDAO: selectCount: [" << qry->lastQuery() << "]";
    if ( result ) {
        if ( qry->first() ) {
            return qry->value(0).toInt();
        }
	} else {
		writeDbMessages(qry.data());
	}
	return -1;
}

/*!
  Cuenta el número de registros que proporcionaría una SELECT sobre tableName con el WHERE where.
  */
int BaseDAO::selectCountWithFrom(const QString &sql, const QString &connection)
{
	QScopedPointer<QSqlQuery> qry (new QSqlQuery(Database::getQDatabase(connection)));
	QString temp = QString("SELECT count(*) FROM %1").arg(sql);

	bool result = qry->prepare(temp);
    if ( result ) result = qry->exec();
	qDebug() << "BaseDAO: selectCountWithFrom: [" << qry->lastQuery() << "]";
    if ( result ) {
        if ( qry->first() ) {
            return qry->value(0).toInt();
        }
	} else {
		writeDbMessages(qry.data());
	}
	return -1;
}

bool BaseDAO::selectFirst(const QSharedPointer<BaseBean> &bean, const QString &where, const QString &order,
			const QString &connection)
{
	return selectFirst(bean.data(), where, order, connection);
}

/*!
  Selecciona el primer registro de base de datos según el wher epasado. Bean debe ser un objeto
  válido y contener un valor de primary key, sobre la que se realizará la búsqueda.
  Caso de no encontrarse el bean, o existir un error, devuelve false. Si se encuentra el bean
  devuelve true
  */
bool BaseDAO::selectFirst(BaseBean *bean, const QString &where, const QString &order,
			const QString &connection)
{
    bool result;
	QString sql;
	QScopedPointer<QSqlQuery> qry (new QSqlQuery(Database::getQDatabase(connection)));

	if ( bean == NULL ) {
		return false;
	}
	QList<DBFieldMetadata *> fields = bean->metadata()->fields();
	buildSqlSelect(bean->metadata()->tableName(), where, order, fields, sql);

	if ( sql.isEmpty() ) {
		return false;
	}
	sql = QString("%1 LIMIT 1").arg(sql);
	if ( bean->metadata()->isCached() && BaseDAO::isBeanCached(bean->metadata()->tableName(), sql) ) {
		result = true;
		QSharedPointer<BaseBean> bCached = BaseDAO::getContentCachedBean(bean->metadata()->tableName(), sql);
		QList<DBField *> fields = bCached->fields();
		foreach ( DBField * fld, fields ) {
			bean->setFieldValueFromInternal(fld->metadata()->dbFieldName(), fld->value());
		}
	} else {
		result = qry->prepare(sql);
		if ( result ) result = qry->exec();
		qDebug() << "BaseDAO: selectFirst: [" << qry->lastQuery() << "]";
		if ( result ) {
			if ( qry->first() ) {
				int j = 0;
				bean->blockSignals(true);
				for ( int i = 0 ; i < bean->metadata()->fieldCount() ; i++ ) {
					// Ojo: Nos saltamos campos calculados
					DBFieldMetadata *fld = bean->field(i)->metadata();
					if ( ! fld->calculated() && !fld->memo() ) {
						bean->setFieldValueFromInternal(i, qry->value(j));
						j++;
					}
				}
				bean->setDbState(BaseBean::UPDATE);
				bean->recalculateCalculatedFields();
				bean->blockSignals(false);
				bean->setLoadTime(QDateTime::currentDateTime());
				result = true;
				BaseDAO::appendToCachedBeans(bean->metadata()->tableName(), sql, bean->clone());
            } else {
                result = false;
            }
		} else {
			writeDbMessages(qry.data());
			result = false;
		}
	}
	return result;
}

/*!
  Selecciona un registro de base de datos a partir del valor de la primary key. Si la primary key
  es sobre varios campos, QVariant debe contener un QVariantMap con la key al nombre del campo
  y el value al valor. Devuelve un puntero compartido al bean, creado dentro de esta función.
  */
QSharedPointer<BaseBean> BaseDAO::selectByPk(QVariant id, const QString &tableName, const QString &connection)
{
	QSharedPointer<BaseBean> bean = BeansFactory::instance()->newQBaseBean(tableName, false);
	if ( !selectByPk(id, bean.data(), connection) ) {
		return QSharedPointer<BaseBean>();
	}
	return bean;
}

/*!
  Selecciona un registro de base de datos a partir del valor de la primary key. Si la primary key
  es sobre varios campos, QVariant debe contener un QVariantMap con la key al nombre del campo
  y el value al valor. Devuelve un puntero compartido al bean, creado dentro de esta función.
  */
bool BaseDAO::selectByPk(QVariant id, BaseBean *bean, const QString &connection)
{
	QString sql, where;
	QScopedPointer<QSqlQuery> qry (new QSqlQuery(Database::getQDatabase(connection)));

	if ( bean == NULL ) {
		return false;
	}
	bean->blockSignals(true);
	QList<DBField *> pk = bean->pkFields();
	if ( pk.isEmpty() ) {
		bean->blockSignals(false);
		return false;
	} else {
		bean->setPkValueFromInternal(id);
	}
	if ( pk.size() > 1 ) {
		QVariantMap pkValues = id.toMap();
		foreach ( DBField *fld, pk ) {
			if ( pkValues.contains(fld->metadata()->dbFieldName())) {
				fld->setValueFromInternal(pkValues.value(fld->metadata()->dbFieldName()));
				if ( where.isEmpty() ) {
					where = fld->sqlWhere("=");
				} else {
					where = where + " AND " + fld->sqlWhere("=");
				}
			}
		}
	} else {
		DBField *fld = pk.at(0);
		where = fld->sqlWhere("=");
	}

	buildSqlSelect(bean->metadata()->tableName(), where, QString(""), bean->metadata()->fields(), sql);

	if ( sql.isEmpty() ) {
		bean->blockSignals(false);
		return false;
	}
	if ( bean->metadata()->isCached() && BaseDAO::isBeanCached(bean->metadata()->tableName(), sql) ) {
		QSharedPointer<BaseBean> bCached = BaseDAO::getContentCachedBean(bean->metadata()->tableName(), sql);
		QList<DBField *> fields = bCached->fields();
		foreach ( DBField * fld, fields ) {
			bean->setFieldValueFromInternal(fld->metadata()->dbFieldName(), fld->value());
		}
	} else {
		bool result = qry->prepare(sql);
		if ( result ) result = qry->exec();
		qDebug() << "BaseDAO: selectByPk: [" << qry->lastQuery() << "]";
		if ( result ) {
			if ( qry->first() ) {
				int j = 0;
				for ( int i = 0 ; i < bean->metadata()->fieldCount() ; i++ ) {
					// Ojo: Nos saltamos campos calculados
					DBFieldMetadata *fld = bean->field(i)->metadata();
					if ( ! fld->calculated() && !fld->memo() ) {
						bean->setFieldValueFromInternal(i, qry->value(j));
						j++;
					}
				}
				bean->recalculateCalculatedFields();
				bean->setDbState(BaseBean::UPDATE);
				bean->setLoadTime(QDateTime::currentDateTime());
            } else {
                bean->blockSignals(false);
                return false;
            }
		} else {
			writeDbMessages(qry.data());
			bean->blockSignals(false);
			return false;
		}
		bean->blockSignals(false);
	}
	return true;
}

/*!
  Construye la sql del select, incluyendo los campos que se obtendrán de la consulta, junto con los alias
  de las tablas si fueran necesarios. Los campos MEMO no se obtienen en esta consulta. Deben de obtenerse de
  una consulta adicional para así agilizar las transacciones con la base de datos, al igual que los campos
  de tipo imágen.
  */
QString BaseDAO::sqlSelectFieldsClausule(const QList<DBFieldMetadata *> &fields, const QHash<QString, QString> &alias)
{
	QString sqlFields;

	// Construimos ahora la zona del select, a partir de los fields pasados
	foreach ( DBFieldMetadata *field, fields ) {
        if ( !field->calculated() && !field->memo() ) {
			if ( alias.contains(field->beanMetadata()->tableName()) ) {
				if ( sqlFields.isEmpty() ) {
					sqlFields = QString("%1.%2").arg(alias.value(field->beanMetadata()->tableName())).
								arg(field->dbFieldName());
				} else {
					sqlFields = QString("%1, %1.%2").arg(sqlFields).arg(alias.value(field->beanMetadata()->tableName())).
								arg(field->dbFieldName());
				}
			} else {
				if ( sqlFields.isEmpty() ) {
					sqlFields = QString("%1").arg(field->dbFieldName());
				} else {
					sqlFields = QString("%1, %2").arg(sqlFields).arg(field->dbFieldName());
				}
			}
		}
	}
	return sqlFields;
}

/*!
  Construye la sql del select. Para que no haya un problema de coordinación
  entre columnas visibles con campos memo y el método ::data
  */
void BaseDAO::buildSqlSelect(const QString &tableName, const QString &where, const QString &order,
							 const QList<DBFieldMetadata *> &fields, QString &sql)
{
	QString sqlFields = sqlSelectFieldsClausule( fields );
	sql = QString("SELECT DISTINCT %1 FROM %2").arg(sqlFields, tableName);
	if ( !where.isEmpty() ) {
		sql = QString("%1 WHERE %2").arg(sql).arg(where);
	}
	if ( !order.isEmpty() ) {
		sql = sql % QString(" ORDER BY ") % order;
	}
}

/*!
  Cuando en el XML se introduce una SQL al efecto, se debe escoger ésta y anexarle las claúsulas WHERE y ORDER
  */
void BaseDAO::buildSqlSelect(const QList<DBFieldMetadata *> &fields, const QHash<QString, QString> &xmlSql, const QString &where, const QString &order, QString &sql)
{
	QString sqlFields = sqlSelectFieldsClausule (fields);
	if ( xmlSql.contains("FROM") ) {
		sql = QString("SELECT DISTINCT %1 FROM %2").arg(sqlFields).arg(xmlSql.value("FROM"));
	}
	if ( xmlSql.contains("WHERE") ) {
		if ( where.isEmpty() ) {
			if ( !xmlSql.value("WHERE").isEmpty() ) {
				sql = sql % QString(" WHERE ") % xmlSql.value("WHERE");
			}
		} else {
			if ( !xmlSql.value("WHERE").isEmpty() ) {
				sql = sql % QString(" WHERE ") % xmlSql.value("WHERE") % QString(" AND ") % where;
			} else {
				sql = sql % QString(" WHERE ") % where;
			}
		}
	} else {
		sql = sql % QString(" WHERE ") % where;
	}
	if ( xmlSql.contains("ORDER") ) {
		if ( order.isEmpty() ) {
			if ( !xmlSql.value("ORDER").isEmpty() ) {
				sql = sql % QString(" ORDER BY ") % xmlSql.value("ORDER");
			}
		} else {
			if ( !xmlSql.value("ORDER").isEmpty() ) {
				sql = sql % QString(" ORDER BY ") % xmlSql.value("ORDER") % QString(" AND ") % order;
			} else {
				sql = sql % QString(" ORDER BY ") % order;
			}
		}
	} else {
		if ( !order.isEmpty() ) {
			sql = sql % QString(" ORDER BY ") % order;
		}
	}
}

/*!
  Hay algunas condiciones en las que un field no entra en una cláusula INSERT o UPDATE, como por ejemplo
  si el campo es un serial, o
  */
bool BaseDAO::insertFieldOnUpdateSql(DBField *field, BaseBean::DbBeanStates state)
{
	if ( field->metadata()->serial() ) {
		return false;
	}
	if ( field->metadata()->calculated() ) {
		if ( field->metadata()->calculatedSaveOnDb() ) {
			return true;
		} else {
			return false;
		}
	}
	if ( field->metadata()->null() && field->value().isNull() && !field->modified() ) {
		return false;
	}
	if ( state == BaseBean::UPDATE && !field->modified() ) {
		return false;
	}
	return true;
}

bool BaseDAO::insert(const QSharedPointer<BaseBean> &bean, bool saveChilds, const QString &connection)
{
	return BaseDAO::insert(bean.data(), saveChilds, connection);
}

/*!
  Construye y ejecuta una sentencia SQL INSERT a partir de la metainformación de BaseBean y de sus valores.
  saveChilds indica si se guardan también los childs hijos añadidos (hijos son los de relaciones 1M y 11
  */
bool BaseDAO::insert(BaseBean *bean, bool saveChilds, const QString &connection)
{
	QString sql, sqlFields, sqlValues, temp;
	QScopedPointer<QSqlQuery> qry (new QSqlQuery(Database::getQDatabase(connection)));
	bool result;
	int i = 0;

	if ( bean == NULL ) {
		return false;
	}
	// Para evitar los problemas de escape de caracteres y demás, se utilizará la claúsula bindValue
	// para lo cual, debemos primero construir un INSERT del tipo:
	// INSERT INTO tabla (field1, field2, field3) VALUES(?, ?,?);

	QList<DBField *> fields = bean->fields();
	foreach ( DBField *field, fields ) {
		if ( insertFieldOnUpdateSql(field, BaseBean::INSERT) ) {
			// Si el campo puede ser nulo, y no se ha dado valor, se deja a cero
            if ( sqlFields.isEmpty() ) {
                sqlFields = QString("%1").arg(field->metadata()->dbFieldName());
            } else {
                sqlFields = QString("%1, %2").arg(sqlFields).arg(field->metadata()->dbFieldName());
            }
            if ( sqlValues.isEmpty() ) {
                sqlValues = QString("?");
            } else {
                sqlValues = QString("%1, ?").arg(sqlValues);
            }
		}
	}
	sql = QString("INSERT INTO %1 (%2) VALUES (%3)").arg(bean->metadata()->tableName()).arg(sqlFields).arg(sqlValues);
	result = qry->prepare(sql);
	foreach ( DBField *field, fields ) {
		if ( insertFieldOnUpdateSql(field, BaseBean::INSERT) ) {
			if ( field->metadata()->type() == QVariant::Pixmap ) {
                QByteArray ba = field->value().toByteArray();
                qry->bindValue(i, ba.toBase64(), QSql::In);
			} else {
				qry->bindValue(i, field->value(), QSql::In);
			}
			i++;
		}
	}
	if ( result ) result = qry->exec();
	qDebug() << "BaseDAO: insert: [" << qry->lastQuery() << "]";
	if ( !result ) {
		writeDbMessages(qry.data());
		return false;
    } else {
		// Si es una tabla de elementos cacheados, limpiamos la cache
		if ( bean->metadata()->isCached() ) {
			BaseDAO::cleanCachedDataIfRequired(bean->metadata()->tableName());
		}
        // Es importante actualizar y obtener el valor de los campos seriales
        if ( Database::getQDatabase(connection).driver()->hasFeature(QSqlDriver::LastInsertId) ) {
			QVariant oid = qry->lastInsertId();
			if ( !oid.isValid() ) {
				BaseDAO::readSerialValuesAfterInsert(bean, -1, connection);
			} else {
				BaseDAO::readSerialValuesAfterInsert(bean, oid.toInt(), connection);
			}
        } else {
			BaseDAO::readSerialValuesAfterInsert(bean, -1, connection);
        }
		// Puede que haya campos que se calculen o tomen valor justo cuando la base de datos guarda
		// el valor (porque tengan un trigger activado). Aseguramos tener el último valor
		reloadFieldChangedAfterSave(bean);
		HistoryDAO::insertEntry(bean, connection);
	}
    if ( saveChilds ) {
		// Salvamos los hijos de las relaciones descendientes
		QList<DBRelation *> relations = bean->relations();
		foreach ( DBRelation *rel, relations ) {
			if ( rel->metadata()->type() == DBRelationMetadata::ONE_TO_ONE || rel->metadata()->type() == DBRelationMetadata::ONE_TO_MANY ) {
				BaseBeanPointerList childs = rel->modifiedChilds();
				foreach ( QSharedPointer<BaseBean> child, childs ) {
					if ( child->modified() && child->dbState() == BaseBean::INSERT ) {
						result = result & insert(child, true, connection);
						if ( !result ) {
							writeDbMessages(qry.data());
							return false;
						}
					}
				}
			}
		}
	}
	return result;
}


bool BaseDAO::update(const QSharedPointer<BaseBean> &bean, bool saveChilds, const QString &connection)
{
	return BaseDAO::update(bean.data(), saveChilds, connection);
}

/*!
  Construye y ejecuta una sentencia SQL UPDATE a partir de la metainformación de BaseBean y de sus valores.
  saveChilds indica si se guardan también los childs hijos añadidos (hijos son los de relaciones 1M y 11
  */
bool BaseDAO::update(BaseBean *bean, bool saveChilds, const QString &connection)
{
	QString sql, sqlFields, temp;
	QScopedPointer<QSqlQuery> qry (new QSqlQuery(Database::getQDatabase(connection)));
	bool result = true;
	int i = 0;

	if ( bean == NULL ) {
		return false;
	}
	if ( bean->modified() ) {
		QList<DBField *> fields = bean->fields();
		foreach ( DBField *field, fields ) {
			// Los campos serial no se incluyen en los updates, asi como los que estan marcados como no modificados
			if ( insertFieldOnUpdateSql(field, BaseBean::UPDATE) ) {
				temp = QString("%1 = ?").arg(field->metadata()->dbFieldName());
				if ( sqlFields.isEmpty() ) {
					sqlFields = QString("%1").arg(temp);
				} else {
					sqlFields = QString("%1, %2").arg(sqlFields).arg(temp);
				}
			}
		}
		// Puede ocurrir que se haya modificado los hijos del bean en la relación y no el bean. En ese caso,
		// no se ejecuta nada, y pasamos a los beans
		if ( !sqlFields.isEmpty() ) {
			sql = QString("UPDATE %1 SET %2 WHERE %3").arg(bean->metadata()->tableName()).
				  arg(sqlFields).arg(bean->sqlWherePk());
			result = qry->prepare(sql);
			foreach ( DBField *field, fields ) {
				if ( insertFieldOnUpdateSql(field, BaseBean::UPDATE) ) {
                    if ( field->metadata()->type() == QVariant::Pixmap ) {
                        QByteArray ba = field->value().toByteArray();
                        qry->bindValue(i, ba.toBase64(), QSql::In);
					} else {
						qry->bindValue(i, field->value(), QSql::In);
					}
					i++;
				}
			}
			if ( result ) result = qry->exec();
			qDebug() << "BaseDAO: update: [" << qry->lastQuery() << "]";
		}
		if ( !result ) {
			writeDbMessages(qry.data());
			return false;
		} else {
			BaseDAO::cleanCachedDataIfRequired(bean->metadata()->tableName());
			HistoryDAO::updateEntry(bean, connection);
			// Si es una tabla de elementos cacheados, limpiamos la cache
			if ( bean->metadata()->isCached() ) {
				BaseDAO::cleanCachedDataIfRequired(bean->metadata()->tableName());
			}
			// Puede que haya campos que se calculen o tomen valor justo cuando la base de datos guarda
			// el valor (porque tengan un trigger activado). Aseguramos tener el último valor
			reloadFieldChangedAfterSave(bean);
		}
	}
    if ( saveChilds ) {
		// Salvamos los hijos de las relaciones descendientes
		QList<DBRelation *> relations = bean->relations();
		foreach ( DBRelation *rel, relations ) {
			if ( rel->metadata()->type() == DBRelationMetadata::ONE_TO_ONE || rel->metadata()->type() == DBRelationMetadata::ONE_TO_MANY ) {
				BaseBeanPointerList childs = rel->modifiedChilds();
				foreach ( QSharedPointer<BaseBean> child, childs ) {
					if ( child->dbState() == BaseBean::INSERT && child->modified() ) {
						result = result & insert(child, true, connection);
					} else if ( child->dbState() == BaseBean::UPDATE && child->modified() ) {
						result = result & update(child, true, connection);
					} else if ( child->dbState() == BaseBean::TO_BE_DELETED ) {
						result = result & remove(child, false, connection);
					}
					if ( !result ) {
						writeDbMessages(qry.data());
						return false;
					}
				}
			}
		}
	}
	return result;
}

bool BaseDAO::remove(const QSharedPointer<BaseBean> &bean, bool firstCall, const QString &connection)
{
	return BaseDAO::remove(bean.data(), firstCall, connection);
}

/*!
  Construye y ejecuta una sentencia SQL DELETE FROM a partir de la metainformación de BaseBean y de sus valores
  firstCall indica si es la primera llamada (de las llamadas en cascada para borrar)
  y por tanto inicia una transacción en base de datos
  */
bool BaseDAO::remove(BaseBean *bean, bool firstCall, const QString &connection)
{
	QString sql;
	QScopedPointer<QSqlQuery> qry (new QSqlQuery(Database::getQDatabase(connection)));
	bool result;

	if ( bean == NULL ) {
		return false;
	}

	// Si este bean tiene hijos en una relación, y se borran en cascada con este, se realiza
	if ( firstCall ) {
		m_beansRemoved.clear();
		BaseDAO::transaction(connection);
	}
	QList<DBRelation *> rels = bean->relations();
	foreach ( DBRelation *rel, rels ) {
		if ( rel->metadata()->avoidDeleteIfIsReferenced() && rel->childsCount() > 0 ) {
			BaseBeanMetadata *relatedTable = BeansFactory::metadataBean(rel->metadata()->tableName());
			BaseDAO::m_lastMessage = trUtf8("El registro de la tabla <b>%1</b> (<i>%2</i>) no puede ser borrado ya que se encuentra "
											"relacionado con la tabla: <b>%3</b> (<i>%4</i>) y en esta tabla aun existen registros "
											"que pertenecen a la tabla <b>%1</b>.").
					arg(bean->metadata()->alias()).
					arg(bean->metadata()->tableName()).
					arg(relatedTable != NULL ? relatedTable->alias() : "").
					arg(rel->metadata()->tableName());
			qDebug() << "BaseDAO: remove: [" << BaseDAO::m_lastMessage << "]";
			if ( firstCall ) {
				BaseDAO::rollback(connection);
			}
			return false;
		}
		if ( rel->metadata()->deleteCascade() ) {
			BaseBeanPointerList childs = rel->childs();
			foreach ( QSharedPointer<BaseBean> child, childs ) {
				if ( !BaseDAO::remove(child, false, connection) ) {
					if ( firstCall ) {
						BaseDAO::rollback(connection);
					}
					return false;
				}
			}
		}
	}
	if ( bean->metadata()->logicalDelete() ) {
		if ( bean->metadata()->field("is_deleted") == NULL ) {
			if ( firstCall ) {
				BaseDAO::rollback(connection);
			}
			BaseDAO::m_lastMessage = trUtf8("El registro de la tabla %1 está marcado para tener un borrado lógico. "
											"Sin embargo no se ha definido una columna is_deleted en la definición de la tabla.").
					arg(bean->metadata()->tableName());
			qDebug() << "BaseDAO: remove: [" << BaseDAO::m_lastMessage << "]";
			return false;
		}
		sql = QString("UPDATE %1 SET is_deleted = true WHERE %2").arg(bean->metadata()->tableName()).
			  arg(bean->sqlWherePk());
	} else {
		sql = QString("DELETE FROM %1 WHERE %2").arg(bean->metadata()->tableName()).
			  arg(bean->sqlWherePk());
	}
	result = qry->prepare(sql);
	if (result) result = qry->exec();
	qDebug() << "BaseDAO: remove: [" << qry->lastQuery() << "]";
	if ( !result ) {
		writeDbMessages(qry.data());
        BaseDAO::rollback(connection);
	} else {
		BaseDAO::cleanCachedDataIfRequired(bean->metadata()->tableName());
		m_beansRemoved.append(bean);
		if ( firstCall ) {
			HistoryDAO::removeEntry(bean, connection);
			BaseDAO::commit(connection);
			foreach ( BaseBean *b, m_beansRemoved ) {
				b->metadata()->afterDeleteScriptExecute(b);
			}
			m_beansRemoved.clear();
		}
	}
	return result;
}

void BaseDAO::writeDbMessages(QSqlQuery *qry)
{
	BaseDAO::m_lastMessage = QString("%1\n%2").arg(BaseDAO::m_lastMessage).
			arg(qry->lastError().databaseText());
	qDebug() << "BaseDAO: writeDbMessages: BBDD LastQuery: [" << qry->lastQuery() << "]";
	qDebug() << "BaseDAO: writeDbMessages: BBDD Message(Error databaseText): [" << qry->lastError().databaseText() << "]";
	qDebug() << "BaseDAO: writeDbMessages: BBDD Message(Error text): [" << qry->lastError().text() << "]";
}

/*!
	Devuelve los últimos mensajes de error generados en la base de datos
 */
QString BaseDAO::getLastDbMessage()
{
	return BaseDAO::m_lastMessage;
}

void BaseDAO::clearDbMessages()
{
	BaseDAO::m_lastMessage.clear();
}

/*!
  Ejecuta una consulta en BBDD
  */
bool BaseDAO::execute(const QString &sql, const QString &connection)
{
	bool result;
	QScopedPointer<QSqlQuery> qry (new QSqlQuery(Database::getQDatabase(connection)));
	result = qry->prepare(sql);
	if ( result ) result = qry->exec();
	qDebug() << "BaseDAO: execute: [" << qry->lastQuery() << "]";
	if ( !result ) {
		writeDbMessages(qry.data());
	}
	return result;
}

/*!
  Ejecuta una consulta en BBDD y obtiene un resultado
  */
bool BaseDAO::execute(const QString &sql, QVariant &result, const QString &connection)
{
	bool r;
	QScopedPointer<QSqlQuery> qry (new QSqlQuery(Database::getQDatabase(connection)));
	r = qry->prepare(sql);
	if ( r ) r = qry->exec();
	qDebug() << "BaseDAO: execute: [" << qry->lastQuery() << "]";
    if ( r ) {
        if ( qry->first() ) {
            result = qry->value(0);
        }
	}
	if ( !r ) {
		writeDbMessages(qry.data());
	}
	return r;
}

/*!
  Ejecuta una consulta en BBDD y obtiene un resultado. Permite el cacheo de los resultados
  */
bool BaseDAO::executeCached(const QString &sql, QVariant &result, const QString &connection)
{
	bool r;
	if ( BaseDAO::m_cachedSimpleQuerys.contains(sql) ) {
		result = BaseDAO::m_cachedSimpleQuerys[sql];
		return true;
	} else {
		QScopedPointer<QSqlQuery> qry (new QSqlQuery(Database::getQDatabase(connection)));
		r = qry->prepare(sql);
		if ( r ) r = qry->exec();
		qDebug() << "BaseDAO: executeCached: [" << qry->lastQuery() << "]";
		if ( r && qry->first() ) {
			result = qry->value(0);
		}
		if ( !r ) {
			writeDbMessages(qry.data());
		} else {
			BaseDAO::m_cachedSimpleQuerys[sql] = result;
		}
	}
	return r;
}

/*!
  Genera un archivo de bloqueo en BBDD
  */
int BaseDAO::newLock (const QString &tableName, const QString &userName, const QVariant &pk, const QString &connection)
{
	QString pkSerialize = serializePk(pk);
	QString sqlCheck = QString("SELECT count(*) FROM %1_locks WHERE tablename = '%2'"
							   " AND pk_serialize = '%3'").arg(configuracion.systemTablePrefix()).arg(tableName).arg(pkSerialize);
	QString sql = QString("INSERT INTO %1_locks (tablename, username, pk_serialize) VALUES ('%2', '%3', '%4')").
				arg(configuracion.systemTablePrefix()).arg(tableName).arg(userName).arg(pkSerialize);
	QString sqlId = QString("SELECT MAX (id) FROM %1_locks").arg(configuracion.systemTablePrefix());
	QVariant vId, count;
	int id = -1;

	if ( execute (sqlCheck, count) ) {
		int iCount = count.toInt();
		if ( iCount == 0 ) {
			if ( transaction() ) {
				if ( execute (sql, connection) && execute (sqlId, vId, connection) ) {
					commit();
					id = vId.toInt();
				} else {
					rollback();
				}
			}
		}
	}
	return id;
}

/*!
  Desbloquea un registro de base de datos
  */
bool BaseDAO::unlock (int id, const QString &connection)
{
	bool result = false;
	QScopedPointer<QSqlQuery> qry (new QSqlQuery(Database::getQDatabase(connection)));
	QString sql = QString("DELETE FROM %1_locks WHERE id = :id").arg(configuracion.systemTablePrefix());

	result = qry->prepare(sql);
	qry->bindValue(":id", id);
	if ( result ) result = qry->exec();
	qDebug() << "BaseDAO: unlock: [" << qry->lastQuery() << "]";
	return result;
}

/*!
  Devuelve la información de bloqueo que existen sobre registros
  */
bool BaseDAO::lockInformation(const QString &tableName, const QVariant &pk, QHash<QString, QVariant> &information, const QString &connection)
{
	QString sql;
	bool result = false;
	QScopedPointer<QSqlQuery> qry (new QSqlQuery(Database::getQDatabase(connection)));

	sql = QString("SELECT id, tablename, username, pk_serialize, ts FROM %1_locks WHERE tablename = :tablename and pk_serialize = :pk_serialize").
			arg(configuracion.systemTablePrefix());
	result = qry->prepare(sql);
	qry->bindValue(":tablename", tableName);
	qry->bindValue(":pk_serialize", serializePk(pk));
    if ( result ) result = qry->exec();
	qDebug() << "BaseDAO: lockInformation: [" << qry->lastQuery() << "]";
    if ( result && qry->size() > 0 && qry->first() ) {
		information["id"] = qry->value(0).toInt();
		information["tablename"] = qry->value(1).toString();
		information["username"] = qry->value(2).toString();
		information["pk_serialize"] = qry->value(3).toString();
		information["ts"] = qry->value(4).toDateTime();
		result = true;
	}
	if ( !result ) {
		writeDbMessages(qry.data());
	}
	return result;
}

/*!
  Comprueba que un bloqueo siga siendo válido: es decir, pertenece al mismo usuario
  */
bool BaseDAO::isLockValid(int id, const QString &tableName, const QString &userName, const QVariant &pk, const QString &connection)
{
    bool result;
	QScopedPointer<QSqlQuery> qry (new QSqlQuery(Database::getQDatabase(connection)));
	QString sql = QString("SELECT id, tablename, username, pk_serialize FROM %1_locks WHERE id = :id").arg(configuracion.systemTablePrefix());
	result = qry->prepare(sql);
	qry->bindValue(":id", id);
    if ( result ) result = qry->exec();
	qDebug() << "BaseDAO: isLockValid: [" << qry->lastQuery() << "]";
    if ( result && qry->first() ) {
		QString pkSerialized = serializePk(pk);
		if ( qry->value(1).toString() == tableName && qry->value(2).toString() == userName &&
			 qry->value(3).toString() == pkSerialized ) {
			result = true;
		}
	}
	if ( ! result ) {
		writeDbMessages(qry.data());
	}
	return result;
}

QString BaseDAO::serializePk(const QVariant &pk)
{
	QString result;
	QMapIterator<QString, QVariant> pkIterator(pk.toMap());

	while (pkIterator.hasNext()) {
		pkIterator.next();
		if ( !result.isEmpty() ) {
			result = QString("%1;").arg(result);
		}
		if ( pkIterator.value().type() == QVariant::Int ) {
			result = QString("%1: %2").arg(pkIterator.key()).arg(pkIterator.value().toInt());
		} else if ( pkIterator.value().type() == QVariant::Double ) {
			result = QString("%1: %2").arg(pkIterator.key()).arg(pkIterator.value().toDouble());
		} else if ( pkIterator.value().type() == QVariant::String ) {
			result = QString("%1: \"%2\"").arg(pkIterator.key()).arg(pkIterator.value().toString());
		} else if ( pkIterator.value().type() == QVariant::Date ) {
			result = QString("%1: %2").arg(pkIterator.key()).arg(pkIterator.value().toDate().toString("YYYY-MM-DD"));
		}
	}
	return result;
}

bool BaseDAO::selectField(DBField *fld, const QString &connection)
{
    bool result;
    QScopedPointer<QSqlQuery> qry (new QSqlQuery(Database::getQDatabase(connection)));
    QString sql = QString("SELECT %1 FROM %2 WHERE %3").arg(fld->metadata()->dbFieldName()).
            arg(fld->bean()->metadata()->tableName()).arg(fld->bean()->sqlWherePk());
    result = qry->prepare(sql);
    if ( result ) result = qry->exec();
	qDebug() << "BaseDAO: selectField: [" << sql << "]";
    if ( result && qry->first() ) {
        if ( fld->metadata()->type() == QVariant::Pixmap ) {
            QByteArray ba = qry->value(0).toByteArray();
            fld->setValueFromInternal(QByteArray::fromBase64(ba));
        } else {
            fld->setValueFromInternal(qry->value(0).toString());
        }
        result = true;
    }
	if ( !result ) {
		writeDbMessages(qry.data());
	}
    return result;
}

/*!
  Función útil para crear una nueva versión de un bean a partir de otro
  */
void BaseDAO::copyBaseBean(const QSharedPointer<BaseBean> &orig, const QSharedPointer<BaseBean> &dest, bool copy1MRelationChils)
{
	QStringList relations;
	if ( orig->metadata()->tableName() != dest->metadata()->tableName() ) {
		return;
	}
	if ( copy1MRelationChils ) {
		QList<DBRelation *> rels = orig->relations();
		foreach ( DBRelation *rel, rels ) {
			if ( rel->metadata()->includeOnCopy() ) {
				relations.append(rel->metadata()->tableName());
			}
		}
	}
	BaseDAO::copyBaseBean(orig, dest, relations);
}

/*!
  Función útil para crear una nueva versión de un bean a partir de otro
  */
void BaseDAO::copyBaseBean(const QSharedPointer<BaseBean> &orig, const QSharedPointer<BaseBean> &dest, const QStringList &relationsChildsToCopy)
{
	if ( orig->metadata()->tableName() != dest->metadata()->tableName() ) {
		return;
	}
	QList<DBField *> flds = orig->fields();
	dest->blockSignals(true);
	foreach ( DBField *fld, flds ) {
		if ( !fld->metadata()->calculated() && !fld->metadata()->serial() && !fld->metadata()->primaryKey() ) {
			dest->setFieldValue(fld->metadata()->index(), fld->value());
		} else if ( fld->metadata()->primaryKey() ) {
			dest->setFieldValue(fld->metadata()->dbFieldName(), dest->defaultFieldValue(fld->metadata()->dbFieldName()));
		}
	}
	foreach ( QString relation, relationsChildsToCopy ) {
		DBRelation *rel = orig->relation(relation);
		DBRelation *relDest = dest->relation(rel->metadata()->tableName());
		if ( relDest != NULL && rel->metadata()->type() == DBRelationMetadata::ONE_TO_MANY ) {
			BaseBeanPointerList childs = rel->childs();
			QSharedPointer<BaseBean> childDest = relDest->newChild();
			foreach ( QSharedPointer<BaseBean> child, childs ) {
				copyBaseBean(child, childDest);
			}
		}
	}
	dest->blockSignals(false);
}

/*!
  Tras insertar un registro, se comprueba si tiene campos seriales, y si es así, se lee el dato serial
  actualizando el bean
  */
void BaseDAO::readSerialValuesAfterInsert(BaseBean *bean, int oid, const QString &connection)
{
	QList<DBField *> fields = bean->fields();
	QScopedPointer<QSqlQuery> qry (new QSqlQuery(Database::getQDatabase(connection)));
	QString where, sql;

	if ( oid == -1 ) {
		foreach ( DBField *field, fields ) {
			if ( insertFieldOnUpdateSql(field, BaseBean::INSERT) ) {
				// Si el campo puede ser nulo, y no se ha dado valor, se deja a cero.
				if ( ! ( field->metadata()->null() && field->value().isNull() ) ) {
					if ( !where.isEmpty() ) {
						where = QString("%1 AND ").arg(where);
					}
					where = QString("%1%2").arg(where).arg(field->sqlWhere("="));
				}
			}
		}
	} else {
		where = QString("oid = %1").arg(oid);
	}
	foreach ( DBField *field, fields ) {
		if ( field->metadata()->serial() ) {
			sql = QString("SELECT %1 FROM %2 WHERE %3").arg(field->metadata()->dbFieldName()).
					arg(bean->metadata()->tableName()).arg(where);
			qDebug() << "BaseDAO: readSerialValuesAfterInsert: [" << sql << "]";
			bool r = qry->prepare(sql);
			if ( r && qry->exec() && qry->first() ) {
				field->setValue( qry->value(0) );
			}
		}
	}
}

/*!
  Esta función recarga de base de datos el bean correspondiente
  */
bool BaseDAO::reloadBeanFromDB(const QSharedPointer<BaseBean> &bean)
{
	QSharedPointer<BaseBean> copy = selectByPk(bean->pkValue(), bean->metadata()->tableName());
	if ( copy.isNull() ) {
		return false;
	}
	QList<DBField *> fldsOrig = bean->fields();
	QList<DBField *> fldsRead = copy->fields();
	for ( int i = 0 ; i < fldsOrig.size() ; i++ ) {
        if ( !fldsOrig.at(i)->metadata()->memo() ) {
            if ( fldsOrig.at(i)->value() != fldsRead.at(i)->value() ) {
                fldsOrig.at(i)->setValue(fldsRead.at(i)->value());
            }
        }
	}
	foreach ( DBRelation *rel, bean->relations() ) {
		rel->unloadChildrens();
	}
	return true;
}

/**
  Recarga de base de datos los fields que estan marcados
  */
bool BaseDAO::reloadFieldChangedAfterSave(BaseBean *bean)
{
	bool result = false;
	QScopedPointer<QSqlQuery> qry (new QSqlQuery(Database::getQDatabase()));
	QList<DBFieldMetadata *> flds;
	foreach ( DBFieldMetadata *fld, bean->metadata()->fields() ) {
		if ( fld->reloadFromDBAfterSave() ) {
			flds.append(fld);
		}
	}
	if ( flds.size() == 0 ) {
		return true;
	}
	QString sqlFields = sqlSelectFieldsClausule(flds);
	QString sql = QString("SELECT %1 FROM %2 ").arg(sqlFields, bean->metadata()->tableName());
	sql = QString("%1 WHERE %2").arg(sql).arg(bean->sqlWherePk());
	bool r = qry->prepare(sql);
	qDebug() << "BaseDAO: reloadFieldChangedAfterSave: [" << sql << "]";
	if ( r && qry->exec() && qry->first() ) {
		int i = 0;
		result = true;
		bean->blockSignals(true);
		foreach ( DBField *fld, bean->fields() ) {
			if ( fld->metadata()->reloadFromDBAfterSave() ) {
				fld->setValueFromInternal(qry->value(i));
			}
		}
		bean->blockSignals(false);
	}
	return result;
}
