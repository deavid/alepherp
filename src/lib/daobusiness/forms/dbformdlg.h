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

#ifndef QMAINDLGPRES_H
#define QMAINDLGPRES_H

#include <QDialog>
#include <QSqlRelationalTableModel>
#include <QSqlRelation>
#include <QModelIndex>
#include <QMessageBox>
#include <QHeaderView>
#include <QStatusBar>
#include <QSignalMapper>
#include <QScriptValue>
#include <alepherpglobal.h>
#include "globales.h"
#include "scripts/perpscript.h"
#include "dao/beans/basebean.h"

namespace Ui {
	class DBFormDlg;
}

class DBFormDlgPrivate;
class BaseBean;
class DBFilterTableView;

/*!
  Este es el formulario basico de la herramienta. Es donde se presentan el conjunto de registros
  por los que el usuario va navegando. Incluye internamente un DBFilterTableView que permite
  realizar un filtrado de registros. Desde aquí se permite añadir, editar y/o eliminar registros.
  Antes de ejecutar cualquier edición de registros:

  DBFormDlg.prototype.beforeDelete = function ()
  DBFormDlg.prototype.beforeInsert = function ()
  DBFormDlg.prototype.beforeUpdate = function ()

  Si estas funciones devuelven true o no existen, se ejecuta la acción. Si devuelven false, no se
  ejecutan.
  */
class Q_ALEPHERP_EXPORT DBFormDlg : public QWidget
{
	Q_OBJECT
	Q_FLAGS(DBFormButtons)

	/** Propiedad para establecer botones visibles o no al usuario. Por defecto, cuando se abre
	  el formulario, siempre están todos los botones visibles, pero con esta propiedad y a traves
	  de QtScript pueden articularse qué botones se verán y cuáles no */
	Q_PROPERTY (DBFormButtons visibleButtons READ visibleButtons WRITE setVisibleButtons)

private:
	Ui::DBFormDlg *ui;
	DBFormDlgPrivate *d;

	void execQs();
	void actions();
	bool construct(const QString &tableName);

protected:
	void closeEvent (QCloseEvent * event);
	void keyPressEvent (QKeyEvent * e);
	bool eventFilter (QObject *target, QEvent *event);
	bool event (QEvent *e);

	void setOpenSuccess(bool value);

	PERPScript *engine();

public:
	DBFormDlg(const QString &tableName, QWidget* parent = 0, Qt::WindowFlags f = 0);
	DBFormDlg(const QString &tableName, QSharedPointer<BaseBean> *selectedBean, QWidget* parent = 0, Qt::WindowFlags f = 0);
	~DBFormDlg();

	/** Identifica a los botones disponibles para el usuario. Utilizado en las funciones que
	  ocultan o hacen visible los botones al usuario. Son visibles desde el motor  de Script a través
	  de propiedades, esto es: thisForm.CREATE */
	enum DBFormButton { CREATE = 1, EDIT = 2, DELETE = 4, SEARCH = 8, COPY = 16, ADJUST_LINES = 32, EXIT = 64, OK = 128 };
	Q_DECLARE_FLAGS(DBFormButtons, DBFormButton)

	void init ();

	Q_INVOKABLE DBFilterTableView *dbFilterTableView();

	DBFormButtons visibleButtons();
	void setVisibleButtons(DBFormButtons buttons);

	QString tableName();
	void setTableName(const QString &value);
	bool openSuccess();

	bool checkPermissionsToOpen();

	static QScriptValue toScriptValue(QScriptEngine *engine, DBFormDlg * const &in);
	static void fromScriptValue(const QScriptValue &object, DBFormDlg * &out);

public slots:
	void refreshFilterTableView();
	QScriptValue callMethod(const QString &method);

protected slots:
	void edit (const QString &insert);
	void deleteRecord(void);
	void search(void);
	void copy();
	void ok();

signals:
	void sigSplashInfo(const QString &);
	void closingWindow(QWidget *objeto);
	void beforeDelete();
	void afterDelete(bool wasDeleted);
	void afterEdit(bool wasSaved);
};

Q_DECLARE_METATYPE(DBFormDlg*)
Q_DECLARE_METATYPE(DBFormDlg::DBFormButtons)
Q_DECLARE_OPERATORS_FOR_FLAGS(DBFormDlg::DBFormButtons)

#endif

