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
#include "basebeanmetadata.h"
#include "configuracion.h"
#include "dao/beans/basebean.h"
#include "dao/beans/dbfieldmetadata.h"
#include "dao/beans/dbrelationmetadata.h"
#include "models/envvars.h"
#include "scripts/perpscript.h"
#include <QDomDocument>
#include <QMessageBox>
#include <QDebug>
#include <QScriptValue>

class BaseBeanMetadataPrivate
{
//	Q_DECLARE_PUBLIC(BaseBeanMetadata)
public:
	/** Configuración en XML del bean: Campos, relaciones... */
	QString m_xml;
	/** Tabla de la base de datos a la que hace referencia */
	QString m_tableName;
	/** Esta es la vista o consulta que se utiliza para obtener los datos que se presentan en el ListView.
	Debe ser una lista o consulta que exista en las definiciones de tablas de PrintingERP */
	QString m_viewOnGrid;
	/** Nombre bonito de la tabla */
	QString m_alias;
	/** Listado de los fields que admite este bean */
	QList<DBFieldMetadata *> m_fields;
	/** Listado de relations que se admiten */
	QList<DBRelationMetadata *> m_relations;
	/** Consulta SQL que se utiliza para obtener los datos que será editados. */
	QHash<QString, QString> m_sql;
	/** Indica si el borrado del bean es lógico o físico. Si es lógico, el registro
	  se marcará como borrado. Para ello, la tabla deberá tener un campo "is_deleted" */
	bool m_logicalDelete;
	/** Cuando este bean se presenta en un DBForm, puede querer filtrarse mediante
	un filtro fuerte, que se deriva de los valores de otra tabla, o de los valores
	de un option list de ese campo. Por ejemplo,
	presupuestos_cabecera tiene un campo, estado, que coge datos de los registros
	marcados en la tabla presupuestos_definicion_estados. El contenido de esta tabla
	se incluirá en el combo. ¿Cómo? se introducirán valores de la siguiente forma:
	<itemsFilterColumn>
		<itemFilter>
			<fieldToFilter>id_tienda</fieldToFilter>
			<relationFieldToShow>nombre</relationFieldToShow>
			<order>another_field_on_relation_to_order</order>
		</itemFilter>
		<itemFilter>
			<fieldToFilter>codejercicio</fieldToFilter>
			<relationFieldToShow>ejercicio</relationFieldToShow>
			<order>another_field_on_relation_to_order</order>
		</itemFilter>
	<itemFilter>
		<fieldToFilter>estado</fieldToFilter>
	</itemFilter>
	</itemsFilterColumn>
	Esto sólo tendrá sentido en relaciones M1 y en campos
 */
	QStringList m_itemsFilterColumn;
	/** Presentar ciertos datos, puede estar ligado a una serie de variables globales.
	  Por ejemplo: queremos mostrar sólo los registros de que pertenecen a un único puesto
	  de trabajo. Desde Javascript se configuraría el valor de esa variable global, y al
	  mostrarse los datos de esta tabla, se harían teniendo en cuenta el filtro que esa variable
	  establece. Aquí se define la lista de variables de entorno que aplican a este bean, y a qué campos
	  La definición se haría
	  <envVars>
		<pair>
			<field>id_tienda</field>
            <varName>idTienda</varName>
		</pair>
	  <(envVars>
	  y el QStringList quedaría: "idTienda;id_tienda"
	  */
	QStringList m_envVars;
	/** Orden inicial de visualización */
	QString m_initialOrderSort;
	/** Script a ejecutar tras el borrado desde base de datos */
	QString m_afterDeleteScript;
	/** Para los scripts asociados */
	PERPScript *m_engine;
	/** Opciones de debug */
	bool m_debug;
	bool m_onInitDebug;
	/** Field de visualización por defecto */
	QString m_defaultVisualizationField;
	/** Hay ciertos beans que no cambian habitualmente. Esos beans pueden ser cacheados en memoria (por ejemplo,
	  ejercicios fiscales, definiciones de productos)... Esta propiedad marca esos beans, que por tanto
	  quedan residentes en memoria para una mejor ejecución del sistema */
	bool m_isCached;

	BaseBeanMetadataPrivate() {
		m_logicalDelete = false;
		m_debug = false;
		m_onInitDebug = false;
		m_isCached = false;
	}
};

BaseBeanMetadata::BaseBeanMetadata(QObject *parent) : QObject(parent), d(new BaseBeanMetadataPrivate)
{
	d->m_engine = new PERPScript(this);
	d->m_engine->setDebug(d->m_debug);
	d->m_engine->setOnInitDebug(d->m_onInitDebug);
}

BaseBeanMetadata::~BaseBeanMetadata()
{
	delete d;
}

QString BaseBeanMetadata::tableName()
{
	return d->m_tableName;
}

void BaseBeanMetadata::setTableName(const QString &value)
{
	d->m_tableName = value;
}

void BaseBeanMetadata::setAlias(const QString &alias)
{
	d->m_alias = alias;
}

QString BaseBeanMetadata::alias()
{
	return d->m_alias;
}

void BaseBeanMetadata::setViewOnGrid(const QString &query)
{
	d->m_viewOnGrid = query;
}

QString BaseBeanMetadata::viewOnGrid()
{
	return d->m_viewOnGrid;
}

bool BaseBeanMetadata::logicalDelete()
{
	return d->m_logicalDelete;
}

void BaseBeanMetadata::setLogicalDelete(bool value)
{
	d->m_logicalDelete = value;
}

void BaseBeanMetadata::setItemsFilterColumn(const QStringList &alias)
{
	d->m_itemsFilterColumn = alias;
}

QStringList BaseBeanMetadata::itemsFilterColumn()
{
	return d->m_itemsFilterColumn;
}

QString BaseBeanMetadata::initialOrderSort()
{
	return d->m_initialOrderSort;
}

void BaseBeanMetadata::setInitialOrderSort(const QString &value)
{
	d->m_initialOrderSort = value;
}

QString BaseBeanMetadata::afterDeleteScript()
{
	return d->m_afterDeleteScript;
}

void BaseBeanMetadata::setAfterDeleteScript(const QString &value)
{
	d->m_afterDeleteScript = value;
}

bool BaseBeanMetadata::debug()
{
	return d->m_debug;
}

void BaseBeanMetadata::setDebug(bool value)
{
	d->m_debug = value;
	d->m_engine->setDebug(d->m_debug);
}

bool BaseBeanMetadata::onInitDebug()
{
	return d->m_onInitDebug;
}

void BaseBeanMetadata::setOnInitDebug(bool value)
{
	d->m_onInitDebug = value;
	d->m_engine->setOnInitDebug(d->m_onInitDebug);
}

QString BaseBeanMetadata::defaultVisualizationField()
{
	if ( d->m_defaultVisualizationField.isEmpty() ) {
		QList<DBFieldMetadata *> pk = pkFields();
		if ( pk.size() > 0 ) {
			d->m_defaultVisualizationField = pk.at(0)->dbFieldName();
		}
	}
	return d->m_defaultVisualizationField;
}

void BaseBeanMetadata::setDefaultVisualizationField(const QString &value)
{
	d->m_defaultVisualizationField = value;
}

QStringList BaseBeanMetadata::envVars()
{
	return d->m_envVars;
}

void BaseBeanMetadata::addSqlPart(const QString &clause, const QString &value)
{
	d->m_sql[clause] = value;
}

QHash<QString, QString> BaseBeanMetadata::sql()
{
	return d->m_sql;
}

QString BaseBeanMetadata::xml()
{
	return d->m_xml;
}

QList<DBFieldMetadata *> BaseBeanMetadata::fields()
{
	return d->m_fields;
}

QList<DBRelationMetadata *> BaseBeanMetadata::relations()
{
	return d->m_relations;
}

bool BaseBeanMetadata::isCached()
{
	return d->m_isCached;
}

void BaseBeanMetadata::setIsCached(bool value)
{
	d->m_isCached = value;
}

void BaseBeanMetadata::setXml (const QString &value)
{
	d->m_xml = value;
	qDeleteAll(d->m_fields);
	d->m_fields.clear();
	qDeleteAll(d->m_relations);
	d->m_relations.clear();
	setConfig();
}

/**
Lee y establece la configuración existente de campos de este bean a partir del XML
de configuración
*/
void BaseBeanMetadata::setConfig()
{
	QString defaultValue, optionList, optionValues, optionIcons, errorString;
	int fieldIndex = 0, errorLine, errorColumn;
	bool nameSpace;

	if ( d->m_xml.isEmpty() ) {
		qCritical() << "setConfig: BaseBean: " << d->m_tableName << ". XML Vacio";
		return;
	}

	QDomDocument domDocument ;
	if ( domDocument.setContent( d->m_xml, &nameSpace, &errorString, &errorLine, &errorColumn ) )  {
		QDomElement root = domDocument.documentElement();
		QDomNode n = root.firstChildElement("name");
		if ( d->m_tableName != n.toElement().text() ) {
			qDebug() << "BaseBean: setConfig(): No coinciden el nombre de la tabla en registro y en XML.";
		}
		n = root.firstChildElement("alias");
		if ( !n.isNull() ) {
			d->m_alias = QObject::trUtf8(n.toElement().text().toUtf8());
		}
		n = root.firstChildElement("viewOnGrid");
		if ( !n.isNull() ) {
			d->m_viewOnGrid = n.toElement().text();
		}
		n = root.firstChildElement("logicalDelete");
		if ( !n.isNull() ) {
			d->m_logicalDelete = (n.toElement().text() == "true" ? true : false);
		}
		n = root.firstChildElement("initOrderSort");
		if ( !n.isNull() ) {
			d->m_initialOrderSort = n.toElement().text();
		}
		n = root.firstChildElement("defaultVisualizationField");
		if ( !n.isNull() ) {
			d->m_defaultVisualizationField = n.toElement().text();
		}
		n = root.firstChildElement("isCached");
		if ( !n.isNull() ) {
			d->m_isCached = (n.toElement().text() == "true" ? true : false);
		}
		n = root.firstChildElement("afterDeleteScript");
		if ( !n.isNull() ) {
			QDomElement element = n.toElement();
			d->m_afterDeleteScript = element.text();
			setDebug( element.attribute("debug", "false") == "true" ? true : false );
			setOnInitDebug( element.attribute("onInitDebug", "false") == "true" ? true : false );
		}

		QDomElement e = root.firstChildElement("itemsFilterColumn");
		if ( !e.isNull() ) {
			readItemsFilterColumn(e);
		}

		e = root.firstChildElement("envVars");
		if ( !e.isNull() ) {
			readEnvVarsFilter(e);
		}

		e = root.firstChildElement("sql");
		if ( !e.isNull() ) {
			readSql(e);
		}
		// Iteramos sobre todos los campos
		QDomNodeList fieldList = root.elementsByTagName("field");
		for ( int i = 0 ; i < fieldList.size() ; i++ ) {
			DBFieldMetadata *field = new DBFieldMetadata(this);
			field->setParent(this);
			QDomElement element = fieldList.at(i).toElement();
			for ( QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling() )	{
				QDomElement e = n.toElement();
				if ( e.tagName() == "name" ) {
					field->setDbFieldName(e.text());
				} else if ( e.tagName() == "alias" ) {
					field->setFieldName(e.text());
				} else if ( e.tagName() == "null" ) {
					field->setNull((e.text() == "true" ? true : false));
				} else if ( e.tagName() == "visiblegrid" ) {
					field->setVisibleGrid((e.text() == "true" ? true : false));
				} else if ( e.tagName() == "pk" ) {
					field->setPrimaryKey((e.text() == "true" ? true : false));
				} else if ( e.tagName() == "type") {
					if ( e.text() == "string" || e.text() == "stringlist" ) {
						field->setType(QVariant::String);
						if ( e.text() == "stringlist" ) {
							// -1 es longitud ilimitada
							field->setLength(-1);
							field->setMemo(true);
						}
					} else if ( e.text() == "double" ) {
						field->setType(QVariant::Double);
					} else if ( e.text() == "int") {
						field->setType(QVariant::Int);
					} else if ( e.text() == "serial" ) {
						field->setType(QVariant::Int);
						field->setSerial(true);
					} else if ( e.text() == "date" ) {
						field->setType(QVariant::Date);
					} else if ( e.text() == "bool" ) {
						field->setType(QVariant::Bool);
					} else if ( e.text() == "image" ) {
						field->setType(QVariant::Pixmap);
                        field->setMemo(true);
                    }
				} else if ( e.tagName() == "default" ) {
					defaultValue = e.text();
				} else if ( e.tagName() == "partI" ) {
					field->setPartI(e.text().toInt());
				} else if ( e.tagName() == "partD" ) {
					field->setPartD(e.text().toInt());
				} else if ( e.tagName() == "length" ) {
					field->setLength(e.text().toInt());
				} else if ( e.tagName() == "relation" ) {
					readDBRelation(e, field);
				} else if ( e.tagName() == "optionList" ) {
					optionList = e.text();
				} else if ( e.tagName() == "optionValues" ) {
					optionValues = e.text();
				} else if ( e.tagName() == "optionIcons" ) {
					optionIcons = e.text();
				} else if ( e.tagName() == "script" ) {
					field->setScript(e.text());
				} else if ( e.tagName() == "calculated" ) {
					field->setCalculated((e.text() == "true" ? true : false));
					field->setCalculatedOneTime( ( e.attribute("calculatedOneTime", "false") == "true" ? true : false) );
					field->setCalculatedSaveOnDb(e.attribute("saveOnDb", "false") == "true" ? true: false);
				} else if ( e.tagName() == "html" ) {
					field->setHtml((e.text() == "true" ? true : false));
				} else if ( e.tagName() == "scriptDefaultValue" ) {
					field->setScriptDefaultValue(e.text());
				} else if ( e.tagName() == "validationRule" ) {
					field->setValidationRuleScript(e.text());
				} else if ( e.tagName() == "debug" ) {
					field->setDebug(e.text() == "true" ? true : false);
					field->setOnInitDebug(( e.attribute("onInitDebug", "false") == "true" ? true : false));
				} else if ( e.tagName() == "aggregateCalc" ) {
					readAggregate(e, field);
				} else if ( e.tagName() == "font" ) {
					readFont(e, field);
				} else if ( e.tagName() == "reloadFromDBAfterSave" ) {
					field->setReloadFromDBAfterSave((e.text() == "true" ? true : false));
				}
			}
			// Ahora que tenemos toda la informacion podemos ajustar el default value
			if ( field->type() == QVariant::Int ) {
				field->setDefaultValue(QVariant(defaultValue.toInt()));
			} else if ( field->type() == QVariant::Double ) {
				field->setDefaultValue(QVariant(defaultValue.toDouble()));
			} else {
				field->setDefaultValue(QVariant(defaultValue));
			}
			// También podemos ajustar los options list
			if ( !optionList.isEmpty() ) {
				QStringList list = optionList.split(",");
				QStringList values;
				QMap<QString, QString> map;
				if ( optionValues.isEmpty() ) {
					optionValues = optionList;
				}
				values = optionValues.split(",");
				for ( int i = 0 ; i < list.size() ; i++ ) {
					if ( values.size() != list.size() ) {
						map[list.at(i).trimmed()] = list.at(i).trimmed();
					} else {
						map[values.at(i).trimmed()] = list.at(i).trimmed();
					}
				}
				field->setOptionList(map);
				values = optionIcons.split(",");
				map.clear();
				for ( int i = 0 ; i < list.size() ; i++ ) {
					if ( values.size() != list.size() ) {
						map[list.at(i).trimmed()] = "";
					} else {
						map[list.at(i).trimmed()] = values.at(i).trimmed();
					}
				}
				field->setOptionIcons(map);
			}
			// Condiciones necesarias: Debemos tener establecido un nombre para el field
			// junto con el tipo de datos
			if ( !field->dbFieldName().isEmpty() && field->type() != QVariant::Invalid ) {
				field->setIndex(fieldIndex);
				d->m_fields.append(field);
				optionList.clear();
				optionValues.clear();
				defaultValue.clear();
				fieldIndex++;
			}
		}
	} else {
		QMessageBox::critical(0, QString::fromUtf8(APP_NAME), trUtf8("El archivo XML de sistema <b>%1</b> no es correcto. "
																	 "El programa no funcionará. Consulte con <i>Aleph Sistemas de Información</i>.").arg(d->m_tableName),
							  QMessageBox::Ok);
		qDebug() << "BaseBeanMetadata: setConfig(): ERROR: Line: " << errorLine << " Column: " << errorColumn << " ERROR: " << errorString;
	}
}

void BaseBeanMetadata::readDBRelation(const QDomElement &e, DBFieldMetadata *field)
{
	DBRelationMetadata *relation = new DBRelationMetadata(this);
	relation->setParent(this);
	relation->setRootFieldName(field->dbFieldName());
	QDomNodeList n = e.childNodes();
	for ( int i = 0 ; i < n.size() ; i++ ) {
		QDomElement e = n.at(i).toElement();
		if ( e.tagName() == "table" ) {
			relation->setTableName(e.text());
		} else if ( e.tagName() == "field" ) {
			relation->setChildFieldName(e.text());
		} else if ( e.tagName() == "card" ) {
			if ( e.text() == "1M" ) {
				relation->setType(DBRelationMetadata::ONE_TO_MANY);
			} else if ( e.text() == "11" ) {
				relation->setType(DBRelationMetadata::ONE_TO_ONE);
			} else if ( e.text() == "M1" ) {
				relation->setType(DBRelationMetadata::MANY_TO_ONE);
			}
		} else if ( e.tagName() == "order" ) {
			relation->setOrder(e.text());
		} else if ( e.tagName() == "editable" ) {
			relation->setEditable((e.text() == "true" ? true : false));
		} else if ( e.tagName() == "delC" ) {
			relation->setDeleteCascade((e.text() == "true" ? true : false));
		} else if ( e.tagName() == "avoidDeleteIfIsReferenced" ) {
			relation->setAvoidDeleteIfIsReferenced((e.text() == "true" ? true : false));
		} else if ( e.tagName() == "includeOnCopy" ) {
			relation->setIncludeOnCopy((e.text() == "true" ? true : false));
		}
	}
	field->addRelation(relation);
	d->m_relations.append(relation);
}

void BaseBeanMetadata::readAggregate(const QDomElement &e, DBFieldMetadata *field)
{
	QString fieldName, relationName, filter;

	QDomNodeList n = e.elementsByTagName("aggregate");
	field->setAggregate(true);
	field->setCalculated(true);
	for ( int i = 0 ; i < n.size() ; i++ ) {
		QDomNodeList a = n.at(i).childNodes();
		fieldName = "";
		relationName = "";
		filter = "";
		for ( int j = 0 ; j < a.size() ; j ++ ) {
			QDomElement e = a.at(j).toElement();
			if ( e.tagName() == "field" ) {
				fieldName = e.text();
			} else if ( e.tagName() == "relation" ) {
				relationName = e.text();
			} else if ( e.tagName() == "filter" ) {
				filter = e.text();
			}
		}
		field->addAggregateCalc(relationName, fieldName, filter);
	}
	QDomNodeList op = e.elementsByTagName("operation");
	if ( op.size() > 0 ) {
		QDomElement operation = op.at(0).toElement();
		field->setAggregateOperation(operation.text());
	}
}

void BaseBeanMetadata::readSql(const QDomElement &e)
{
	QDomNodeList n = e.childNodes();
	for ( int i = 0 ; i < n.size() ; i++ ) {
		QDomElement e = n.at(i).toElement();
		if ( e.tagName() == "from" ) {
			addSqlPart("FROM", e.text());
		} else if ( e.tagName() == "where" ) {
			addSqlPart("WHERE", e.text() );
		} else if ( e.tagName() == "order" ) {
			addSqlPart("ORDER", e.text());
		} else if ( e.tagName() == "selectCount" ) {
			addSqlPart("SELECTCOUNT", e.text());
		}
	}
}

void BaseBeanMetadata::readFont(const QDomElement &e, DBFieldMetadata *field)
{
	QDomNodeList n = e.childNodes();
	QFont font;
	for ( int i = 0 ; i < n.size() ; i++ ) {
		QDomElement e = n.at(i).toElement();
		if ( e.tagName() == "name" ) {
			font.setFamily(e.text());
		} else if ( e.tagName() == "size" ) {
			font.setPointSize(e.text().toInt());
		} else if ( e.tagName() == "weight" ) {
			if ( e.text() == "light" ) {
				font.setWeight(QFont::Light);
			} else if ( e.text() == "normal" ) {
				font.setWeight(QFont::Normal);
			} else if ( e.text() == "demiBold" ) {
				font.setWeight(QFont::DemiBold);
			} else if ( e.text() == "bold" ) {
				font.setWeight(QFont::Bold);
			} else if ( e.text() == "black" ) {
				font.setWeight(QFont::Black);
			} else {
				font.setWeight(QFont::Normal);
			}
		} else if ( e.tagName() == "italic" ) {
			font.setItalic(e.text() == "true" ? true : false);
		} else if ( e.tagName() == "color" ) {
			QColor color;
			color.setNamedColor(e.text());
			field->setColor(color);
		} else if ( e.tagName() == "backgroundColor" ) {
			QColor color;
			color.setNamedColor(e.text());
			field->setBackgroundColor(color);
		}
	}
	field->setFontOnGrid(font);
}
/*!
  Lee del XML los fields que se mostrarán en el DBFormDlg como filtros fuertas
  */
void BaseBeanMetadata::readItemsFilterColumn(const QDomElement &e)
{
	QDomNodeList n = e.childNodes();
	for ( int i = 0 ; i < n.size() ; i++ ) {
		if ( n.at(i).toElement().tagName() == "itemFilter" ) {
			QDomNodeList p = n.at(i).childNodes();
			QString fieldToFilter = "";
			QString relationFieldToShow = "";
			QString order;
			for ( int j = 0 ; j < p.size() ; j++ ) {
				QDomElement final = p.at(j).toElement();
				if ( final.tagName() == "fieldToFilter" ) {
					fieldToFilter = final.text();
				} else if ( final.tagName() == "relationFieldToShow" ) {
					relationFieldToShow = final.text();
				} else if ( final.tagName() == "order" ) {
					order = final.text();
				}
			}
			d->m_itemsFilterColumn << QString("%1;%2;%3").arg(fieldToFilter).arg(relationFieldToShow).arg(order);
		}
	}
}

void BaseBeanMetadata::readEnvVarsFilter(const QDomElement &e)
{
	QString field, varName;
	QDomNodeList n = e.childNodes();
	for ( int i = 0 ; i < n.size() ; i++ ) {
		if ( n.at(i).toElement().tagName() == "pair" ) {
			QDomNodeList p = n.at(i).childNodes();
			for ( int j = 0 ; j < p.size() ; j++ ) {
				QDomElement final = p.at(j).toElement();
				if ( final.tagName() == "field" ) {
					field = final.text();
				} else if ( final.tagName() == "varName" ) {
					varName = final.text();
				}
			}
			if ( !field.isEmpty() && !varName.isEmpty() ) {
				QString temp = QString("%1;%2").arg(varName).arg(field);
				d->m_envVars << temp;
			}
		}
	}
}

/**
  Devuelve todos aquellos campos que componen la primaryKey
  */
QList<DBFieldMetadata *> BaseBeanMetadata::pkFields ()
{
	QList<DBFieldMetadata *> fields;
	for ( int i = 0 ; i < d->m_fields.size() ; i++ ) {
		if ( d->m_fields.at(i)->primaryKey() ) {
			fields.append(d->m_fields.at(i));
		}
	}
	if ( fields.isEmpty() ) {
		qCritical() << "pkFields: BaseBean: " << tableName() << ". No existe primary key.";
	}
	return fields;
}

int BaseBeanMetadata::fieldCount()
{
	return d->m_fields.size();
}

DBFieldMetadata * BaseBeanMetadata::field(const QString &dbFieldName)
{
	DBFieldMetadata *field = NULL;
	for ( int i = 0 ; i < d->m_fields.size() ; i++ ) {
		if ( d->m_fields.at(i)->dbFieldName() == dbFieldName ) {
			field = d->m_fields.at(i);
		}
	}
	if ( field == NULL ) {
		qCritical() << "field: BaseBean: " << tableName() << ". No existe el campo: " << dbFieldName;
	}
	return field;
}

DBFieldMetadata * BaseBeanMetadata::field(int index)
{
	if ( index < 0 || index >= d->m_fields.size() ) {
		return NULL;
	}
	return d->m_fields.at(index);
}

int BaseBeanMetadata::fieldIndex(const QString &dbFieldName)
{
	int index = -1;
	for ( int i = 0 ; i < d->m_fields.size() ; i++ ) {
		if ( d->m_fields.at(i)->dbFieldName() == dbFieldName ) {
			index = i;
		}
	}
	if ( index == -1 ) {
		qCritical() << "fieldIndex: BaseBean: " << tableName() << ". No existe el campo: " << dbFieldName;
	}
	return index;
}

DBRelationMetadata * BaseBeanMetadata::relation(const QString &table)
{
	DBRelationMetadata *rel = NULL;
	for ( int i = 0 ; i < d->m_relations.size() ; i++ ) {
		if ( d->m_relations.at(i)->tableName() == table ) {
			rel = d->m_relations.at(i);
		}
	}
	if ( rel == NULL ) {
		qCritical() << "relation: BaseBean: " << tableName() << ". No existe la relacion: " << table;
	}
	return rel;
}

/*!
  Obtiene la cláusula where para obtener los registros filtrados según las variables de entorno
  */
QString BaseBeanMetadata::whereFromEnvVars()
{
	QHash<QString, QVariant> vars = EnvVars::vars();
	QHashIterator<QString, QVariant> it(vars);
	QString where;

	// Se comprueba si los metadatos de este bean, contienen algún field
	// relacionado con alguna variable global con datos establecidos
	while ( it.hasNext() ) {
		it.next();
		foreach ( QString mEnvVar, d->m_envVars ) {
			QStringList parts = mEnvVar.split(";");
			if ( parts.size() == 2 ) {
				if ( parts.at(0) == it.key() ) {
					DBFieldMetadata *fld = field(parts.at(1));
					if ( fld != NULL && it.value().toString() != "" ) {
						if ( !where.isEmpty() ) {
							where = QString("%1 AND ").arg(where);
						}
						where = QString ("%1%2").arg(where).arg(fld->sqlWhere("=", it.value()));
					}
				}
			}
		}
	}
	return where;
}

/*!
Tras borrarse un registro de base de datos (después del commit), se llama a esta función, que ejecutará
el codigo asociado
*/
void BaseBeanMetadata::afterDeleteScriptExecute(BaseBean *b)
{
	// Al ser un campo calculado, ejecutamos el script asociado. Este se ejecuta
	// sobre el bean, de modo, que parent debe estar visible en el script
	d->m_engine->addAvailableObject("bean", qobject_cast<QObject *>(b));
	d->m_engine->setScript(d->m_afterDeleteScript);
	d->m_engine->evaluateQsFunction(QString("afterDelete"));
}
