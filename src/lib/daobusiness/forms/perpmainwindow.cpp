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
#include "perpmainwindow.h"
#include "configuracion.h"
#include "forms/dbformdlg.h"
#include "forms/qdlgacercade.h"
#include "forms/seleccionestilodlg.h"
#include "forms/changepassworddlg.h"
#include "dao/beans/beansfactory.h"
#include "scripts/perpscript.h"
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QApplication>
#include <QAction>
#include <QMenu>
#include <QCloseEvent>
#include <QPointer>

class PERPMainWindowPrivate
{
public:
	/** Permitirá un mismo slot para la apertura de todos los actions */
	QSignalMapper *m_signalMapper;
	QMdiArea *m_mdiArea;
	/** En las ventanas MDI de Qt, cuando se añade un MainWindow al MDIArea, automáticamente
	le crea un objeto MDISubwindow, y esta equivalencia es la que guardamos */
	QMap<QString, QMdiSubWindow *> m_mapeoVentanas;
	PERPScript m_engine;

	PERPMainWindowPrivate() {}
};

PERPMainWindow::PERPMainWindow(QWidget* parent, Qt::WFlags fl)
: QMainWindow( parent, fl ), d(new PERPMainWindowPrivate)
{
	d->m_mdiArea = new QMdiArea(this);
	d->m_mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	d->m_mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	if ( configuracion.mdiTabView() ) {
		d->m_mdiArea->setViewMode(QMdiArea::TabbedView);
	}
	setCentralWidget(d->m_mdiArea);
}

PERPMainWindow::~PERPMainWindow()
{
	delete d;
}

void PERPMainWindow::closeEvent(QCloseEvent * event)
{
	configuracion.saveDimensionForm(this);
	configuracion.savePosForm(this);
	
	event->accept();
}

/*!
  Este formulario puede contener cierto código script a ejecutar en su inicio. Esta función lo lanza
  inmediatamente. El código script está en printingerp_system, con el nombre de la tabla principal
  acabado en main.PERPMainWindow.qs. Es el primer código Qs que se ejecuta en la aplicación
  */
bool PERPMainWindow::execQs()
{
	QString qsName("main.qmaindlg.qs");

	/** Ejecutamos el script asociado. La filosofía fundamental de ese script es proporcionar
	  algo de código básico que justifique este formulario de búsqueda */
	if ( !BeansFactory::tableWidgetsScripts.contains(qsName) ) {
		return true;
	}

	d->m_engine.setScriptName(qsName);
	d->m_engine.setDebug(BeansFactory::tableWidgetsScriptsDebug.value(qsName));
	d->m_engine.setOnInitDebug(BeansFactory::tableWidgetsScriptsDebugOnInit.value(qsName));
	d->m_engine.setScriptObject("MainDlg");
	d->m_engine.setUi(this);
	d->m_engine.addAvailableObject("thisForm", this);
	if ( !d->m_engine.createQsObject() ) {
		QMessageBox::warning(this, trUtf8(APP_NAME), trUtf8("Ha ocurrido un error al cargar el script asociado a este "
                                "formulario. Es posible que algunas funciones no est\303\251n disponibles."),
                                QMessageBox::Ok);
#if !defined(QT_NO_SCRIPTTOOLS)
		int ret = QMessageBox::information(this, trUtf8(APP_NAME), trUtf8("El script ejecutado contiene errores. ¿Desea editarlo?"),
										   QMessageBox::Yes | QMessageBox::No);
		if ( ret == QMessageBox::Yes ) {
			d->m_engine.editScript(this);
		}
#endif
		return false;
	}
	return true;
}

void PERPMainWindow::openForm(const QString &actionName)
{
	QString tableName = actionName;
	tableName.replace("table_", "");
	QString formName = QString("mdiform_%1").arg(tableName);
	if ( d->m_mapeoVentanas.contains(formName) ) {
		d->m_mdiArea->setActiveSubWindow(d->m_mapeoVentanas.value(formName));
		return;
	}
	DBFormDlg *dlg = new DBFormDlg(tableName, this);
	if ( dlg->openSuccess() ) {
		// Muy importante para el mapeo MDI
		dlg->setObjectName(formName);
		dlg->setAttribute(Qt::WA_DeleteOnClose);
		QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
		dlg->init();
		QApplication::restoreOverrideCursor();

		QMdiSubWindow *subWin = d->m_mdiArea->addSubWindow(dlg);
		d->m_mapeoVentanas.insert(formName, subWin);
		subWin->setOption(QMdiSubWindow::RubberBandResize, true);
		subWin->setOption(QMdiSubWindow::RubberBandMove, true);
		// Guardamos una referencia al objeto
		subWin->setAttribute(Qt::WA_DeleteOnClose);
		QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
		subWin->show();
		QApplication::restoreOverrideCursor();

		connect (dlg, SIGNAL(closingWindow(QWidget *)), this, SLOT(closeSubWindow(QWidget *)));
		connect (subWin, SIGNAL(aboutToActivate()), dlg, SLOT(refreshFilterTableView()));
		configuracion.applyDimensionForm(subWin);
	} else {
		delete dlg;
	}
}

void PERPMainWindow::init()
{
	if ( !execQs() ) {
		this->close();
		return;
	}
	d->m_signalMapper = new QSignalMapper(this);

	QList<QAction *> actions = this->findChildren<QAction *>();
	QHash<QString, QVariant> permissions = qApp->property("permissions").toHash();
	foreach ( QAction *action, actions ) {
		// Las acciones deben tener el mismo nombre que las tablas principales
		if ( action->objectName().contains("table") ) {
			QString tableName = action->objectName();
			tableName.replace("table_", "");
			QString p = permissions.value(tableName).toString();
			if ( p.isEmpty() || (!p.contains("r") && !p.contains("w")) ) {
				action->setVisible(false);
			}
			d->m_signalMapper->setMapping(action, action->objectName());
			connect(action, SIGNAL(triggered()), d->m_signalMapper, SLOT (map()));
		}
	}
	// Borramos los menús sin nada
	QList<QMenu *> menus = findChildren<QMenu *>();
	foreach ( QMenu *menu, menus ) {
		if ( menu->isEmpty() ) {
			menu->setVisible(false);
		}
	}

	connect(d->m_signalMapper, SIGNAL(mapped(const QString &)), this, SLOT(openForm(const QString &)));

	QAction *actionAbout = findChild<QAction *>("actionAbout");
	QAction *actionStyle = findChild<QAction *>("actionStyle");
	QAction *actionClose = findChild<QAction *>("actionClose");
	QAction *actionChangePassword = findChild<QAction *> ("actionChangePassword");
	if ( actionAbout ) {
		connect (actionAbout, SIGNAL(triggered()), this, SLOT(about()));
	}
	if ( actionStyle ) {
		connect (actionStyle, SIGNAL(triggered()), this, SLOT(style()));
	}
	if ( actionClose ) {
		connect (actionClose, SIGNAL(triggered()), this, SLOT(close()));
	}
	if ( actionChangePassword ) {
		connect (actionChangePassword, SIGNAL(triggered()), this, SLOT(changePassword()));
	}
}

void PERPMainWindow::about()
{
	QPointer<QDlgAcercaDe> dlg = new QDlgAcercaDe(this);
	dlg->setAttribute(Qt::WA_DeleteOnClose);
	dlg->setModal(true);
	dlg->show();
	delete dlg;
}

void PERPMainWindow::style(void )
{
	QPointer<SeleccionEstiloDlg> dlg = new SeleccionEstiloDlg(this);
	dlg->setAttribute(Qt::WA_DeleteOnClose);
	dlg->setModal(true);
	dlg->exec();
}

void PERPMainWindow::changePassword(void )
{
	QPointer<ChangePasswordDlg> dlg = new ChangePasswordDlg(false, this);
	dlg->setAttribute(Qt::WA_DeleteOnClose);
	dlg->setModal(true);
	dlg->exec();
}

void PERPMainWindow::closeSubWindow(QWidget * objeto)
{
	if ( objeto != NULL ) {
		QMdiSubWindow *subWin = d->m_mapeoVentanas.value(objeto->objectName());
		bool wasMaximized = subWin->isMaximized();
		if ( subWin != NULL ) {
			d->m_mdiArea->removeSubWindow(d->m_mapeoVentanas.value(objeto->objectName()));
			objeto->disconnect(SIGNAL(closingWindow(QWidget *)));
			d->m_mapeoVentanas.remove(objeto->objectName());
			QMdiSubWindow *newActiveSubWindow = d->m_mdiArea->currentSubWindow();
			if ( newActiveSubWindow != NULL ) {
				d->m_mdiArea->setActiveSubWindow(newActiveSubWindow);
				if ( wasMaximized ) {
					newActiveSubWindow->showMaximized();
				}
			}
		}
	}
}

/*!
  Fuerza un refresco de los datos visibles en el DBFilterTableView de una ventana hija, si lo hubiera
  */
void PERPMainWindow::refreshSubWindowData(QMdiSubWindow *mdiWindow)
{
	if ( mdiWindow != NULL ) {
		DBFormDlg *wid = qobject_cast<DBFormDlg *>(mdiWindow->widget());
		if ( wid != NULL ) {
			wid->refreshFilterTableView();
		}
	}
}
