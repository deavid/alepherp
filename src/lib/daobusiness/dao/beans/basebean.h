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
#ifndef BASEBEAN_H
#define BASEBEAN_H

#include <QString>
#include <QVariant>
#include <QDomDocument>
#include <QScriptValue>
#include <QScriptEngine>
#include <QDateTime>
#include "dao/dbobject.h"
#include "dao/beans/basebeanvalidator.h"

class DBField;
class DBRelation;
class BaseBeanObserver;
class BaseBeanPrivate;
class BaseBeanMetadata;
class DBFieldMetadata;
class DBRelationMetadata;
class BaseDAO;

/** La idea de esta lista, es la de que se puedan crear punteros dinámicamente, y se borren
  solos al destruirse el objeto BaseBeanPointerList */
typedef QList<QSharedPointer<BaseBean> > BaseBeanPointerList;

/**
	Clase base de todos los Beans de la aplicación. Es una clase abstracta, 
	que implementa la interfaz necesaria para tratar con los Beans de forma genérica.
	Esta clase será usada cuando tratemos con un bean sin importarnos el tipo.
	@author David Pinelo <david.pinelo@alephsistemas.es>
 */
class BaseBean : public DBObject {

	Q_OBJECT
	Q_ENUMS(DbBeanStates)

	Q_PROPERTY(QList<DBField *> fields READ fields)
	Q_PROPERTY(QList<DBRelation *> relations READ relations)
	/** Flag que indica el estado en que se encuentra en base de datos */
	Q_PROPERTY(DbBeanStates dbState READ dbState WRITE setDbState)
	/** Flag que indica si en este bean han modificado sus datos (los fields) */
	Q_PROPERTY(bool modified READ modified)
	/** Flag para saber si este bean puede guardarse autonomamente en base de datos */
	Q_PROPERTY(bool canSaveOnDbDirectly READ canSaveOnDbDirectly WRITE setCanSaveOnDbDirectly)
	/** Marca de tiempo, que indica cuándo fue la última vez que se leyó el registro de base de datos */
	Q_PROPERTY(QDateTime loadTime READ loadTime WRITE setLoadTime)

	/** BaseDAO utiliza unas funciones específicas de DBField y BaseBean para así saber
	  cuándo la lectura de un dato se ha producid por lectura de base de datos, de modo
	  que no se modifique m_modified */
	friend class BaseDAO;
	friend class BeansFactory;

	void setMetadata(BaseBeanMetadata *m, bool setDefaultValue);

private:
	Q_DISABLE_COPY(BaseBean)
	BaseBeanPrivate *d;

	/** Funciones factoria para crear los datos internos de este bean */
	DBRelation *newRelation(DBRelationMetadata *m);
	/** Funciones factoria para crear los datos internos de este bean */
	DBField *newField(DBFieldMetadata *m);
	void setLoadTime(const QDateTime &time);

public:
	explicit BaseBean(QObject *parent = 0);
	virtual ~BaseBean();

	/** Valores con respecto a la base de datos en la que se encuentra el bean.
	  Los valores de la enumeración se ponen de forma binaria para poder hacer combinaciones
	  de estado */
	enum DbBeanStates { INSERT = 1, UPDATE = 2, TO_BE_DELETED = 4 };

	BaseBeanMetadata * metadata() const;

	void setDbState(DbBeanStates state);
	DbBeanStates dbState();
	bool modified();
	bool canSaveOnDbDirectly();
	void setCanSaveOnDbDirectly(bool value);
	QDateTime loadTime();

	QList<DBField *> fields();
	Q_INVOKABLE DBField * field(const QString &dbFieldName);
	Q_INVOKABLE DBField * field(int index);
	Q_INVOKABLE QList<DBField *> pkFields ();
	int fieldIndex(const QString &dbFieldName);
	int fieldCount();

	QList<DBRelation *> relations();
	Q_INVOKABLE DBRelation * relation(const QString &tableName);
	int relationIndex(const QString &relationName);
	Q_INVOKABLE BaseBeanPointerList relationChilds(const QString &relationName, const QString &order = "");
	Q_INVOKABLE QSharedPointer<BaseBean> relationChildByPk(const QString &relationName, const QVariant &id);
	Q_INVOKABLE QSharedPointer<BaseBean> relationChildByField(const QString &relationName, const QString &fieldName, const QVariant &id);
	Q_INVOKABLE QSharedPointer<BaseBean> relationFirstChild(const QString &relationName, const QString &order = "");
	Q_INVOKABLE BaseBeanPointerList relationChildsByFilter(const QString &relationName, const QString &filter, const QString &order = "");
	Q_INVOKABLE QSharedPointer<BaseBean> father(const QString &relationName);
	Q_INVOKABLE QVariant fatherFieldValue(const QString &relationName, const QString &field);
	Q_INVOKABLE QString fatherDisplayFieldValue(const QString &relationName, const QString &field);
	Q_INVOKABLE int relationChildsCount(const QString &relationName, bool includeToBeDeleted = true);

	DBObject * navigateThrough(const QString &relationName, const QString &relationFilters);

	Q_INVOKABLE QVariant fieldValue(int dbField);
	Q_INVOKABLE QVariant fieldValue(const QString &dbFieldName);
	Q_INVOKABLE virtual void setFieldValue(const QString &dbFieldName, QVariant value);
	Q_INVOKABLE virtual void setFieldValue(int index, QVariant value);

	Q_INVOKABLE QVariant pkValue();
	Q_INVOKABLE QString pkSerializedValue();
	Q_INVOKABLE bool pkEqual(const QVariant &value);
	Q_INVOKABLE void setPkValue(const QVariant &id);
	Q_INVOKABLE QVariant defaultFieldValue(const QString &dbFieldName);
	Q_INVOKABLE QString displayFieldValue(int iField);
	Q_INVOKABLE QString displayFieldValue(const QString & iField);
	Q_INVOKABLE QString sqlFieldValue(int iField);
	Q_INVOKABLE QString sqlFieldValue(const QString & iField);

	Q_INVOKABLE QString sqlWhere(const QString &fieldName, const QString &op);
	Q_INVOKABLE QString sqlWherePk();

	Q_INVOKABLE bool checkFilter(const QString &filterExpression, Qt::CaseSensitivity sensivity = Qt::CaseInsensitive);

	Q_INVOKABLE bool validate();
	Q_INVOKABLE QString validateMessages();
	Q_INVOKABLE QString validateHtmlMessages();

//	Q_INVOKABLE QVariant calculateAggregate(const QString &operation, const QString &relation,
//											const QString &dbFieldName, const QString &filter = "");

	/** Función que sólo podrán ser llamadas desde BaseDAO. Establece el valor
	  sin modificar m_modified */
	virtual void setFieldValueFromInternal(const QString &dbFieldName, QVariant value);
	/** Función que sólo podrán ser llamadas desde BaseDAO. Establece el valor
	  sin modificar m_modified */
	virtual void setFieldValueFromInternal(int index, QVariant value);

	virtual void deleteObserver();

	static QScriptValue toScriptValue(QScriptEngine *engine, BaseBean * const &in);
	static void fromScriptValue(const QScriptValue &object, BaseBean * &out);
	static QScriptValue toScriptValueSharedPointer(QScriptEngine *engine, const QSharedPointer<BaseBean> &in);
	static void fromScriptValueSharedPointer(const QScriptValue &object, QSharedPointer<BaseBean> &out);

	QSharedPointer<BaseBean> clone();

public slots:
	void uncheckModifiedFields();
    bool save(bool saveChilds = true);
	void backupValues();
	void restoreValues();
	void setSerialUniqueId();
	void clean(bool onlyFields = true, bool childs = false, bool fathers = false);

private slots:
	void setModified();
	void recalculateCalculatedFields();
	void emitFieldModified(const QString &dbFieldName, const QVariant &value);
	void setPkValueFromInternal(const QVariant &id);

signals:
	/** El bean se ha modificado, por alguna de las siguientes razones: Ha cambiado el value de algún field,
	  se ha agregado, modificado o borrado algún child, o se ha modificado algún field de algún child. Es
	  una señal muy genérica, ideal para utilizar en caso de comprobar si hay que guardar en base de datos, o
	  para actualizar la marca setWindowModified de un widget */
	void beanModified(bool value);
	void beanModified(BaseBean *, bool value);
	/** Cuando se modifica un field de este bean, se emite esta señal */
	void fieldModified(const QString &dbFieldName, const QVariant &value);
	void fieldModified(BaseBean *, const QString &dbFieldName, const QVariant &value);
	/** Cuando se modifica el estado del bean, se emite esta señal */
	void dbStateModified(int);
	void dbStateModified(BaseBean *bean, int);
	/** Cuando se modifica el estado de un child hijo, se emite esta señal */
	void dbChildStateModified(const QSharedPointer<BaseBean> &bean, int);
};

Q_DECLARE_METATYPE(BaseBean*)
Q_DECLARE_METATYPE(QSharedPointer<BaseBean>)
Q_DECLARE_METATYPE(BaseBeanPointerList)
Q_DECLARE_METATYPE(BaseBean::DbBeanStates)

#endif
