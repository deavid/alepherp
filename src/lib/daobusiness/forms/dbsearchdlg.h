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
#ifndef DBSEARCHDLG_H
#define DBSEARCHDLG_H

#include <QDialog>
#include <QScriptEngine>
#include <QItemSelectionModel>
#include <alepherpglobal.h>
#include "forms/perpbasedialog.h"
#include "scripts/perpscript.h"

class BaseBean;
class DBBaseBeanModel;
class FilterBaseBeanModel;
class BaseBeanMetadata;
class DBSearchDlgPrivate;

namespace Ui {
	class DBSearchDlg;
}

/**
  La idea es tener un formulario de búsqueda genérico. Para ello, el interfaz se definirá a través
  de base de datos, donde se almacenará el archivo .ui, de tal manera que sea fácilmente editable.
  El pie del formulario serán siempre los mismos botones (Buscar, aplicar filtro, editar y cerrar).
  Este formulario tendrá un objeto de filtro adecuado, y se expondrán métodos de este objeto al
  script asociado para así introducir los datos que el usuario ha ido escribiendo.
  @author David Pinelo <david.pinelo@alephsistemas.es>
  */
class Q_ALEPHERP_EXPORT DBSearchDlg : public PERPBaseDialog
{
    Q_OBJECT
private:
	Ui::DBSearchDlg *ui;

	DBSearchDlgPrivate *d;
	Q_DECLARE_PRIVATE(DBSearchDlg)

	void setupWidget();
	bool setupExternalWidget();
	void setupQs();
    void connectObjectsToSearch();

public:
	DBSearchDlg(const QString &tableName, QWidget *parent = 0);
	~DBSearchDlg();

	QSharedPointer<BaseBean> selectedBean();
	bool canApplyFilter();
	bool userClickOk();
	void setCanApplyfilter(bool value);
	void setFilterData(const QString &value);

signals:

public slots:
	void init();
	void setColumnFilter (const QString &name, const QVariant &value, const QString &op);
	void setColumnBetweenFilter (const QString &name, const QVariant &fecha1, const QVariant &fecha2);
	void select(const QModelIndex &index = QModelIndex());
    void search();
};

#endif // DBSEARCHDLG_H
