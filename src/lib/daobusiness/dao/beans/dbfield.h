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
#ifndef DBFIELD_H
#define DBFIELD_H

#include <QObject>
#include <QWidget>
#include <QVariant>
#include <QDate>
#include <QScriptValue>
#include <QScriptEngine>
#include "dao/basedao.h"
#include "dao/dbfieldobserver.h"
#include "dao/dbobject.h"

class BaseBean;
class DBFieldObserver;
class PERPScript;
class DBFieldPrivate;
class DBField;
class DBFieldMetadata;

/**
  * Clase que almacena un valor de una columna de base de datos. Contiene
  * también la interacción con los campos visibles de edición
  * @author David Pinelo
  */
class DBField : public DBObject
{
    Q_OBJECT

	/** Valor del campo o registro */
    Q_PROPERTY(QVariant value READ value WRITE setValue)
	/** Valor por defecto */
    Q_PROPERTY(QVariant defaultValue READ defaultValue)
	/** Valor que se visualiza */
    Q_PROPERTY(QVariant displayValue READ displayValue)
	/** Indica si el campo se ha modificado después de su última lectura o modificación
	  en base de datos */
    Q_PROPERTY(bool modified READ modified WRITE setModified)
	/** Almacena una referencia al bean que contiene este campo */
    Q_PROPERTY(BaseBean * bean READ bean)
	/** Indica si se ha cargado el campo memo */
	Q_PROPERTY(bool memoLoaded READ memoLoaded WRITE setMemoLoaded)
	/** Nombre del campo en base de datos */
	Q_PROPERTY(QString dbFieldName READ dbFieldName)

	/** BaseDAO utiliza unas funciones específicas de DBField y BaseBean para así saber
	  cuándo la lectura de un dato se ha producid por lectura de base de datos, de modo
	  que no se modifique m_modified */
	friend class BaseDAO;
	friend class BaseBean;

private:
	Q_DISABLE_COPY(DBField)

	/** Puntero d para mejorar la compatibilidad de binarios:
	http://techbase.kde.org/Policies/Library_Code_Policy#D-Pointers
	*/
	DBFieldPrivate *d;
	Q_DECLARE_PRIVATE(DBField)

	void setMetadata(DBFieldMetadata *m);

public:
	explicit DBField(QObject *parent = 0);
	~DBField();

	DBFieldMetadata * metadata() const;

	/** Este operador sobrecargado, permitirá ordenaciones */
	bool operator < (DBField &field);

	QVariant value();
	QString displayValue();
	bool modified();
	void setModified(bool value);
	BaseBean *bean();
	QVariant defaultValue();
	bool memoLoaded();
	void setMemoLoaded(bool value);
	QString dbFieldName();

	QList<DBRelation *> relations();
	Q_INVOKABLE DBRelation *relation(const QString &tableName);
	void addRelation (DBRelation *rel);
	void setRelations(const QList<DBRelation *> rels);
	void clearRelations();

	Q_INVOKABLE QString sqlValue(bool includeQuotesOnString = true);
	Q_INVOKABLE QString sqlWhere(const QString &op);

	Q_INVOKABLE bool checkValue(const QVariant &value, const QString &op = QString("="), Qt::CaseSensitivity cs = Qt::CaseInsensitive);
    Q_INVOKABLE bool checkValue(const QVariant &value1, const QVariant &value2);

	Q_INVOKABLE bool blockSignals ( bool block );

	static QScriptValue toScriptValue(QScriptEngine *engine, DBField * const &in);
	static void fromScriptValue(const QScriptValue &object, DBField * &out);
	static QScriptValue toScriptValueSharedPointer(QScriptEngine *engine, const QSharedPointer<DBField> &in);
	static void fromScriptValueSharedPointer(const QScriptValue &object, QSharedPointer<DBField> &out);

public slots:
	/** Distinguimos entre estas funciones, que son utilizada por funciones */
	void setValue(const QVariant &value);
	void setValue(const QString &string) { QVariant dato(string); setValue(dato); }
	void setValue(const QDate &date) { QVariant dato(date); setValue(dato); }
	void setValue(int number) { QVariant dato(number); setValue(dato); }
	void setValue(double number) { QVariant dato(number); setValue(dato); }

signals:
	/** Indica que el usuario ha modificado un valor, al editar los datos directamente */
	void valueModified(const QString &fieldName, const QVariant &);
	void valueModified(const QVariant &);
	void valueModified(const QString &);
	void valueModified(QDate);
	void valueModified(int);
	void valueModified(double);
	void valueModified(bool);

private slots:
	void emitValueModifiedByUser();
	void setValueFromInternal(const QVariant &value);
};

Q_DECLARE_METATYPE(QList<DBField*>)
Q_DECLARE_METATYPE(DBField*)
Q_DECLARE_METATYPE(QSharedPointer<DBField>)

#endif // DBFIELD_H
