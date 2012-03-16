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
#include "dao/beans/dbrelation.h"
#include "dao/beans/basebean.h"
#include "dao/beans/basebeanmetadata.h"
#include "dao/beans/dbfield.h"
#include "dao/beans/dbfieldmetadata.h"
#include "dao/beans/beansfactory.h"
#include "dao/beans/dbrelationmetadata.h"
#include "dao/basedao.h"
#include "widgets/dbdetailview.h"
#include <QDebug>
#include <QSharedPointer>

class DBRelationPrivate
{
//	Q_DECLARE_PUBLIC(DBRelation)
public:
	/** Referencia al bean propietario de esta relación */
	BaseBean *m_ownerBean;
	/** Si estamos en una relación M1, una referencia al bean father. TODO No me gusta mucho esto, pero
	  por el momento es lo tengo. La idea aquí es: Tenemos esta estructura:
	  presupuestos_cabecera ---> relation("presupuestos_ejemplares") ---> presupuestos_ejemplares
	  ---> relation("presupuestos_cabecera") ---> presupuestos_cabecera
	  Ese último presupuestos_cabecera debe ser establecido en memoria manualmente */
	QSharedPointer<BaseBean> m_father;
	/** Este será el padre que tendrán las relaciones que descienda de esta y que
	  devuelvan el padre original
	  Presupuestos_Cabecera -----> RelationEjemplares (fatherForM1Relation = Presupuestos_Cabecera)
		---> PresupuestosEjemplares ---> RelationPresCab (cogerá fahter de fatherForRelationM1) */
	QSharedPointer<BaseBean> m_fatherForM1Relation;
	/** Referencia a los hijos del bean raiz de esta relación */
	BaseBeanPointerList m_childs;
	/** Se obtienen los hijos con algún filtro */
	QString m_filter;
	/** ¿Están los hijos cargados? */
	bool m_childrensLoaded;
	/** ¿Hay hijos cuyos datos se han modificado? */
	bool m_childrensModified;
	/** Puntero a los metadatos */
	DBRelationMetadata *m;

	DBRelationPrivate();
	void sortChilds(BaseBeanPointerList &list, const QString &order);
	int colocar(BaseBeanPointerList &list, int beginning, int end, const QString &order);
	void Quicksort(BaseBeanPointerList &list, int beginning, int end, const QString &order);
};

DBRelationPrivate::DBRelationPrivate()
{
	m_childrensLoaded = false;
	m_childrensModified = false;
	m = NULL;
	m_ownerBean = NULL;
}

DBRelation::DBRelation(QObject *parent) : DBObject(parent), d(new DBRelationPrivate)
{
	d->m_ownerBean = qobject_cast<BaseBean *>(parent);
}

DBRelation::~DBRelation()
{
	d->m_childs.clear();
	delete d;
}

/*!
  Indica si los hijos de esta relación se han cargado en memoria.
  Si el bean root se está creando de nuevo, siempre tiene los hijos cargados
  */
bool DBRelation::childrensLoaded()
{
	if ( d->m_ownerBean->dbState() == BaseBean::INSERT &&
			(d->m->type() == DBRelationMetadata::ONE_TO_MANY ||
			 d->m->type() == DBRelationMetadata::ONE_TO_ONE) ) {
		return true;
	}
	return d->m_childrensLoaded;
}

/*!
  Indica si algún hijo ha sido modificado
  */
bool DBRelation::childrensModified()
{
	return d->m_childrensModified;
}

DBRelationMetadata * DBRelation::metadata() const
{
	return d->m;
}

void DBRelation::setMetadata(DBRelationMetadata *m)
{
	d->m = m;
	DBField *rootField = d->m_ownerBean->field(d->m->rootFieldName());
	if ( rootField != NULL ) {
		connect (rootField, SIGNAL(valueModified(QVariant)), this, SLOT(updateChildrens()));
	}
}

/*!
 Reacciona a los eventos que se desencadenan cuando el value del field maestro de esta relación
 cambia su valor. ¿Hay que actualizar los childs?
*/
void DBRelation::updateChildrens()
{
	DBField *rootField = d->m_ownerBean->field(d->m->rootFieldName());

	if ( rootField == NULL ) {
		return;
	}
	if ( d->m->type() == DBRelationMetadata::MANY_TO_ONE ) {
		if ( !d->m_father.isNull() ) {
			QString where = QString("%1 = %2").arg(d->m->childFieldName()).arg(rootField->sqlValue());
			if ( BaseDAO::selectFirst(d->m_father, where) ) {
				d->m_father->blockSignals(true);
				d->m_father->setDbState(BaseBean::UPDATE);
				d->m_father->blockSignals(false);
			} else {
				d->m_father->clean(true);
				d->m_father->setDbState(BaseBean::INSERT);
			}
			if ( d->m_father->observer() != NULL ) {
				d->m_father->observer()->sync();
			}
		}
	} else if ( d->m->type() == DBRelationMetadata::ONE_TO_ONE || d->m->type() == DBRelationMetadata::ONE_TO_MANY ) {
		foreach ( QSharedPointer<BaseBean> bean, d->m_childs ) {
			DBField *fldChild = bean->field(d->m->childFieldName());
			if ( fldChild != NULL ) {
				fldChild->setValue(rootField->value());
			}
		}
	}
	emit (rootFieldChanged());
}

/*!
  Obtiene el primer bean de la lista d->m_childs. Útil para la función father
  @see father
  */
QSharedPointer<BaseBean> DBRelation::firstChild(const QString &order)
{
	if ( d->m_childs.size() == 0 ) {
		if ( !childrensLoaded() ) {
			childs(order);
		}
		if ( d->m_childs.size() == 0 ) {
			return QSharedPointer<BaseBean>();
		}
	} else {
		if ( !order.isEmpty() ) {
			BaseBeanPointerList list = d->m_childs;
			d->sortChilds(list, order);
			return list.at(0);
		}
	}
	return d->m_childs.at(0);
}

/*!
  Devuelve el hijo en la posición index
  */
QSharedPointer<BaseBean> DBRelation::child(int index)
{
	if ( index > -1 && index < d->m_childs.size() ) {
		return d->m_childs.at(index);
	}
	return QSharedPointer<BaseBean>();
}

/*!
  Devuelve el hijo de esta relación cuya primary key es id
  */
QSharedPointer<BaseBean> DBRelation::childByPk(const QVariant &id)
{
	QSharedPointer<BaseBean> result;
	QVariantMap map = id.toMap();

	foreach ( QSharedPointer<BaseBean> child, d->m_childs ) {
		bool check = true;
		QMapIterator<QString, QVariant> i(map);
		while (i.hasNext()) {
			i.next();
			DBField *fld = child->field(i.key());
			if ( fld == NULL || fld->value() != i.value() ) {
				check = false;
			}
		}
		if ( check ) {
			result = child;
		}
	}
	return result;
}

/*!
  Devuelve el PRIMER hijo de esta relación cuyo value en el field dbField es value
  */
QSharedPointer<BaseBean> DBRelation::childByField(const QString &dbField, const QVariant &value)
{
	QSharedPointer<BaseBean> result;
	foreach ( QSharedPointer<BaseBean> child, d->m_childs ) {
		DBField *fld = child->field(dbField);
		if ( fld->value() == value ) {
			return child;
		}
	}
	return result;
}

/*!
  Devuelve el primer hijo que cumpla la condición especificada de la forma
  dbFieldNamePrimaryKey1='value1';dbFieldNamePrimaryKey2='value2'
  También admite condiciones como
  actividades.id_categoria = 2
  Es decir, mira sobre campos padres
  */
QSharedPointer<BaseBean> DBRelation::childByFilter(const QString &filter)
{
	// Primero obtenemos las condiciones
	QStringList conditions = filter.split(";");

    if ( d->m->type() == DBRelationMetadata::MANY_TO_ONE ) {
        return father();
    } else {
        // Iteramos por cada bean hijo, y por cada condición de filtrado.
        foreach ( QSharedPointer<BaseBean> bean, childs() ) {
            bool result = true;
            foreach ( QString condition, conditions ) {
                result = bean->checkFilter(condition);
            }
            if ( result ) {
                return bean;
            }
        }
    }
	return QSharedPointer<BaseBean>();
}

/*!
  Devuelve los hijos que cumplan con la condición especificada de la forma
  dbFieldNamePrimaryKey1='value1' AND dbFieldNamePrimaryKey2='value2'
  También admite condiciones como
  actividades.id_categoria = 2
  Es decir, mira sobre campos padres
  */
BaseBeanPointerList DBRelation::childsByFilter(const QString &filter, const QString &order)
{
	// Primero obtenemos las condiciones
	BaseBeanPointerList list;

	if ( filter.isEmpty() ) {
		return childs();
	}
	// Iteramos por cada bean hijo, y por cada condición de filtrado.
	foreach ( QSharedPointer<BaseBean> bean, childs() ) {
		bool result = true;
		result = bean->checkFilter(filter);
		if ( result ) {
			list << bean;
		}
	}
	if ( !order.isEmpty() ) {
		d->sortChilds(list, order);
	}
	return list;
}

/*!
  Cuando hay alguna modificación, este slot será llamado al producirse la señal beanModified
  y se marcará que algún hijo, ha sido modificado. También emite el hijo que ha sido modificado
  */
void DBRelation::setChildrensModified(BaseBean *bean, bool value)
{
	d->m_childrensModified = true;
	emit childModified(bean, value);
}

void DBRelation::uncheckChildrensModified()
{
	d->m_childrensModified = false;
}

/*!
 Indica cuántos hijos tiene esta relación teniendo en cuenta el filtro. Si no se han cargado
 los hijos, entonces, se realiza una consulta SQL a base de datos, para conocer el número de hijos.
 Si se han cargado, devuelve el tamaño de d->m_childs.
 Examina además los childs que están marcados como para borrar, de cara a tenerlos en cuenta
 para el conteo
*/
int DBRelation::childsCount(bool includeToBeDeleted)
{
	int count = 0;
	if ( childrensLoaded() ) {
		count = d->m_childs.size();
	} else {
		count = BaseDAO::selectCount(d->m->tableName(), sqlRelation());
	}
	if ( !includeToBeDeleted ) {
		foreach (QSharedPointer<BaseBean> bean, d->m_childs) {
			if ( bean->dbState() == BaseBean::TO_BE_DELETED ) {
				count--;
			}
		}
	}
	return count;
}

/*!
  Crea un child (es una función factoría) y la añade a esta relación. Emite la señal childInserted
  */
QSharedPointer<BaseBean> DBRelation::newChild(QSharedPointer<BaseBean> copy, int pos)
{
	/* Contendrá la clave en el bean padre */
	QVariant fatherKeyValue = d->m_ownerBean->fieldValue(d->m->rootFieldName());
	QSharedPointer<BaseBean> child;
	if ( copy.isNull() ) {
		child = BeansFactory::instance()->newQBaseBean(d->m->tableName(), false);
	} else {
		child = copy;
	}

	child->setDbState(BaseBean::INSERT);
	// Le damos al bean que se va a crear, el valor del identificador de la relación
	child->setFieldValueFromInternal(d->m->childFieldName(), fatherKeyValue);

	/** Ahora debemos ver si hay dentro relaciones que apunten a este bean (M1). La idea aquí es: Tenemos esta estructura:
	presupuestos_cabecera ---> relation("presupuestos_ejemplares") ---> presupuestos_ejemplares
		---> relation("presupuestos_cabecera") ---> presupuestos_cabecera
	Ese último presupuestos_cabecera debe ser el mismo que inicia la cadena. Esto se ajusta manualmente*/
	DBRelation *rel = child->relation(d->m_ownerBean->metadata()->tableName());
	if ( rel != NULL && rel->metadata()->type() == DBRelationMetadata::MANY_TO_ONE ) {
		rel->setFather(d->m_fatherForM1Relation);
	}

	// Puede que en los defaultValueScript se necesite acceder al valor del father de la relación. Por
	// eso ahora se inician los valores por defecto, ya que en este punto del código estamos seguros de que
	// el bean nuevo está correctamente inicializado
	foreach (DBField *fld, child->fields()) {
		fld->defaultValue();
	}

	if ( pos != -1 ) {
		d->m_childs.insert(pos, child);
	} else {
		d->m_childs.append(child);
	}
	connections(child);
	if ( pos == -1 ) {
		emit childInserted(child.data(), d->m_childs.size());
	} else {
		emit childInserted(child.data(), pos);
	}
	emit childDbStateModified(child.data(), BaseBean::INSERT);
	return child;
}

/*!
 Establece las conexiones necesarias entre los hijos y este objeto relación, para conocer
 cuándo estos modifican su estado
 */
void DBRelation::connections(const QSharedPointer<BaseBean> &child)
{
	// Cuando se modifique un hijo, se modifica el padre de esta relación. Ello se hará
	// porque el padre, está enganchado a la señal childModified, y sabrá que el hijo ha sido modificado
	// Guardamos internamente que hay hijos modificados. setChildresModified emite la señal childModified
	connect(child.data(), SIGNAL(beanModified(BaseBean *, bool)), this, SLOT(setChildrensModified(BaseBean *, bool)));
	// De la misma manera, cuando un bean hijo modifica su estado, el padre aparece como modificado.
	// Por ejemplo, se marcha un child para ser borrado. El padre ha sido modificado por tanto.
	connect(child.data(), SIGNAL(dbStateModified(BaseBean *, int)), this, SIGNAL(childDbStateModified(BaseBean *,int)));
	connect(child.data(), SIGNAL(fieldModified(QString,QVariant)), this, SLOT(childFieldBeanModified(QString,QVariant)));
}

/*!
  Se ha modificado un field de algún child. En ese caso, se emite una señal que avise, por ejemplo a los
  modelos que presentan los resultados de esta relación. Se avisa también al bean padre que lo mismo
  debe recalcular sus datos
  */
void DBRelation::childFieldBeanModified(const QString &fieldName, const QVariant &value)
{
	BaseBean *sender = qobject_cast<BaseBean *> (QObject::sender());
	if ( sender != NULL ) {
		emit fieldChildModified(sender, fieldName, value);
	}
	foreach ( DBField *fld, d->m_ownerBean->fields() ) {
		QList<QHash<QString, QString> > aggregateCalcs = fld->metadata()->aggregateCalcs();
		for ( int i = 0 ; i < aggregateCalcs.size() ; i++ ) {
			QHash<QString, QString> hash = aggregateCalcs.at(i);
			if ( hash.value("relation") == d->m->tableName() ) {
				fld->value();
			}
		}
	}
}

/*!
  Elimina de la relación, el hijo de primary key PK. Lo elimina de forma fuerte,
  esto es, borrando el bean.
  */
void DBRelation::removeChild(QVariant pk)
{
	for ( int i = 0 ; i < d->m_childs.size() ; i++ ) {
		QSharedPointer<BaseBean> child = d->m_childs.at(i);
		if ( child->pkEqual(pk) ) {
			emit childDeleted(child.data(), i);
			d->m_childs.removeAt(i);
			return;
		}
	}
}

/*!
  Elimina de la relación, el hijo cuyo objectName es el pasado. El objectName
  es un valor único para cualquier entidad de este sistema
  */
void DBRelation::removeChildByObjectName(const QString &objectName)
{
	for ( int i = 0 ; i < d->m_childs.size() ; i++ ) {
		QSharedPointer<BaseBean> child = d->m_childs.at(i);
		if ( child->objectName() == objectName ) {
			emit childDeleted(child.data(), i);
			d->m_childs.removeAt(i);
			return;
		}
	}
}

/*!
  Elimina de forma de fuerte (borrando el bean de la memoria) todos los beans hijos
  */
void DBRelation::removeAllChilds()
{
	for ( int i = 0 ; i < d->m_childs.size() ; i++ ) {
		QSharedPointer<BaseBean> child = d->m_childs.at(i);
		emit childDeleted(child.data(), i);
	}
	d->m_childs.clear();
}

/*!
  Devuelve únicamente los childs que han sido modificados (por ello no realizará ningún tipo de consulta
  a base de datos). Es una función muy útil para actualizaciones en cascada
  */
BaseBeanPointerList DBRelation::modifiedChilds()
{
	BaseBeanPointerList list;
	if ( d->m_childs.isEmpty() || !d->m_childrensLoaded ) {
		return list;
	}
	foreach ( QSharedPointer<BaseBean> bean, d->m_childs ) {
		if ( bean->modified() ) {
			list.append(bean);
		}
	}
	return list;
}

/*!
  Devuelve los hijos de esta relación. Si no los ha obtenido de base de datos
  se va a ella a obtenerlos. Teóricamente, podemos devolver esta lísta porque se compone
  de punteros QSharedPointer, y el puntero se comparte de forma segura.
  */
BaseBeanPointerList DBRelation::childs (const QString &order)
{
	// ¿Se han obtenido los hijos de esta relación? Si no es así, se obtienen. Ojo,
	// si se está creando el bean padre, y aquí vienen los relacionados, los hijos siempre
	// estarán cargados
	QString finalOrder;
	finalOrder = (order.isEmpty() ? d->m->order() : order);
	if ( !childrensLoaded() && d->m_ownerBean != NULL &&
			d->m_ownerBean->dbState() != BaseBean::INSERT &&
		 ( (d->m->type() == DBRelationMetadata::ONE_TO_MANY || d->m->type() == DBRelationMetadata::ONE_TO_ONE ) ) ) {
		BaseBeanPointerList results;
		// Esta WHERE que se construye aquí contiene lo necesario desde el FROM:
		BaseDAO::select(results, d->m->tableName(), sqlRelation(), finalOrder);
		// Lo hacemos así para que se realicen las conexiones necesarias de cara a conocer las modificaciones
		foreach ( QSharedPointer<BaseBean> bean, results ) {
			bean->blockSignals(true);
			bean->setDbState(BaseBean::UPDATE);
			d->m_childs.append(bean);
			bean->blockSignals(false);
			connections(bean);
		}
		d->m_childrensLoaded = true;
	} else if ( d->m->type() == DBRelationMetadata::MANY_TO_ONE ) {
		BaseBeanPointerList tmp;
		tmp.append(father());
		return tmp;
	}
	if ( !order.isEmpty() ) {
		d->sortChilds(d->m_childs, order);
	}
	return d->m_childs;
}

/*!
  Para una relación de tipo M->1, obtiene el bean de esa relación, que tiene
  un nombre especial: Padre
  */
QSharedPointer<BaseBean> DBRelation::father()
{
	if ( d->m->type() == DBRelationMetadata::MANY_TO_ONE ) {
		if ( d->m_father.isNull() ) {
			d->m_father = BeansFactory::instance()->newQBaseBean(d->m->tableName(), false);
			if ( d->m_father.isNull() ) {
				qDebug() << "DBRelation::father: No existe la tabla [ " << d->m->tableName() << " ]";
				return d->m_father;
			}
			DBField *fld = d->m_ownerBean->field(d->m->rootFieldName());
			QString where = QString("%1 = %2").arg(d->m->childFieldName()).arg(fld->sqlValue());
			if ( BaseDAO::selectFirst(d->m_father, where) ) {
				d->m_father->blockSignals(true);
				d->m_father->setDbState(BaseBean::UPDATE);
				d->m_father->blockSignals(false);
			} else {
				d->m_father->setDbState(BaseBean::INSERT);
			}
		}
	}
	return d->m_father;
}

QSharedPointer<BaseBean> DBRelation::fatherForM1Relation()
{
	return d->m_fatherForM1Relation;
}

void DBRelation::setFather (const QSharedPointer<BaseBean> &bean)
{
	d->m_father = bean;
}

void DBRelation::setFatherForM1Relation(const QSharedPointer<BaseBean> &bean)
{
	d->m_fatherForM1Relation = bean;
}

QString DBRelation::filter()
{
	return d->m_filter;
}

void DBRelation::setFilter(const QString &filter)
{
	if ( filter != d->m_filter ) {
		d->m_filter = filter;
		d->m_childs.clear();
		d->m_childrensLoaded = false;
	}
}


/*!
  Con esta función se obtiene el sql necesario para encontrar los beans childs que
  dependen del bean root, a partir de las columnas de base de datos definidas para ello
  */
QString DBRelation::fetchChildSqlWhere (const QString &aliasChild)
{
	QString sql, alias;
	if ( !aliasChild.isEmpty() ) {
		alias = QString("%1.").arg(aliasChild);
	}
	if ( d->m_ownerBean != NULL ) {
		DBField *orig = d->m_ownerBean->field(d->m->rootFieldName());
		if ( orig != NULL ) {
			sql = QString("%1%2 = %3").arg(alias).arg(d->m->childFieldName()).arg(orig->sqlValue());
		} else {
			qDebug() << "DBRelation::fetchChildSqlWhere: [" << d->m->tableName() << "]. No tiene definido el campo de relacion en el padre-root.";
		}
	}
	return sql;
}

QString DBRelation::fetchFatherSqlWhere(const QString &aliasRoot)
{
	QString sql, alias;
	if ( !aliasRoot.isEmpty() ) {
		alias = QString(".%1").arg(aliasRoot);
	}
	if ( d->m_ownerBean != NULL ) {
		DBField *orig = d->m_ownerBean->field(d->m->rootFieldName());
		if ( orig != NULL ) {
			sql = QString("%1%2 = %3").arg(alias).arg(d->m->rootFieldName()).arg(orig->sqlValue());
		} else {
			qDebug() << "DBRelation::fetchFatherSqlWhere: [" << d->m->tableName() << "]. No tiene definido el campo de relacion en el padre-root.";
		}
	}
	return sql;
}

/*!
  Devuelve el SQL necesario para obtener los hijos en la función child. Añadirá
  los alias necesarios, y obtendrá la información de filtro de d->m_filter
  */
QString DBRelation::sqlRelation()
{
	QString sql;

	if ( d->m_filter.isEmpty() ) {
		sql = QString("%2").arg(fetchChildSqlWhere());
	} else {
		sql = QString("%1 AND %2").arg(fetchChildSqlWhere()).arg(d->m_filter);
	}

	return sql;
}

/*!
  Devuelve el alias con el que esta tabla aparecerá en las sql
  */
QString DBRelation::sqlChildTableAlias()
{
	QString result;
	// Obtenemos el número de esta relación entre todas las del bean raíz
	int idx = d->m_ownerBean->relationIndex(d->m->tableName());
	if ( idx != -1 ) {
		result = QString("t%1").arg(idx);
	} else {
		qDebug() << "DBRelation::sqlChildTableAlias: [" << d->m->tableName() << "]. Esta relacion no esta definida en el root.";
	}
	return result;
}

/*!
 DBField del que depende la relación. Es la key master de la que se obtienen el resto
*/
DBField * DBRelation::masterField()
{
	DBField *fld = NULL;
	if ( d->m_ownerBean != NULL ) {
		fld = d->m_ownerBean->field(d->m->rootFieldName());
	}
	return fld;
}

BaseBean *DBRelation::ownerBean()
{
	return d->m_ownerBean;
}

/*!
  Tenemos que decirle al motor de scripts, que DBSearchDlg se convierte de esta forma a un valor script
  */
QScriptValue DBRelation::toScriptValue(QScriptEngine *engine, DBRelation * const &in)
{
	return engine->newQObject(in);
}

void DBRelation::fromScriptValue(const QScriptValue &object, DBRelation * &out)
{
	out = qobject_cast<DBRelation *>(object.toQObject());
}

QScriptValue DBRelation::toScriptValueSharedPointer(QScriptEngine *engine, const QSharedPointer<DBRelation> &in)
{
	return engine->newQObject(in.data());
}

void DBRelation::fromScriptValueSharedPointer(const QScriptValue &object, QSharedPointer<DBRelation> &out)
{
	out = QSharedPointer<DBRelation>(qobject_cast<DBRelation *>(object.toQObject()));
}

void DBRelationPrivate::sortChilds(BaseBeanPointerList &list, const QString &order)
{
	Quicksort(list, 0, list.size() -1 , order);
}

int DBRelationPrivate::colocar(BaseBeanPointerList &list, int beginning, int end, const QString &order)
{
	int i;
	int pivote;
	DBField *valor_pivote;

	pivote = beginning;
	valor_pivote = list.at(pivote)->field(order);
	for ( i = beginning + 1 ; i <= end ; i++ ) {
		DBField *temp = list.at(i)->field(order);
		if ( *temp < *valor_pivote ) {
				pivote++;
				list.swap(i, pivote);

		}
	}
	list.swap(beginning, pivote);
	return pivote;
}

void DBRelationPrivate::Quicksort(BaseBeanPointerList &list, int beginning, int end, const QString &order)
{
	 int pivote;
	 if ( beginning < end ) {
		pivote = colocar(list, beginning, end, order);
		Quicksort(list, beginning, pivote-1, order);
		Quicksort(list, pivote + 1, end, order);
	 }
}

/*!
  Descarga los beans de memoria. No significa que los borre, simplemente los quita de memoria.
  Esta función es útil cuando por ejemplo, se quieren recargar todos los beans de base de datos.
  Hay que decidir qué hacer con los beans que no se hayan salvado en base de datos. Si ignoreNotSavedBeans
  está a true, directamente se ignoran los cambios y se eliminan de memoria. Si está a false
  y hay beans hijos sin cargar no los descarga y devuelve false
  */
bool DBRelation::unloadChildrens(bool ignoreNotSavedBeans)
{
	if ( !ignoreNotSavedBeans ) {
		if ( childrensModified() ) {
			return false;
		}
	}
	d->m_childs.clear();
	d->m_childrensLoaded = false;
	d->m_childrensModified = false;
	return true;
}
