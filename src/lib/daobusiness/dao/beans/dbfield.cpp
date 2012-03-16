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
#include "dbfield.h"
#include <QVariant>
#include <QDate>
#include <QScriptEngine>
#include <QCoreApplication>
#include <QFile>

#include "configuracion.h"
#include "globales.h"
#include "widgets/dbbasewidget.h"
#include "dao/beans/dbfieldmetadata.h"
#include "dao/beans/basebean.h"
#include "dao/beans/basebeanmetadata.h"
#include "dao/beans/dbrelation.h"
#include "dao/beans/dbrelationmetadata.h"
#include "dao/dbfieldobserver.h"
#include "dao/observerfactory.h"
#include "qnumeric.h"

class DBFieldPrivate
{
//	Q_DECLARE_PUBLIC(DBFieldPrivate)
public:
	/** Valor del campo o registro */
	QVariant m_value;
	/** Indica si el campo se ha modificado después de su última lectura o modificación
	  en base de datos */
	bool m_modified;
	/** Almacena una referencia al bean que contiene este campo */
	BaseBean *m_bean;
	/** El valor de un DBField puede marcar la existencia de DBRelation hijos. De hecho en el XML se definen
	  así. Por tanto se guarda una referencia a la relación que mantiene con sus hijos. BaseBean también
	  guarda una referencia */
	QList<DBRelation* > m_relations;
	/** Metadatos */
	DBFieldMetadata *m;
	/** Si este es un campo memo, indica si se ha cargado de base de datos */
	bool m_memoLoaded;
	/** Si es un campo calculado, y se calcula sólo una vez, este flag guarda
	  si se ha calculado o no*/
	bool m_hasBeenCalculated;

	DBFieldPrivate();
	QVariant calculateAggregateValue();
	QVariant setDataToType(const QVariant &value);

};

DBFieldPrivate::DBFieldPrivate ()
{
	m_modified = false;
	m_bean = NULL;
	m = NULL;
	m_memoLoaded = false;
	m_hasBeenCalculated = false;
}

DBField::DBField(QObject *parent) : DBObject(parent), d(new DBFieldPrivate)
{
	d->m_bean = qobject_cast<BaseBean *>(parent);
}

DBField::~DBField()
{
	delete d;
}

DBFieldMetadata * DBField::metadata() const
{
	return d->m;
}

void DBField::setMetadata(DBFieldMetadata *m)
{
	d->m = m;
	// Ponemos m_Value, que es un variant a un valor adecuado
	d->m_value = d->setDataToType(d->m_value);
}

QList<DBRelation *> DBField::relations()
{
	return d->m_relations;
}

DBRelation * DBField::relation(const QString &tableName)
{
	foreach ( DBRelation *rel, d->m_relations ) {
		if ( rel->metadata()->tableName() == tableName) {
			return rel;
		}
	}
	return NULL;
}

void DBField::addRelation (DBRelation *rel)
{
	d->m_relations << rel;
}

void DBField::setRelations(const QList<DBRelation *> rels)
{
	d->m_relations = rels;
}

void DBField::clearRelations()
{
	d->m_relations.clear();
}

/**
  Puede existir un problema: Obtenemos de base de datos un double. El valor es cero. El QVariant
  contiene 0, pero al ponerse el valor, quizás QVariant entienda 0 como false, y su type se
  pone a QVariant::Bool. Esta función obliga a QVariant a contener el tipo definido en el metadato
  */
QVariant DBFieldPrivate::setDataToType(const QVariant &v)
{
	QVariant result;
	if ( m->type() == QVariant::Int ) {
		int temp = v.toInt();
		result = QVariant(temp);
	} else if ( m->type() == QVariant::Double ) {
		double temp = v.toDouble();
		result = QVariant(temp);
	} else if ( m->type() == QVariant::Date ) {
		QDate temp = v.toDate();
		result = QVariant(temp);
	} else if ( m->type() == QVariant::Bool ) {
		bool temp = v.toBool();
		result = QVariant(temp);
	} else if ( m->type() == QVariant::String ) {
		QString temp = v.toString();
		result = QVariant(temp);
	} else if ( m->type() == QVariant::Pixmap ) {
        return v;
	} else {
		qDebug() << "DBField::setDataToType: field " << m->dbFieldName() << " tiene un tipo no determinado: " << v.typeName();
		result = v;
	}
	return result;
}

/*!
  Formatea la salida de value. Es muy útil para presentar los datos en TableView y demás. Si value
  es nulo, entonces, formatea la salida de d->m_value;
  */
QString DBField::displayValue()
{
	return d->m->displayValue(value());
}

QVariant DBField::value()
{
    if ( d->m_bean->dbState() == BaseBean::UPDATE &&
            (d->m->type() == QVariant::String || d->m->type() == QVariant::Pixmap)
			&& d->m->memo() && !d->m_memoLoaded && !d->m->calculated() ) {
        d->m_memoLoaded = BaseDAO::selectField(this);
	}
	if ( d->m->calculated() ) {
		if ( d->m->aggregate() ) {
			QVariant r = d->calculateAggregateValue();
			if ( r != d->m_value ) {
				d->m_value = r;
				emit valueModified(d->m_value);
			}
		} else {
			if ( !d->m->calculatedOneTime() || (d->m->calculatedOneTime() && !d->m_hasBeenCalculated )) {
				QVariant data = d->m->calculateValue(this);
				if ( data.isValid() && d->m_value != data ) {
					QString temp = data.toString();
					if ( temp != "nan" ) {
						d->m_value = data;
						emit valueModified(d->m_value);
						if ( d->m->calculatedOneTime() ) {
							d->m_hasBeenCalculated = true;
						}
					}
				}
			}
		}
	}
	return d->m_value;
}

QVariant DBField::defaultValue()
{
	return d->m->defaultValue(this);
}

/*!
  Almacena el valor pasado en el parámetro. Emite señal de valueModified. Está pensada
  para ser el receptor de las señales de modificación de los diferentes controles de edición.
  */
void DBField::setValue(const QVariant &newValue)
{
	QVariant data;
	QVariant v;

	// No se puede dar valor a campos calculados
	if ( d->m->calculated() ) {
		return;
	}
	v = value();
	// Puede ser que estemos estableciendo el valor desde un hash. En ese caso
	// buscamos la key que corresponde a este valor
	if ( newValue.type() == QVariant::Hash ) {
		QHashIterator<QString, QVariant> it(newValue.toHash());
		while ( it.hasNext() ) {
			it.next();
			if ( it.key() == d->m->dbFieldName() ) {
				data = it.value();
			}
		}
	} else if ( newValue.type() == QVariant::Map ) {
		QMapIterator<QString, QVariant> it(newValue.toMap());
		while ( it.hasNext() ) {
			it.next();
			if ( it.key() == d->m->dbFieldName() ) {
				data = it.value();
			}
		}
	} else {
		data = d->setDataToType(newValue);
	}
	data = d->setDataToType(data);
	// Esto evita las llamadas recursivas entre widget y field
	if ( data.isValid() && v != data ) {
		d->m_value = data;
		d->m_modified = true;
		emitValueModifiedByUser();
	}
}

/*!
  Almacena el valor pasado en el parámetro. Pero no emite señales. Es una
  función de uso interno de la aplicación.
  */
void DBField::setValueFromInternal(const QVariant &newValue)
{
	QVariant data = d->setDataToType(newValue);
	QVariant v = d->m_value;
	// Esto evita las llamadas recursivas entre widget y field
	if ( v != data ) {
		d->m_value = data;
	}
}

void DBField::emitValueModifiedByUser()
{
	switch (d->m->type()) {
	case QVariant::String:
		emit valueModified(d->m_value.toString());
		break;

	case QVariant::Date:
		emit valueModified(d->m_value.toDate());
		break;

	case QVariant::Int:
		emit valueModified(d->m_value.toInt());
		break;

	case QVariant::Double:
		emit valueModified(d->m_value.toDouble());
		break;

	case QVariant::Bool:
		emit valueModified(d->m_value.toBool());
		break;

	default:
		break;
	}
	emit valueModified(d->m_value);
	emit valueModified(d->m->dbFieldName(), d->m_value);
}

bool DBField::modified()
{
	return d->m_modified;
}

void DBField::setModified(bool value)
{
	d->m_modified = value;
}

bool DBField::memoLoaded()
{
	return d->m_memoLoaded;
}

void DBField::setMemoLoaded(bool value)
{
	d->m_memoLoaded = value;
}

QString DBField::dbFieldName() {
	return d->m->dbFieldName();
}

BaseBean * DBField::bean()
{
	return d->m_bean;
}

/*!
  Esta función formatea d->m_value de cara a que sea insertable en una SQL, obviando locales y demás.
  Así un string sería devuelto como 'Hola' con comillas incluídas si includeQuotesOnString está a true
  o sin comillas, si está a false
  */
QString DBField::sqlValue(bool includeQuotesOnString)
{
	QString result;
	if ( d->m->type() == QVariant::Int ) {
		result = QString("%1").arg(value().toInt());
	} else if ( d->m->type() == QVariant::Double ) {
		result = QString::number(value().toDouble(), 'f', DB_DOUBLE_PRECISION);
	} else if ( d->m->type() == QVariant::Date ) {
		QDate fecha = value().toDate();
		if ( includeQuotesOnString ) {
			result = QString("\'%1\'").arg(fecha.toString("yyyy-MM-dd"));
		} else {
			result = fecha.toString("yyyy-MM-dd");
		}
	} else if ( d->m->type() == QVariant::Bool ) {
		result = (value().toBool() ? QString("true") : QString("false"));
	} else if ( d->m->type() == QVariant::String ) {
		if ( includeQuotesOnString ) {
			result = QString("\'%1\'").arg(value().toString());
		} else {
			result = value().toString();
		}
	} else if ( d->m->type() == QVariant::Pixmap ) {
		result = "";
	} else {
		qCritical() << "sqlValue: DBField: " << d->m->dbFieldName() << ". No tiene definido un tipo de datos. Asignando el tipo por defecto.";
		result = QString("\'%1\'").arg(value().toString());
	}
	return result;
}

/*!
  Proporciona una claúsula where para la el campo. Útil para updates y deletes
  */
QString DBField::sqlWhere(const QString &op)
{
	QString result;

	if ( d->m->type() == QVariant::Double ) {
		result = QString ("round(%1::numeric, %2) %3 %4").arg(d->m->dbFieldName()).
				arg(DB_DOUBLE_PRECISION).arg(op).
				arg(sqlValue());
	} else {
		result = QString("%1 %2 %3").arg(d->m->dbFieldName()).
					 arg(op).arg(sqlValue());
	}
	return result;
}

/*!
  Pasa un checkeo de filtro. Indica si el valor pasado corresponde a parte del valor almacenado
  (caso de una cadena) o es igual, caso de números, fechas... Muy útil para proxys de filtros
  Existen algunas reglas:
  -Las cadenas deben de ir entre comillas.
  -Para cadenas, se pueden utilizar expresiones regulares
  -Los rangos de fechas deberán ir separados por "-", en formato dd/MM/yyy de forma "01/01/2010-12/12/2012"
  TODO: Pasar a formatos de fechas internacionales
  */
bool DBField::checkValue(const QVariant &chkValue, const QString &op, Qt::CaseSensitivity cs)
{
	bool result;

	if ( d->m->type() == QVariant::Int ) {
		if ( op == "<" ) {
			return ( chkValue.toInt() > value().toInt() );
		} else if ( op == ">" ) {
			return ( chkValue.toInt() < value().toInt() );
		} else if ( op == "=" ) {
			return ( chkValue.toInt() == value().toInt() );
		} else if ( op == "!=" ) {
			return ( chkValue.toInt() != value().toInt() );
		} else {
			return false;
		}
	} else if ( d->m->type() == QVariant::Double ) {
		if ( op == "<" ) {
			return ( chkValue.toDouble() > value().toDouble() );
		} else if ( op == ">" ) {
			return ( chkValue.toDouble() < value().toDouble() );
		} else if ( op == "=" ) {
			return ( chkValue.toDouble() == value().toDouble() );
		} else if ( op == "!=" ) {
			return ( chkValue.toDouble() != value().toDouble() );
		} else {
			return false;
		}
	} else if ( d->m->type() == QVariant::Date ) {
		if ( chkValue.toString().contains("-") ) {
			QStringList dates = chkValue.toString().split("-");
			QDate date1 = QDate::fromString(dates.at(0), "dd/MM/yyyy");
			QDate date2 = QDate::fromString(dates.at(1), "dd/MM/yyyy");
			return (value().toDate() >= date1 && value().toDate() <= date2 );
		} else {
			if ( op == "<" ) {
				return ( value().toDate() < chkValue.toDate());
			} else if ( op == ">" ) {
				return ( value().toDate() > chkValue.toDate() );
			} else if ( op == "=" ) {
				return ( value().toDate() == chkValue.toDate() );
			} else if ( op == "!=" ) {
				return ( value().toDate() != chkValue.toDate() );
			} else {
				return false;
			}
		}
	} else if ( d->m->type() == QVariant::Bool ) {
		bool invert = ( op == "!=" ? true : false );
		result = (chkValue.toBool() == value().toBool());
		if ( invert ) result = !result;
	} else if ( d->m->type() == QVariant::String ) {
		QString tmpChkValue = chkValue.toString().remove("\"");
		tmpChkValue = tmpChkValue.remove("\'");
		QRegExp tmp(tmpChkValue.toLower());
		bool invert = ( op == "!=" ? true : false );
		if ( tmp.isValid() ) {
			int index = tmp.indexIn(value().toString().toLower());
			result = ( index == -1 ? false : true );
		} else {
			result = value().toString().compare(chkValue.toString(), cs);
		}
		if ( invert ) result = !result;
	} else {
        qCritical() << "checkValue: DBField: " << d->m->dbFieldName() << ". No tiene definido un tipo de datos. Asignando el tipo por defecto.";
		bool invert = ( op == "!=" ? true : false );
		result = value().toString().contains(chkValue.toString(), Qt::CaseInsensitive);
		if ( invert ) result = !result;
	}
	return result;
}

/*!
  Función adecuada para realizar un chequeo BETWEEN entre dos valores. Tiene sentido
  para campos de tipo Numérico, o fecha
  */
bool DBField::checkValue(const QVariant &value1, const QVariant &value2)
{
    if ( d->m->type() == QVariant::Int ) {
        return ( value().toInt() >= value1.toInt() && value().toInt() <= value2.toInt() );
    } else if ( d->m->type() == QVariant::Double ) {
        return ( value().toDouble() >= value1.toDouble() && value().toDouble() <= value2.toDouble() );
    } else if ( d->m->type() == QVariant::Date ) {
        bool result = value().toDate() >= value1.toDate() && value().toDate() <= value2.toDate();
        return result;
    } else if ( d->m->type() == QVariant::Bool ) {
        return (value().toBool() == value1.toBool() && value().toBool() == value2.toBool() );
    } else if ( d->m->type() == QVariant::String ) {
        return (value().toString() >= value1.toString() && value().toString() <= value2.toString() );
    } else {
        qCritical() << "checkValue: DBField: " << d->m->dbFieldName() << ". No tiene definido un tipo de datos. Asignando el tipo por defecto.";
        return false;
    }
}

/*!
  Si el field es calculated y aggregated, se ejecuta esta función para obtener su valor. La idea es que
  el valor de este field se calcula a partir de una operación con el resto de beans iguales a este
  y según un field: Por ejemplo, para la suma de un total
  */
QVariant DBFieldPrivate::calculateAggregateValue()
{
	int countResult = 0;
	double sumResult = 0;
	QVariant result;
	QList<QHash<QString, QString> > calcs = m->aggregateCalcs();

	for ( int i = 0 ; i < calcs.size() ; i++ ) {
		QHash<QString, QString> hash = calcs.at(i);
		DBRelation *rel = m_bean->relation(hash.value("relation"));
		if ( rel != NULL ) {
			BaseBeanPointerList beans = rel->childsByFilter(hash.value("filter"));
			foreach ( QSharedPointer<BaseBean> bean, beans ) {
				if ( bean->dbState() != BaseBean::TO_BE_DELETED ) {
					DBField *fld = bean->field(hash.value("field"));
					if ( fld != NULL ) {
						countResult++;
						sumResult = sumResult + fld->value().toDouble();
					}
				}
			}
		} else {
			qDebug() << "DBField::calculateAggregateValue: nombre de la relación incorrecta: " << hash.value("relation");
		}
	}
	if ( m->aggregateOperation() == "sum" ) {
		result = QVariant(sumResult);
	} else if ( m->aggregateOperation() == "count" ) {
		result = QVariant(countResult);
	} else if ( m->aggregateOperation() == "avg" ) {
		result = QVariant (sumResult / countResult);
	}
	return result;
}

/*!
  Tenemos que decirle al motor de scripts, que DBSearchDlg se convierte de esta forma a un valor script
  */
QScriptValue DBField::toScriptValue(QScriptEngine *engine, DBField* const &in)
{
	return engine->newQObject(in);
}

void DBField::fromScriptValue(const QScriptValue &object, DBField* &out)
{
	out = qobject_cast<DBField *>(object.toQObject());
}

QScriptValue DBField::toScriptValueSharedPointer(QScriptEngine *engine, const QSharedPointer<DBField> &in)
{
	return engine->newQObject(in.data());
}

void DBField::fromScriptValueSharedPointer(const QScriptValue &object, QSharedPointer<DBField> &out)
{
	out = QSharedPointer<DBField>(qobject_cast<DBField *>(object.toQObject()));
}

/*!
  Para permitir hacer comparaciones entre campos
  */
bool DBField::operator < (DBField &field)
{
	if ( d->m->type() == QVariant::Int ) {
		return ( value().toInt() < field.value().toInt() );
	} else if ( d->m->type() == QVariant::Double ) {
		return (  value().toDouble() < field.value().toDouble() );
	} else if ( d->m->type() == QVariant::Date ) {
		return ( value().toDate() < field.value().toDate());
	} else if ( d->m->type() == QVariant::Bool ) {
		return ( field.value().toBool() != value().toBool() );
	} else if ( d->m->type() == QVariant::String ) {
		return ( value().toString() < field.value().toString() );
	}
	return false;
}

bool DBField::blockSignals ( bool block )
{
	return QObject::blockSignals(block);
}
