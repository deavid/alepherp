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
#include <QSqlRecord>
#include <QSqlField>
#include <QSqlError>
#include "perpquerymodel.h"
#include "dao/database.h"

PERPQueryModelPrivate::PERPQueryModelPrivate(QObject *parent) : QObject(parent)
{
	m_limit = 50;
	m_offset = 0;
	m_rowCount = -1;
}

PERPQueryModelPrivate::~PERPQueryModelPrivate()
{
	qDeleteAll(m_datas);
}

/*!
  Introduce nuevos datos en la estructura. Está pensado para conectarse
  con la señal dataAvailable del thread
  */
void PERPQueryModelPrivate::newData(int row, int column, const QVariant &value)
{
	PERPDataset *recDataset;
	// Load on memory the datasets
	if ( !m_datas.contains(row) ) {
		recDataset = new PERPDataset;
		m_mutex.lock();
		m_datas[ row ] = recDataset;
		m_mutex.unlock();
	} else {
		recDataset = m_datas[row];
	}
	m_mutex.lock();
	recDataset->setColumn(column, value);
	m_mutex.unlock();
}

PERPQueryModel::PERPQueryModel(bool loadDataBackground, QObject *parent) : BaseBeanModel( parent )
{
	m_loadDataBackground = loadDataBackground;
	d = new PERPQueryModelPrivate(this);
	d->m_useThread = m_loadDataBackground;
	if ( m_loadDataBackground ) {
		m_thread = new PERPThreadReadData(d, this);
		connect(m_thread, SIGNAL(dataAvailable(int, int, const QVariant &)), d, SLOT(newData(int, int, const QVariant &)));
	} else {
		m_thread = NULL;
	}
}

PERPQueryModel::~PERPQueryModel()
{
	if ( m_thread != NULL ) {
		delete m_thread;
	}
}

PERPDataset::PERPDataset()
{
}

PERPDataset::~PERPDataset()
{
	m_datas.clear();
}

void PERPDataset::setColumn(int column, const QVariant &data)
{
	m_datas[column] = data;
}

PERPDataset::PERPDataset (const PERPDataset &_a)
{
	m_datas = _a.m_datas;
}

const PERPDataset & PERPDataset::operator = (const PERPDataset &_a)
{
	if ( &_a != NULL && &_a != this ) {
		m_datas = _a.m_datas;
	}
	return *this;
}

QVariant PERPDataset::getColumn(int column)
{
	if ( column < 0 || column > m_datas.size() ) {
		return QVariant();
	}
	return m_datas[column];
}

QModelIndex PERPQueryModel::index(int row, int column, const QModelIndex &parent) const
{
	Q_UNUSED(parent)
	return createIndex(row, column, 0);
}

QModelIndex PERPQueryModel::parent(const QModelIndex &index) const
{
	Q_UNUSED(index)
	return QModelIndex();
}

int PERPQueryModel::rowCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent)
	return d->m_rowCount;
}

int	PERPQueryModel::columnCount ( const QModelIndex & parent ) const
{
	Q_UNUSED(parent)
	if ( d->m_query.isNull() ) {
		return 0;
	}
	QSqlRecord rec = d->m_query->record();
	return rec.count();
}

QVariant PERPQueryModel::data(const QModelIndex& item, int role) const
{
    QVariant v;
	int row;

	if ( !item.isValid() || d->m_query.isNull() )
        return QVariant();

	// Qt::UserRole será utilizado más arriba para determinar el campo de base de datos sin aplicar locale
	if ( role & ~(Qt::DisplayRole | Qt::EditRole | Qt::UserRole) )
		return v;

	QSqlRecord rec = d->m_query->record();
    if ( !rec.isGenerated(item.column()) )
        return v;
    
	if ( item.row() > d->m_rowCount ) {
		return v;
	}

	// The data is on memory?
	if ( !d->m_datas.contains( item.row() ) ) {
		// We made this to avoid decimal places
		d->m_offset = item.row() / d->m_limit;
		d->m_offset = d->m_offset * d->m_limit;
		d->openQuery();
		row = d->m_offset;
		while ( d->m_query->next() ) {
			// Load on memory the datasets
			for ( int i = 0 ; i < rec.count() ; i ++ ) {
				d->newData(row, i, d->m_query->value(i));
			}
			row++;
		}
	}
	d->m_mutex.lock();
	v = d->m_datas[ item.row() ]->getColumn(item.column());
	d->m_mutex.unlock();
	return v;
}

/*!
  \fn PERPQueryModel::match (QList<int> columns, int role, const QList<QVariant> & value,
					   int hits = 1, Qt::MatchFlags flags = Qt::MatchFlags( Qt::MatchStartsWith | Qt::MatchWrap ) ) const;
	Realiza una búsqueda en las columnas indicadas, para los valores indicados. Devuelve hits resultados o todos si hits=-1
	*/
QModelIndexList PERPQueryModel::match (const QHash<int, QVariant> &values, int role,
					   int hits, Qt::MatchFlags flags ) const
{
	Q_UNUSED(flags)
	QModelIndexList results;
	bool checked;

	for ( int row = 0 ; row < this->rowCount() ; row++ ) {
		if ( results.size() == hits ) {
			return results;
		}
		QModelIndex idx;
		QHashIterator<int, QVariant> it(values);
		checked = true;
		while (it.hasNext()) {
			it.next();
			idx = index(row, it.key());
			QVariant dato = data(idx, role);
			checked = checked & ( dato == it.value() );
		}
		if ( checked ) {
			results.append(idx);
		}
	}
	return results;
}


QModelIndex PERPQueryModel::indexInQuery(const QModelIndex &item) const
{
	if ( d->m_query.isNull() ) {
		return QModelIndex();
	}
	QSqlRecord rec = d->m_query->record();
    if ( item.column() < 0 || item.column() >= rec.count() || !rec.isGenerated(item.column()) )
        return QModelIndex();
    
	// Con columnas ocultas
/*	return createIndex(item.row(), item.column() - d->colOffsets[item.column()],
                       item.internalPointer());*/
	return createIndex(item.row(), item.column(), item.internalPointer());
}

bool PERPQueryModel::insertColumns(int column, int count, const QModelIndex &parent)
{
	if ( d->m_query.isNull() ) {
		return false;
	}
	QSqlRecord rec = d->m_query->record();
    if ( count <= 0 || parent.isValid() || column < 0 || column > rec.count())
        return false;

    beginInsertColumns(parent, column, column + count - 1);
    for (int c = 0; c < count; ++c) {
        QSqlField field;
        field.setReadOnly(true);
        field.setGenerated(false);
        rec.insert(column, field);
    }
    endInsertColumns();
    return true;
}

bool PERPQueryModel::removeColumns(int column, int count, const QModelIndex &parent)
{
	if ( d->m_query.isNull() ) {
		return false;
	}
	QSqlRecord rec = d->m_query->record();
    if ( count <= 0 || parent.isValid() || column < 0 || column >= rec.count() )
        return false;

    beginRemoveColumns(parent, column, column + count - 1);

    int i;
    for ( i = 0; i < count; i++ ) {
        rec.remove(column);
	}

    endRemoveColumns();
    return true;
}

QSqlRecord PERPQueryModel::record(int row) const
{
	QSqlRecord rec;
	if ( d->m_query.isNull() || row < 0 ) {
		return rec;
	}
	
    rec = d->m_query->record();
    for ( int i = 0 ; i < rec.count() ; i++ ) {
		rec.setValue(i, data(createIndex(row, i), Qt::EditRole));
	}
    return rec;
}

QSqlRecord PERPQueryModel::record() const
{
	if ( d->m_query.isNull() ) {
		return QSqlRecord();
	} 
    return d->m_query->record();
}

void PERPQueryModel::setQuery ( const QString & query, const QString &queryCount, const QSqlDatabase & db )
{
	int row = 0;
	QSqlRecord rec;
	
	if ( m_loadDataBackground && m_thread->isRunning() ) {
		m_thread->terminate();
		m_thread->wait();
	}
	
	clear();
	
	d->m_sql = query;
	d->m_sqlCount = queryCount;
	d->m_db = db;
	d->openQuery();
	if ( !d->m_query.isNull() ) {
		rec = d->m_query->record();
		// Load the first m_limit rows if m_useThread is enabled
		while ( d->m_query->next() ) {
			// Load on memory the datasets
			for ( int i = 0 ; i < rec.count() ; i ++ ) {
				d->newData(row, i, d->m_query->value(i));
			}
			row++;
		}
	}
	d->countResults();

	beginInsertRows(QModelIndex(), 0, d->m_rowCount - 1);
	endInsertRows();
	
	// And now, we call the thread that, in background, will populate the data
	if ( m_loadDataBackground ) {
		m_thread->start();
	}
}

bool PERPQueryModelPrivate::openQuery()
{
	QString sql;
	if ( m_useThread ) {
		sql = QString("%1 LIMIT %2 OFFSET %3").arg(m_sql).arg(m_limit).arg(m_offset);
	} else {
		sql = m_sql;
	}
	bool result = true;

	qDebug() << "PERPQueryModelPrivate::openQuery:: " << sql;
	if ( m_query.isNull() ) {
		m_query = QSharedPointer<QSqlQuery>(new QSqlQuery( m_db ));
	}
	m_query->prepare( sql );
	if ( !m_query->exec() ) {
		m_lastError = m_query->lastError().text();
		qDebug() << "PERPQueryModelPrivate::openQuery::ERROR: " << m_lastError;
		result = false;
	}
	return result;
}

void PERPQueryModelPrivate::countResults()
{
	if ( m_rowCount == -1 ) {
		QScopedPointer<QSqlQuery> qry (new QSqlQuery(m_db));
		qry->prepare( m_sqlCount );
		if ( qry->exec() && qry->first() ) {
			m_rowCount = qry->value(0).toInt();
		} else {
			m_lastError = qry->lastError().text();
			m_rowCount = 0;
		}
	}
}

QString PERPQueryModel::lastError()
{
	return d->m_lastError;
}

void PERPQueryModel::clear()
{
	beginRemoveRows(QModelIndex(), 0, d->m_rowCount);
	d->m_offset = 0;
	d->m_rowCount = -1;
	d->m_sql = "";
	d->m_sqlCount = "";
	qDeleteAll(d->m_datas);
	d->m_datas.clear();
	if ( !d->m_query.isNull() && d->m_query->isActive() ) {
		d->m_query->clear();
	}
	endRemoveRows();
}

void PERPQueryModel::refresh()
{
	QString query, queryCount;
	query = PERPQueryModel::getSql();
	queryCount = PERPQueryModel::getSqlCount();
	PERPQueryModel::clear();	
	setQuery(query, queryCount, d->m_db);
}

QVariant PERPQueryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Horizontal && !d->m_query.isNull()) {
		QSqlRecord rec = d->m_query->record();
        QVariant val = d->m_headers.value(section).value(role);
        if (role == Qt::DisplayRole && !val.isValid())
            val = d->m_headers.value(section).value(Qt::EditRole);
        if (val.isValid())
            return val;
        if (role == Qt::DisplayRole && rec.count() > section)
            return rec.fieldName(section);
    }
    return QAbstractItemModel::headerData(section, orientation, role);
}

bool PERPQueryModel::setHeaderData(int section, Qt::Orientation orientation,
                                   const QVariant &value, int role)
{
    if (orientation != Qt::Horizontal || section < 0 || columnCount() <= section)
        return false;

    if (d->m_headers.size() <= section)
        d->m_headers.resize(qMax(section + 1, 16));
    d->m_headers[section][role] = value;
    emit headerDataChanged(orientation, section, section);
    return true;
}

QString PERPQueryModel::getSql()
{
	return d->m_sql;
}

QString PERPQueryModel::getSqlCount()
{
	return d->m_sqlCount;
}

PERPThreadReadData::PERPThreadReadData(PERPQueryModelPrivate *data, QObject * parent) : QThread (parent)
{
	m_data = data;
}

PERPThreadReadData::~PERPThreadReadData()
{
}
		
void PERPThreadReadData::run()
{
	QString sql;
	QSqlRecord rec;
	int row;
	QString connectionName = "ThreadConnection";
	bool bbdd = false;
	QSqlDatabase db;

	if ( m_data != NULL ) {
		if ( ! QSqlDatabase::contains(connectionName) ) {
			if ( Database::createConnection(connectionName) ) {
				db = QSqlDatabase::database(connectionName);
				bbdd = true;
			} else {
				qWarning() << "PERPThreadReadData::run: Base de datos no ha podido abrirse.";
				return;
			}
		} else {
			db = QSqlDatabase::database(connectionName);
			bbdd = true;
		}
		QScopedPointer<QSqlQuery> query (new QSqlQuery(db));
		if ( bbdd ) {
			qDebug() << "Inicio del thread para leer los presupuestos";
			sql = QString("%1 OFFSET %2").arg(m_data->m_sql).arg(m_data->m_limit);
			query->prepare( sql );
			if ( query->exec() ) {
				rec = query->record();
				row = m_data->m_limit;
				while ( query->next() ) {
					for ( int i = 0 ; i < rec.count() ; i++ ) {
						emit dataAvailable(row, i, query->value(i));
					}
					row++;
				}
			}
			qDebug() << "Fin del thread para leer los presupuestos";
		}
	}
}

QSharedPointer<BaseBean> PERPQueryModel::bean (QModelIndex &index, bool onlyVisibleData)
{
	Q_UNUSED(index)
	Q_UNUSED(onlyVisibleData)
	return QSharedPointer<BaseBean>();
}
