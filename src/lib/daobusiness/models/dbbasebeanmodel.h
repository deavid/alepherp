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
#ifndef DBTABLEMODEL_H
#define DBTABLEMODEL_H

#include "models/basebeanmodel.h"
#include <QObject>
#include <QSharedPointer>
#include <QString>
#include <QList>
#include "dao/beans/basebean.h"

class DBField;
class DBBaseBeanModelPrivate;
class BaseBeanMetadata;

/**
  Esta clase obtiene sus datos de una consulta de base de datos, bien sea directamente de una tabla
  o de una vista. Se pueden acceder a los resultados a través de BaseBean. Los resultados de esta
  consulta son siempre sólo-lectura. Internamente almacena una lista de BaseBeans
  @author David Pinelo <david.pinelo@alephsistemas.es>
  */
class DBBaseBeanModel : public BaseBeanModel
{
	Q_OBJECT
	/** Esta propiedad nos chivará si el modelo está basado en BaseBean, con lo cual
	  podrá obtener, a través de la propiedad tableName, el bean base que editan los modelos */
	Q_PROPERTY (bool baseBeanModel READ baseBeanModel)
	/** Hay modelos que por su naturaleza emintenemente estática no necesitan una recarga del bean
	  de base de datos cuando se leen. Por ejemplo: Un combobox que muestra nacionalidades, que se leen
	  de una tabla que no se edita nunca, o si se edita, no es necesario que se lea nada más que una vez
	  al principio. Son modelos "estáticos". Se utilizan en ese tipo de controles. Esta propiedad
	  marca si este modelo lee los datos de base de datos una vez en su creación, o cada vez que se le
	  solicite un bean. Si staticModel es false, por defecto, se recargarán de base de datos cada vez
	  que se llame al método bean, o cada intervalo de tiempo. */
	Q_PROPERTY (bool staticModel READ staticModel WRITE setStaticModel)

private:
	Q_DISABLE_COPY(DBBaseBeanModel)
	class DBBaseBeanModelPrivate *d;
	Q_DECLARE_PRIVATE(DBBaseBeanModel)

public:
	explicit DBBaseBeanModel(const QString &tableName, const QString &where = "",
							 const QString &order = "", bool staticModel = false, QObject *parent = 0);
	~DBBaseBeanModel();

	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
	QModelIndex parent(const QModelIndex &index) const;
	int rowCount(const QModelIndex& parent = QModelIndex()) const;

	int columnCount ( const QModelIndex & parent ) const;
	QVariant headerData ( int section, Qt::Orientation orientation, int role ) const;
	Qt::ItemFlags flags ( const QModelIndex & index ) const;
	QVariant data ( const QModelIndex & item, int role ) const;
	bool removeRows ( int row, int count, const QModelIndex & parent );
	bool setData ( const QModelIndex & index, const QVariant & value, int role );

	bool insertRows ( int row, int count, const QModelIndex & parent = QModelIndex() );

	// Funciones propias
	QSharedPointer<BaseBean> bean (const QModelIndex &index);
	QSharedPointer<BaseBean> beanToBeEdited (const QModelIndex &index);
	bool hasBeenFetched(const QModelIndex &index);

	BaseBeanMetadata * metadata();

	QList<DBFieldMetadata *> fields();
	QModelIndex indexByPk(const QVariant &value);

	Q_INVOKABLE void setWhere(const QString &where, bool refresh = true);
	QString whereClausule();
	Q_INVOKABLE void setOrder(const QString &order);
	QString orderClausule();
	QString actualSql();
	QString actualCountSql();

	bool baseBeanModel() { return true; }
	void refresh();
	void setDeleteFromDB(bool value);

	bool staticModel();
	void setStaticModel(bool value);
	int offset();
	int simulateRowCount(const QString where);

public slots:
	void stopReloading();
	void startReloading();

private slots:
	void fieldBaseBeanModified(BaseBean *bean, const QString &dbFieldName, const QVariant &value);
	void reloadModel();
};

#endif // DBTABLEMODEL_H
