/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/

#ifndef PERPQUERYMODEL_H
#define PERPQUERYMODEL_H

#include <QAbstractItemModel>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QSqlError>
#include <QList>
#include <QHash>
#include <QVector>
#include <QThread>
#include <QMutex>
#include <QSharedPointer>
#include "models/basebeanmodel.h"

class PERPDataset;
class PERPQueryModelPrivate;
class PERPThreadReadData;

class PERPQueryModel : public BaseBeanModel
{

Q_OBJECT

private:
	PERPQueryModelPrivate *d;
	Q_DECLARE_PRIVATE(PERPQueryModel)
	PERPThreadReadData *m_thread;
	/** Indica si se utilizará un thread en background para leer datos */
	bool m_loadDataBackground;

	QModelIndex indexInQuery(const QModelIndex &item) const;

public:
	PERPQueryModel(bool loadDataBackground, QObject *parent = NULL);
	~PERPQueryModel();

	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
	QModelIndex parent(const QModelIndex &index) const;
	int rowCount(const QModelIndex& parent = QModelIndex()) const;
	int columnCount ( const QModelIndex & parent = QModelIndex() ) const;
	QVariant data ( const QModelIndex & item, int role = Qt::DisplayRole ) const;
	bool insertColumns(int column, int count, const QModelIndex &parent);
	bool removeColumns(int column, int count, const QModelIndex &parent);
	QSqlRecord record(int row) const;
	QSqlRecord record() const;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value,
				   int role = Qt::EditRole);
	QModelIndexList match (const QHash<int, QVariant> &values, int role,
						   int hits = 1, Qt::MatchFlags flags = Qt::MatchFlags( Qt::MatchStartsWith | Qt::MatchWrap ) ) const;

	void setQuery ( const QString & query, const QString &queryCount, const QSqlDatabase & db = QSqlDatabase() );

	void clear();
	void refresh();

	QString lastError();
	QString getSql();
	QString getSqlCount();

	virtual QSharedPointer<BaseBean> bean (QModelIndex &index, bool onlyVisibleData);
};

class PERPQueryModelPrivate : public QObject
{
	Q_OBJECT
//	Q_DECLARE_PUBLIC(PERPQueryModel)
public:
	QSharedPointer<QSqlQuery> m_query;
	QString m_sqlCount;
	QString m_sql;
	QString m_lastError;
	int m_limit;
	int m_offset;
	int m_rowCount;
	QSqlDatabase m_db;
	QHash <int, PERPDataset *> m_datas;
	QVector<QHash<int, QVariant> > m_headers;
	QMutex m_mutex;
	/** Indica si se utilizará un thread en background para leer datos */
	bool m_useThread;

	PERPQueryModelPrivate(QObject *parent = 0);
	~PERPQueryModelPrivate();

	bool openQuery();
	void countResults();

public slots:
	void newData(int row, int column, const QVariant &value);
};


class PERPDataset
{
private:
	QHash<int, QVariant> m_datas;

public:
	PERPDataset();
	~PERPDataset();

	// Constructor de copia y operador de asignación
	PERPDataset (const PERPDataset &_a);
	const PERPDataset & operator = (const PERPDataset &_a);

	void setColumn(int column, const QVariant &data);
	QVariant getColumn(int column);
};


class PERPThreadReadData : public QThread
{
	Q_OBJECT

private:
	PERPQueryModelPrivate *m_data;

public:
	PERPThreadReadData(PERPQueryModelPrivate *data, QObject * parent = 0);
	~PERPThreadReadData();

	void run();

signals:
	void dataAvailable(int row, int column, const QVariant &value);
};

#endif // PERPQUERYMODEL_H
