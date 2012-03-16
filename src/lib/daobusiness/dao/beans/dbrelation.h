/***************************************************************************
 *   Copyright (C) 2010 by David Pinelo   *
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

#ifndef DBRELATION_H
#define DBRELATION_H

#include <QObject>
#include <QScriptValue>
#include <QScriptEngine>
#include "dao/basedao.h"
#include "dao/dbobject.h"

class BaseBean;
class BeansFactory;
class DBField;
class DBRelationMetadata;
class DBRelationPrivate;

/**
	Modela una relación de base de datos. Este objeto será parte de beans que tengan
	como sucesores a otros beans o como ancestros
	*/
class DBRelation : public DBObject
{
    Q_OBJECT

	friend class BaseBean;
	friend class BeansFactory;

private:
	Q_DISABLE_COPY(DBRelation)
	DBRelationPrivate *d;
	Q_DECLARE_PRIVATE(DBRelation)

	QString sqlRelation();

	void connections(const QSharedPointer<BaseBean> &child);
	void setMetadata(DBRelationMetadata *m);

	void setFather(const QSharedPointer<BaseBean> &bean);
	void setFatherForM1Relation(const QSharedPointer<BaseBean> &bean);

public:
	explicit DBRelation(QObject *parent = 0);
	~DBRelation();

	DBRelationMetadata * metadata() const;

	Q_INVOKABLE QSharedPointer<BaseBean> firstChild(const QString &order = "");
	Q_INVOKABLE QSharedPointer<BaseBean> child(int index);
	Q_INVOKABLE QSharedPointer<BaseBean> childByPk(const QVariant &id);
	Q_INVOKABLE QSharedPointer<BaseBean> childByField(const QString &dbField, const QVariant &value);
	Q_INVOKABLE QSharedPointer<BaseBean> childByFilter(const QString &filter);
	Q_INVOKABLE BaseBeanPointerList childsByFilter(const QString &filter, const QString &order = "");
	Q_INVOKABLE BaseBeanPointerList childs(const QString &order = "");
	Q_INVOKABLE int childsCount(bool includeToBeDeleted = true);
	Q_INVOKABLE QSharedPointer<BaseBean> newChild(QSharedPointer<BaseBean> bean = QSharedPointer<BaseBean>(), int pos = -1);
	Q_INVOKABLE void removeChild(QVariant pk);
	Q_INVOKABLE void removeChildByObjectName(const QString &objectName);
	Q_INVOKABLE void removeAllChilds();
	Q_INVOKABLE BaseBeanPointerList modifiedChilds();

	QSharedPointer<BaseBean> father();
	QSharedPointer<BaseBean> fatherForM1Relation();
	QString filter();
	void setFilter(const QString &filter);
	bool childrensLoaded();
	bool childrensModified();
	bool unloadChildrens(bool ignoreNotSavedBeans = true);

	QString fetchChildSqlWhere (const QString &aliasChild = QString (""));
	QString fetchFatherSqlWhere(const QString &aliasChild = QString (""));
	QString sqlChildTableAlias();

	Q_INVOKABLE DBField * masterField();
	BaseBean *ownerBean();

	static QScriptValue toScriptValueSharedPointer(QScriptEngine *engine, const QSharedPointer<DBRelation> &in);
	static void fromScriptValueSharedPointer(const QScriptValue &object, QSharedPointer<DBRelation> &out);
	static QScriptValue toScriptValue(QScriptEngine *engine, DBRelation * const &in);
	static void fromScriptValue(const QScriptValue &object, DBRelation * &out);

signals:
	/** Señal que se emite cuando algún hijo de la relación se modifica */
	void childModified(BaseBean *, bool);
	/** Señal que se emite al insertar algún hijo en la relación */
	void childInserted(BaseBean *, int pos);
	/** Se emite cuando YA se ha borrado un hijo de la relación */
	void childDeleted(BaseBean *, int pos);
	/** Se emite cuando se modifica el estado de un child de esta relación */
	void childDbStateModified(BaseBean *, int state);
	/** Señal que indica que el field de un child se ha modificado. Es muy útil cuando
	  haya que extender la modificación a los widgets asociados. La idea, estamos presentando los
	  datos de un cliente en un presupuesto. Eso se hace utilizando la relación. Se modifica el codcliente,
	  se actualiza la relación, se emite esta señal y el widget se entera de que se ha
	  modificado el valor */
	void fieldChildModified(BaseBean *, const QString &field, const QVariant &value);
	/** Ocurre cuando se borran todos los hijos de una relación (porque se haya cambiado el campo maestro */
	void rootFieldChanged();

private slots:
	void setChildrensModified(BaseBean *bean, bool value);
	void updateChildrens();
	void childFieldBeanModified(const QString &fieldName, const QVariant &value);

public slots:
	void uncheckChildrensModified();
};

Q_DECLARE_METATYPE(QList<DBRelation*>)
Q_DECLARE_METATYPE(DBRelation*)
Q_DECLARE_METATYPE(QSharedPointer<DBRelation>)

#endif // DBRELATION_H
