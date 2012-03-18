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
#include "configuracion.h"
#include "dao/beans/basebeanmetadata.h"
#include "dao/beans/basebean.h"
#include "dao/beans/dbfield.h"
#include "dao/beans/dbrelation.h"
#include "dao/beans/dbfieldmetadata.h"
#include "dao/beans/dbrelationmetadata.h"
#include "dao/beans/beansfactory.h"
#include "dao/dbfieldobserver.h"
#include "dao/basebeanobserver.h"
#include "widgets/dbbasewidget.h"
#include <QDebug>
#include <QMessageBox>
#include <QCoreApplication>

class BaseBeanPrivate {
//	Q_DECLARE_PUBLIC(BaseBean)
public:
	/** Listado de campos de este field y sus valores */
	QList<DBField *> m_fields;
	QList<DBRelation *> m_relations;
	/** Flag que indica el estado en que se encuentra en base de datos */
	BaseBean::DbBeanStates m_dbState;
	/** Flag que indica si en este bean han modificado sus datos (los fields) */
	bool m_modified;
	/** Copia de seguridad de los valores de los fields del bean. Utilizado por backupValues y restoreValues */
	QVariantMap m_backup;
	/** Puntero a los metadatos que definen a este bean */
	BaseBeanMetadata *m;
	/** Indica si este bean tiene potestad para guardarse sólo en base de datos. Esta propiedad
	  será utilizada por ejemplo por RelationBaseBeanModel. Esta relación modela los beans hijos de otro (padre).
	  Imaginemos que el padre aún no se ha almacenado en base de datos (el usuario lo está creando):
	  No tendría sentido que el bean hijo intentara guardarse en base de datos. Esto último puede ocurrir
	  ya que los formularios tienen acceso al método save. */
	bool m_canSaveOnDbDirectly;
	/** Indica el tiempo de la ultima carga de datos de BBDD */
	QDateTime m_loadTime;

	BaseBeanPrivate();

	QString extractFilterOperator(const QString &filter);
	void setDefaultValues();
};

BaseBeanPrivate::BaseBeanPrivate()
{
	m_modified = false;
	m_dbState = BaseBean::INSERT;
	m_canSaveOnDbDirectly = true;
}

void BaseBeanPrivate::setDefaultValues()
{
	foreach ( DBField *fld, m_fields ) {
		fld->blockSignals(true);
		fld->setValue(fld->defaultValue());
		fld->blockSignals(false);
	}
}

BaseBean::BaseBean(QObject *parent) : DBObject(parent), d(new BaseBeanPrivate)
{
	m_observer = ObserverFactory::instance()->newObserver(this);
}

BaseBean::~BaseBean()
{
	delete d;
}

void BaseBean::setMetadata(BaseBeanMetadata *m, bool setDefaultValue)
{
	d->m = m;
	QList<DBFieldMetadata *> fieldsMetadata = d->m->fields();
	QList<DBRelationMetadata *>relationsMetadata = d->m->relations();
	foreach (DBFieldMetadata *metadata, fieldsMetadata) {
		DBField *fld = newField(metadata);
		if ( setDefaultValue ) {
			fld->setValue(metadata->defaultValue(fld));
		}
	}
	foreach (DBRelationMetadata *metadata, relationsMetadata) {
		DBRelation *rel = newRelation(metadata);
		DBField *fld = field(rel->metadata()->rootFieldName());
		if ( fld != NULL ) {
			fld->addRelation(rel);
		}
	}
}

BaseBeanMetadata * BaseBean::metadata() const
{
	return d->m;
}

bool BaseBean::canSaveOnDbDirectly()
{
	return d->m_canSaveOnDbDirectly;
}

void BaseBean::setCanSaveOnDbDirectly(bool value)
{
	d->m_canSaveOnDbDirectly = value;
}

/*!
  Método factoría para crear DBFields.
  Agrega el field al listado de fields, y conecta las señales de modificación
  */
DBField *BaseBean::newField(DBFieldMetadata *m)
{
	// Poniendo como padre a este objeto, garantizamos que cuando
	// este bean se borre, se borrarán los campos
	DBField *field = new DBField(this);
	field->setParent(this);
	field->setMetadata(m);
	d->m_fields << field;
	connect (field, SIGNAL(valueModified(QVariant)), this, SLOT(setModified()));
	connect (field, SIGNAL(valueModified(QVariant)), this, SLOT(recalculateCalculatedFields()));
	connect (field, SIGNAL(valueModified(QString,QVariant)), this, SIGNAL(fieldModified(QString, QVariant)));
	connect (field, SIGNAL(valueModified(QString,QVariant)), this, SLOT(emitFieldModified(QString, QVariant)));
	return field;
}

void BaseBean::emitFieldModified(const QString &dbFieldName, const QVariant &value)
{
	emit fieldModified(this, dbFieldName, value);
}

/*!
  Método factoría para generar nuevas relaciones en el bean. Crea el objeto y lo agrega
  al listado de DBRelation
  */
DBRelation *BaseBean::newRelation(DBRelationMetadata *m)
{
	// Poniendo como padre a este objeto, garantizamos que cuando
	// este bean se borre, se borrarán los campos
	DBRelation *rel = new DBRelation(this);
	rel->setParent(this);
	rel->setMetadata(m);
	d->m_relations << rel;
	connect (rel, SIGNAL(childModified(BaseBean *, bool)), this, SLOT(setModified()));
	connect (rel, SIGNAL(childDeleted(BaseBean *,int)), this, SLOT(setModified()));
	connect (rel, SIGNAL(childDbStateModified(BaseBean *,int)), this, SLOT(setModified()));
	// Si se borra un hijo de una relación de este bean, es probable que haya campos agregados
	// o calculados que haya que recalcular
	connect (rel, SIGNAL(childDbStateModified(BaseBean*,int)), this, SLOT(recalculateCalculatedFields()));

	return rel;
}

QList<DBField *> BaseBean::fields()
{
	return d->m_fields;
}

/*!
  Conjunto de todas las relaciones del basebean. Pueden ser relaciones con hijos o con padres
  */
QList<DBRelation *> BaseBean::relations()
{
	return d->m_relations;
}

/*!
  Devuelve el orden de esta relación entre todas las del bean. Muy utilizado para construir alias
  */
int BaseBean::relationIndex(const QString &relationName)
{
	QList<DBRelationMetadata *> relations = d->m->relations();
	for ( int i = 0 ; i < relations.size() ; i++ ) {
		if ( relationName == relations.at(i)->tableName() ) {
			return i;
		}
	}
	return -1;
}

/*!
  Devuelve los hijos de la relación dada. Busca el objeto relation, y obtiene directamente los hijos
  */
BaseBeanPointerList BaseBean::relationChilds(const QString &relationName, const QString &order)
{
	BaseBeanPointerList empty;
	DBRelation *rel = relation(relationName);
	if ( rel != NULL ) {
		return rel->childs(order);
	}
	return empty;
}

/*!
  Devuelve el número de beans hijos de la relación
  */
int BaseBean::relationChildsCount(const QString &relationName, bool includeToBeDeleted)
{
	DBRelation *rel = relation(relationName);
	if ( rel != NULL ) {
		return rel->childsCount(includeToBeDeleted);
	}
	return 0;
}

/*!
  Para la relación relationName, devuelve el bean cuya primarykey es id
  */
QSharedPointer<BaseBean> BaseBean::relationChildByPk(const QString &relationName, const QVariant &id)
{
	QSharedPointer<BaseBean> bean;
	DBRelation *rel = relation(relationName);
	if ( rel != NULL ) {
		bean = rel->childByPk(id);
	}
	return bean;
}

/*!
  Para la relación relationName, devuelve el bean cuyo field fieldName es id
  */
QSharedPointer<BaseBean> BaseBean::relationChildByField(const QString &relationName, const QString &fieldName, const QVariant &id)
{
	QSharedPointer<BaseBean> bean;
	DBRelation *rel = relation(relationName);
	if ( rel != NULL ) {
		bean = rel->childByField(fieldName, id);
	}
	return bean;
}

/*!
  Devuelve los hijos de la relación \a relationName según un filtro elaborado en \a filter
  y ordenados según \a order
  */
BaseBeanPointerList BaseBean::relationChildsByFilter(const QString &relationName, const QString &filter, const QString &order)
{
	BaseBeanPointerList beans;
	DBRelation *rel = relation(relationName);
	if ( rel != NULL ) {
		beans = rel->childsByFilter(filter, order);
	}
	return beans;
}

void BaseBean::setFieldValue(int index, QVariant value)
{
	if ( index >= 0 && index < d->m_fields.size() ) {
		d->m_fields.at(index)->setValue(value);
	}
}

void BaseBean::setFieldValue(const QString &dbFieldName, QVariant value)
{
	foreach ( DBField *field, d->m_fields ) {
		if ( field->metadata()->dbFieldName() == dbFieldName ) {
			field->setValue(value);
		}
	}
}

void BaseBean::setFieldValueFromInternal(int index, QVariant value)
{
	if ( index >= 0 && index < d->m_fields.size() ) {
		d->m_fields.at(index)->setValueFromInternal(value);
	}
}

void BaseBean::setFieldValueFromInternal(const QString &dbFieldName, QVariant value)
{
	foreach ( DBField *field, d->m_fields ) {
		if ( field->metadata()->dbFieldName() == dbFieldName ) {
			field->setValueFromInternal(value);
		}
	}
}

DBField * BaseBean::field(const QString &dbFieldName)
{
	DBField *field = NULL;
	for ( int i = 0 ; i < d->m_fields.size() ; i++ ) {
		if ( d->m_fields.at(i)->metadata()->dbFieldName() == dbFieldName ) {
			field = d->m_fields.at(i);
		}
	}
	if ( field == NULL ) {
		qDebug() << "BaseBean::field: [" << d->m->tableName() << "]. No existe el campo: [" << dbFieldName << "]";
	}
	return field;
}

DBField * BaseBean::field(int index)
{
	if ( index < 0 || index >= d->m_fields.size() ) {
		return NULL;
	}
	return d->m_fields.at(index);
}

/**
  Devuelve todos aquellos campos que componen la primaryKey
  */
QList<DBField *> BaseBean::pkFields ()
{
	QList<DBField *> fields;
	for ( int i = 0 ; i < d->m_fields.size() ; i++ ) {
		if ( d->m_fields.at(i)->metadata()->primaryKey() ) {
			fields.append(d->m_fields.at(i));
		}
	}
	if ( fields.isEmpty() ) {
		qDebug() << "BaseBean::pkFields: [" << d->m->tableName() << "]. No existe primary key.";
	}
	return fields;
}

int BaseBean::fieldIndex(const QString &dbFieldName)
{
	int index = -1;
	for ( int i = 0 ; i < d->m_fields.size() ; i++ ) {
		if ( d->m_fields.at(i)->metadata()->dbFieldName() == dbFieldName ) {
			index = i;
		}
	}
	if ( index == -1 ) {
		qDebug() << "BaseBean::fieldIndex: [" << d->m->tableName() << "]. No existe el campo: [" << dbFieldName << "]";
	}
	return index;
}

int BaseBean::fieldCount()
{
	return d->m_fields.size();
}

DBRelation * BaseBean::relation(const QString &table)
{
	DBRelation *rel = NULL;
	for ( int i = 0 ; i < d->m_relations.size() ; i++ ) {
		if ( d->m_relations.at(i)->metadata()->tableName() == table ) {
			rel = d->m_relations.at(i);
		}
	}
	if ( rel == NULL ) {
		qDebug() << "BaseBean::relation: [" << d->m->tableName() << "]. No existe la relacion: [" << table << "]";
	}
	return rel;
}

/*!
  Devuelve el primer hijo de la relación \a relationName según el orden \a order
  */
QSharedPointer<BaseBean> BaseBean::relationFirstChild(const QString &relationName, const QString &order)
{
	DBRelation *rel = relation(relationName);
	if ( rel == NULL ) {
		qDebug() << "BaseBean::relationFirstChild: [" << d->m->tableName() << "]. No existe la relacion: [" << relationName << "]";
	} else {
		return rel->firstChild(order);
	}
	return QSharedPointer<BaseBean> ();
}

/*!
Obtiene, de una relación de tipo M1, el bean padre
*/
QSharedPointer<BaseBean> BaseBean::father(const QString &relationName)
{
	QSharedPointer<BaseBean> bean;
	DBRelation *rel = relation(relationName);
	if ( rel == NULL ) {
		qDebug() << "BaseBean::father: [" << d->m->tableName() << "]. No existe la relacion: [" << relationName << "]";
	} else {
		if ( rel->metadata()->type() == DBRelationMetadata::MANY_TO_ONE ) {
			bean = rel->father();
		}
	}
	return bean;
}

QVariant BaseBean::fatherFieldValue(const QString &relationName, const QString &field)
{
	QVariant v;
	QSharedPointer<BaseBean> f = father(relationName);
	if ( !f.isNull() ) {
		v = f->fieldValue(field);
	}
	return v;
}

QString BaseBean::fatherDisplayFieldValue(const QString &relationName, const QString &field)
{
	QString v;
	QSharedPointer<BaseBean> f = father(relationName);
	if ( !f.isNull() ) {
		v = f->displayFieldValue(field);
	}
	return v;
}

/*!
  Devuelve el valor del campo dbFieldName
  */
QVariant BaseBean::fieldValue(int dbField)
{
	if ( dbField > -1 && dbField < d->m_fields.size() ) {
		return d->m_fields.at(dbField)->value();
	}
	qDebug() << "BaseBean::fieldValue: [" << d->m->tableName() << "]. No existe el field: [" << dbField << "]";
	return QVariant();
}

/*!
  Devuelve el valor del campo dbFieldName
  */
QVariant BaseBean::fieldValue(const QString &dbFieldName)
{
	QVariant value;

	if ( dbFieldName.isEmpty() ) {
		return value;
	}
	DBField *fld = field(dbFieldName);
	if ( fld == NULL ) {
		qDebug() << "BaseBean::fieldValue: [" << d->m->tableName() << "]. No existe el field: [" << dbFieldName << "]";
	} else {
		value = fld->value();
	}
	return value;
}

/*!
  Devuelve el valor formateado del campo dbFieldName
  */
QString BaseBean::displayFieldValue(int iField)
{
	if ( iField > -1 && iField < d->m_fields.size() ) {
		return d->m_fields.at(iField)->displayValue();
	}
	qDebug() << "BaseBean::displayFieldValue: [" << d->m->tableName() << "]. No existe el field: [" << iField << "]";
	return QString();
}

/*!
  Devuelve el valor formateado del campo dbFieldName
  */
QString BaseBean::displayFieldValue(const QString & dbFieldName)
{
	QString value;

	if ( dbFieldName.isEmpty() ) {
		return value;
	}
	DBField *fld = field(dbFieldName);
	if ( fld == NULL ) {
		qDebug() << "BaseBean::displayFieldValue: [" << d->m->tableName()
				 << "]. No existe el field: [" << dbFieldName << "]";
	} else {
		value = fld->displayValue();
	}
	return value;
}

/*!
  Devuelve el valor formateado del campo dbFieldName para ser insertado en base de datos
  */
QString BaseBean::sqlFieldValue(int iField)
{
	if ( iField > -1 && iField < d->m_fields.size() ) {
		return d->m_fields.at(iField)->sqlValue(true);
	}
	qDebug() << "BaseBean::displayFieldValue: [" << d->m->tableName() << "]. No existe el field: [" << iField << "]";
	return QString();
}

/*!
  Devuelve el valor formateado del campo dbFieldName para ser insertado en base de datos
  */
QString BaseBean::sqlFieldValue(const QString & dbFieldName)
{
	QString value;

	if ( dbFieldName.isEmpty() ) {
		return value;
	}
	DBField *fld = field(dbFieldName);
	if ( fld == NULL ) {
		qDebug() << "BaseBean::displayFieldValue: [" << d->m->tableName()
				 << "]. No existe el field: [" << dbFieldName << "]";
	} else {
		value = fld->sqlValue(true);
	}
	return value;
}

QVariant BaseBean::defaultFieldValue(const QString &dbFieldName)
{
	QVariant value;

	if ( dbFieldName.isEmpty() ) {
		return value;
	}
	DBField *fld = field(dbFieldName);
	if ( fld == NULL ) {
		qDebug() << "BaseBean::defaultFieldValue: [" << d->m->tableName()
				 << "]. No existe el field: [" << dbFieldName << "]";
	} else {
		value = fld->metadata()->defaultValue(fld);
	}
	return value;
}


/*!
 Proporciona una claúsula where
*/
QString BaseBean::sqlWhere(const QString &fieldName, const QString &op)
{
	QString sql;
	DBField *fld = field(fieldName);
	if ( fld == NULL ) {
		qDebug() << "BaseBean::sqlWhere: [" << d->m->tableName()
				 << "]. No existe el field: [" << fieldName << "]";
	} else {
		sql = fld->sqlWhere(op);
	}
	return sql;
}


/*!
 Proporciona una claúsula where para la primary key de este bean
*/
QString BaseBean::sqlWherePk()
{
	QString where;

	QList<DBField *> pk = pkFields();
	if ( pk.isEmpty() ) {
		return where;
	}
	foreach ( DBField *fld, pk ) {
		if ( where.isEmpty() ) {
			where = fld->sqlWhere("=");
		} else {
			where = where + " AND " + fld->sqlWhere("=");
		}
	}
	return where;
}

QVariant BaseBean::pkValue()
{
	QVariantMap pkValues;
	QList<DBField *> pk = pkFields();
	if ( pk.isEmpty() ) {
		return QVariant();
	}
	foreach ( DBField *fld, pk ) {
		pkValues[fld->metadata()->dbFieldName()] = fld->value();
	}
	return pkValues;
}

/*!
  Devuelve el valor serializado de la primary key
  */
QString BaseBean::pkSerializedValue()
{
	return BaseDAO::serializePk(pkValue());
}

/*!
  Realiza una comparación entre la primary key de este bean, y
  la pasada en value
  */
bool BaseBean::pkEqual(const QVariant &value)
{
	QVariantMap pkThis, pkOther;
	pkThis = pkValue().toMap();
	pkOther = value.toMap();
	QMapIterator<QString, QVariant> it (pkThis);
	while ( it.hasNext() ) {
		it.next();
		if ( !pkOther.contains(it.key()) || pkOther.value(it.key()) != it.value() ) {
			return false;
		}
	}
	return true;
}

void BaseBean::setPkValue(const QVariant &id)
{
	QVariantMap pkValues = id.toMap();
	QMapIterator<QString, QVariant> i(pkValues);
	while ( i.hasNext() ) {
		i.next();
		DBField *fld = field(i.key());
		fld->setValue(i.value());
	}
}

void BaseBean::setPkValueFromInternal(const QVariant &id)
{
	QVariantMap pkValues = id.toMap();
	QMapIterator<QString, QVariant> i(pkValues);
	while ( i.hasNext() ) {
		i.next();
		DBField *fld = field(i.key());
		fld->setValueFromInternal(i.value());
	}
}

void BaseBean::setDbState(BaseBean::DbBeanStates value)
{
	d->m_dbState = value;
	/** Por definición, si se marca un bean para ser borrado, pasa a está modificado */
	if ( value == BaseBean::TO_BE_DELETED ) {
		d->m_modified = true;
	} else if ( value == BaseBean::INSERT ) {
		blockSignals(true);
		d->setDefaultValues();
		blockSignals(false);
	}
	emit dbStateModified(value);
	emit dbStateModified(this, value);
	emit beanModified(true);
	emit beanModified(this, true);
}

BaseBean::DbBeanStates BaseBean::dbState()
{
	return d->m_dbState;
}

/*!
  Chivato de modificación: Indica si el bean se ha modificado desde que se creó
  o desde que se leyó de base de datos
  */
bool BaseBean::modified ()
{
	return d->m_modified;
}

/*!
  Recorre todos los fields, y recalcula todos los los fields hijos
  */
void BaseBean::setModified()
{
	d->m_modified = true;
	emit beanModified(d->m_modified);
	emit beanModified(this, d->m_modified);
}

/*!
  Recorre todos los fields, y recalcula todos los los fields hijos
  */
void BaseBean::recalculateCalculatedFields()
{
	foreach ( DBField *fld, d->m_fields ) {
		if ( fld->metadata()->calculated() ) {
			fld->value();
		}
	}
}

/*!
  Marca como no modificados los campos e hijos de este bean
  */
void BaseBean::uncheckModifiedFields()
{
	foreach (DBField *fld, d->m_fields) {
		fld->setModified(false);
	}
	foreach (DBRelation *rel, d->m_relations) {
		rel->uncheckChildrensModified();
	}
	d->m_modified = false;
	emit beanModified(d->m_modified);
	emit beanModified(this, d->m_modified);
}

/*!
  Guarda en BBDD el contenido de este bean. Ejecuta una inserción o update. Para que sea
  una inserción, insert debe estar a true. Si se guarda en base de datos bien, se pondrá
  insert a false.
  Un bean a través de sus relaciones puede tener hijos. Esos hijos pueden no guardarse
  poniendo saveChilds a false. Por defecto, se guardan siempre, saveChilds = true
  */
bool BaseBean::save(bool saveChilds)
{
	bool result = false;
	if ( !d->m_canSaveOnDbDirectly ) {
		return true;
	}
	BaseDAO::transaction();
	if ( dbState() == BaseBean::INSERT ) {
        result = BaseDAO::insert(this, saveChilds);
	} else if ( dbState() == BaseBean::UPDATE ){
        result = BaseDAO::update(this, saveChilds);
	}
	if ( !result ) {
		BaseDAO::rollback();
        qDebug() << "BaseBean::save: rollback por fallo.";
	} else {
		BaseDAO::commit();
	}
	if ( result && dbState() == BaseBean::INSERT ) {
		setDbState(BaseBean::UPDATE);
	}
	// TODO :
	/*QScriptValue r;
	d->m_engine.callQsObjectFunction(r, "afterSaveRecord");*/
	return result;
}

/*!
  Realiza en memoria una copia de los Values de los DBFields del bean. Es muy útil cuando se
  va a editar los datos de un bean, se modifica y se cancela la edición. Se restaura la copia
  con restoreValues.
  @see restoreValues
  */
void BaseBean::backupValues()
{
	foreach ( DBField *fld, d->m_fields ) {
        if ( !fld->metadata()->memo() || fld->memoLoaded() ) {
            fld->blockSignals(true);
            d->m_backup[fld->metadata()->dbFieldName()] = fld->value();
            fld->blockSignals(false);
        }
	}
}

/*!
  Restaura la copia de seguridad de los datos que se hizo con backupValues.
  @see backupValues;
  */
void BaseBean::restoreValues()
{
	QMapIterator<QString, QVariant> it(d->m_backup);
	while ( it.hasNext() ) {
		it.next();
		setFieldValue(it.key(), it.value());
		DBField *fld = field(it.key());
		if ( fld != NULL ) {
			fld->setModified(false);
		}
	}
	d->m_modified = false;
}

/*!
  Cuando se inserta un nuevo bean, y se trabaja con él, puede ocurrir que sea necesario
  que los campos seriales tengan un valor único para poderlos identificar (pueden ser un
  primary key). Esta función se encarga de esa asignación
  @see backupValues;
  */
void BaseBean::setSerialUniqueId()
{
	QList<DBFieldMetadata *> pkFields = d->m->pkFields();
	foreach ( DBField *fld, d->m_fields ) {
		if ( fld->metadata()->serial() && pkFields.contains(fld->metadata()) ) {
			fld->setValueFromInternal( configuracion.uniqueId() );
		}
	}
}

/*!
  Tenemos que decirle al motor de scripts, que BaseBean se convierte de esta forma a un valor script
  */
QScriptValue BaseBean::toScriptValue(QScriptEngine *engine, BaseBean * const &in)
{
	return engine->newQObject(in);
}

void BaseBean::fromScriptValue(const QScriptValue &object, BaseBean * &out)
{
	out = qobject_cast<BaseBean *>(object.toQObject());
}

QScriptValue BaseBean::toScriptValueSharedPointer(QScriptEngine *engine, const QSharedPointer<BaseBean> &in)
{
	return engine->newQObject(in.data(), QScriptEngine::QtOwnership);
}

void BaseBean::fromScriptValueSharedPointer(const QScriptValue &object, QSharedPointer<BaseBean> &out)
{
	out = QSharedPointer<BaseBean>(qobject_cast<BaseBean *>(object.toQObject()));
}

QString BaseBeanPrivate::extractFilterOperator(const QString &filter)
{
	QStringList ops;
	QString op;
	ops << "=" << "<" << ">" << "!=";

	foreach ( QString temp, ops ) {
		if ( filter.indexOf(temp) != -1 ) {
			op = temp;
		}
	}
	return op;
}

/*!
  Pasa un checkeo de filtro. Pero este chequeo permite anidaciones en relacion M1. Por ejemplo, si hacemos
  "categoria.id_categoria = 2" buscará la relación categoria. Pero también puede hacer
  "subfamilia.familia.id_familia = 2" y devolverá si id_familia de familias es 2.
  También aceptará expresiones como subfamilia.familia.id_familia < 2.
  Por otro lado se pueden chequear varios campos. En ese caso hay que separar los filtros
  con la expresión " AND " o " and "
  Un ejemplo de filtro
  actividad.nombre='prueba' and id_tipo_trabajo='FO'
  */
bool BaseBean::checkFilter(const QString &filterExpression, Qt::CaseSensitivity sensivity)
{
	QString op, fields;
	QStringList parts, relations;
	BaseBean *bean = this;
	DBRelation *rel = NULL;
	DBField *fld = NULL;
	QString data;
	QDate filterDate, filterDate2;

	if ( filterExpression.isEmpty() ) {
		return true;
	}

	QStringList conditions = filterExpression.split(QRegExp(" (AND|and) "));

	foreach ( QString filter, conditions ) {
		op = d->extractFilterOperator(filter);
		if ( op.isEmpty() ) {
			return false;
		}
		parts = filter.split(op);
		if ( parts.size() != 2 ) {
			return false;
		}
		fields = parts.at(0).trimmed();
		data = parts.at(1).trimmed();
		relations = fields.split(".");
		// Si es una relación, obtenemos el bean padre de esa relación
		for ( int i = 0 ; i < relations.size() - 1; i++ ) {
			if ( bean != NULL ) {
				rel = bean->relation(relations.at(i));
				if ( rel != NULL ) {
					QSharedPointer<BaseBean> shBean;
					if ( rel->metadata()->type() == DBRelationMetadata::MANY_TO_ONE ) {
						shBean = rel->father();
					} else {
						shBean = rel->firstChild();
					}
					bean = shBean.data();
				} else {
					bean = NULL;
				}
			}
		}
		if ( bean == NULL ) {
			return false;
		}
		fld = bean->field(relations.last());
		if ( fld == NULL ) {
			return false;
		}
		if ( fld->metadata()->type() == QVariant::Bool ) {
				if ( op == "=" ) {
					bool v = (data == "true" ? true : false);
					if (!fld->checkValue(v)) {
						return false;
					}
				} else if ( op == "!=" ) {
					bool v = (data == "true" ? true : false);
					if (fld->checkValue(v)) {
						return false;
					}
				} else {
					return false;
				}
		} else if ( fld->metadata()->type() == QVariant::Date ) {
			/** ¿es un intervalo de fechas?. Los intervalos de fecha se ponen
			fecha=2010-11-21;2010-12-30 */
				if ( data.contains(";") ) {
					QStringList dates = data.split(";");
					if ( dates.size() == 2 ) {
						filterDate = QDate::fromString(dates.at(0), Qt::SystemLocaleShortDate);
						filterDate2 = QDate::fromString(dates.at(1), Qt::SystemLocaleShortDate);
						if ( !filterDate.isValid() || !filterDate2.isValid() ) {
							return false;
						}
						if ( filterDate > filterDate2 ) {
							QDate tmp = filterDate2;
							filterDate2 = filterDate;
							filterDate = tmp;
						}
						if ( ! ( fld->value().toDate() > filterDate && fld->value().toDate() < filterDate2 ) ) {
							return false;
						}
					}
				} else {
					filterDate = QDate::fromString(data, Qt::SystemLocaleShortDate);
					if ( !filterDate.isValid() ) {
						return false;
					}
					if ( !fld->checkValue(filterDate, op) ) {
						return false;
					}
				}
		} else {
			if ( !fld->checkValue(data, op, sensivity) ) {
				return false;
			}
		}
	}

	return true;
}

/*!
  Esta función es una que encuentra la relación adecuada o el field adecuado según el filtro explicado
  indicado en \a relationFilter y \a relationName. Por ejemplo:
  relationName: presupuestos_ejemplares.presupuestos_actividades
  relationFilter: id_numejemplares=1234
  */
DBObject * BaseBean::navigateThrough(const QString &path, const QString &relationFilters)
{
	QStringList filters = relationFilters.split(";");
	QStringList paths = path.split(".");
	DBRelation *rel = NULL;
	DBField *field = NULL;
	BaseBean *tmp = this;
	if ( filters.size() <= paths.size() ) {
		for ( int i = 0 ; i < paths.size() ; i++ ) {
			if ( tmp != NULL ) {
				rel = tmp->relation(paths.at(i));
				if ( rel != NULL ) {
					QSharedPointer<BaseBean> shBean;
					if ( filters.size() <= i || filters.at(i).isEmpty() ) {
						if ( rel->metadata()->type() == DBRelationMetadata::MANY_TO_ONE ) {
							shBean = rel->father();
						} else {
							shBean = rel->firstChild();
						}
					} else {
						shBean = rel->childByFilter(filters.at(i));
					}
					tmp = shBean.data();
				} else {
					if ( i == (paths.size() -1) ) {
						field = tmp->field(paths.at(i));
					} else {
						return NULL;
					}
				}
			}
		}
	}
	if ( field != NULL ) {
		return field;
	}
	return rel;
}

/*!
  Se borra el observador actual, y además los de los fields hijos
  */
void BaseBean::deleteObserver()
{
	if ( m_observer != NULL ) {
		delete m_observer;
		m_observer = NULL;
	}
	foreach ( DBField *fld, d->m_fields ) {
		fld->deleteObserver();
	}
}

/*!
  Valida si los datos del bean, cumplen con las reglas de validación establecidas
  */
bool BaseBean::validate()
{
	if ( this->m_observer != NULL ) {
		BaseBeanObserver *obs = qobject_cast<BaseBeanObserver *>(m_observer);
		return obs->validate();
	} else {
		QScopedPointer<BaseBeanValidator> val (new BaseBeanValidator(this));
		val->setBean(this);
		return val->validate();
	}
}

/*!
  Devuelve los mensajes de validación incorrectos que pudiesen surgir tras obtener validate() = false.
  Es decir, lo normal sera tener algo asi en codigo javascript:
  if ( !bean.validate() ) {
	QMessageBox.warning(bean.validateMessagesHtml();
  }
 */
QString BaseBean::validateMessages()
{
	if ( this->m_observer != NULL ) {
		BaseBeanObserver *obs = qobject_cast<BaseBeanObserver *>(m_observer);
		return obs->validateMessages();
	} else {
		QScopedPointer<BaseBeanValidator> val (new BaseBeanValidator(this));
		val->setBean(this);
		return val->validateMessages();
	}
}

/*!
  Devuelve los mensajes de validación incorrectos que pudiesen surgir tras obtener validate() = false.
  Los devuelve correctamente formateados en HTML.
  Es decir, lo normal sera tener algo asi en codigo javascript:
  if ( !bean.validate() ) {
	QMessageBox.warning(bean.validateMessagesHtml();
  }
 */
QString BaseBean::validateHtmlMessages()
{
	if ( this->m_observer != NULL ) {
		BaseBeanObserver *obs = qobject_cast<BaseBeanObserver *>(m_observer);
		return obs->validateHtmlMessages();
	} else {
		QScopedPointer<BaseBeanValidator> val (new BaseBeanValidator(this));
		val->setBean(this);
		return val->validateHtmlMessages();
	}
}

QDateTime BaseBean::loadTime()
{
	return d->m_loadTime;
}

void BaseBean::setLoadTime(const QDateTime &time)
{
	d->m_loadTime = time;
}

/*!
  Limpia toda la estructura de datos de este bean. OJO: Si hay hijos o padres con datos pendientes de guardar,
  estos se perderán. Si hay fields modificados, estos se perderán también
  @params onlyFields Excluyente, si está a true, nunca limpiará hijos ni padres, aunque childs o fathers esté a true
  @params childs Incluye en el limpiado el borrado de los hijos
  @params fathers Incluye en el limpiado el borrado de los padres.
  */
void BaseBean::clean(bool onlyFields, bool childs, bool fathers)
{
	foreach (DBField *fld, d->m_fields) {
		fld->setValue(QVariant());
		fld->setModified(false);
	}
	if ( onlyFields ) {
		return;
	}
	if ( childs ) {
		foreach (DBRelation *rel, d->m_relations) {
			if ( rel->metadata()->type() == DBRelationMetadata::ONE_TO_MANY || rel->metadata()->type() == DBRelationMetadata::ONE_TO_ONE ) {
				rel->removeAllChilds();
			}
		}
	}
	if ( fathers ) {
		foreach (DBRelation *rel, d->m_relations) {
			if ( rel->metadata()->type() == DBRelationMetadata::MANY_TO_ONE  ) {
				rel->father()->clean(true);
			}
		}
	}
}

/*!
  Devuelve un objeto idéntico a este
  */
QSharedPointer<BaseBean> BaseBean::clone()
{
	if ( d->m->tableName().isEmpty() ) {
		return QSharedPointer<BaseBean>();
	}
	QSharedPointer<BaseBean> copyBean = BeansFactory::instance()->newQBaseBean(d->m->tableName(), false);
	foreach (DBField *fld, d->m_fields) {
		DBField *fldCopy = copyBean->field(fld->dbFieldName());
		if ( fldCopy != NULL ) {
			fldCopy->setValueFromInternal(fld->value());
		}
	}
	copyBean->setDbState(BaseBean::UPDATE);
	return copyBean;
}
