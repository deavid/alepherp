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

#include "ui_dbformdlg.h"
#include "dbformdlg.h"
#include "configuracion.h"
#include "models/dbbasebeanmodel.h"
#include "models/perphtmldelegate.h"
#include "models/filterbasebeanmodel.h"
#include "forms/dbsearchdlg.h"
#include "forms/dbrecorddlg.h"
#include "dao/beans/basebean.h"
#include "dao/beans/basebeanmetadata.h"
#include "dao/beans/beansfactory.h"
#include "dao/beans/dbfield.h"
#include "dao/basedao.h"
#include "widgets/fademessage.h"
#include "widgets/dbtableview.h"
#ifndef QT_NO_SCRIPTTOOLS
 #include <QScriptEngineDebugger>
 #include <QMainWindow>
#endif
#include <QStatusTipEvent>
#include <QScriptValue>
#include <QPointer>

class DBFormDlgPrivate
{
public:
	/** Tabla o vista de la que se presentarán los registros */
	QSignalMapper *m_signalMapper;
	QMainWindow *m_mainWindow;
	QSharedPointer<BaseBean> *m_selectedBean;
	DBFormDlg::DBFormButtons m_buttons;
	/** Nombre del bean que edita este formulario, si edita alguno */
	QString m_tableName;
	/** Indicará si el formulario se ha abierto correctamente */
	bool m_openSuccess;
	/** Motor de script para las funciones */
	PERPScript m_engine;
	/** Chivato que nos indicará si se ha abierto algún formulario... */
	bool m_childFormOpen;

	DBFormDlgPrivate() {
		m_signalMapper = NULL;
		m_mainWindow = NULL;
		m_selectedBean = NULL;
		m_openSuccess = false;
		m_childFormOpen = false;
	}
};

DBFormDlg::DBFormDlg(const QString &tableName, QSharedPointer<BaseBean> *selectedBean, QWidget* parent, Qt::WindowFlags f)
	: QWidget( parent, f ), ui(new Ui::DBFormDlg), d(new DBFormDlgPrivate)
{
	d->m_selectedBean = selectedBean;
	d->m_buttons = CREATE | EDIT | DELETE | SEARCH | COPY | ADJUST_LINES | EXIT | OK;
	setTableName(tableName);
	if ( construct(tableName) ) {
		ui->dbFilterTableView->setTableName(tableName);
		execQs();
		ui->pbOk->setVisible(true);
		setOpenSuccess(true);
	} else {
		setOpenSuccess(false);
		close();
	}
}

DBFormDlg::DBFormDlg(const QString &tableName, QWidget* parent, Qt::WindowFlags f)
	: QWidget( parent, f ), ui(new Ui::DBFormDlg), d(new DBFormDlgPrivate)
{
	setTableName(tableName);
	if ( construct(tableName) ) {
		ui->dbFilterTableView->setTableName(tableName);
		execQs();
		ui->pbOk->setVisible(false);
		setOpenSuccess(true);
	} else {
		setOpenSuccess(false);
		close();
	}
}

bool DBFormDlg::construct(const QString &tableName)
{
	setObjectName(QString("%1%2").arg(objectName()).arg(tableName));
	if ( !checkPermissionsToOpen() ) {
		return false;
	}

	ui->setupUi(this);

	BaseBeanMetadata *metadata = BeansFactory::metadataBean(tableName);
	if ( metadata != NULL ) {
		setWindowTitle(metadata->alias());
	} else {
		qDebug() << "No existe la tabla: " << tableName;
		QMessageBox::warning(this, QString::fromUtf8(APP_NAME), trUtf8("No existe la tabla %1").arg(tableName), QMessageBox::Ok);
		close();
		return false;
	}

	actions();

	QObject *tmp = this->parent();
	while ( tmp != 0 ) {
		d->m_mainWindow = qobject_cast<QMainWindow *>(tmp);
		if ( d->m_mainWindow != NULL ) {
			tmp = 0;
		} else {
			tmp = tmp->parent();
		}
	};

	d->m_signalMapper = new QSignalMapper(this);
	d->m_signalMapper->setMapping(ui->actionEdit, QString("false"));
	d->m_signalMapper->setMapping(ui->actionCreate, QString("true"));
	connect (ui->actionEdit, SIGNAL(triggered()), d->m_signalMapper, SLOT(map()));
	connect (ui->actionCreate, SIGNAL(triggered()), d->m_signalMapper, SLOT(map()));
	connect (d->m_signalMapper, SIGNAL(mapped(const QString &)), this, SLOT(edit(const QString &)));
	connect (ui->actionExit, SIGNAL(triggered()), this, SLOT(close()));
	connect (ui->actionAdjustLines, SIGNAL(triggered()), ui->dbFilterTableView, SLOT(resizeRowsToContents ()));
	connect (ui->actionCopy, SIGNAL(triggered()), this, SLOT(copy()));
	connect (ui->actionDelete, SIGNAL(triggered()), this, SLOT(deleteRecord()));
	connect (ui->actionSearch, SIGNAL(triggered()), this, SLOT(search()));
	connect (ui->actionOk, SIGNAL(triggered()), this, SLOT(ok()));
	connect (ui->dbFilterTableView->dbTableView(), SIGNAL(enterPressedOnValidIndex(QModelIndex)), ui->actionEdit, SLOT(trigger()));
	connect (ui->dbFilterTableView->dbTableView(), SIGNAL(doubleClickOnValidIndex(const QModelIndex&)), ui->actionEdit, SLOT(trigger()));
	return true;
}

DBFormDlg::~DBFormDlg()
{
	delete ui;
	delete d;
}

PERPScript *DBFormDlg::engine()
{
	return &(d->m_engine);
}

QString DBFormDlg::tableName()
{
	return d->m_tableName;
}

void DBFormDlg::setTableName(const QString &value)
{
	d->m_tableName = value;
}

bool DBFormDlg::openSuccess()
{
	return d->m_openSuccess;
}

void DBFormDlg::setOpenSuccess(bool value)
{
	d->m_openSuccess = value;
}

void DBFormDlg::actions()
{
	connect(ui->pbAdjust, SIGNAL(clicked()), ui->actionAdjustLines, SLOT(trigger()));
	connect(ui->pbClose, SIGNAL(clicked()), ui->actionExit, SLOT(trigger()));
	connect(ui->pbDelete, SIGNAL(clicked()), ui->actionDelete, SLOT(trigger()));
	connect(ui->pbOk, SIGNAL(clicked()), ui->actionOk, SLOT(trigger()));
	connect(ui->pbEdit, SIGNAL(clicked()), ui->actionEdit, SLOT(trigger()));
	connect(ui->pbNew, SIGNAL(clicked()), ui->actionCreate, SLOT(trigger()));
	connect(ui->pbSearch, SIGNAL(clicked()), ui->actionSearch, SLOT(trigger()));
	connect(ui->pbCopy, SIGNAL(clicked()), ui->actionCopy, SLOT(trigger()));
}

bool DBFormDlg::event(QEvent *e) {
	if( e->type() == QEvent::StatusTip ){
		QStatusTipEvent *ev = (QStatusTipEvent*) e;
		if ( d->m_mainWindow != NULL && d->m_mainWindow->statusBar() != NULL ) {
			d->m_mainWindow->statusBar()->showMessage(ev->tip());
		}
		return true;
	}
	return QWidget::event(e);
}

void DBFormDlg::closeEvent(QCloseEvent * event)
{
	// Guardamos las dimensiones del usuario
	configuracion.savePosForm(this);
	configuracion.saveDimensionForm(this);
	emit closingWindow(this);
	QWidget::closeEvent(event);
}

void DBFormDlg::keyPressEvent (QKeyEvent * e)
{
	if ( e->key() == Qt::Key_Escape ) {
		close();
		e->accept();
	}
	QWidget::keyPressEvent(e);
}

/*!
  Vamos a obtener y guardar cuándo el usuario ha modificado un control
  */
bool DBFormDlg::eventFilter (QObject *target, QEvent *event)
{
	if ( !target->inherits("QTextEdit") && !target->inherits("DBHtmlEditor") &&
			!target->inherits("DBCodeEdit") && !target->inherits("QTableView")
			&& event->type() == QEvent::KeyPress ) {
		QKeyEvent *ev = static_cast<QKeyEvent *>(event);
		if ( ev->key() == Qt::Key_Enter || ev->key() == Qt::Key_Return ) {
			event->accept();
			focusNextChild();
			return true;
		}
	}
	return QWidget::eventFilter(target, event);
}

/*!
 Esto realiza ya la consulta en BBDD
*/
void DBFormDlg::init ()
{
	ui->dbFilterTableView->init();
}

/*!
  Este formulario puede contener cierto código script a ejecutar en su inicio. Esta función lo lanza
  inmediatamente. El código script está en printingerp_system, con el nombre de la tabla principal
  acabado en dbform.qs
  */
void DBFormDlg::execQs()
{
	QString qsName = QString ("%1.dbform.qs").arg(tableName());

	/** Ejecutamos el script asociado. La filosofía fundamental de ese script es proporcionar
	  algo de código básico que justifique este formulario de búsqueda */
	if ( !BeansFactory::tableWidgetsScripts.contains(qsName) ) {
		return;
	}

    engine()->setScript(BeansFactory::tableWidgetsScripts.value(qsName));
    engine()->setDebug(BeansFactory::tableWidgetsScriptsDebug.value(qsName));
    engine()->setOnInitDebug(BeansFactory::tableWidgetsScriptsDebugOnInit.value(qsName));
    engine()->setScriptObject("DBFormDlg");
    engine()->setUi(this);
    engine()->addAvailableObject("thisForm", this);
    if ( !engine()->createQsObject() ) {
		QMessageBox::warning(this, trUtf8(APP_NAME), trUtf8("Ha ocurrido un error al cargar el script asociado a este "
															"formulario. Es posible que algunas funciones no est\303\251n disponibles."),
							 QMessageBox::Ok);
	}
}

void DBFormDlg::edit (const QString &insert)
{
	DbRecordOpenType::DbRecordOpenType openType;
	bool userSaveData;
	QString functionName;

	if ( insert == "false" ) {
		openType = DbRecordOpenType::UPDATE;
		functionName = "beforeEdit";
	} else {
		openType = DbRecordOpenType::INSERT;
		functionName = "beforeInsert";
	}
	if ( engine()->existQsFunction(functionName) ) {
		QScriptValue result;
		if ( engine()->callQsObjectFunction(result, functionName) ) {
			if ( !result.toBool() ){
				return;
			}
		}
	}
	d->m_childFormOpen = true;
	ui->dbFilterTableView->stopReloadingModel();
	QPointer<DBRecordDlg> dlg = new DBRecordDlg(ui->dbFilterTableView->model(), ui->dbFilterTableView->selectionModel(),
						  &userSaveData, openType, this);

	if ( dlg->openSuccess() ) {
		dlg->setAttribute(Qt::WA_DeleteOnClose);
		dlg->setModal(true);
		dlg->exec();
		emit afterEdit(userSaveData);
	} else {
		delete dlg;
	}
	ui->dbFilterTableView->startReloadingModel();
	// Forzamos un refresco del modelo
	ui->dbFilterTableView->refresh();
	d->m_childFormOpen = false;
}

void DBFormDlg::deleteRecord(void )
{
	QItemSelectionModel *sel = ui->dbFilterTableView->selectionModel();
	QModelIndexList indexes = sel->selectedIndexes();
	QString message;
	QList<int> rows;

	if ( !indexes.isEmpty() ) {
		message = trUtf8("¿Está seguro de que desea borrar el(los) registro(s) actualmente seleccionado(s)?", "", indexes.size());
		d->m_childFormOpen = true;
		int ret = QMessageBox::information(this, QString::fromUtf8(APP_NAME), message, QMessageBox::Yes | QMessageBox::No);
		d->m_childFormOpen = false;
		if ( ret == QMessageBox::Yes ) {
			bool wasDelete = true;
			ui->dbFilterTableView->stopReloadingModel();
			emit beforeDelete();
			foreach ( QModelIndex idx, indexes ) {
				if ( !rows.contains(idx.row()) ) rows << idx.row();
			}
			if ( engine()->existQsFunction("beforeDelete") ) {
				QScriptValue result;
				if ( engine()->callQsObjectFunction(result, "beforeDelete") ) {
					if ( !result.toBool() ){
						return;
					}
				}
			}
			foreach ( int row, rows ) {
				if ( ui->dbFilterTableView->model()->removeRow(row) == false ) {
					wasDelete = false;
					QMessageBox::warning(this, QString::fromUtf8(APP_NAME),
										 QString::fromUtf8("Ha ocurrido un error. No se ha podido borrar el registro. "
														   "<br/><i>Error</i>: %1").arg(BaseDAO::instance()->getLastDbMessage()),
										 QMessageBox::Ok);
				}
			}
			emit afterDelete(wasDelete);
			ui->dbFilterTableView->startReloadingModel();
			ui->dbFilterTableView->refresh();
		}
	} else {
		QMessageBox::warning(this, QString::fromUtf8(APP_NAME),
							 trUtf8("Debe seleccionar algún registro para borrar"), QMessageBox::Ok);
	}
}

/*!
	Esta función realiza la siguiente acción: Si no se estaba filtrando, se abre el formulario
	de búsqueda para que el usuario seleccione los presupuestos que desea ver.
	Si se estaba filtrando, esta función elimina y deshabilita el filtro
*/
void DBFormDlg::search(void)
{
	QPointer<DBSearchDlg> dlg = new DBSearchDlg(tableName(), this);
	d->m_childFormOpen = true;
	if ( dlg->openSuccess() ) {
		dlg->setAttribute(Qt::WA_DeleteOnClose);
		dlg->setModal(true);
		dlg->init();
		dlg->exec();
	} else {
		delete dlg;
	}
	d->m_childFormOpen = false;
}

void DBFormDlg::copy()
{
	QItemSelectionModel *sel = ui->dbFilterTableView->selectionModel();
	QModelIndexList indexes = sel->selectedIndexes();
	FilterBaseBeanModel *mdl = ui->dbFilterTableView->model();
	QString message;

	if ( !indexes.isEmpty() && indexes.at(0).isValid() ) {
		QModelIndex idx = indexes.at(0);
		message = trUtf8("¿Desea realizar una copia de el registro actualmente seleccionado?", "", indexes.size());
		int ret = QMessageBox::information(this, QString::fromUtf8(APP_NAME), message, QMessageBox::Yes | QMessageBox::No);
		if ( ret == QMessageBox::Yes ) {
			ui->dbFilterTableView->stopReloadingModel();
			int row = mdl->rowCount();
			QSharedPointer<BaseBean> orig = mdl->beanToBeEdited(idx);
			if ( !orig.isNull() ) {
				mdl->insertRow(row);
				QSharedPointer<BaseBean> dest = mdl->beanToBeEdited(row);
				if ( !dest.isNull() ) {
					BaseDAO::copyBaseBean(orig, dest);
					if ( !dest->save() ) {
						QMessageBox::warning(this, trUtf8(APP_NAME), trUtf8("Ha ocurrido un error generando la copia."), QMessageBox::Ok);
						mdl->removeRow(row);
						ui->dbFilterTableView->startReloadingModel();
						return;
					}
				}
			}
			QModelIndex newIdx = mdl->index(row, 0);
			sel->setCurrentIndex(newIdx, QItemSelectionModel::Rows);
			sel->select(newIdx, QItemSelectionModel::Rows);
			int ret = QMessageBox::information(this, trUtf8(APP_NAME),
											   trUtf8("¿Desea editar el nuevo registro copiado?"), QMessageBox::Yes | QMessageBox::No);
			if ( ret == QMessageBox::Yes ) {
				edit("false");
			}
			ui->dbFilterTableView->startReloadingModel();
		}
	} else {
		QMessageBox::warning(this, QString::fromUtf8(APP_NAME),
							 trUtf8("Debe seleccionar algún registro para copiar"), QMessageBox::Ok);
	}
}

void DBFormDlg::ok()
{
	QItemSelectionModel *sel = ui->dbFilterTableView->selectionModel();
	QModelIndexList indexes = sel->selectedIndexes();
	FilterBaseBeanModel *mdl = ui->dbFilterTableView->model();

	if ( !indexes.isEmpty() ) {
		QModelIndex idx = indexes.at(0);
		if ( d->m_selectedBean != NULL ) {
			*(d->m_selectedBean) = mdl->bean(idx);
		}
	}
	close();
}

DBFormDlg::DBFormButtons DBFormDlg::visibleButtons()
{
	return d->m_buttons;
}

/*!
  Permite ocultar botones
  */
void DBFormDlg::setVisibleButtons(DBFormDlg::DBFormButtons buttons)
{
	d->m_buttons = buttons;
	ui->pbNew->setVisible(buttons.testFlag(CREATE));
	ui->pbEdit->setVisible(buttons.testFlag(EDIT));
	ui->pbDelete->setVisible(buttons.testFlag(DELETE));
	ui->pbSearch->setVisible(buttons.testFlag(SEARCH));
	ui->pbCopy->setVisible(buttons.testFlag(COPY));
	ui->pbAdjust->setVisible(buttons.testFlag(ADJUST_LINES));
	ui->pbClose->setVisible(buttons.testFlag(EXIT));
	ui->pbOk->setVisible(buttons.testFlag(OK));
}

/*!
  Tenemos que decirle al motor de scripts, que DBFormDlg se convierte de esta forma a un valor script
  */
QScriptValue DBFormDlg::toScriptValue(QScriptEngine *engine, DBFormDlg * const &in)
{
	return engine->newQObject(in);
}

void DBFormDlg::fromScriptValue(const QScriptValue &object, DBFormDlg * &out)
{
	out = qobject_cast<DBFormDlg *>(object.toQObject());
}

DBFilterTableView *DBFormDlg::dbFilterTableView()
{
	return ui->dbFilterTableView;
}

/*!
  Fuerza un refresco de los datos del filterTableView
  */
void DBFormDlg::refreshFilterTableView()
{
	if ( !d->m_childFormOpen ) {
		ui->dbFilterTableView->refresh();
	}
}

/*!
  Comprueba si el usuario tiene permisos para siquiera abrir este formulario
*/
bool DBFormDlg::checkPermissionsToOpen()
{
	QHash<QString, QVariant> permissions = qApp->property("permissions").toHash();
	QString p = permissions.value(d->m_tableName).toString();
	if ( p.isEmpty() || ( !p.contains("r") && !p.contains("w") ) ) {
		QMessageBox::warning(this, trUtf8(APP_NAME), trUtf8("No tiene permisos para acceder a estos datos."), QMessageBox::Ok);
		return false;
	}
	return true;
}

/*!
  Permite llamar a un método de la clase que controla al formulario. Muy interesante
  para obtener valores determinados del formulario.
  */
QScriptValue DBFormDlg::callMethod(const QString &method)
{
	QScriptValue result;
	d->m_engine.callQsObjectFunction(result, method);
	return result;
}
