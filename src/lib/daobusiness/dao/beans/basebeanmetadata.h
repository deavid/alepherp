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
#ifndef BASEBEANMETADATA_H
#define BASEBEANMETADATA_H

#include <QObject>
#include <QString>
#include <QList>
#include <QDomElement>
#include <QVariant>
#include <QStringList>

class DBFieldMetadata;
class DBRelationMetadata;
class BaseBeanMetadataPrivate;
class BaseBean;

class BaseBeanMetadata : public QObject
{
	Q_OBJECT

	/** Configuración en XML del bean: Campos, relaciones... */
	Q_PROPERTY(QString xml READ xml WRITE setXml)
	/** Tabla de la base de datos a la que hace referencia */
	Q_PROPERTY(QString tableName READ tableName WRITE setTableName)
	/** Esta es la vista o consulta que se utiliza para obtener los datos que
	en los controles que utiliza a modelos derivados de BaseBeanModel */
	Q_PROPERTY(QString viewOnGrid READ viewOnGrid WRITE setViewOnGrid)
	/** Nombre bonito de la tabla */
	Q_PROPERTY(QString alias READ alias WRITE setAlias)
	/** Indica si el borrado es lógico o físico. Si es lógico, el registro
	  no podrá ser borrado de base de datos. En su lugar se marca la columna
	  is_deleted como true */
	Q_PROPERTY(bool logicalDelete READ logicalDelete WRITE setLogicalDelete)
	/** Cuando este bean se presenta en un DBForm, puede querer filtrarse mediante
	un filtro fuerte, que se deriva de los valores de otra tabla, o de los valores
	de un option list de ese campo. Por ejemplo,
	presupuestos_cabecera tiene un campo, estado, que coge datos de los registros
	marcados en la tabla presupuestos_definicion_estados. El contenido de esta tabla
	se incluirá en el combo. Ejemplo:
	Para poner un filtro fuerte para los vendís y demás, lo único que tienes es que configurar el
	XML de la tabla así

	<table>
		<name>alepherp_vendi</name>
		<alias>Vendis</alias>
		<itemsFilterColumn>
			<itemFilter>
				<fieldToFilter>id_tienda</fieldToFilter>
				<relationFieldToShow>nombre</relationFieldToShow>
			</itemFilter>
		</itemsFilterColumn>
	....

	donde fieldToFilter es el campo de los registros del DBFormDlg que estás presentado por el que
	quieres filtrar, y relationFieldToShow es qué campo de la relación M1 que tendría ese campo,
	se mostrará en el combobox de filtro (el name de la tienda).

	Si fieldToFilter no tiene relación definida pero tomas sus valores de un optionList, esos
	valores se agregarán sólos.
 */
	Q_PROPERTY(QStringList itemsFilterColumn READ itemsFilterColumn WRITE setItemsFilterColumn)
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
   </envVars>
   y el QStringList quedaría: "idTienda:id_tienda"
	  */
	Q_PROPERTY(QStringList envVarsFilter READ envVars)
	/** Si se arranca la aplicación por primera vez, éste sera el orden en el que se mostrarán los datos.
	Es una claúsula tal que asi
	id_vendi DESC, fecha ASC...*/
	Q_PROPERTY(QString initialOrderSort READ initialOrderSort WRITE setInitialOrderSort)
	/** Este código se ejecuta justo después de borrarse el bean de base de datos (no al ser marcado para borrar)
	Tiene acceso a los datos borrados */
	Q_PROPERTY(QString afterDeleteScript READ afterDeleteScript WRITE setAfterDeleteScript)
	/** Habrá un DBField predefinido de visualización. Si no se especifica nada, y quiere mostrarse
	  un valor de este DBField, se escogerá este DBField. Se establece en el XML con la cláusula
	  defaultVisualizationField */
	Q_PROPERTY(QString defaultVisualizationField READ defaultVisualizationField WRITE setDefaultVisualizationField)
	/** Hay ciertos beans que no cambian habitualmente. Esos beans pueden ser cacheados en memoria (por ejemplo,
	  ejercicios fiscales, definiciones de productos)... Esta propiedad marca esos beans, que por tanto
	  quedan residentes en memoria para una mejor ejecución del sistema */
	Q_PROPERTY(bool isCached READ isCached WRITE setIsCached)

private:
	Q_DISABLE_COPY(BaseBeanMetadata)
	BaseBeanMetadataPrivate *d;
	Q_DECLARE_PRIVATE(BaseBeanMetadata)

	void setConfig();
	void readDBRelation(const QDomElement &e, DBFieldMetadata *field);
	void readAggregate(const QDomElement &e, DBFieldMetadata *field);
	void readSql(const QDomElement &e);
	void readItemsFilterColumn(const QDomElement &e);
	void readEnvVarsFilter(const QDomElement &e);
	void readFont(const QDomElement &e, DBFieldMetadata *field);

public:
	explicit BaseBeanMetadata(QObject *parent = NULL);
	virtual ~BaseBeanMetadata();

	QString xml();
	void setXml (const QString &value);
	QString tableName();
	void setTableName(const QString &value);
	void setAlias(const QString &alias);
	QString alias();
	void setViewOnGrid(const QString &query);
	QString viewOnGrid();
	bool logicalDelete();
	void setLogicalDelete(bool value);
	void setItemsFilterColumn(const QStringList &alias);
	QStringList itemsFilterColumn();
	QStringList envVars();
	QString initialOrderSort();
	void setInitialOrderSort(const QString &value);
	QString afterDeleteScript();
	void setAfterDeleteScript(const QString &value);
	bool debug();
	void setDebug(bool value);
	bool onInitDebug();
	void setOnInitDebug(bool value);
	QString defaultVisualizationField();
	void setDefaultVisualizationField(const QString &value);
	bool isCached();
	void setIsCached(bool value);

	void addSqlPart(const QString &clause, const QString &value);
	QHash<QString, QString> sql();

	QList<DBFieldMetadata *> fields();
	QList<DBRelationMetadata *> relations();

	QList<DBFieldMetadata *> pkFields ();
	int fieldCount();
	DBFieldMetadata * field(const QString &dbFieldName);
	DBFieldMetadata * field(int index);
	int fieldIndex(const QString &dbFieldName);

	DBRelationMetadata * relation(const QString &table);

	QString whereFromEnvVars();
	void afterDeleteScriptExecute(BaseBean *bean);
};

#endif // BASEBEANMETADATA_H
