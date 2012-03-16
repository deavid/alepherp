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
#include "dbfieldmetadata.h"
#include "configuracion.h"
#include "dao/beans/basebean.h"
#include "dao/beans/basebeanmetadata.h"
#include "dao/beans/dbfield.h"
#include "scripts/perpscript.h"
#include <QDate>

class DBFieldMetadataPrivate
{
//	Q_DECLARE_PUBLIC(DBFieldMetadata)
public:
	/** Número cardinal que identifica al field */
	int m_index;
	/** Nombre en base de datos de la columna donde se almacena el dato */
	QString m_dbFieldName;
	/** Nombre interno de la aplicación con el que se accede a este método */
	QString m_fieldName;
	/** Indica si es un campo de sólo lectura */
	bool m_readOnly;
	/** ¿Puede este campo ser nulo ? */
	bool m_null;
	/** ¿Es este campo una primary key? */
	bool m_primaryKey;
	/** Tipo del campo, segun los valores de QVariant */
	enum QVariant::Type m_type;
	/** Longitud del campo */
	int m_length;
	/** Valor por defecto */
	QVariant m_defaultValue;
	/** Longitud de la parte entera si es un numero */
	int m_partI;
	/** Longitud de la parte decimal si es un numero */
	int m_partD;
	/** Indica si es autonumerico */
	bool m_serial;
	/** Indica si se visualiza en los modelos como columna */
	bool m_visibleGrid;
	/** Para campos cuyos valores estan fijos se utiliza esto: son los valores que ve el usuario y los que se almacenan en base de datos */
	QMap<QString, QString> m_optionsList;
	/** Para campos cuyos valores estan fijos se utiliza esto: son los valores que ve el usuario y los que se almacenan en base de datos */
	QMap<QString, QString> m_optionsIcons;
	/** Algunos campos pueden ser calculados */
	bool m_calculated;
	/** Pero ese cálculo puede que sólo se haga una vez y quede almacenado, para optimizar
	  recursos */
	bool m_calculatedOneTime;
	/** Un campo calculado, podría querer guardarse en base de datos... Raro, pero puede ocurrir, si es fruto de un
	  cálculo complejo y quiere ahorrarse trabajo en informes, por ejemplo */
	bool m_calculatedSaveOnDB;
	/** Algunos campos podrán ser calculados, a partir de los datos del propio bean.
	  El cálculo de esos campos se realiza mediante el script que almacena esta variable. */
	QString m_script;
	/** El campo calculado puede ser, un aggregate, esto es, su valor depende de otros beans como él
	  que pertenecen al bean referencia. Por ejemplo, una suma de un determinado campo. Este campo
	  marca si lo es, y sobre qué field realiza el cálculo */
	bool m_aggregate;
	/** Conjunto: relación, campo y filtro. El conjunto proporciona la cuenta */
	QList<QHash<QString, QString> > m_aggregateCalcs;
	/** Tipo de operación de agregado que se realiza. Puede ser suma (sum), media (avg), cuenta (count)... */
	QString m_aggregateOperation;
	/** Si el campo es un String, indicará si almacena código HTML */
	bool m_html;
	/** Script a ejecutar para obtener el default value */
	QString m_scriptDefaultValue;
	/** Flag para activar el debug de los scripts de cálculo de este field */
	bool m_debug;
	/** Flag para activar el debug de los scripts de cálculo de este field */
	bool m_onInitDebug;
	QList<DBRelationMetadata *> m_relations;
	/** Para el cálculo de los campos calculados */
	PERPScript *m_engine;
	/** Indica si el campo es un memo. Es necesario que type sea QVariant::String */
	bool m_memo;
	/** Script para las reglas de validación antes de guardar en base de datos. Por ejemplo, esta regla de validación
	  tiene en cuenta si la edad es mayor de 18
	<field>
		<name>fecha_nacimiento</name>
		<alias>Fecha de Nacimento</alias>
		<null>false</null>
		<pk>false</pk>
		<type>date</type>
		<visiblegrid>true</visiblegrid>
		<validationRule>
<![CDATA[
// Esta función devuelve un array con lo siguientes 2 valores:
// 1.- true o false si la validación es correcta
// 2.- Mensaje formateados a presentar al usuario
// Ej:
// return [false, "<i>El cliente no tiene la edad suficiente</i>"];
function validationRule() {
	var hoy=new Date();
	var edad;
	var ano = fecha.getFullYear();
	var mes = fecha.getMonth();
	var dia = fecha.getDay()
	edad=hoy.getFullYear() - 1- ano;
	if (hoy.getMonth() - mes > 0){
		edad = edad+1;
	}

	if ((hoy.getMonth() == mes) && (hoy.getDay() - 1 - dia >= 0)){
		edad = edad + 1
	}
	if (edad<18){
		return [false, "<i>El cliente no tiene la edad suficiente</i>"];
	}else{
		return [true, ""];
	}
}
]]>
		</validationRule>
	</field>
</table>
*/
	QString m_validationRuleScript;
	/** Fuente a presentar en los grid */
	QFont m_fontOnGrid;
	QColor m_color;
	QColor m_backgroundColor;
	bool m_reloadFromDBAfterSave;

	DBFieldMetadataPrivate () {
		m_index = 0;
		m_readOnly = false;
		m_null = false;
		m_primaryKey = false;
		m_type = QVariant::Int;
		m_length = 0;
		m_partI = 0;
		m_partD = 0;
		m_serial = false;
		m_calculated = false;
		m_calculatedOneTime = false;
		m_calculatedSaveOnDB = false;
		m_html = false;
		m_visibleGrid = true;
		m_debug = false;
		m_aggregate = false;
		m_memo = false;
		m_onInitDebug = false;
		m_debug = false;
		m_onInitDebug = false;
		m_reloadFromDBAfterSave = false;
	}
};

DBFieldMetadata::DBFieldMetadata(QObject *parent) : QObject(parent), d(new DBFieldMetadataPrivate)
{
	d->m_engine = new PERPScript(this);
	d->m_engine->setDebug(d->m_debug);
	d->m_engine->setOnInitDebug(d->m_onInitDebug);
}

DBFieldMetadata::~DBFieldMetadata()
{
	delete d;
}

int DBFieldMetadata::index()
{
	return d->m_index;
}

void DBFieldMetadata::setIndex(int value)
{
	d->m_index = value;
}

QString DBFieldMetadata::dbFieldName()
{
	return d->m_dbFieldName;
}

void DBFieldMetadata::setDbFieldName(const QString &name)
{
	d->m_dbFieldName = name;
	d->m_engine->setScriptName(QString("%1.%2").arg(beanMetadata()->tableName()).arg(name));
}

bool DBFieldMetadata::calculated()
{
	return d->m_calculated;
}

void DBFieldMetadata::setCalculated(bool value)
{
	d->m_calculated = value;
}

QFont DBFieldMetadata::fontOnGrid()
{
	return d->m_fontOnGrid;
}

void DBFieldMetadata::setFontOnGrid(const QFont &font)
{
	d->m_fontOnGrid = font;
}

QColor DBFieldMetadata::color()
{
	return d->m_color;
}

void DBFieldMetadata::setColor(const QColor &color)
{
	d->m_color = color;
}

QColor DBFieldMetadata::backgroundColor()
{
	return d->m_backgroundColor;
}

void DBFieldMetadata::setBackgroundColor(const QColor &color)
{
	d->m_backgroundColor = color;
}

bool DBFieldMetadata::reloadFromDBAfterSave()
{
	return d->m_reloadFromDBAfterSave;
}

void DBFieldMetadata::setReloadFromDBAfterSave(bool v)
{
	d->m_reloadFromDBAfterSave = v;
}

bool DBFieldMetadata::calculatedOneTime()
{
	return d->m_calculatedOneTime;
}

void DBFieldMetadata::setCalculatedOneTime(bool value)
{
	d->m_calculatedOneTime = value;
}

void DBFieldMetadata::setCalculatedSaveOnDb(bool value)
{
	d->m_calculatedSaveOnDB = value;
}

bool DBFieldMetadata::calculatedSaveOnDb()
{
	return d->m_calculatedSaveOnDB;
}

QString DBFieldMetadata::script()
{
	return d->m_script;
}

void DBFieldMetadata::setScript(const QString &value)
{
	d->m_script = value;
}

QString DBFieldMetadata::scriptDefaultValue()
{
	return d->m_scriptDefaultValue;
}

void DBFieldMetadata::setScriptDefaultValue(const QString &string)
{
	d->m_scriptDefaultValue = string;
}

bool DBFieldMetadata::aggregate()
{
	return d->m_aggregate;
}

void DBFieldMetadata::setAggregate(bool value)
{
	d->m_aggregate = value;
}

QList<QHash<QString, QString> > DBFieldMetadata::aggregateCalcs() const
{
	return d->m_aggregateCalcs;
}

void DBFieldMetadata::addAggregateCalc(const QString &relation, const QString &field, const QString &filter)
{
	QHash<QString, QString> hash;
	hash["relation"] = relation;
	hash["field"] = field;
	hash["filter"] = filter;
	d->m_aggregateCalcs << hash;
}

QString DBFieldMetadata::aggregateOperation() const
{
	return d->m_aggregateOperation;
}

void DBFieldMetadata::setAggregateOperation(const QString &value)
{
	d->m_aggregateOperation = value;
}

QString DBFieldMetadata::fieldName()
{
	return d->m_fieldName;
}

void DBFieldMetadata::setFieldName(const QString &name)
{
	d->m_fieldName = trUtf8(name.toUtf8());
}

bool DBFieldMetadata::readOnly()
{
	return d->m_readOnly;
}

void DBFieldMetadata::setReadOnly(bool value)
{
	d->m_readOnly = value;
}

bool DBFieldMetadata::html()
{
	return d->m_html;
}

void DBFieldMetadata::setHtml(bool value)
{
	d->m_html = value;
}

bool DBFieldMetadata::debug()
{
	return d->m_debug;
}

void DBFieldMetadata::setDebug(bool value)
{
	d->m_debug = value;
	d->m_engine->setDebug(d->m_debug);
}

bool DBFieldMetadata::onInitDebug()
{
	return d->m_onInitDebug;
}

void DBFieldMetadata::setOnInitDebug(bool value)
{
	d->m_onInitDebug = value;
	d->m_engine->setOnInitDebug(d->m_onInitDebug);
}

bool DBFieldMetadata::null()
{
	return d->m_null;
}

void DBFieldMetadata::setNull(bool value)
{
	d->m_null = value;
}

bool DBFieldMetadata::primaryKey()
{
	return d->m_primaryKey;
}

void DBFieldMetadata::setPrimaryKey(bool value)
{
	d->m_primaryKey = value;
}

enum QVariant::Type DBFieldMetadata::type()
{
	return d->m_type;
}

void DBFieldMetadata::setType(enum QVariant::Type value)
{
	d->m_type = value;
}

int DBFieldMetadata::length()
{
	return d->m_length;
}

void DBFieldMetadata::setLength(int value)
{
	d->m_length = value;
}

int DBFieldMetadata::partI()
{
	return d->m_partI;
}

void DBFieldMetadata::setPartI(int value)
{
	d->m_partI = value;
}
int DBFieldMetadata::partD()
{
	return d->m_partD;
}
void DBFieldMetadata::setPartD(int value)
{
	d->m_partD = value;
}

bool DBFieldMetadata::serial()
{
	return d->m_serial;
}

void DBFieldMetadata::setSerial(bool value)
{
	d->m_serial = value;
}

bool DBFieldMetadata::visibleGrid()
{
	return d->m_visibleGrid;
}

void DBFieldMetadata::setVisibleGrid(bool value)
{
	d->m_visibleGrid = value;
}

QString DBFieldMetadata::validationRuleScript()
{
	return d->m_validationRuleScript;
}

void DBFieldMetadata::setValidationRuleScript(const QString &value)
{
	d->m_validationRuleScript = value;
}

bool DBFieldMetadata::memo()
{
	return d->m_memo;
}

void DBFieldMetadata::setMemo(bool value)
{
	d->m_memo = value;
}

QMap<QString, QString> DBFieldMetadata::optionsList()
{
	return d->m_optionsList;
}

void DBFieldMetadata::setOptionList(const QMap<QString, QString> &optionList)
{
	d->m_optionsList = optionList;
}

QMap<QString, QString> DBFieldMetadata::optionsIcons()
{
	return d->m_optionsIcons;
}

void DBFieldMetadata::setOptionIcons(const QMap<QString, QString> &optionList)
{
	d->m_optionsIcons = optionList;
}

QList<DBRelationMetadata *> DBFieldMetadata::relations()
{
	return d->m_relations;
}

void DBFieldMetadata::addRelation(DBRelationMetadata *rel)
{
	d->m_relations.append(rel);
}

BaseBeanMetadata *DBFieldMetadata::beanMetadata()
{
	return qobject_cast<BaseBeanMetadata *>(parent());
}

/*!
  Formatea la salida de data, según la configuración del campo
  */
QString DBFieldMetadata::displayValue(QVariant data)
{
	QString result;
	if ( type() == QVariant::Int ) {
		result = configuracion.getLocale()->toString(data.toInt());
	} else if ( type() == QVariant::Double ) {
		result = configuracion.getLocale()->toString(data.toDouble(), 'f', partD());
	} else if ( type() == QVariant::Date ) {
        //result = configuracion.getLocale()->toString(data.toDate(), QLocale::ShortFormat);
        result = configuracion.getLocale()->toString(data.toDate(), "dd/MM/yyyy");
	} else {
		result = data.toString();
	}
	return result;
}

/*!
  Valor por defecto del campo cuando se crea un nuevo bean
  */
QVariant DBFieldMetadata::defaultValue(DBField *parent)
{
	QVariant data;
	if ( !d->m_scriptDefaultValue.isEmpty() ) {
		QVariant result;
		d->m_engine->addAvailableObject("bean", qobject_cast<QObject *> (parent->bean()));
		d->m_engine->setScript(d->m_scriptDefaultValue);
		data = d->m_engine->evaluateQsFunction(QString("defaultValue"));
	} else {
		data = d->m_defaultValue;
	}

	return data;
}

void DBFieldMetadata::setDefaultValue(const QVariant &value)
{
	d->m_defaultValue = value;
}

QVariant DBFieldMetadata::calculateValue(DBField *parent)
{
	// Al ser un campo calculado, ejecutamos el script asociado. Este se ejecuta
	// sobre el bean, de modo, que parent debe estar visible en el script
	QVariant data;
	d->m_engine->addAvailableObject("bean", qobject_cast<QObject *> (parent->bean()));
	d->m_engine->setScript(d->m_script);
	data = d->m_engine->evaluateQsFunction(QString("value"));
	return data;
}

/*!
  Esta función formatea value de cara a que sea insertable en una SQL, obviando locales y demás.
  Así un string sería devuelto como 'Hola' con comillas incluídas
  */
QString DBFieldMetadata::sqlValue(const QVariant &value)
{
	QString result;
	if ( type() == QVariant::Int ) {
		result = QString("%1").arg(value.toInt());
	} else if ( type() == QVariant::Double ) {
		result = QString("%1").arg(value.toDouble());
	} else if ( type() == QVariant::Date ) {
		QDate fecha = value.toDate();
		result = QString("\'%1\'").arg(fecha.toString("yyyy-MM-dd"));
	} else if ( type() == QVariant::Bool ) {
		result = (value.toBool() ? QString("true") : QString("false"));
	} else if ( type() == QVariant::String ) {
		result = QString("\'%1\'").arg(value.toString());
	} else {
		qCritical() << "sqlValue: DBField: " << dbFieldName() << ". No tiene definido un tipo de datos. Asignando el tipo por defecto.";
		result = QString("\'%1\'").arg(value.toString());
	}
	return result;
}

/*!
  Proporciona una claúsula where para la el campo. Útil para updates y deletes
  */
QString DBFieldMetadata::sqlWhere(const QString &op, const QVariant &value)
{
	QString result;

	result = QString("%1 %2 %3").arg(dbFieldName()).
				 arg(op).arg(sqlValue(value));
	return result;
}

/*!
  Ejecuta la regla de validación asociada al DBField si hay alguna
  */
QString DBFieldMetadata::validateRule(DBField *parent, bool &validate)
{
	// Al ser un campo calculado, ejecutamos el script asociado. Este se ejecuta
	// sobre el bean, de modo, que parent debe estar visible en el script
	QString message = "";
	if ( d->m_validationRuleScript.isEmpty() ) {
		validate = true;
		return message;
	}
	d->m_engine->addAvailableObject("bean", qobject_cast<QObject *> (parent->bean()));
	d->m_engine->setScript(d->m_validationRuleScript);
	QVariant temp = d->m_engine->evaluateQsFunction(QString("validationRule"));
	if ( temp.isValid() ) {
		if ( temp.type() == QVariant::List ) {
			QVariantList list = temp.toList();
			if ( list.size() < 2 ) {
				validate = false;
			} else {
				validate = list.at(0).toBool();
				message = list.at(1).toString();
			}
		} else {
			validate = temp.toBool();
		}
	}
	return message;
}
