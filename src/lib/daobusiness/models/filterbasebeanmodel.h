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
#ifndef FILTERBASEBEANMODEL_H
#define FILTERBASEBEANMODEL_H

#include <QSortFilterProxyModel>
#include <QSharedPointer>

class FilterBaseBeanModelPrivate;
class BaseBean;
class DBFieldMetadata;

/**
  Clase que dará soporte al filtrado de modelos basados en Beans.
  @author David Pinelo <david.pinelo@alephsistemas.es>
  */
class FilterBaseBeanModel : public QSortFilterProxyModel
{
    Q_OBJECT
	/** Esta propiedad nos chivará si el modelo está basado en BaseBean, con lo cual
	  podrá obtener, a través de la propiedad tableName, el bean base que editan los modelos */
	Q_PROPERTY (bool baseBeanModel READ baseBeanModel)
    /** El filtrado puede realizarse adicionalmente sobre campos memo. Esto podría
      ralentizar bastante la obtención de información (los campos memo se obtienen en consulta
      aparte y bajo demanda, cuando se necesitan, para mejorar el rendimiento. Esta propiedad deja
      a decisión del programador qué hacer. Por defecto, este filtro no se aplicará sobre
      campos memo */
    Q_PROPERTY (bool includeMemoFieldsOnFilter READ includeMemoFieldsOnFilter WRITE setIncludeMemoFieldsOnFilter)

private:
	FilterBaseBeanModelPrivate *d;

public:
	explicit FilterBaseBeanModel(QObject *parent = 0);
	~FilterBaseBeanModel();

	void setDbStates(int state);
	int dbStates();
	Q_INVOKABLE void removeFilterKeyColumn (const QString &dbFieldName);
	Q_INVOKABLE void setFilterKeyColumn (const QString &dbFieldName, const QVariant &value, const QString &op = "=");
	Q_INVOKABLE void setFilterKeyColumn (const QString &dbFieldName, const QRegExp &regExp);
	Q_INVOKABLE void setFilterKeyColumnBetween (const QString &dbFieldName, const QVariant &value1, const QVariant &value2);
	Q_INVOKABLE void setFilterPkColumn (const QVariant &pk);
	Q_INVOKABLE void setFilter (const QString &field);
	Q_INVOKABLE void setFilterByLevel (int level, const QString &filter);
	QHash<QString, QPair<QString, QVariant> > filter();

	bool baseBeanModel() { return true; }
	void resetFilter();

	void setSortColumns(const QHash<int, QPair<int, QString> > &sort);

    bool includeMemoFieldsOnFilter();
    void setIncludeMemoFieldsOnFilter(bool value);

	QList<DBFieldMetadata *> visibleFields() const;

	QSharedPointer<BaseBean> bean (const QModelIndex &index);
	QSharedPointer<BaseBean> bean (int row);
	QSharedPointer<BaseBean> beanToBeEdited (const QModelIndex &index);
	QSharedPointer<BaseBean> beanToBeEdited (int row);

	virtual QModelIndex mapFromSource ( const QModelIndex & sourceIndex ) const;
	virtual QModelIndex mapToSource ( const QModelIndex & proxyIndex ) const;
	virtual int	columnCount ( const QModelIndex & parent = QModelIndex() ) const;
	virtual void sort ( int column, Qt::SortOrder order = Qt::AscendingOrder );
	virtual QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

protected:
	virtual bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
	virtual bool filterAcceptsColumn (int source_column, const QModelIndex & source_parent) const;
	virtual bool lessThan (const QModelIndex & left, const QModelIndex & right) const;

signals:

public slots:

};

#endif // FILTERBASEBEANMODEL_H
