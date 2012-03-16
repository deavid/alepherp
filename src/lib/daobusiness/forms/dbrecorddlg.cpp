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
#include "ui_dbrecorddlg.h"
#include "dbrecorddlg.h"
#include "configuracion.h"
#include "scripts/perpscriptwidget.h"
#include "dao/beans/basebean.h"
#include "dao/beans/beansfactory.h"
#include "dao/beans/basebeanmetadata.h"
#include "dao/observerfactory.h"
#include "dao/basebeanobserver.h"
#include "dao/database.h"
#include "models/basebeanmodel.h"
#include "models/filterbasebeanmodel.h"
#include "forms/perpscripteditdlg.h"
#include "forms/perpbasedialog_p.h"
#include "forms/historyviewdlg.h"
#include "widgets/fademessage.h"
#include "widgets/dbhtmleditor.h"
#include "widgets/dbcodeedit.h"
#include <QDir>
#include <QUiLoader>
#include <QMessageBox>
#include <QCloseEvent>
#include <QPropertyAnimation>
#include <QDateTime>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QScriptValue>
#include <QSignalMapper>
#include <QTextEdit>
#include <QTableView>
#include <QPlainTextEdit>
#include <QGroupBox>

class DBRecordDlgPrivate
{
//	Q_DECLARE_PUBLIC(DBRecordDlg)
public:
	/** BaseBean que se edita */
	QSharedPointer<BaseBean> m_bean;
	/** Row del modelo en el que está el bean */
	int m_sourceModelRow;
	/** Widget principal */
	QWidget *m_widget;
	/** Indica si el usuario ha guardado los datos o ha cancelado la edición */
	bool *m_userSaveData;
	/** Para saber en qué modo se abre el formulario */
	DbRecordOpenType::DbRecordOpenType m_openType;
	/** Observer de este formulario */
	BaseBeanObserver *m_observer;
	/** Número de bloqueo del registro */
	int m_lockId;
	/** Modelo sobre el que opera el formulario */
	FilterBaseBeanModel *m_model;
	/** Modelo de selección para mantener sincronía */
	QItemSelectionModel *m_selectionModel;
	QSignalMapper *m_signalMapper;
	/** Se determina si al pulsar el botón de cerrar se pregunta o no */
	bool m_closeButtonAskForSave;
    /** Si el desarrollador pone m_closeButtonAskForSave, necesitamos saber de alguna manera
      si el bean que el usuario ha ido introduciendo es valido: sin esta propiedad, el evento
      closeEvent se cargaria el bean. El bean sera válido si el desarrollador llama
      a closeButtonAskForSave = false y m_beanIsValid = true */
    bool m_beanIsValid;

	DBRecordDlgPrivate() {
		m_closeButtonAskForSave = true;
		m_widget = NULL;
		m_observer = NULL;
		m_model = NULL;
		m_selectionModel = NULL;
		m_signalMapper = NULL;
        m_beanIsValid = false;
        m_closeButtonAskForSave = false;
	}

	void readBeanFromModel(const QModelIndex &idx);
	void readBeanFromModel(int row);
	void insertRow( QItemSelectionModel *selectionModel);
	QModelIndex nextIndex(const QModelIndex actual, const QString &direction);
};

void DBRecordDlgPrivate::readBeanFromModel(int row)
{
	if ( m_model == NULL ) {
		return;
	}
	QModelIndex idx = m_model->index(row, 0);
	readBeanFromModel(idx);
}

void DBRecordDlgPrivate::readBeanFromModel(const QModelIndex &idx)
{
	m_bean = m_model->beanToBeEdited(idx);
}

void DBRecordDlgPrivate::insertRow(QItemSelectionModel *selectionModel)
{
	// Es mejor insertar directamente el modelo padre, ya que insertar en el filtro
	// suele dar problemas
	BaseBeanModel *mdl = qobject_cast<BaseBeanModel *> (m_model->sourceModel());
	m_sourceModelRow = mdl->rowCount();
	mdl->insertRow(m_sourceModelRow);
	QModelIndex sourceIdx = mdl->index(m_sourceModelRow, 0);
	QModelIndex filterIdx = m_model->mapFromSource(sourceIdx);
	m_bean = mdl->beanToBeEdited(sourceIdx);
	selectionModel->setCurrentIndex(filterIdx, QItemSelectionModel::Rows);
	selectionModel->select(filterIdx, QItemSelectionModel::Rows);
}

DBRecordDlg::DBRecordDlg(QSharedPointer<BaseBean> bean, bool *userSaveData,
			DbRecordOpenType::DbRecordOpenType openType, QWidget* parent, Qt::WFlags fl) :
	PERPBaseDialog(parent, fl), ui(new Ui::DBRecordDlg), d(new DBRecordDlgPrivate)
{
	d->m_model = NULL;
	d->m_selectionModel = NULL;
	d->m_bean = bean;
	d->m_userSaveData = userSaveData;
	// Este chivato indica si el registro lo guardará este formulario en base de datos o no
	d->m_openType = openType;
	// Si se pulsa Guardar, se pondrá esto a true
	*(d->m_userSaveData) = false;
	if ( !init() ) {
		close();
		setOpenSuccess(false);
		return;
	}
	ui->pbNext->setVisible(false);
	ui->pbFirst->setVisible(false);
	ui->pbLast->setVisible(false);
	ui->pbPrevious->setVisible(false);
	setOpenSuccess(true);
}

/*!
  El DBRecordDlg es el formulario de edición de los beans. Se le pasa el modelo del que deberá
  obtener el bean, según el selectionModel pasado.
  */
DBRecordDlg::DBRecordDlg(FilterBaseBeanModel *model, QItemSelectionModel *selectionModel, bool *userSaveData, DbRecordOpenType::DbRecordOpenType openType, QWidget* parent, Qt::WFlags fl) :
	PERPBaseDialog(parent, fl), ui(new Ui::DBRecordDlg), d(new DBRecordDlgPrivate)
{
	d->m_model = model;
	d->m_selectionModel = selectionModel;
	d->m_userSaveData = userSaveData;
	// Este chivato indica si el registro lo guardará este formulario en base de datos o no
	d->m_openType = openType;

	if ( d->m_openType == DbRecordOpenType::UPDATE ) {
		QModelIndexList list = d->m_selectionModel->selectedIndexes();
		if ( list.size() == 0 ) {
			QMessageBox::warning(this, trUtf8(APP_NAME), trUtf8("No existe ningún registro seleccionado."), QMessageBox::Ok);
			close();
			setOpenSuccess(false);
			return;
		}
		d->readBeanFromModel(list.at(0));
		if ( d->m_userSaveData == NULL || d->m_bean.isNull() ) {
			QMessageBox::warning(this, trUtf8(APP_NAME), trUtf8("Ha ocurrido un error seleccionando los datos."), QMessageBox::Ok);
			close();
			setOpenSuccess(false);
			return;
		}
	} else if ( d->m_openType == DbRecordOpenType::INSERT ) {
		d->insertRow(selectionModel);
	}
	// Si se pulsa Guardar, se pondrá esto a true
	*(d->m_userSaveData) = false;
	if ( !init() ) {
		close();
		setOpenSuccess(false);
		return;
	}
	d->m_signalMapper = new QSignalMapper(this);
	d->m_signalMapper->setMapping(ui->pbNext, QString("next"));
	d->m_signalMapper->setMapping(ui->pbPrevious, QString("previous"));
	d->m_signalMapper->setMapping(ui->pbLast, QString("last"));
	d->m_signalMapper->setMapping(ui->pbFirst, QString("first"));
	connect (ui->pbNext, SIGNAL(clicked()), d->m_signalMapper, SLOT(map()));
	connect (ui->pbPrevious, SIGNAL(clicked()), d->m_signalMapper, SLOT(map()));
	connect (ui->pbFirst, SIGNAL(clicked()), d->m_signalMapper, SLOT(map()));
	connect (ui->pbLast, SIGNAL(clicked()), d->m_signalMapper, SLOT(map()));
	connect (d->m_signalMapper, SIGNAL(mapped(const QString &)), this, SLOT(navigate(const QString &)));
	setOpenSuccess(true);
}

bool DBRecordDlg::init()
{
	setTableName(d->m_bean->metadata()->tableName());
	if ( !checkPermissionsToOpen() ) {
		return false;
	}

	ui->setupUi(this);
	if ( d->m_bean.isNull() ) {
		return true;
	}

	// Si es una versión de desarrollo, no se puede editar el script
	if ( !configuracion.debuggerEnabled() ) {
		ui->pbEditScript->setVisible(false);
	}

	setWindowFlags(windowFlags() | Qt::WindowMinMaxButtonsHint |
			Qt::WindowSystemMenuHint | Qt::WindowContextHelpButtonHint);
	// Leemos y establecemos de base de datos los widgets de este form
	setupMainWidget();
	setWindowTitle(trUtf8("Edici\303\263n de %1 [*]").arg(d->m_bean->metadata()->alias()));

	// Para poder visualizar el contenido de los beans
	d->m_observer = qobject_cast<BaseBeanObserver *>(d->m_bean->observer());
	d->m_bean->observer()->installWidget(this);

	connect (ui->pbClose, SIGNAL(clicked()), this, SLOT(close()));
	connect (ui->pbSave, SIGNAL(clicked()), this, SLOT(save()));
    connect (ui->pbEditScript, SIGNAL(clicked()), engine(), SLOT(editScript()));
	connect (ui->pbHistory, SIGNAL(clicked()), this, SLOT(showHistory()));
	QSqlDatabase db = Database::getQDatabase();
	QStringList tmp = db.driver()->subscribedToNotifications();
	foreach ( QString a, tmp ) {
		qDebug() << a;
	}

	// Si estamos creando un nuevo registro, la ventana aparece como no modificada
	if ( d->m_openType == DbRecordOpenType::UPDATE ) {
		// Hacemos una copia de seguridad de los datos, por si hay cancelación
		d->m_bean->backupValues();
		lock();
	}

	connect (db.driver(), SIGNAL(notification(const QString &)), this, SLOT(lockBreaked(const QString &)));
	installEventFilters();
	setWindowModified(false);
	// Código propio del formulario
	execQs();
	return true;
}

DBRecordDlg::~DBRecordDlg()
{
	if ( d->m_openType == DbRecordOpenType::UPDATE ) {
		BaseDAO::unlock(d->m_lockId);
	}
	// Esta llamada es MUY importante
	if ( !d->m_bean.isNull() ) {
		d->m_bean->observer()->uninstallWidget(this);
	}
	delete ui;
	delete d;
}

void DBRecordDlg::installEventFilters()
{
	QList<QWidget *> childs = findChildren<QWidget *>();
	foreach ( QWidget *child, childs ) {
		if ( child->property("perpControl").toBool() ) {
			child->installEventFilter(this);
		}
	}
}

bool DBRecordDlg::closeButtonAskForSave()
{
	return d->m_closeButtonAskForSave;
}

void DBRecordDlg::setCloseButtonAskForSave(bool value)
{
	d->m_closeButtonAskForSave = value;
}

void DBRecordDlg::lock()
{
	if ( d->m_openType == DbRecordOpenType::INSERT ) {
		return;
	}
	d->m_lockId = BaseDAO::newLock(d->m_bean->metadata()->tableName(), qApp->property("userName").toString(), d->m_bean->pkValue());
	if ( d->m_lockId == -1 ) {
		QHash<QString, QVariant> info;
		if ( BaseDAO::lockInformation(d->m_bean->metadata()->tableName(), d->m_bean->pkValue(), info) ) {
			QDateTime blockDate = info.value("ts").toDateTime();
			QString message = trUtf8("El registro actual se encuentra bloqueado por el usuario: <b>%1</b>. "
									 "Fue bloqueado: </i>%2</i>. "
									 "¿Desea desbloquearlo? Si lo desbloquea, el usuario %3 perderá "
									 "todos sus datos.").
							  arg(info.value("username").toString()).
							  arg(configuracion.getLocale()->toString(blockDate, configuracion.getLocale()->dateFormat())).
							  arg(info.value("username").toString());
			int ret = QMessageBox::information(this, trUtf8(APP_NAME), message, QMessageBox::Yes | QMessageBox::No);
			if ( ret == QMessageBox::Yes ) {
				int idLock = info["id"].toInt();
				if ( BaseDAO::unlock(idLock) ) {
					d->m_lockId = BaseDAO::newLock(d->m_bean->metadata()->tableName(), qApp->property("userName").toString(), d->m_bean->pkValue());
					if ( d->m_lockId == -1 ) {
						QMessageBox::warning(this, trUtf8(APP_NAME), trUtf8("Ocurrió un error escogiendo un bloqueo. Es probable que no pueda guardar los datos que modifique."), QMessageBox::Ok);
					}
				}
			}
		}
	}
}

/*!
  Este slot comprueba si el lock que se tenía establecido se ha roto. Si es así, se informa
  al usuario
  */
void DBRecordDlg::lockBreaked(const QString &notification)
{
	Q_UNUSED(notification)
	qDebug() << "Se ha roto un bloqueo.";
	if ( BaseDAO::isLockValid(d->m_lockId, d->m_bean->metadata()->tableName(), configuracion.userDb(), d->m_bean->pkValue()) ) {
		return;
	}
	d->m_lockId = -1;
	QHash<QString, QVariant> info;
	if ( BaseDAO::lockInformation(d->m_bean->metadata()->tableName(), d->m_bean->pkValue(), info) ) {
		QString message = trUtf8("El usuario <b>%1</b> se ha apropiado de este registro. Los cambios que realice puede que no estén sincronizados con la última versión de base de datos, y ambos pierdan sus datos.").
						  arg(info["username"].toString());
		qDebug() << "Se ha roto un bloqueo.";
		FadeMessage *msg = new FadeMessage(message, this);
		msg->move(0, 0);
		msg->show();
	}
}

void DBRecordDlg::showEvent(QShowEvent * event)
{
	Q_UNUSED(event)
	PERPBaseDialog::showEvent(event);
}

/*!
  Cancela la edición del registro actual. Cierra la ventana actual sin comprobar si hay modificaciones o no.
  */
void DBRecordDlg::cancel()
{
	if ( d->m_openType == DbRecordOpenType::UPDATE ) {
		// Se ha cancelado la edición, restauramos los valores de la copia de seguridad interna
		d->m_bean->restoreValues();
	} else if ( d->m_openType == DbRecordOpenType::INSERT ) {
		// Si cancelamos insertando, indicamos que NO se han introducido cambios, para que no se guarden
		d->m_bean->uncheckModifiedFields();
	}
	setWindowModified(false);
	this->close();
}

void DBRecordDlg::closeEvent(QCloseEvent * event)
{
	PERPBaseDialog::closeEvent(event);
	if ( !d->m_closeButtonAskForSave ) {
		if ( d->m_model != NULL && d->m_openType == DbRecordOpenType::INSERT && !d->m_beanIsValid ) {
			d->m_model->sourceModel()->removeRow(d->m_sourceModelRow);
        }
        event->accept();
		return;
	}

	if ( d->m_bean.isNull() ) {
		event->accept();
		return;
	}

	if ( isWindowModified() ) {
		int ret = QMessageBox::information(this, QString::fromUtf8(APP_NAME),
										   trUtf8("Se han producido cambios. ¿Desea guardarlos?"), QMessageBox::Yes | QMessageBox::No);
		if ( ret == QMessageBox::Yes ) {
			if ( !save() ) {
				int ret = QMessageBox::warning(this, QString::fromUtf8(APP_NAME),
											   trUtf8("Se ha producido un error guardando los datos. ¿Desea aún así cerrar el formulario?"), QMessageBox::Yes | QMessageBox::No);
				if ( ret == QMessageBox::No ) {
					event->ignore();
					return;
				}
			}
		} else {
			if ( d->m_openType == DbRecordOpenType::UPDATE ) {
				// Se ha cancelado la edición, restauramos los valores de la copia de seguridad interna
				d->m_bean->restoreValues();
			} else if ( d->m_openType == DbRecordOpenType::INSERT ) {
				// Si cancelamos insertando, indicamos que NO se han introducido cambios, para que no se guarden
				d->m_bean->uncheckModifiedFields();
				if ( d->m_model != NULL ) {
					d->m_model->sourceModel()->removeRow(d->m_sourceModelRow);
				}
			}
			*(d->m_userSaveData) = false;
		}
	} else {
		if ( d->m_openType == DbRecordOpenType::INSERT && d->m_bean->dbState() == BaseBean::INSERT &&
			 !d->m_bean->modified() ) {
			// Si cancelamos insertando, indicamos que NO se han introducido cambios, para que no se guarden
			d->m_bean->uncheckModifiedFields();
			if ( d->m_model != NULL ) {
				d->m_model->sourceModel()->removeRow(d->m_sourceModelRow);
			}
		}
		*(d->m_userSaveData) = false;
	}
	event->accept();
}

/*!
  Vamos a obtener y guardar cuándo el usuario ha modificado un control
  */
bool DBRecordDlg::eventFilter (QObject *target, QEvent *event)
{
	if ( target->property("perpControl").toBool() ) {
		if ( event->spontaneous() ) {
			if ( target->inherits("QCheckBox") || target->inherits("QDateTimeEdit") ) {
				if ( event->type() == QEvent::ModifiedChange ) {
					target->setProperty("userModified", true);
				}
			}
			if ( target->inherits("QTextEdit") || target->inherits("QComboBox") || target->inherits("QLineEdit") ) {
				if ( event->type() == QEvent::KeyPress ) {
					target->setProperty("userModified", true);
				}
			}
		}
	}
    QTextEdit *tmp1 = qobject_cast<QTextEdit *>(target);
    DBHtmlEditor *tmp2 = qobject_cast<DBHtmlEditor *>(target);
    DBCodeEdit *tmp3 = qobject_cast<DBCodeEdit *>(target);
    QTableView *tmp4 = qobject_cast<QTableView *>(target);
    QPlainTextEdit *tmp5 = qobject_cast<QPlainTextEdit *>(target);
    if ( tmp1 == NULL && tmp2 == NULL && tmp3 == NULL && tmp4 == NULL && tmp5 == NULL
            && event->type() == QEvent::KeyPress ) {
        QKeyEvent *ev = static_cast<QKeyEvent *>(event);
        if ( ev->key() == Qt::Key_Enter || ev->key() == Qt::Key_Return ) {
            focusNextChild();
            ev->accept();
			return true;
        } else if ( ev->key() == Qt::Key_Escape ) {
            close();
            ev->accept();
			return true;
		} else {
			return PERPBaseDialog::eventFilter(target, event);
		}
    } else {
        return PERPBaseDialog::eventFilter(target, event);
    }
}

/*!
  Carga el formulario ui definido en base de datos, y que define la interfaz de usuario. Puede haber
  dos formularios: nombre_tabla.new.dbrecord.ui que se utilza para insertar un nuevo registro
  o nombre_tabla.dbrecord.ui que se utiliza para editar y para insertar un nuevo registro
  si nombre_tabla.new.dbrecord.ui no existe
  */
void DBRecordDlg::setupMainWidget()
{
	QString fileNewName = QString("%1/%2.new.dbrecord.ui").
			arg(QDir::fromNativeSeparators(configuracion.tempPath())).
			 arg(d->m_bean->metadata()->tableName());
	QString fileEditName = QString("%1/%2.dbrecord.ui").
					   arg(QDir::fromNativeSeparators(configuracion.tempPath())).
						arg(d->m_bean->metadata()->tableName());
	QString fileName;
	QUiLoader uiLoader;
    QString pluginDir = QString("%1/plugins/designer").arg(qApp->applicationDirPath());

	if ( QFile::exists(fileNewName) && d->m_openType == DbRecordOpenType::INSERT ) {
		fileName = fileNewName;
		// Nombre único para identificar las propiedades de este formulario
		setObjectName(QString("%1.new.dbrecord.ui").arg(d->m_bean->metadata()->tableName()));
	} else {
		fileName = fileEditName;
		// Nombre único para identificar las propiedades de este formulario
		setObjectName(QString("%1.dbrecord.ui").arg(d->m_bean->metadata()->tableName()));
	}

	if ( QFile::exists(fileName) ) {
		QFile file (fileName);
		uiLoader.addPluginPath(pluginDir);
		file.open( QFile::ReadOnly );
		d->m_widget = uiLoader.load(&file, 0);
		if ( d->m_widget != NULL ) {
			d->m_widget->setParent(this);
			ui->widgetLayout->addWidget(d->m_widget);

			/** Los PERPScriptWidgets se crean y dimensiona con este formulario ya creado y visible.
			  Eso provoca problemas en las redimensiones, que se intentan corregir aquí */
			QList<PERPScriptWidget *> scriptWidgets = this->findChildren<PERPScriptWidget *> ();
			foreach ( PERPScriptWidget *wid, scriptWidgets ) {
				connect(wid, SIGNAL(ready()), this, SLOT(resizeToSavedDimension()));
			}
		} else {
			QMessageBox::warning(this, trUtf8(APP_NAME), trUtf8("No se ha podido cargar la interfaz de usuario de este formulario. Existe un problema en la definición de las tablas de sistema de su programa."),
								 QMessageBox::Ok);
			close();
		}
		file.close();
	} else {
		QLayout *lay = new QVBoxLayout;
		QGroupBox *gb = new QGroupBox(this);
		setupWidgetFromBaseBeanMetadata(d->m_bean->metadata(), lay);
		gb->setLayout(lay);
		ui->widgetLayout->addWidget(gb);
	}
}

/*!
  Este formulario puede contener cierto código script a ejecutar en su inicio. Esta función lo lanza
  inmediatamente. El código script está en presupuestos_system, con el nombre de la tabla principal
  acabado en dbform.qs
  */
void DBRecordDlg::execQs()
{
	QString qsName;

	if ( d->m_openType == DbRecordOpenType::INSERT ) {
		qsName = QString ("%1.new.dbrecord.qs").arg(d->m_bean->metadata()->tableName());
		if ( !BeansFactory::tableWidgetsScripts.contains(qsName) ) {
			qsName = QString ("%1.dbrecord.qs").arg(d->m_bean->metadata()->tableName());
		}
	} else {
		qsName = QString ("%1.dbrecord.qs").arg(d->m_bean->metadata()->tableName());
	}

	/** Ejecutamos el script asociado. La filosofía fundamental de ese script es proporcionar
	  algo de código básico que justifique este formulario de edición de registros */
	if ( !BeansFactory::tableWidgetsScripts.contains(qsName) ) {
		return;
	}
    engine()->setScriptName(qsName);
    engine()->setDebug(BeansFactory::tableWidgetsScriptsDebug.value(qsName));
    engine()->setOnInitDebug(BeansFactory::tableWidgetsScriptsDebugOnInit.value(qsName));
    engine()->setScriptObject("DBRecordDlg");
    engine()->setUi(d->m_widget);
    engine()->addAvailableObject("thisForm", this);
    engine()->addAvailableObject("bean", d->m_bean.data());
    if ( !engine()->createQsObject() ) {
		QMessageBox::warning(this, trUtf8(APP_NAME), trUtf8("Ha ocurrido un error al cargar el script asociado a este "
															"formulario. Es posible que algunas funciones no est\303\251n disponibles."),
							 QMessageBox::Ok);
#if !defined(QT_NO_SCRIPTTOOLS)
		int ret = QMessageBox::information(this, trUtf8(APP_NAME), trUtf8("El script ejecutado contiene errores. ¿Desea editarlo?"),
										   QMessageBox::Yes | QMessageBox::No);
		if ( ret == QMessageBox::Yes ) {
            engine()->editScript(this);
		}
#endif
	}
}

/*!
  Realiza una validación de los datos para saber si los que el usuario ha introducido
  pueden guardarse.
  */
bool DBRecordDlg::validate()
{
	// Primero validamos
	if ( !d->m_observer->validate() ) {
		QString message = trUtf8("<p>No se han cumplido los requisitos necesarios para guardar este registro: </p>%1").arg(d->m_observer->validateHtmlMessages());
		QMessageBox::information(this, QString::fromUtf8(APP_NAME), message, QMessageBox::Ok);
		QWidget *obj = d->m_observer->focusWidgetOnBadValidate();
		if ( obj != NULL ) {
			obj->setFocus(Qt::OtherFocusReason);
		}
		return false;
	}
	return true;
}

/*!
  Comprueba el bloqueo, valida y guarda los datos editados por el usuario
  */
bool DBRecordDlg::save()
{
	bool result = false;

	// ¿Tenemos el bloqueo correcto?
    if ( d->m_openType == DbRecordOpenType::UPDATE && d->m_bean->canSaveOnDbDirectly() &&
			!BaseDAO::isLockValid(d->m_lockId, d->m_bean->metadata()->tableName(), configuracion.userDb(), d->m_bean->pkValue()) ) {
		int ret = QMessageBox::warning(this, trUtf8(APP_NAME), trUtf8("Otro usuario está también trabajando en este registro. "
															"Si guarda los datos, ese otro usuario no tendrá conocimiento de los datos "
															"que usted guarda, y podría sobreescribirlos. ¿Desea continuar?"), QMessageBox::Yes | QMessageBox::No);
		if ( ret == QMessageBox::No ) {
			return false;
		}
	}
	if ( validate() ) {
		result = d->m_bean->save();
		if ( !result ) {
			QMessageBox::warning(this, QString::fromUtf8(APP_NAME),
										   trUtf8("Se ha producido un error guardando los datos."), QMessageBox::Ok);
			return result;
		}
        d->m_beanIsValid = true;
		setWindowModified(false);
		*(d->m_userSaveData) = true;
	}
	return result;
}

void DBRecordDlg::setWindowModified(bool value)
{
	if ( isWindowModified() != value ) {
		ui->pbSave->setEnabled(value);
	}
	QDialog::setWindowModified(value);
}

bool DBRecordDlg::isWindowModified()
{
	return QDialog::isWindowModified();
}

QSharedPointer<BaseBean> DBRecordDlg::bean()
{
	return d->m_bean;
}

void DBRecordDlg::navigate(const QString &direction)
{
	QModelIndexList selectedIndexes = d->m_selectionModel->selectedIndexes();
	QModelIndex actual, next;
	if ( d->m_model == NULL ) {
		return;
	}
	if ( selectedIndexes.size() > 0 ) {
		actual = selectedIndexes.at(0);
	} else {
		actual = d->m_selectionModel->currentIndex();
	}
	next = d->nextIndex(actual, direction);
	if ( !actual.isValid() || !next.isValid() ) {
		return;
	}
	if ( isWindowModified() ) {
		if ( ui->chkNavigateSavingChanges->isChecked() ) {
			save();
		} else {
			int ret = QMessageBox::information(this, QString::fromUtf8(APP_NAME),
											   trUtf8("Se han producido cambios. ¿Desea guardarlos?"), QMessageBox::Yes | QMessageBox::No);
			if ( ret == QMessageBox::Yes ) {
				save();
			} else {
				if ( d->m_openType == DbRecordOpenType::INSERT ) {
					d->m_model->sourceModel()->removeRow(d->m_sourceModelRow);
					d->m_openType = DbRecordOpenType::UPDATE;
				}
			}
		}
	} else {
		if ( d->m_openType == DbRecordOpenType::INSERT ) {
			d->m_model->sourceModel()->removeRow(d->m_sourceModelRow);
			d->m_openType = DbRecordOpenType::UPDATE;
		}
	}
	BaseDAO::unlock(d->m_lockId);
	d->m_bean->observer()->uninstallWidget(this);
	d->readBeanFromModel(next);
	// Hacemos una copia de seguridad de los datos, por si hay cancelación
	d->m_bean->backupValues();
	lock();
	d->m_observer = qobject_cast<BaseBeanObserver *>(d->m_bean->observer());
	d->m_bean->observer()->installWidget(this);
	d->m_bean->observer()->sync();
	d->m_selectionModel->select(next, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
	d->m_selectionModel->setCurrentIndex(next, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
}

QModelIndex DBRecordDlgPrivate::nextIndex(const QModelIndex actual, const QString &direction)
{
	QModelIndex next;
	if ( direction == "next" ) {
		if ( actual.row() < (m_model->rowCount() - 1) ) {
			next = m_model->index(actual.row()+1, actual.column());
		}
	} else if ( direction == "previous" ) {
		if ( actual.row() > 0 ) {
			next = m_model->index(actual.row()-1, actual.column());
		}
	} else if ( direction == "first" ) {
		if ( actual.row() != 0 ) {
			next = m_model->index(0, actual.column());
		}
	} else if ( direction == "last" ) {
		if ( actual.row() != m_model->rowCount() - 1 ) {
			next = m_model->index(m_model->rowCount() - 1, actual.column());
		}
	}
	return next;
}

void DBRecordDlg::keyPressEvent (QKeyEvent * e)
{
	bool accept = true;
	if ( e->modifiers() == Qt::ControlModifier ) {
		if ( e->key() == Qt::Key_PageDown ) {
			navigate("next");
		} else if ( e->key() == Qt::Key_PageUp ) {
			navigate("previous");
		} else if ( e->key() == Qt::Key_Home ) {
			navigate("first");
		} else if ( e->key() == Qt::Key_End ) {
			navigate("last");
		} else {
			accept = false;
		}
	} else if ( e->key() == Qt::Key_Escape ) {
		close();
	} else {
		accept = false;
	}
	if ( accept ) {
		e->accept();
	}
}

void DBRecordDlg::hideDBButtons()
{
	ui->chkNavigateSavingChanges->setVisible(false);
	ui->frameButtons->setVisible(false);
}

void DBRecordDlg::showDBButtons()
{
	ui->chkNavigateSavingChanges->setVisible(true);
	ui->frameButtons->setVisible(true);
}

void DBRecordDlg::showHistory()
{
	QString pKey = d->m_bean->pkSerializedValue();
	HistoryViewDlg *dlg = new HistoryViewDlg(d->m_bean->metadata()->tableName(), pKey, this);
	dlg->setAttribute(Qt::WA_DeleteOnClose);
	dlg->setModal(true);
	dlg->exec();
}

/*!
  Guarda el registro actual, e inicia la inserción de uno nuevo. Permite una edición
  rápida de detalles
  */
void DBRecordDlg::saveAndNew()
{
	if ( save() ) {
		d->insertRow(d->m_selectionModel);
	}
}
