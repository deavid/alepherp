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
#ifndef DBRECORDDLG_H
#define DBRECORDDLG_H

#include <alepherpglobal.h>
#include <QItemSelectionModel>
#include "forms/perpbasedialog.h"
#include "scripts/perpscript.h"
#include "dao/beans/basebean.h"
#include "models/filterbasebeanmodel.h"

class BaseBeanObserver;
class DBRecordDlgPrivate;

namespace Ui {
	class DBRecordDlg;
}

namespace DbRecordOpenType {
	enum DbRecordOpenType { INSERT, UPDATE };
}

/**
  Clase base que controlará la edición de registros. Para componer un formulario
  de edición de registros son necesarios tres entradas en la tabla printingerp_system:
  <ul>
  <li>nombre_de_la_tabla archivo de tipo table</li>
  <li>nombre_de_la_tabla.dbrecord.qs archivo con código JavaScript que lo controlará</li>
  <li>nombre_de_la_tabla.dbrecord.ui archivo con la interfaz de usuario.</li>
  </ul>
  A esta interfaz de usuario esta clase le agregará los botones para salir, guardar registros...
  El código Javascript deberá ser una clase con el nombre DBRecordDlg. Podrá tener
  algunos métodos que serán llamados por DBRecordDlg en determinados eventos como:
  afterSaveRecord	Se llama justo después de guardar el registro
  */
class Q_ALEPHERP_EXPORT DBRecordDlg : public PERPBaseDialog
{
	Q_OBJECT
	Q_PROPERTY(bool windowModified READ isWindowModified WRITE setWindowModified)
	/** Esta propiedad permite determinar la funcionalidad al pulsar el botón de cerrar en la esquina superior
	  derecha de la ventana:
	  -Si está a true, el formulario preguntará si se guardan los datos, que será el comportamiento por defecto.
	  -El desarrollador puede poner esta propiedad a false, y al pinchar el botón cerrar se cierra el formulario
	   sin preguntar si hay datos guardados */
	Q_PROPERTY(bool closeButtonAskForSave READ closeButtonAskForSave WRITE setCloseButtonAskForSave)

private:
	Ui::DBRecordDlg *ui;
	DBRecordDlgPrivate *d;
	Q_DECLARE_PRIVATE(DBRecordDlg)

	bool init();
	void setupMainWidget();
	void execQs();
	void lock();

protected:
	void showEvent (QShowEvent *event);
	void closeEvent (QCloseEvent * event);
	bool eventFilter (QObject *target, QEvent *event);
	void keyPressEvent (QKeyEvent * e);

public:
	DBRecordDlg(QSharedPointer<BaseBean> bean, bool *userSaveData,
				DbRecordOpenType::DbRecordOpenType openType,
				QWidget* parent = 0, Qt::WFlags fl = 0);
	DBRecordDlg(FilterBaseBeanModel *model, QItemSelectionModel *idx, bool *userSaveData,
				DbRecordOpenType::DbRecordOpenType openType,
				QWidget* parent = 0, Qt::WFlags fl = 0);
	~DBRecordDlg();

	void setWindowModified(bool value);
	bool isWindowModified();
	void installEventFilters();
	bool closeButtonAskForSave();
	void setCloseButtonAskForSave(bool value);

	Q_INVOKABLE QSharedPointer<BaseBean> bean();
	Q_INVOKABLE void hideDBButtons();
	Q_INVOKABLE void showDBButtons();
	Q_INVOKABLE bool validate();

private slots:
	void lockBreaked(const QString &notificacion);
	void navigate(const QString &direction);

public slots:
	bool save();
	void first() { navigate("first"); }
	void next() { navigate("next"); }
	void previous() { navigate("previous"); }
	void last() { navigate("last"); }
	void cancel();
	void showHistory();
	void saveAndNew();
};

#endif // DBRECORDDLG_H
