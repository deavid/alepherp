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
#ifndef DBFILTERTABLEVIEW_H
#define DBFILTERTABLEVIEW_H

#include <QWidget>
#include <QAbstractItemModel>
#include <QItemSelectionModel>
#include <QScriptValue>
#include <alepherpglobal.h>
#include "dao/beans/basebean.h"

namespace Ui {
	class DBFilterTableView;
}

class DBBaseBeanModel;
class FilterBaseBeanModel;
class DBField;
class DBFilterTableViewPrivate;
class DBTableView;
class DBFieldMetadata;

/**
  Presenta los registros que se obtienen de una tabla que pueda modelarse con un modelo DBBaseBeanModel,
  y no otro, junto con combos y demás elementos necesarios para realizar filtrados. Internamente,
  utiliza DBFilterTableView como elemento interno para presentar los datos. Está pensada como
  tabla de Sólo lectura.
  El número de registros que puede presentar es bastante elevado, de tal manera que este componente
  para criterios de búsqueda "laxos" creará filtros fuertes en el modelo.
  @see DBFilterTableView

  @author David Pinelo
  */
class Q_ALEPHERP_EXPORT DBFilterTableView : public QWidget
{
    Q_OBJECT
	/** Tabla principal, de la que se devolverán BEANS. Pero no tiene porqué ser la
		tabla que se visualiza. Si esta table name tiene viewOnGrid a otra visualización,
		esa es la que se obtendrá y visualizará, aunque se editen los contenidos de tableName. */
	Q_PROPERTY (QString tableName READ tableName WRITE setTableName)

private:
	Ui::DBFilterTableView *ui;
	DBFilterTableViewPrivate *d;
	Q_DECLARE_PRIVATE(DBFilterTableView)

	void addFieldsCombo();
	void buildFilterWhere(const QString &aditionalSql = "");
	void addOptionsCombo(DBFieldMetadata *fld);
	void prepareFilterControls();
	DBFieldMetadata * dbFieldSelectedOnCombo();
	void clearFilter();

protected:
	bool event(QEvent *e);
	void closeEvent (QCloseEvent * event);
	bool eventFilter(QObject *obj, QEvent *ev);

public:
	explicit DBFilterTableView(QWidget *parent = 0);
	virtual ~DBFilterTableView();

	void setTableName(const QString &name);
	QString tableName();
	FilterBaseBeanModel *model();
	QItemSelectionModel *selectionModel();

	Q_INVOKABLE DBTableView *dbTableView();

	Q_INVOKABLE QSharedPointer<BaseBean> selectedBean();
	Q_INVOKABLE void setSelectedBean(const QSharedPointer<BaseBean> &bean);

    static QScriptValue toScriptValue(QScriptEngine *engine, DBFilterTableView * const &in);
    static void fromScriptValue(const QScriptValue &object, DBFilterTableView * &out);

protected slots:
	void fastFilterByText();
	void fastFilterByNumbers();
	void fastFilterByCombo(int);
	void fastFilterByDate();
	void filter();
	void filterWithFastUserFilter();
	void changedComboField(int index);
	void sortForm();
	void prepareFilterControlsByOperator();

public slots:
	void init(bool createStrongFilter = true);
	void refresh();
	void resizeRowsToContents ();
	void destroyStrongFilter(const QString &dbFieldName = "");
	void createStrongFilter();
	void stopReloadingModel();
	void startReloadingModel();
};

Q_DECLARE_METATYPE(DBFilterTableView*)

#endif // DBFILTERTABLEVIEW_H
