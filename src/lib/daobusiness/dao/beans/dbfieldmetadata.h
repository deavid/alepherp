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
#ifndef DBFIELDMETADATA_H
#define DBFIELDMETADATA_H

#include <QObject>
#include <QString>
#include <QVariant>
#include <QFont>
#include <QColor>

class DBRelationMetadata;
class DBFieldMetadataPrivate;
class BaseBeanMetadata;
class DBField;

class DBFieldMetadata : public QObject
{
	Q_OBJECT

	/** Número cardinal que identifica al field */
	Q_PROPERTY(int index READ index WRITE setIndex)
	/** Nombre en base de datos de la columna donde se almacena el dato */
	Q_PROPERTY(QString dbFieldName READ dbFieldName WRITE setDbFieldName)
	/** Nombre interno de la aplicación con el que se accede a este método */
	Q_PROPERTY(QString fieldName READ fieldName WRITE setFieldName)
	/** Indica si es un campo de sólo lectura */
	Q_PROPERTY(bool readOnly READ readOnly WRITE setReadOnly)
	/** ¿Puede este campo ser nulo ? True si puede serlo */
	Q_PROPERTY(bool null READ null WRITE setNull)
	/** ¿Es este campo una primary key? */
	Q_PROPERTY(bool primaryKey READ primaryKey WRITE setPrimaryKey)
	/** Longitud del campo */
	Q_PROPERTY(int length READ length WRITE setLength)
	/** Longitud de la parte entera si es un numero */
	Q_PROPERTY(int partI READ partI WRITE setPartI)
	/** Longitud de la parte decimal si es un numero */
	Q_PROPERTY(int partD READ partD WRITE setPartD)
	/** Indica si es autonumerico */
	Q_PROPERTY(bool serial READ serial WRITE setSerial)
	/** Indica si se visualiza en los modelos como columna */
	Q_PROPERTY(bool visibleGrid READ visibleGrid WRITE setVisibleGrid)
	/** Algunos campos pueden ser calculados */
	Q_PROPERTY(bool calculated READ calculated WRITE setCalculated)
	/** Los campos calculados pueden calcularse sólo una vez para mejorar el rendimiento */
	Q_PROPERTY(bool calculatedOneTime READ calculatedOneTime WRITE setCalculatedOneTime)
	/** Algunos campos podrán ser calculados, a partir de los datos del propio bean.
	  El cálculo de esos campos se realiza mediante el script que almacena esta variable
	  */
	Q_PROPERTY(QString script READ script WRITE setScript)
	/** Si el campo es un String, indicará si almacena código HTML */
	Q_PROPERTY(bool html READ html WRITE setHtml)
	/** Script a ejecutar para obtener el default value */
	Q_PROPERTY(QString scriptDefaultValue READ scriptDefaultValue WRITE setScriptDefaultValue)
	/** Flag para activar el debug de los scripts de cálculo de este field */
	Q_PROPERTY(bool debug READ debug WRITE setDebug)
	/** Indica si el script se ejecuta desde el debugger, por ejemplo paso a paso */
	Q_PROPERTY(bool onInitDebug READ onInitDebug WRITE setOnInitDebug)
	/** El campo calculado puede ser, un aggregate, esto es, su valor depende de otros beans como él
	  que pertenecen al bean referencia. Por ejemplo, una suma de un determinado campo. Este campo
	  marca si lo es, y sobre qué field realiza el cálculo */
	Q_PROPERTY(bool aggregate READ aggregate WRITE setAggregate)
	/** Indica si el campo almacena un campo MEMO. Hay que distinguirlos, porque obtenerlos de base
	  de datos es muy gravoso. */
	Q_PROPERTY(bool memo READ memo WRITE setMemo)
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
	Q_PROPERTY (QString validationRuleScript READ validationRuleScript WRITE setValidationRuleScript)
	/** En la visualización en grid, permite determinar la fuente, que aparece */
	Q_PROPERTY(QFont fontOnGrid READ fontOnGrid WRITE setFontOnGrid)
	/** Color de la fuente en el grid */
	Q_PROPERTY(QColor color READ color WRITE setColor)
	/** Background Color de la fuente en el grid */
	Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor)
	/** Tras un INSERT, o un UPDATE exitoso en base de datos, puede ser necesario, reobtener el valor
	  de un field, porque, por ejemplo, se ha asociado un trigger al mismo a ejecutar en el proceso de actualización,
	  con lo que el valor del que dispone AlephERP puede no ser el actual. Esta propiedad permite marcar
	  ese tipo de campos. Un ejemplo clásico, es un contador único que surge de una tabla de contadores, y que
	  se asigna a una tabla (por ejemplo, facturas) */
	Q_PROPERTY(bool reloadFromDBAfterSave READ reloadFromDBAfterSave WRITE setReloadFromDBAfterSave)
private:
	Q_DISABLE_COPY(DBFieldMetadata)
	DBFieldMetadataPrivate *d;
	Q_DECLARE_PRIVATE(DBFieldMetadata)

public:
	DBFieldMetadata(QObject *parent = NULL);
	~DBFieldMetadata();

	int index();
	void setIndex(int value);
	QString dbFieldName();
	void setDbFieldName(const QString &name);
	QString fieldName();
	void setFieldName(const QString &name);
	bool readOnly();
	void setReadOnly(bool value);
	bool html();
	void setHtml(bool html);
	bool debug();
	void setDebug(bool value);
	bool onInitDebug();
	void setOnInitDebug(bool value);
	bool calculated();
	void setCalculated(bool value);
	bool calculatedOneTime();
	void setCalculatedOneTime(bool value);
	void setCalculatedSaveOnDb(bool vlaue);
	bool calculatedSaveOnDb();
	QString script();
	void setScript(const QString &value);
	bool null();
	void setNull(bool value);
	bool primaryKey();
	void setPrimaryKey(bool value);
	enum QVariant::Type type();
	void setType(enum QVariant::Type value);
	int length();
	void setLength(int value);
	bool editable();
	void setEditable(bool value);
	int partI();
	void setPartI(int value);
	int partD();
	void setPartD(int value);
	bool serial();
	void setSerial(bool value);
	bool visibleGrid();
	void setVisibleGrid(bool value);
	bool memo();
	void setMemo(bool value);
	QMap<QString, QString> optionsList();
	void setOptionList(const QMap<QString, QString> &option);
	QMap<QString, QString> optionsIcons();
	void setOptionIcons(const QMap<QString, QString> &optionList);
	QString scriptDefaultValue();
	void setScriptDefaultValue(const QString &string);
	bool aggregate();
	void setAggregate(bool value);
	QList<QHash<QString, QString> > aggregateCalcs() const;
	void addAggregateCalc(const QString &relation, const QString &field, const QString &filter);
	QString aggregateOperation() const;
	void setAggregateOperation(const QString &value);
	QString validationRuleScript();
	void setValidationRuleScript(const QString &value);
	QFont fontOnGrid();
	void setFontOnGrid(const QFont &font);
	QColor color();
	void setColor(const QColor &color);
	QColor backgroundColor();
	void setBackgroundColor(const QColor &color);
	bool reloadFromDBAfterSave();
	void setReloadFromDBAfterSave(bool v);

	QVariant defaultValue(DBField *parent);
	void setDefaultValue(const QVariant &value);
	QVariant calculateValue(DBField *parent);

	QList<DBRelationMetadata *> relations();
	void addRelation(DBRelationMetadata *rel);

	BaseBeanMetadata *beanMetadata();
	QString displayValue(QVariant v);

	QString validateRule(DBField *parent, bool &validate);

	QString sqlWhere(const QString &op, const QVariant &value);
	QString sqlValue(const QVariant &value);

};

#endif // DBFIELDMETADATA_H
