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
#include "dbdetailview.h"
#include "ui_dbdetailview.h"
#include "configuracion.h"
#include "dao/beans/basebean.h"
#include "dao/beans/dbrelation.h"
#include "dao/beans/beansfactory.h"
#include "dao/observerfactory.h"
#include "dao/dbrelationobserver.h"
#include "models/relationbasebeanmodel.h"
#include "models/filterbasebeanmodel.h"
#include "forms/dbrecorddlg.h"
#include "widgets/dbtableview.h"
#include <QPointer>
#include <QtGui>

#define MSG_REGISTRO_NO_SELECCIONADO	QT_TR_NOOP("No se ha seleccionado ning\303\272n registro a editar. Por favor, seleccione un registro de la lista para su edici\303\263n.")

class DBDetailViewPrivate
{
public:
	/** Para unificar llamadas a slots */
	QSignalMapper *m_signalMapper;
	/** Edición directa en el DBDetailView */
	bool m_inlineEdit;

	DBDetailViewPrivate() {
		m_signalMapper = NULL;
		m_inlineEdit = false;
	}
};

DBDetailView::DBDetailView(QWidget *parent) :
	QWidget(parent), DBAbstractViewInterface(),
	ui(new Ui::DBDetailView), d(new DBDetailViewPrivate)
{
    ui->setupUi(this);
	m_internalData = true;

	ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

	d->m_signalMapper = new QSignalMapper(this);
	// No se admiten bool, por eso se pasa como string
	d->m_signalMapper->setMapping(ui->pbAdd, "true");
	d->m_signalMapper->setMapping(ui->pbEdit, "false");
	connect(ui->pbAdd, SIGNAL(clicked()), d->m_signalMapper, SLOT(map()));
	connect(ui->pbEdit, SIGNAL(clicked()), d->m_signalMapper, SLOT(map()));
	connect(d->m_signalMapper, SIGNAL(mapped(const QString &)), this, SLOT(editRecord(const QString &)));
	connect(ui->tableView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(editRecord()));
	connect(ui->pbDelete, SIGNAL(clicked()), this, SLOT(deleteRecord()));
}

DBDetailView::~DBDetailView()
{
	emit destroyed(this);
    delete ui;
	delete d;
}

void DBDetailView::setValue(const QVariant &value)
{
	Q_UNUSED(value)
}

QAbstractItemView::EditTriggers DBDetailView::editTriggers () const
{
	return ui->tableView->editTriggers();
}

void DBDetailView::setEditTriggers (QAbstractItemView::EditTriggers triggers)
{
	ui->tableView->setEditTriggers(triggers);
}

bool DBDetailView::inlineEdit () const
{
	return d->m_inlineEdit;
}

void DBDetailView::setInlineEdit(bool value)
{
	d->m_inlineEdit = value;
	ui->pbEdit->setVisible(!d->m_inlineEdit);
	ui->tableView->setAtRowsEndNewRow(true);
	ui->tableView->setNavigateOnEnter(true);
	ui->tableView->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked |
								   QAbstractItemView::AnyKeyPressed | QAbstractItemView::EditKeyPressed );
}

void DBDetailView::editRecord(const QString &insert)
{
	DbRecordOpenType::DbRecordOpenType openType;
	bool userSaveData;

	if ( m_filterModel == NULL ) {
		return;
	}
	if ( insert == "false" ) {
		openType = DbRecordOpenType::UPDATE;
	} else {
		openType = DbRecordOpenType::INSERT;
	}

	if ( d->m_inlineEdit ) {
		if ( openType == DbRecordOpenType::INSERT ) {
			int row = m_model->rowCount();
			m_model->insertRow(row);
			QModelIndex idx = m_model->index(row, 0);
			ui->tableView->setCurrentIndex(idx);
			ui->tableView->setFocus();
			ui->tableView->edit(idx);
		}
	} else {
		// Y ahora creamos el formulario que presentará los datos de este bean
		QPointer<DBRecordDlg> dlg = new DBRecordDlg(m_filterModel, ui->tableView->selectionModel(),
										   &userSaveData, openType, this);
		if ( dlg->openSuccess() ) {
			// Guardar los datos de los hijos agregados, será responsabilidad del bean padre
			// que se está editando
			dlg->setAttribute(Qt::WA_DeleteOnClose);
			dlg->setModal(true);
			dlg->exec();
		} else {
			delete dlg;
		}
	}
}

void DBDetailView::deleteRecord()
{
	// Vamos a actuar así: Seleccionaremos toda la fila del item y preguntaremos
	QModelIndex index = ui->tableView->currentIndex();
    QAbstractItemModel *mdl = ( m_filterModel != NULL ? m_filterModel : m_model );

	if ( mdl == NULL ) {
		return;
	}

	if ( !index.isValid() ) {
		QMessageBox::information(this, QString::fromUtf8(APP_NAME),
								 trUtf8("Debe seleccionar registros para borrar."), QMessageBox::Ok);
		return;
	} else {
		ui->tableView->selectRow(index.row());
		ui->tableView->update();
	}
	QString mensaje = trUtf8("\302\277Está seguro de querer borrar el registro?");

	int ret = QMessageBox::information(this, QString::fromUtf8(APP_NAME), mensaje,
									   QMessageBox::Yes | QMessageBox::No );
	if ( ret == QMessageBox::Yes ) {
		// Esta función llama al removeRow del source model
		mdl->removeRow(index.row(), QModelIndex());
		if ( m_filterModel != NULL ) {
			m_filterModel->invalidate();
		}
	}
}

void DBDetailView::setupInternalModel()
{
	DBAbstractViewInterface::setupInternalModel();
	if ( !m_externalModel && m_metadata != NULL ) {
		if ( m_filterModel != NULL ) {
			m_filterModel->setDbStates(BaseBean::INSERT | BaseBean::UPDATE);
		}
	}
}

void DBDetailView::applyFieldProperties()
{
	if ( !dataEditable() ) {
		ui->pbAdd->setVisible(false);
		ui->pbDelete->setVisible(false);
		ui->pbEdit->setVisible(false);
	}
}

/*!
  El observador que alimenta de datos a este control se ha borrado. Actuamos reseteándolo
  */
void DBDetailView::observerUnregistered()
{
	DBAbstractViewInterface::observerUnregistered();
	blockSignals(true);
	ui->tableView->reset();
	blockSignals(false);
}

/*!
  Pide un nuevo observador si es necesario
  */
void DBDetailView::refresh()
{
	observer();
	if ( m_observer != NULL ) {
		m_observer->sync();
	}
}

/*!
  Al mostrarse el control es cuando se crean los modelos y demás
  */
void DBDetailView::showEvent ( QShowEvent * event )
{
	DBBaseWidget::showEvent(event);
	init(true);
	if ( m_filterModel != NULL ) {
		// Poniendo de parent a this, garantizamos que el objeto se borra al cerrarse el formulario
		ui->tableView->setModel(m_filterModel);
		ui->tableView->setSelectionModel(m_selectionModel);
		RelationBaseBeanModel *mdl = qobject_cast<RelationBaseBeanModel *>(m_model);
		if ( mdl == 0 ) {
			ui->pbAdd->setVisible(false);
			ui->pbEdit->setVisible(false);;
			ui->pbDelete->setVisible(false);;
		}
	}
}

/*!
  Tenemos que decirle al motor de scripts, que DBFormDlg se convierte de esta forma a un valor script
  */
QScriptValue DBDetailView::toScriptValue(QScriptEngine *engine, DBDetailView * const &in)
{
	return engine->newQObject(in, QScriptEngine::QtOwnership, QScriptEngine::PreferExistingWrapperObject);
}

void DBDetailView::fromScriptValue(const QScriptValue &object, DBDetailView * &out)
{
	out = qobject_cast<DBDetailView *>(object.toQObject());
}
