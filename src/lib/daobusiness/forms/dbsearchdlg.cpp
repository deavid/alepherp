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
#include "ui_dbsearchdlg.h"
#include "dbsearchdlg.h"
#include "configuracion.h"
#include "dao/basedao.h"
#include "dao/beans/basebean.h"
#include "dao/beans/basebeanmetadata.h"
#include "dao/beans/beansfactory.h"
#include "dao/beans/dbfieldmetadata.h"
#include "models/dbbasebeanmodel.h"
#include "models/filterbasebeanmodel.h"
#include "widgets/dbnumberedit.h"
#include "widgets/dblineedit.h"
#include "widgets/dbcombobox.h"
#include <QVBoxLayout>
#include <QScriptEngine>
#include <QDir>
#include <QUiLoader>
#include <QFile>
#include <QMessageBox>
#include <QLabel>
#include <QDate>
#ifndef QT_NO_SCRIPTTOOLS
 #include <QScriptEngineDebugger>
 #include <QMainWindow>
#endif

class DBSearchDlgPrivate
{
//	Q_DECLARE_PUBLIC(DBSearchDlg)
public:
	/** Bean maestro para conocer los datos a editar */
	BaseBeanMetadata *m_metadata;
	/** Bean en el que quedarán los datos que el usuario ha seleccionado en su búsqueda */
	QSharedPointer <BaseBean> m_bean;
	/** Modelo sobre el que se realizará la búsqueda */
	DBBaseBeanModel *m_model;
	/** Proxy que hace el filtro */
	FilterBaseBeanModel *m_filter;
	/** Filtro fuerte que establece los datos que puede ver este formulario */
	QString m_filterData;
	/** Interfaz leída de la base de datos */
	QWidget *m_widget;
	/** Para tratar con las selecciones */
	QItemSelectionModel *m_modelSelection;
	/** Indica si este diálogo puede devolver un filtro para aplicar en el formulario que lo ha llamado */
	bool m_canApplyFilter;
	/** Indica si el usuario realizó la búsqueda o pinchó el botón de salir sin buscar */
	bool m_userClickOk;

	DBSearchDlgPrivate() {}
};

DBSearchDlg::DBSearchDlg(const QString &tableName, QWidget *parent) :
	PERPBaseDialog(parent), ui(new Ui::DBSearchDlg), d(new DBSearchDlgPrivate)
{
	setTableName(tableName);
	if ( !checkPermissionsToOpen() ) {
		close();
		return;
	}
	d->m_metadata = BeansFactory::metadataBean(tableName);
	ui->setupUi(this);
	if ( d->m_metadata == NULL ) {
		qDebug() << "DBSearchDlg: No existe la tabla: [" << tableName << "]";
		close();
		setOpenSuccess(false);
		return;
	}
	setObjectName(QString("%1.search.ui").arg(tableName));
	d->m_userClickOk = false;
	setupWidget();
	setOpenSuccess(true);
}

DBSearchDlg::~DBSearchDlg()
{
	delete d;
	delete ui;
}

void DBSearchDlg::init()
{
	// Creemos los objetos de negocio
	d->m_model = new DBBaseBeanModel(tableName(), d->m_filterData, "", false, this);
	d->m_filter = new FilterBaseBeanModel(this);
	d->m_modelSelection = new QItemSelectionModel(d->m_filter, this);
	d->m_filter->setSourceModel(d->m_model);
	ui->tvResults->setModel(d->m_filter);
	ui->tvResults->setSelectionModel(d->m_modelSelection);

	setupQs();
	connect(ui->pbSalir, SIGNAL(clicked()), this, SLOT(close()));
	connect(ui->tvResults, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(select(QModelIndex)));
	connect(ui->pbOk, SIGNAL(clicked()), this, SLOT(select()));
	connectObjectsToSearch();
}

bool DBSearchDlg::canApplyFilter()
{
	return d->m_canApplyFilter;
}

bool DBSearchDlg::userClickOk()
{
	return d->m_userClickOk;
}

void DBSearchDlg::setFilterData(const QString &value)
{
	d->m_filterData = value;
}

void DBSearchDlg::setCanApplyfilter(bool value)
{
	d->m_canApplyFilter = value;
	ui->pbFiltrar->setVisible(d->m_canApplyFilter);
	ui->pbOk->setVisible(!d->m_canApplyFilter);
}

void DBSearchDlg::setupWidget()
{
	if ( setupExternalWidget() ) {
		return;
	}
	QLayout *layout = ui->gbSearchFields->layout();
	if ( layout != NULL ) {
		delete layout;
	}
	layout = new QVBoxLayout();
	setupWidgetFromBaseBeanMetadata(d->m_metadata, layout, true, true);
	ui->gbSearchFields->setLayout(layout);
}

bool DBSearchDlg::setupExternalWidget()
{
	QString fileName = QString("%1/%2.search.ui").
					   arg(QDir::fromNativeSeparators(configuracion.tempPath())).
						arg(tableName());
	QUiLoader uiLoader;
    QString pluginDir = QString("%1/plugins/designer").arg(qApp->applicationDirPath());
	QFile file (fileName);
	bool result;

	if ( file.exists() ) {
		uiLoader.addPluginPath(pluginDir);
		file.open( QFile::ReadOnly );
		d->m_widget = uiLoader.load(&file, 0);
		if ( d->m_widget != NULL ) {
			QLayout *layout = ui->gbSearchFields->layout();
			d->m_widget->setParent(this);
			layout->setContentsMargins(0, 0, 0, 0);
			layout->addWidget(d->m_widget);
			if ( ui->gbSearchFields->layout() != 0 ) {
				delete ui->gbSearchFields->layout();
			}
			ui->gbSearchFields->setLayout(layout);
			d->m_widget->show();
		} else {
			QMessageBox::warning(this, trUtf8(APP_NAME), trUtf8("No se ha podido cargar la interfaz de usuario de dbsearch de este formulario. Existe un problema en la definición de las tablas de sistema de su programa."),
								 QMessageBox::Ok);
			close();
		}
		result = true;
	} else {
		result = false;
	}
	file.close();
	return result;
}

void DBSearchDlg::setupQs()
{
	QString qsName = QString ("%1.search.qs").arg(tableName());
	QString uiName = QString("%1.search.ui").arg(tableName());
	/** Ejecutamos el script asociado. La filosofía fundamental de ese script es proporcionar
	  algo de código básico que justifique este formulario de búsqueda */
	if ( !BeansFactory::tableWidgetsScripts.contains(qsName) ||
		 !BeansFactory::tableWidgets.contains(uiName) ) {
		return;
	}

    engine()->setScriptName(qsName);
    engine()->setDebug(BeansFactory::tableWidgetsScriptsDebug.value(qsName));
    engine()->setOnInitDebug(BeansFactory::tableWidgetsScriptsDebugOnInit.value(qsName));
    engine()->setScriptObject("DBSearchDlg");
    engine()->setUi(d->m_widget);
    engine()->addAvailableObject("thisForm", this);
    engine()->addAvailableObject("perpModel", d->m_model);
    engine()->addAvailableObject("perpFilter", d->m_filter);
    if ( !engine()->createQsObject() ) {
		QMessageBox::warning(this, trUtf8(APP_NAME), trUtf8("Ha ocurrido un error al cargar el script asociado a este "
															"formulario. Es posible que algunas funciones no est\303\251n disponibles."),
							 QMessageBox::Ok);
	}
}

/*!
  Este slot será al que se conectará el script de Qt que controla este formulario, para introducir
  los datos de filtrado del usuario
  */
void DBSearchDlg::setColumnFilter (const QString &name, const QVariant &value, const QString &op)
{
	// Si el variant es un QList, entonces es un tipo primary key, y seguro que trae un conjunto
	// de pares columna:valor
	if ( value.canConvert<QMap<QString, QVariant> >() ) {
		d->m_filter->setFilterPkColumn(value);
	} else {
		d->m_filter->setFilterKeyColumn(name, value, op);
	}
	d->m_filter->invalidate();
}

/*!
  Este slot será al que se conectará el script de Qt que controla este formulario, para introducir
  los datos de filtrado del usuario. Esta función está pensada para intervalos de fecha
  */
void DBSearchDlg::setColumnBetweenFilter (const QString &name, const QVariant &value1, const QVariant &value2)
{
	d->m_filter->setFilterKeyColumnBetween(name, value1, value2);
	d->m_filter->invalidate();
}

/*!
  Responde al evento de DobleClick en el QTableView, selecciona el item, y cierra el formulario
  */
void DBSearchDlg::select (const QModelIndex &index)
{
	QModelIndex source = d->m_filter->mapToSource(index);
	if ( index.isValid() ) {
		d->m_bean = d->m_model->bean(source);
	} else {
		QModelIndex idx = d->m_modelSelection->currentIndex();
		if ( idx.isValid() ) {
			source = d->m_filter->mapToSource(idx);
			d->m_bean = d->m_model->bean(source);
		}
	}
	d->m_userClickOk = true;
	close();
}

/*!
  Devuelve el bean seleccionado en el QTableView
  */
QSharedPointer<BaseBean> DBSearchDlg::selectedBean()
{
	return d->m_bean;
}

/*!
  Conecta todos los objetos creados para la búsqueda con la propia búsqueda
  */
void DBSearchDlg::connectObjectsToSearch()
{
    QList<QWidget *> widgets = findChildren<QWidget *>();
    foreach ( QWidget *widget, widgets ) {
        QVariant property = widget->property("fieldName");
        if ( property.isValid() ) {
			QComboBox *cb = qobject_cast<QComboBox *> (widget);
			if ( cb != NULL ) {
				connect(widget, SIGNAL(currentIndexChanged(int)), this, SLOT(search()));
			} else {
				connect(widget, SIGNAL(valueEdited(QVariant)), this, SLOT(search()));

			}
        }
    }
}

/*!
  Selecciona de todos los controles, la lista de valores por los que filtrar...
  Este slot es el que realmente filtra.
  */
void DBSearchDlg::search()
{
    QList<QWidget *> widgets = findChildren<QWidget *>();
    foreach ( QWidget *widget, widgets ) {
        QVariant property = widget->property("fieldName");
        if ( property.isValid() ) {
			QComboBox *temp = qobject_cast<QComboBox *>(widget);
			if ( temp == NULL ) {
				QComboBox *cb = findChild<QComboBox *>(QString("cb_%1").arg(property.toString()));
				if ( cb != NULL ) {
					if ( cb->currentIndex() == BETWEEN_ELECTION_INDEX ) {
						if ( widget->objectName().contains("_1") ) {
							QWidget *widget2 = findChild<QWidget *>(QString("db_%1_2").arg(property.toString()));
							if ( widget2 != NULL ) {
								DBBaseWidget *wid1 = dynamic_cast<DBBaseWidget *> (widget);
								DBBaseWidget *wid2 = dynamic_cast<DBBaseWidget *> (widget2);
								d->m_filter->setFilterKeyColumnBetween(property.toString(), wid1->value(), wid2->value());
							}
						}
					} else {
						DBBaseWidget *wid = dynamic_cast<DBBaseWidget *> (widget);
						DBFieldMetadata *fld = d->m_metadata->field(property.toString());
						QVariant v = wid->value();
						if ( ! (fld->type() == QVariant::Date && v.toDate() == configuracion.minimunDate()) ) {
							if ( cb->currentIndex() == MINUS_ELECTION ) {
								d->m_filter->setFilterKeyColumn(property.toString(), wid->value(), "<");
							} else if ( cb->currentIndex() == EQUAL_ELECTION ) {
								d->m_filter->setFilterKeyColumn(property.toString(), wid->value(), "=");
							} else if ( cb->currentIndex() == MORE_ELECTION ) {
								d->m_filter->setFilterKeyColumn(property.toString(), wid->value(), ">");
							}
						}
					}
				} else {
					DBBaseWidget *wid = dynamic_cast<DBBaseWidget *> (widget);
					d->m_filter->setFilterKeyColumn(property.toString(), wid->value());
				}
			}
        }
    }
	d->m_filter->invalidate();
}
