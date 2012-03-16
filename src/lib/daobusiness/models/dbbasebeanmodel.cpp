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
#include <QSqlQueryModel>
#include <QCoreApplication>
#include <QStringBuilder>
#include <QMessageBox>
#include "configuracion.h"
#include "dbbasebeanmodel.h"
#include "dao/database.h"
#include "dao/basedao.h"
#include "dao/beans/basebean.h"
#include "dao/beans/beansfactory.h"
#include "dao/beans/dbfield.h"
#include "dao/beans/basebeanmetadata.h"
#include "dao/beans/dbfieldmetadata.h"
#include "models/basebeanmodel.h"
#include "models/envvars.h"
#include "models/querythread.h"
#include <math.h>
#include <QTimer>

class DBBaseBeanModelPrivate
{
	Q_DECLARE_PUBLIC(DBBaseBeanModel)
public:
	/** Tabla a la que da soporte este modelo. Si viewOnGrid no es nulo, se presentarán
		los datos de viewOnGrid, aunque el método bean devolverá el bean original */
	QString m_tableName;
	/** Metadatos del bean que presenta este modelo. Estos metadatos pueden corresponder
	a la vista que presenta el modelo. */
	BaseBeanMetadata *m_metadata;
	/** Estructura con los beans que se tienen. Se van leyendo progresivamente de la BBDD */
	QVector<QSharedPointer<BaseBean> > m_vectorBean;
	/** Para los beans de la estructura anterior, indica cuáles se han obtenido de bbdd y cuáles no */
	QVector<bool> m_beansFecthed;
	/** Número de registros que pertenece a este modelo. No tiene porqué coincidir con m_vectorBean.size()
	  ya que esta estructura se va cargando poco a poco. */
	int m_rowCount;
	/** Esto es un filtro fuerte: De base de datos (por ejemplo para evitar un número muy elevado
	  de registros en la consulta */
	QString m_where;
	/** Este órden sólo hace referencia al orden INICIAL con el que se obtienen los datos de base de datos */
	QString m_order;
	/** Esta es la ventana con la que se obtendrán los beans (el offset) */
	int m_offset;
	/** Indica si se borran físicamente los registros de base de datos o no */
	bool m_deleteFromDB;
	/** Timer que permitira comprobar la validez de los registros que se muestran */
	QTimer *m_timer;
	/** Modelo estático (sólo lee una vez de base de datos) o no */
	bool m_staticModel;

	DBBaseBeanModel *q_ptr;

	DBBaseBeanModelPrivate(DBBaseBeanModel *qq) : q_ptr(qq) {
		m_rowCount = 0;
		m_offset = 50;
		m_deleteFromDB = false;
		m_staticModel = false;
	}

	void buildSqlSelect(const QString &tableName, const QString &where, const QString &order,
								const QList<DBFieldMetadata *> &fields, QString &sql);
	void countTotalBeans();
	void fetchBeans(int position);
	void extractOrder();
	void addEnvVarToWhere();
};

/*!
  Añade las variables de entorno que se hayan establecido a la claúsula where.
  Se añaden si los metadatos que se visualizarán con este modelo contienen una referencia
  a la variable de entorno, por que está en \a envVarsFilter, y que además contiene
  un campo con ese nombre.
*/
void DBBaseBeanModelPrivate::addEnvVarToWhere()
{
	if ( m_metadata == NULL ) {
		return;
	}
	QString where = m_metadata->whereFromEnvVars();
	if ( !where.isEmpty() ) {
		if ( !m_where.isEmpty() ) {
			m_where = QString("%1 AND ").arg(m_where);
		}
		m_where = QString ("%1%2").arg(m_where).arg(m_metadata->whereFromEnvVars());
	}
}


/*!
  Crea un modelo para presentar los beans de tableName. Si viewOnGrid no es nulo,
  se mostrarán los datos de viewOnGrid, aunque el método bean, devolverá el bean original.
  Se puede indicar también un filtro fuerte en \a where que se
  traducirá en una claúsula SQL. \a order hará referencia a un primer filtrado de los datos. Aunque
  este modelo se utiliza con \a FilterBaseBeanModel, puede ser útil dar un primer orden para evitar
  una primer procesamiento de datos intensivos por parte \a FilterBaseBeanModel en la primera
  visualización.
  @see FilterBaseBeanModel
  @see BaseBeanMetadata
 */
DBBaseBeanModel::DBBaseBeanModel(const QString &tableName, const QString &where, const QString &order, bool staticModel, QObject *parent) :
	BaseBeanModel(parent), d(new DBBaseBeanModelPrivate(this))
{
	d->m_where = where;
	d->m_tableName = tableName;
	d->m_order = order;
	d->m_metadata = BeansFactory::metadataBean(tableName);
	d->m_timer = new QTimer(this);
	d->m_staticModel = staticModel;

	if ( !d->m_staticModel ) {
		startReloading();
	}

	if ( d->m_metadata == NULL ) {
		qCritical() << QString("DBBaseBeanModel::DBBaseBeanModel: No se ha podido crear el masterbean [%1]. La factoria no funciona correctamente.").arg(tableName);
	} else {
		if ( !d->m_metadata->viewOnGrid().isEmpty() ) {
			d->m_metadata = BeansFactory::metadataBean(d->m_metadata->viewOnGrid());
		}
		d->addEnvVarToWhere();
		d->extractOrder();
		d->countTotalBeans();
	}
}

DBBaseBeanModel::~DBBaseBeanModel ()
{
	foreach (QSharedPointer<BaseBean> bean, d->m_vectorBean) {
		if ( !bean.isNull() ) {
			bean.clear();
		}
	}
	delete d;
}

/*!
  Permite cambiar la cláusula de filtro fuerte (que se aplica en la SQL). Si refresh es true
  se actualiza todo el modelo. Si no, sólo se cambia internamente el valor
  */
void DBBaseBeanModel::setWhere(const QString &where, bool refresh)
{
	if ( refresh )
		emit QAbstractItemModel::layoutAboutToBeChanged();
	if ( d->m_where != where ) {
		d->m_where = where;
		d->addEnvVarToWhere();
		if ( refresh ) {
			d->countTotalBeans();
		}
	}
	if ( refresh )
		emit QAbstractItemModel::layoutChanged();
}

QString DBBaseBeanModel::whereClausule()
{
	return d->m_where;
}

void DBBaseBeanModel::setOrder(const QString &order)
{
	emit QAbstractItemModel::layoutAboutToBeChanged();
	if ( d->m_order != order ) {
		d->m_order = order;
		d->countTotalBeans();
	}
	emit QAbstractItemModel::layoutChanged();
}

QString DBBaseBeanModel::orderClausule()
{
	return d->m_order;
}

/*!
  Devuelve la SQL actualmente utilizada para obtener los datos del modelo
  */
QString DBBaseBeanModel::actualSql()
{
	QString query;
	d->buildSqlSelect(d->m_metadata->tableName(), d->m_where, d->m_order, d->m_metadata->fields(), query);
	return query;
}

/*!
  Devuelve la SQL utilizada para obtener el número de registros del modelo
  */
QString DBBaseBeanModel::actualCountSql()
{
	QString countQuery = QString ("SELECT count(*) FROM (%1) AS FOO").arg(actualSql());
	return countQuery;
}

bool DBBaseBeanModel::staticModel()
{
	return d->m_staticModel;
}

void DBBaseBeanModel::setStaticModel(bool value)
{
	d->m_staticModel = value;
}

int DBBaseBeanModel::offset()
{
	return d->m_offset;
}

QModelIndex DBBaseBeanModel::index(int row, int column, const QModelIndex &parent) const
{
	Q_UNUSED(parent)
	return createIndex(row, column, 0);
}

QModelIndex DBBaseBeanModel::parent(const QModelIndex &index) const
{
	Q_UNUSED(index)
	return QModelIndex();
}

int DBBaseBeanModel::rowCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent)
	return d->m_rowCount;
}

/*!
  Puede establecerse un orden inicial de visualización o de extracción de los datos.
  Esta función extrae ese orden. Esta idea es útil para un número muy elevado de registros
  a mostrar de forma que evita un primer ordenado por parte del modelo proxy.
  Si \a order está vacío, crea ese orden inicial a partir de la primary key.
  */
void DBBaseBeanModelPrivate::extractOrder()
{
	if ( m_order.isEmpty() ) {
		QList<DBFieldMetadata *> flds = m_metadata->pkFields();
		if ( flds.size() > 0 ) {
			m_order = QString("%1 ASC").arg(flds.at(0)->dbFieldName());
		}
	}
}

/*!
  Construye la sql del select. Para agilizar la consulta sólo se obtienen los datos que son
  visibles en grid y que forman parte de la Pk.
  */
void DBBaseBeanModelPrivate::buildSqlSelect(const QString &tableName, const QString &where, const QString &order,
							 const QList<DBFieldMetadata *> &fields, QString &sql)
{
	QList<DBFieldMetadata *> pkFields = m_metadata->pkFields();
	foreach ( DBFieldMetadata *field, fields ) {
		if ( !field->calculated() ) {
			if ( field->visibleGrid() || pkFields.contains(field) ) {
				if ( sql.isEmpty() ) {
					sql = QString("%1").arg(field->dbFieldName());
				} else {
					sql = QString("%1, %2").arg(sql).arg(field->dbFieldName());
				}
			} else {
				if ( sql.isEmpty() ) {
					sql = QString("'notVisible'");
				} else {
					sql = QString("%1, 'notVisible'").arg(sql);
				}
			}
		}
	}
	sql = QString("SELECT DISTINCT %1 FROM %2").arg(sql, tableName);
	if ( !where.isEmpty() ) {
		sql = sql % QString(" WHERE ") % where;
	}
	if ( !order.isEmpty() ) {
		sql = sql % QString(" ORDER BY ") % order;
	}
}

/*!
  Obtiene el número de beans que presenta este modelo. Este número se realiza
  mediante consulta a base de datos, pero no implica que se obtengan los beans.
  Deja en m_rowCount el número de filas, necesario para poblar el modelo
  */
void DBBaseBeanModelPrivate::countTotalBeans()
{
	QString query, queryCount;
	QVariant result;

	if ( m_metadata == NULL ) {
		return;
	}

	foreach (QSharedPointer<BaseBean> bean, m_vectorBean) {
		if ( !bean.isNull() ) {
			QObject::disconnect(bean.data(), SIGNAL(fieldModified(BaseBean *, QString, QVariant)),
				   q_ptr, SLOT(fieldBaseBeanModified(BaseBean *, QString, QVariant)));
		}
	}
	m_rowCount = 0;
	m_vectorBean.clear();
	m_beansFecthed.clear();

	buildSqlSelect(m_metadata->tableName(), m_where, m_order, m_metadata->fields(), query);
	queryCount = QString("SELECT count(*) FROM (%1) AS FOO").arg(query);

	// Si la query está cacheada, se optimiza todo esto
	if ( !m_metadata->isCached() ) {
		if ( BaseDAO::execute(queryCount, result) && result.isValid() ) {
			m_rowCount = result.toInt();
		} else {
			m_rowCount = 0;
		}
	} else {
		// Deben obtenerse a partir de position, la ventana en la que obtener beans
		if ( BaseDAO::executeCached(queryCount, result) ) {
			m_rowCount = result.toInt();
		} else {
			m_rowCount = 0;
		}
	}
	m_vectorBean.resize(m_rowCount);
	m_beansFecthed.resize(m_rowCount);
	m_beansFecthed.fill(false);
}

/*!
  Obtiene los beans definidos en m_offset a partir de la posición \a position
  */
void DBBaseBeanModelPrivate::fetchBeans(int position)
{
	// Deben obtenerse a partir de position, la ventana en la que obtener beans
	int offsetMultiply = ceil(position / m_offset);
	BaseBeanPointerList results;

	if ( BaseDAO::select(results, m_metadata->tableName(), m_where, m_order, m_offset, m_offset * offsetMultiply) ) {
		for ( int i = 0 ; i < results.size() ; i++ ) {
			m_vectorBean[i + (m_offset * offsetMultiply)] = results.at(i);
			m_beansFecthed[i + (m_offset * offsetMultiply)] = true;
			QObject::connect(results.at(i).data(), SIGNAL(fieldModified(BaseBean *, QString, QVariant)),
					   q_ptr, SLOT(fieldBaseBeanModified(BaseBean *, QString, QVariant)));
		}
	}
}

void DBBaseBeanModel::fieldBaseBeanModified(BaseBean *bean, const QString &dbFieldName, const QVariant &value)
{
	Q_UNUSED(value)
	for ( int i = 0 ; i < d->m_vectorBean.size() ; i++ ) {
		if ( !d->m_vectorBean.at(i).isNull() ) {
			if ( d->m_vectorBean.at(i)->objectName() == bean->objectName() ) {
				DBFieldMetadata *fld = d->m_vectorBean.at(i)->metadata()->field(dbFieldName);
				if ( fld != NULL ) {
					QModelIndex idx = index(i, fld->index());
					emit dataChanged(idx, idx);
				}
			}
		}
	}
}

int DBBaseBeanModel::columnCount ( const QModelIndex & parent ) const
{
	Q_UNUSED(parent);
	if ( d->m_metadata != NULL ) {
		return d->m_metadata->fields().size();
	}
	return 0;
}


QVariant DBBaseBeanModel::headerData ( int section, Qt::Orientation orientation, int role ) const
{
	QVariant returnData;
	QFont font;
	DBFieldMetadata *field = d->m_metadata->field(section);
	if ( field == NULL || orientation == Qt::Vertical ) {
		return BaseBeanModel::headerData(section, orientation, role);
	}
	switch ( role ) {
	case Qt::DisplayRole:
		return QObject::trUtf8(field->fieldName().toUtf8());

	case Qt::UserRole:
		return field->dbFieldName();

	case Qt::FontRole:
		font.setBold(true);
		return font;

	case Qt::DecorationRole:
	case Qt::ToolTipRole:
	case Qt::StatusTipRole:
	case Qt::SizeHintRole:
		// TODO: Estaria guapo esto
		break;
	case Qt::TextAlignmentRole:
		if ( field->type() == QVariant::Int || field->type() == QVariant::Double ) {
			returnData = int (Qt::AlignVCenter | Qt::AlignRight);
		} else {
			returnData = int (Qt::AlignVCenter | Qt::AlignLeft);
		}
		break;
	}
	return returnData;
}

Qt::ItemFlags DBBaseBeanModel::flags ( const QModelIndex & index ) const
{
	Qt::ItemFlags flags;
	DBFieldMetadata *field = d->m_metadata->field(index.column());
    if ( field == NULL ) {
        return flags;
    }
	if ( !field->serial() ) {
		flags = Qt::ItemIsEnabled ;
	}
	if ( field->type() == QVariant::Bool || BaseBeanModel::checkColumns().contains(field->dbFieldName())) {
		flags = flags | Qt::ItemIsUserCheckable;
	}
	// Importante añadir esto. Si no, ningún item será seleccionable
	flags = flags | Qt::ItemIsSelectable;
	return flags;
}

/*!
  Formatea y presenta los datos que internamente ha obtenido PERPQueryModel. Además
  va construyendo la estructura interna de beans.
  */
QVariant DBBaseBeanModel::data ( const QModelIndex & item, int role ) const
{
    QVariant dato, displayData;
	if ( !item.isValid() ) {
		return dato;
	}
	int column = item.column();
	DBFieldMetadata *field = d->m_metadata->field(column);
	if ( field == NULL ) {
		return dato;
	}

	if ( d->m_beansFecthed.size() == 0 || item.row() >= d->m_beansFecthed.size() ) {
		return dato;
	}

	// Ahora veamos si se ha obtenido el bean. Si no es así, se obtienen los beans en esa ventana
	if ( !d->m_beansFecthed.at(item.row()) ) {
		d->fetchBeans(item.row());
	}
	if ( d->m_beansFecthed.at(item.row()) ) {
		QSharedPointer<BaseBean> bean = d->m_vectorBean.at(item.row());
		if ( !bean.isNull() ) {
            dato = bean->fieldValue(item.column());
            displayData = bean->displayFieldValue(item.column());
		} else {
			return dato;
		}
	}

	if ( role == Qt::TextAlignmentRole ) {
		if ( field->type() == QVariant::Int || field->type() == QVariant::Double ) {
			return int(Qt::AlignRight | Qt::AlignVCenter);
		} else if ( field->type() == QVariant::String ) {
			return int(Qt::AlignLeft | Qt::AlignVCenter);
		} else {
			return int(Qt::AlignRight | Qt::AlignVCenter);
		}
	} else if ( role == Qt::DisplayRole && field->type() != QVariant::Bool ) {
		QMap<QString, QString> optionList = field->optionsList();
		if ( optionList.size() != 0 ) {
			return optionList[dato.toString()];
		} else {
            return displayData;
		}
	} else if ( role == Qt::UserRole ) {
		return dato;
	} else if ( role == Qt::CheckStateRole && BaseBeanModel::checkColumns().contains(field->dbFieldName()) ) {
		if ( m_checkedItems.contains(item.row()) && m_checkedItems[item.row()] ) {
			return Qt::Checked;
		} else {
			return Qt::Unchecked;
		}
	} else if ( role == Qt::FontRole ) {
		return field->fontOnGrid();
	} else if ( role == Qt::BackgroundRole && field->backgroundColor().isValid() ) {
		return QBrush(field->backgroundColor());
	} else if ( role == Qt::ForegroundRole && field->color().isValid() ) {
		return QBrush(field->color());
	} else if ( role == Qt::DecorationRole ) {
		if ( field->type() == QVariant::Bool ) {
			if ( dato.toBool() ) {
				QPixmap pix(":/aplicacion/images/ok.png");
				return pix.scaled(16, 16, Qt::KeepAspectRatio);
			} else {
				QPixmap pix(":/generales/images/delete.png");
				return pix.scaled(16, 16, Qt::KeepAspectRatio);
			}
		} else {
			QMap<QString, QString> optionIcons = field->optionsIcons();
			if ( optionIcons.size() != 0 ) {
				QString pixName = optionIcons[dato.toString()];
				QPixmap pix(pixName);
				if ( !pix.isNull() ) {
					return pix;
				}
			}
		}
	}
	return QVariant();
}

bool DBBaseBeanModel::setData ( const QModelIndex & index, const QVariant & value, int role )
{
	if ( !index.isValid() ) {
		return false;
	}
	DBFieldMetadata *field = d->m_metadata->field(index.column());
	if ( field == NULL ) {
		return false;
	}
	if ( role == Qt::CheckStateRole ) {
		bool ok;
		int v = value.toInt(&ok);
		if ( !ok ) { return false; }
		m_checkedItems[index.row()] = (v == Qt::Checked);
	}
	return true;
}

bool DBBaseBeanModel::removeRows ( int row, int count, const QModelIndex & parent )
{
	Q_UNUSED(parent)

	bool result = true;
	beginRemoveRows(QModelIndex(), row, row + count - 1);
	for ( int i = 0; i < count ; ++i ) {
		if ( d->m_vectorBean.size() > 0 && row < d->m_vectorBean.size() ) {
			QSharedPointer<BaseBean> bean = d->m_vectorBean.at(row);
			if ( !bean.isNull() ) {
				disconnect(bean.data(), SIGNAL(fieldModified(BaseBean *, QString, QVariant)),
						   this, SLOT(fieldBaseBeanModified(BaseBean *, QString, QVariant)));
				if ( bean->dbState() != BaseBean::INSERT && d->m_deleteFromDB ) {
					if ( bean->metadata()->tableName() != d->m_tableName ) {
						QVariant id = bean->pkValue();
						QSharedPointer<BaseBean> beanOrig = BaseDAO::selectByPk(id, d->m_tableName);
						if ( !beanOrig.isNull() ) {
							result = result & BaseDAO::instance()->remove(beanOrig);
						}
					} else {
						result = result & BaseDAO::instance()->remove(bean);
					}
				}
				d->m_vectorBean.remove(row);
				d->m_beansFecthed.remove(row);
				d->m_rowCount--;
			}
		}
	}
	endRemoveRows();
	return result;
}


/*!
  Devuelve el bean correspondiente al índice index. Antes de devolverlo comprueba
  que el bean esté actualizado, esto es lo recupera de base de datos.
*/
QSharedPointer<BaseBean> DBBaseBeanModel::bean (const QModelIndex &index)
{
	QSharedPointer<BaseBean> bean;
    bool beansHasBeenFetched = false;

	if ( !index.isValid() ) {
		return bean;
	}
	if ( d->m_beansFecthed.size() == 0 || index.row() >= d->m_beansFecthed.size() ) {
		return bean;
	}
	if ( !d->m_beansFecthed.at(index.row()) ) {
		d->fetchBeans(index.row());
        beansHasBeenFetched = true;
	}
	if ( !d->m_beansFecthed.at(index.row()) ) {
		return bean;
	}
	bean = d->m_vectorBean.at(index.row());
	if ( bean->dbState() == BaseBean::UPDATE ) {
		// Evitamos una recarga innecesaria. Hay que poner una marca de tiempo
		if ( !d->m_staticModel ) {
			if ( !beansHasBeenFetched || bean->loadTime().msecsTo(QDateTime::currentDateTime()) > configuracion.timeBetweenReloads() ) {
				BaseDAO::reloadBeanFromDB(bean);
			} else {
				qDebug() << "DBBaseBeanModel: bean: Bean reciente. No es necesario obtenerlo";
			}
		}
	}
	bean->uncheckModifiedFields();
	return bean;
}

/*!
  Indica si un bean determinado se ha obtenido o no de base datos
  */
bool DBBaseBeanModel::hasBeenFetched(const QModelIndex &index)
{
	int row = index.row();
	if ( d->m_beansFecthed.size() != 0 && row < d->m_beansFecthed.size() ) {
		return d->m_beansFecthed.at(row);
	}
	return false;
}

QSharedPointer<BaseBean> DBBaseBeanModel::beanToBeEdited (const QModelIndex &index)
{
	QSharedPointer<BaseBean> b = bean(index);
	QSharedPointer<BaseBean> beanOriginal;

	if ( !index.isValid() || b.isNull() ) {
		return b;
	}
	if ( b->metadata()->tableName() != d->m_tableName ) {
		beanOriginal = BaseDAO::instance()->selectByPk(b->pkValue(), d->m_tableName);
		if ( beanOriginal.isNull() ) {
			return BeansFactory::instance()->newQBaseBean(d->m_tableName);
		}
	} else {
		beanOriginal = b;
	}
	beanOriginal->uncheckModifiedFields();
	return beanOriginal;
}

/*!
  Devuelve los metadatos de visualización del modelo.
*/
BaseBeanMetadata *DBBaseBeanModel::metadata()
{
	return d->m_metadata;
}

/*!
  Devuelve los fields del masterbean de este modelo
*/
QList<DBFieldMetadata *> DBBaseBeanModel::fields()
{
	if ( d->m_metadata != NULL ) {
		return d->m_metadata->fields();
	}
	return QList<DBFieldMetadata *>();
}

/*!
  Busca en el modelo el indice QModelIndex del bean cuya primary key es value
*/
QModelIndex DBBaseBeanModel::indexByPk(const QVariant &value)
{
	QVariantMap values = value.toMap();
	for ( int i = 0 ; i < d->m_vectorBean.size() ; i++ ) {
		if ( !d->m_beansFecthed.at(i) ) {
			d->fetchBeans(i);
		}
		if ( d->m_beansFecthed.at(i) ) {
			QSharedPointer<BaseBean> bean = d->m_vectorBean.at(i);
			if ( bean->pkValue() == values ) {
				return createIndex(i, 0, 0);
			}
		}
	}
	return QModelIndex();
}

void DBBaseBeanModel::setDeleteFromDB(bool value)
{
	d->m_deleteFromDB = value;
}

void DBBaseBeanModel::refresh()
{
	emit QAbstractItemModel::layoutAboutToBeChanged();
	foreach ( QSharedPointer<BaseBean> bean, d->m_vectorBean ) {
		if ( !bean.isNull() ) {
			disconnect(bean.data(), SIGNAL(fieldModified(BaseBean *, QString, QVariant)),
				   this, SLOT(fieldBaseBeanModified(BaseBean *, QString, QVariant)));
		}
	}
	d->m_vectorBean.clear();
	d->m_beansFecthed.clear();
	d->m_rowCount = 0;
	d->countTotalBeans();
	emit QAbstractItemModel::layoutChanged();
}

void DBBaseBeanModel::reloadModel()
{
	refresh();
}

/**
  En modelos que presenten vistas, no se podran insertar filas...
  */
bool DBBaseBeanModel::insertRows (int row, int count, const QModelIndex & parent)
{
	if ( d->m_metadata == NULL ) {
		return BaseBeanModel::insertRows(row, count, parent);
	}
	QSharedPointer<BaseBean> bean = BeansFactory::instance()->newQBaseBean(d->m_metadata->tableName());
	bean->setDbState(BaseBean::INSERT);

	beginInsertRows(parent, row, row + count - 1);
	d->m_vectorBean.append(bean);
	d->m_beansFecthed.append(true);
	connect(bean.data(), SIGNAL(fieldModified(BaseBean *, QString, QVariant)),
			   this, SLOT(fieldBaseBeanModified(BaseBean *, QString, QVariant)));
	d->m_rowCount++;
	endInsertRows();
	return true;
}

/*!
  Hay ocasiones donde no interesa que el modelo se esté recargando: por ejemplo, al editar un registro
  o durante las acciones de borrado... Este slot permite parar la recarga
  */
void DBBaseBeanModel::stopReloading()
{
	if ( !d->m_staticModel ) {
		d->m_timer->disconnect(this, SLOT(reloadModel()));
	}
}

/*!
  Inicio de las operaciones de recarga
  */
void DBBaseBeanModel::startReloading()
{
	connect(d->m_timer, SIGNAL(timeout()), this, SLOT(reloadModel()));
	d->m_timer->start(configuracion.modelRefreshTimeout());
}

/*!
  Antes de asignar una cláusula where puede ser interesante realizar un conteo de cuántos registros
  se obtendrían con una determinada claúsula where. Aquí puede hacerse
  */
int DBBaseBeanModel::simulateRowCount(const QString where)
{
	QString query, queryCount;
	QVariant result;

	if ( d->m_metadata == NULL ) {
		return -1;
	}
	d->buildSqlSelect(d->m_metadata->tableName(), where, d->m_order, d->m_metadata->fields(), query);
	queryCount = QString("SELECT count(*) FROM (%1) AS FOO").arg(query);
	if ( !d->m_metadata->isCached() ) {
		if ( BaseDAO::execute(queryCount, result) && result.isValid() ) {
			return result.toInt();
		}
	} else {
		if ( BaseDAO::executeCached(query, result) ) {
			return result.toInt();
		}
	}
	return -1;
}
