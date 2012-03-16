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
#include "dbfiltertableview.h"
#include "ui_dbfiltertableview.h"
#include "configuracion.h"
#include "dao/beans/basebean.h"
#include "dao/beans/basebeanmetadata.h"
#include "dao/beans/beansfactory.h"
#include "dao/beans/dbfield.h"
#include "dao/beans/dbfieldmetadata.h"
#include "dao/beans/dbrelation.h"
#include "dao/beans/dbrelationmetadata.h"
#include "models/dbbasebeanmodel.h"
#include "models/filterbasebeanmodel.h"
#include "widgets/dbtableviewcolumnorderform.h"
#include "widgets/dbnumberedit.h"
#include <QMessageBox>
#include <QDate>
#include <QDebug>

#define MSG_NO_COLUMN_SELECCIONADA QT_TR_NOOP("Debe seleccionar una columna de filtrado.")
#define CB_OPERATOR_EQUAL		0
#define CB_OPERATOR_BETWEEN		1
#define CB_OPERATOR_MORE		2
#define CB_OPERATOR_LESS		3
#define CB_OPERATOR_DISTINCT	4

#define LAST_FAST_FILTER_TYPE_EMPTY		0
#define LAST_FAST_FILTER_TYPE_STRONG	1
#define LAST_FAST_FILTER_TYPE_PROXY		2
#define FILTER_HISTORY	"filterHistory"

class DBFilterTableViewPrivate
{
	Q_DECLARE_PUBLIC(DBFilterTableView)
public:
	DBBaseBeanModel *m_model;
	FilterBaseBeanModel *m_modelFilter;
	/** Establece si el formulario presenta datos filtrados o no */
	bool m_filtrando;
	/** Indica la clausula where con la que se esta filtrando el modelo */
	QString m_whereFilter;
	/** Tabla principal, de la que se devolverán BEANS. Pero no tiene porqué ser la
		tabla que se visualiza. Si esta table name tiene viewOnGrid a otra visualización,
		esa es la que se obtendrá y visualizará, aunque se editen los contenidos de tableName. */
	QString m_tableName;
	/** Campo de filtro dentro de la propia tabla, para el filtrado fuerte */
	QString m_filterField;
    /** Indica si se crea filtro fuerte o no... Se utiliza porque puede ocurrir que se llame
      a destroyStrongFilter antes que a createStrongFilter, y esta variable, evitaría esa creación */
	QStringList m_removedStrongFilter;
	/** Indica si en apertura, el número de registros es un número alto */
	bool m_largeResultSets;
	/** Metadatos que se muestran: OJO, si es una vista la que se muestra, son los metadatos de la vista
	  aunque posteriormente se edite el registro de la tabla */
	BaseBeanMetadata * m_metadata;

	DBFilterTableView *q_ptr;

	DBFilterTableViewPrivate(DBFilterTableView *qq) : q_ptr(qq) {
		m_filtrando = false;
		m_model = NULL;
		m_modelFilter = NULL;
		m_largeResultSets = false;
		m_metadata = NULL;
	}

	QString initSort();
	QString initOrderedColumn();
	QString initOrderedColumnSort();
	void createStrongFilter();
	void destroyStrongFilter(const QString &dbFieldName);
	bool filterTypeToApply(QObject *obj, const QString &textFilter, int rowCount, int filterCount);
};

DBFilterTableView::DBFilterTableView(QWidget *parent) : QWidget(parent), ui(new Ui::DBFilterTableView), d(new DBFilterTableViewPrivate(this))
{
    ui->setupUi(this);
	ui->tvListView->setAutomaticName(false);
	ui->cbFastFilterValue->setVisible(false);
	ui->dateEdit1->setSpecialValueText(trUtf8("Seleccione fecha"));
	ui->dateEdit2->setSpecialValueText(trUtf8("Seleccione fecha"));
	ui->dateEdit1->setDisplayFormat("dd/MM/yyyy");
	ui->dateEdit2->setDisplayFormat("dd/MM/yyyy");
	ui->dateEdit1->setDate(configuracion.minimunDate());
    ui->dateEdit2->setDate(QDate::currentDate());
	ui->txtFastFilter->installEventFilter(this);
	ui->cbFastFilterValue->installEventFilter(this);
	ui->dateEdit1->installEventFilter(this);
	ui->dateEdit2->installEventFilter(this);

	connect (ui->txtFastFilter, SIGNAL(textChanged(const QString &)), this, SLOT(fastFilterByText()));
	connect (ui->cbFastFilter, SIGNAL(currentIndexChanged(int)), this, SLOT(changedComboField(int)));
	connect (ui->cbFastFilterValue, SIGNAL(currentIndexChanged(int)), this, SLOT(fastFilterByCombo(int)));
	connect (ui->dateEdit1, SIGNAL(dateChanged(QDate)), this, SLOT(fastFilterByDate()));
	connect (ui->dateEdit2, SIGNAL(dateChanged(QDate)), this, SLOT(fastFilterByDate()));
	connect (ui->cbFastFilterValue, SIGNAL(currentIndexChanged(int)), this, SLOT(fastFilterByCombo(int)));
	connect (ui->pbSort, SIGNAL(clicked()), this, SLOT(sortForm()));
	connect (ui->cbOperators, SIGNAL(currentIndexChanged(int)), this, SLOT(prepareFilterControlsByOperator()));
	connect (ui->dbFastFilter, SIGNAL(textChanged(const QString &)), this, SLOT(fastFilterByNumbers()));
	connect (ui->dbFastFilter2, SIGNAL(textChanged(const QString &)), this, SLOT(fastFilterByNumbers()));

	ui->tvListView->setExternalModel(true);
}

DBFilterTableView::~DBFilterTableView()
{
    delete ui;
	delete d;
}

QString DBFilterTableView::tableName()
{
	return d->m_tableName;
}

FilterBaseBeanModel *DBFilterTableView::model()
{
	return d->m_modelFilter;
}

QItemSelectionModel * DBFilterTableView::selectionModel()
{
	return ui->tvListView->selectionModel();
}

DBTableView *DBFilterTableView::dbTableView()
{
	return ui->tvListView;
}

void DBFilterTableView::setTableName (const QString &tableName)
{
	d->m_tableName = tableName;
	ui->tvListView->setTableName(tableName);
	QString objName = QString("dbTableView_dbFilterTableView_%1").arg(tableName);
	ui->tvListView->setObjectName(objName);
	d->m_metadata = BeansFactory::metadataBean(tableName);
	if ( d->m_metadata == NULL ) {
		QMessageBox::warning(this, trUtf8(APP_NAME), trUtf8("Su aplicación no está bien configurada. Existen tablas de sistemas no creadas. Consulte con Aleph Sistemas de Información."), QMessageBox::Ok);
		return;
	}
	if ( !d->m_metadata->viewOnGrid().isEmpty() ) {
		d->m_metadata = BeansFactory::metadataBean(d->m_metadata->viewOnGrid());
		if ( d->m_metadata == NULL ) {
			QMessageBox::warning(this, trUtf8(APP_NAME), trUtf8("Su aplicación no está bien configurada. Existen tablas de sistemas no creadas. Consulte con Aleph Sistemas de Información."), QMessageBox::Ok);
			return;
		}
	}
}

/*!
  Crea la SQL de ordenación inicial. Ésta depende de lo que el usuario hubiese configurado
  en la ventana de orden de las columnas.
  */
QString DBFilterTableViewPrivate::initSort()
{
	QString sort;
	QStringList order = configuracion.tableViewColumnOrder(q_ptr->ui->tvListView);
	QStringList orderSort = configuracion.tableViewColumnSort(q_ptr->ui->tvListView);

	if ( m_metadata == NULL ) {
		return QString();
	}

	// El orden inicial es el de la primary key, o el de la primera columna
	// que el usuario tenga en su orden. Aquí construimos el orden inicial del control
	// para que no haya demasiada interacción con la base de datos.
	if ( order.size() > 0 ) {
		for ( int i = 0 ; i < order.size() ; i++ ) {
			DBFieldMetadata *fld = m_metadata->field(order.at(i));
			if ( fld != NULL && !fld->calculated() ) {
				QString s;
				if ( orderSort.size() > i ) {
					s = orderSort.at(i);
				} else {
					s = "DESC";
				}
				if ( sort.isEmpty() ) {
					sort = QString("%1 %2").arg(order.at(i)).arg(s);
				} else {
					sort = QString("%1, %2 %3").arg(sort).arg(order.at(i)).arg(s);
				}
			}
		}
	} else {
		if ( m_metadata->initialOrderSort().isEmpty() ) {
			QList<DBFieldMetadata *> fields = m_metadata->fields();
			foreach ( DBFieldMetadata *fld, fields ) {
				if ( fld->visibleGrid() && (!fld->calculated() || (fld->calculated() && fld->calculatedSaveOnDb())) ) {
					sort = QString("%1 DESC").arg(fld->dbFieldName());
					break;
				}
			}
		} else {
			sort = m_metadata->initialOrderSort();
		}
	}
	return sort;
}

/*!
  Devuelve la columna que inicialmente tendrá la marca de ordenación. Esta función
  se utiliza para que la primera SQL que se lance esté ya preparada por orden y sea
  más eficiente
  */
QString DBFilterTableViewPrivate::initOrderedColumn()
{
	QStringList order = configuracion.tableViewColumnOrder(q_ptr->ui->tvListView);
	if ( m_metadata == NULL ) {
		return QString();
	}

	if ( order.size() > 0 ) {
		return order.at(0);
	} else {
		QList<DBFieldMetadata *> fields = m_metadata->fields();
		foreach ( DBFieldMetadata *fld, fields ) {
			if ( fld->visibleGrid() ) {
				return fld->dbFieldName();
			}
		}
	}
	return "";
}

/*!
  Devuelve el orden que inicialmente tendrá la columna primera. Esta función
  se utiliza para que la primera SQL que se lance esté ya preparada por orden y sea
  más eficiente
  */
QString DBFilterTableViewPrivate::initOrderedColumnSort()
{
	QStringList order = configuracion.tableViewColumnSort(q_ptr->ui->tvListView);
	if ( m_metadata == NULL ) {
		return "ASC";
	}
	if ( order.size() > 0 ) {
		return order.at(0);
	}
	return "DESC";
}

/*!
  Realiza todas las operaciones necesarias de inicio del control
  */
void DBFilterTableView::init(bool createStrongFilter)
{
	QString sort = d->initSort();

	if ( createStrongFilter ) {
		d->createStrongFilter();
	}
	buildFilterWhere();
	d->m_model = new DBBaseBeanModel(d->m_tableName, d->m_whereFilter, sort, false, this);
	d->m_model->setDeleteFromDB(true);
	d->m_modelFilter = new FilterBaseBeanModel(this);
	d->m_modelFilter->setSourceModel (d->m_model);
    d->m_modelFilter->setFilterCaseSensitivity( Qt::CaseInsensitive );
	/** Este tipo de controles que tiran directamente de base de datos, visualizan inicialmente
	  todos los registros para una carga rápida */
	d->m_modelFilter->setDbStates(BaseBean::INSERT | BaseBean::UPDATE | BaseBean::TO_BE_DELETED);
	if ( d->m_model->rowCount() > configuracion.strongFilterRowCountLimit() ) {
		d->m_largeResultSets = true;
	} else {
		d->m_largeResultSets = false;
	}
	// Debe hacerse en este orden para que no se produzcan retrasos de ordenación.
	// Es importante indicar la columna por la que se ordene y que está ordenado
	// antes de introducir el modelo. El modelo aseguramos que viene en ese orden.
	// Lo que se garantiza con esto es que de entrada, FilterBaseBeanModel no ordene
	// y la ordenación la haga la base de datos.
	QList<DBFieldMetadata *> list = d->m_modelFilter->visibleFields();
	QString initOrderColumn = d->initOrderedColumn();
	QString initOrderColumnSort = d->initOrderedColumnSort();
	for ( int i = 0 ; i < list.size() ; i++ ) {
		if ( list.at(i)->dbFieldName() == initOrderColumn ) {
			ui->tvListView->QTableView::sortByColumn(i, initOrderColumnSort == "DESC" ? Qt::DescendingOrder : Qt::AscendingOrder);
			ui->tvListView->setSortingEnabled(true);
			ui->tvListView->horizontalHeader()->setSortIndicatorShown(true);
			break;
		}
	}

	ui->tvListView->setModel(d->m_modelFilter);
	addFieldsCombo();

	QStringList columnOrder = configuracion.tableViewColumnOrder(ui->tvListView);
	int index = ui->cbFastFilter->currentIndex();
	if ( !columnOrder.isEmpty() ) {
		index = ui->cbFastFilter->findData(columnOrder.at(0));
		if ( index == -1 ) {
			index = ui->cbFastFilter->currentIndex();
		}
	}
	ui->cbFastFilter->blockSignals(true);
	ui->cbFastFilter->setCurrentIndex(index);
	prepareFilterControls();
	ui->cbFastFilter->blockSignals(false);
}

/*!
  Se podrán realizar filtros fuertes por cada formulario, para evitar un trasiego importante de base de datos.
  Esta función creará combobox que permitirán filtrar. Esos combobox presentarán los option list de un field
  o los datos de una columna
  */
void DBFilterTableViewPrivate::createStrongFilter()
{
	if ( m_metadata == NULL ) {
		return;
	}
	QStringList filters = m_metadata->itemsFilterColumn();
	int i = 0;

	foreach ( QString filter, filters ) {
		if ( !m_removedStrongFilter.contains(filter) ) {
			QStringList parts = filter.split(";");
			if ( parts.size() >= 1 ) {
				QString fieldToFilter = parts.at(0);
				QString relationFieldToShow = (parts.size() > 2 ? parts.at(1) : "");
				QString order = (parts.size() >= 3 ? parts.at(2) : "");
				// Obtenemos el campo por el que se realizará el filtro fuerte
				DBFieldMetadata *fld = m_metadata->field(fieldToFilter);
				if ( fld != NULL ) {
					QComboBox *cb = new QComboBox(q_ptr);
					QLabel *lbl = new QLabel(q_ptr);
					QHBoxLayout *lay = qobject_cast<QHBoxLayout *>(q_ptr->ui->gbFilter->layout());
					cb->setObjectName(QString("cbStrongFilter%1").arg(fld->dbFieldName()));
					lbl->setObjectName(QString("lblStrongFilter%1").arg(fld->dbFieldName()));
					lay->insertWidget(i*2, lbl);
					lay->insertWidget(i*2 + 1, cb);
					i++;
					lbl->setText(fld->fieldName());
					if ( fld->type() == QVariant::Bool ) {
						QString filterItem = fld->sqlWhere("=", true);
						cb->addItem(QIcon(":/aplicacion/images/ok.png"), QObject::trUtf8("Verdadero"), filterItem);
						filterItem = fld->sqlWhere("=", false);
						cb->addItem(QIcon(":/generales/images/delete.png"), QObject::trUtf8("Falso"), filterItem);
					} else {
						// Agregamos las opciones de este campo al combobox
						if ( fld->optionsList().isEmpty() ) {
							QList<DBRelationMetadata *> rels = fld->relations();
							DBRelationMetadata *rel = NULL;
							foreach (DBRelationMetadata *relation, rels) {
								if ( relation->type() == DBRelationMetadata::MANY_TO_ONE ) {
									rel = relation;
								}
							}
							if ( rel != NULL ) {
								BaseBeanPointerList list;
								if ( BaseDAO::select(list, rel->tableName(), "", order) ) {
									// Añadimos los hijos de la relación al combo
									foreach ( QSharedPointer<BaseBean> child, list ) {
										QString where = QString("%1=%2").arg(fieldToFilter).arg(child->sqlFieldValue(rel->childFieldName()));
										cb->addItem(child->displayFieldValue(relationFieldToShow), where);
									}
								}
							}
						} else {
							QMap<QString, QString> optionList = fld->optionsList();
							QMap<QString, QString> optionIcons = fld->optionsIcons();
							QMapIterator<QString, QString> i(optionList);
							while ( i.hasNext() ) {
								i.next();
								QString filterItem = fld->sqlWhere("=", i.key());
								if ( optionIcons.contains(i.key()) ) {
									cb->addItem(QIcon(optionIcons.value(i.key())), i.value().trimmed(), filterItem);
								} else {
									cb->addItem(i.value().trimmed(), filterItem);
								}
							}
						}
					}
					cb->addItem(QObject::trUtf8("Ver todos"), "");
					QString key = QString("%1%2").arg(m_tableName).arg(cb->objectName());
					QVariant v = configuracion.loadRegistryValue(key);
					int index = cb->findData(v);
					if ( index != -1 ) {
						cb->setCurrentIndex(index);
					} else {
						if ( cb->count() > 0 ) {
							cb->setCurrentIndex(0);
						}
					}
					// Lo dotamos de funcionalidad
					q_ptr->connect (cb, SIGNAL(currentIndexChanged(int)), q_ptr, SLOT(filter()));
				}
			}
		}
	}
}

/*!
  Destruye los combobox que hubiese de filtros fuertes
  */
void DBFilterTableViewPrivate::destroyStrongFilter(const QString &dbFieldName)
{
	if ( m_metadata == NULL ) {
		return;
	}
	if ( dbFieldName == "" ) {
		QList<QComboBox*> listCb = q_ptr->findChildren<QComboBox*>(QRegExp("cbStrongFilter.+"));
		QList<QLabel*> listLbl = q_ptr->findChildren<QLabel*>(QRegExp("lblStrongFilter.+"));
		qDeleteAll(listCb);
		qDeleteAll(listLbl);
		m_removedStrongFilter = m_metadata->itemsFilterColumn();
	} else {
		QComboBox *cb = q_ptr->findChild<QComboBox *>(QString("cbStrongFilter%1").arg(dbFieldName));
		QLabel *listLb1 = q_ptr->findChild<QLabel *>(QString("lblStrongFilter%1").arg(dbFieldName));
		if ( cb != NULL ) {
			delete cb;
		}
		if ( listLb1 != NULL ) {
			delete listLb1;
		}
		foreach (QString filter, m_metadata->itemsFilterColumn()) {
			if (filter.contains(QString("%1;").arg(dbFieldName))) {
				m_removedStrongFilter.append(filter);
				return;
			}
		}
	}
}

void DBFilterTableView::destroyStrongFilter(const QString &dbFieldName)
{
	d->destroyStrongFilter(dbFieldName);
}

void DBFilterTableView::createStrongFilter()
{
	d->createStrongFilter();
}

/*!
  Construye la claúsula where de los modelos a partir del valor seleccionado en los combos de filtros
  fuertes. Adicionalmente se le podrán pasar otros datos que pueden provenir de aplicar como filtro
  fuerte datos típicos de filtros leves
  */
void DBFilterTableView::buildFilterWhere(const QString &aditionalSql)
{
	QList<QComboBox*> list = this->findChildren<QComboBox*>(QRegExp("cbStrongFilter.+"));
	d->m_whereFilter = "";
	foreach ( QComboBox *cb, list ) {
		if ( cb->currentIndex() != -1 ) {
			QString filter = cb->itemData(cb->currentIndex()).toString();
			if ( !filter.isEmpty() ) {
				if ( d->m_whereFilter.isEmpty() ) {
					d->m_whereFilter = filter;
				} else {
					d->m_whereFilter = QString("%1 AND %2").arg(d->m_whereFilter).arg(filter);
				}
			}
		}
	}
	if ( !aditionalSql.isEmpty() ) {
		if ( d->m_whereFilter.isEmpty() ) {
			d->m_whereFilter = aditionalSql;
		} else {
			d->m_whereFilter = QString("%1 AND %2").arg(d->m_whereFilter).arg(aditionalSql);
		}
	}
}

/*!
  Añade al combo todos las columnas visibles de la tabla, para el filtrado rápido.
  No se añaden aquellos campos que pudiesen estar en filtrado fuerte
  */
void DBFilterTableView::addFieldsCombo()
{
	QList<DBFieldMetadata *> fields = d->m_metadata->fields();
	ui->cbFastFilter->blockSignals(true);
	ui->cbFastFilter->clear();
	foreach ( DBFieldMetadata *fld, fields ) {
		bool visibleStrongFilter = false;
		QStringList itemFilterColumn = d->m_metadata->itemsFilterColumn();
		foreach ( QString item, itemFilterColumn ) {
			QStringList temp = item.split(";");
			if ( temp.size() > 0 && temp.at(0) == fld->dbFieldName() ) {
				visibleStrongFilter = true;
			}
		}
		if ( !visibleStrongFilter && fld->visibleGrid() ) {
			ui->cbFastFilter->addItem(fld->fieldName(), fld->dbFieldName());
		}
	}
	ui->cbFastFilter->blockSignals(false);
}

/*!
  Para campos con lista de valores, añade las posibles opciones
  */
void DBFilterTableView::addOptionsCombo(DBFieldMetadata *fld)
{
	ui->cbFastFilterValue->blockSignals(false);
	ui->cbFastFilterValue->clear();
	ui->cbFastFilterValue->addItem(trUtf8("Ver todos"), QString(""));
	QMapIterator<QString, QString> optionList(fld->optionsList());
	while ( optionList.hasNext() ) {
		optionList.next();
		ui->cbFastFilterValue->addItem(optionList.key(), optionList.value());
	}
	ui->cbFastFilterValue->blockSignals(false);
}

bool DBFilterTableView::event(QEvent *e)
{
	if ( e->type() == QEvent::Close ) {
		closeEvent(dynamic_cast<QCloseEvent*>(e));
	}
	return QWidget::event(e);
}

void DBFilterTableView::closeEvent(QCloseEvent * event)
{
	QList<QComboBox *> list = findChildren<QComboBox *>(QRegExp("cbStrongFilter.+"));
	foreach ( QComboBox *cb, list ) {
		if ( cb->currentIndex() != -1 ) {
			QString key = QString("%1%2").arg(d->m_tableName).arg(cb->objectName());
			configuracion.saveRegistryValue(key, cb->itemData(cb->currentIndex()));
		}
	}
	event->accept();
}

/*!
  Se irá guardando, asociado al control, el listado de búsquedas realizados, para así, cuando el usuario
  pinche en Backspace sepamos ya qué tipo de búsqueda se hizo (y no haga falta deshacer el filtro para contar
  el número de registros total del modelo y obtener el rowCount de verdad)...
  */
bool DBFilterTableViewPrivate::filterTypeToApply(QObject *obj, const QString &textFilter, int rowCount, int filterCount)
{
	bool strongFilter = true;
	QMap<QString, QVariant> filterInformation;
	if ( rowCount < configuracion.strongFilterRowCountLimit() ) {
		return false;
	}
	if ( filterCount == 0 ) {
		return true;
	}
	if ( obj->property(FILTER_HISTORY).isValid() ) {
		filterInformation = obj->property(FILTER_HISTORY).toMap();
	} else {
		filterInformation[""] = LAST_FAST_FILTER_TYPE_STRONG;
	}
	if ( filterInformation.contains(textFilter) ) {
		if ( filterInformation.value(textFilter) == LAST_FAST_FILTER_TYPE_STRONG ) {
			strongFilter = true;
		} else if ( filterInformation.value(textFilter) == LAST_FAST_FILTER_TYPE_PROXY ) {
			strongFilter = false;
		}
	} else {
		if ( rowCount > configuracion.strongFilterRowCountLimit() ) {
			filterInformation[textFilter] = LAST_FAST_FILTER_TYPE_STRONG;
			strongFilter = true;
		} else {
			filterInformation[textFilter] = LAST_FAST_FILTER_TYPE_PROXY;
			strongFilter = false;
		}
	}
	obj->setProperty(FILTER_HISTORY, filterInformation);
	return strongFilter;
}

/*!
	Slot que realiza un filtrado rápido de los registros presentados, en función de lo que el
	usuario haya introducido en los campos. Este filtrado se realiza a partir del objeto Filter definido.
	Pero si el número de registros del modelo es muy grande, se realiza un filtrado fuerte
*/
void DBFilterTableView::fastFilterByText()
{
	QHash<QString, QPair<QString, QVariant> > filter = d->m_modelFilter->filter();
	QString textFilter = ui->txtFastFilter->text();

	/* Si no hay filtro establecido no se hace nada */
	if ( filter.isEmpty() && textFilter.isEmpty() && !ui->txtFastFilter->property(FILTER_HISTORY).isValid() ) {
		return;
	}

	if ( ui->cbFastFilter->currentIndex() == -1 ) {
		QMessageBox::warning(this, trUtf8(APP_NAME),
							 trUtf8(MSG_NO_COLUMN_SELECCIONADA), QMessageBox::Ok);
	} else {
		int rowCount = d->m_model->rowCount();
		bool strongFilter = d->filterTypeToApply(ui->txtFastFilter, textFilter, rowCount, filter.count());
		QString dbFieldName = ui->cbFastFilter->itemData(ui->cbFastFilter->currentIndex()).toString();
		if ( strongFilter ) {
			d->m_modelFilter->removeFilterKeyColumn(dbFieldName);
			filterWithFastUserFilter();
		} else {
			d->m_modelFilter->setFilterKeyColumn( dbFieldName, ui->txtFastFilter->text() );
			d->m_modelFilter->invalidate();
		}
	}
}

/*!
	Slot que realiza un filtrado rápido de los registros presentados, en función de lo que el
	usuario haya introducido en los campos. Este filtrado se realiza a partir del objeto Filter definido.
	Pero si el número de registros del modelo es muy grande, se realiza un filtrado fuerte
*/
void DBFilterTableView::fastFilterByNumbers()
{
	QHash<QString, QPair<QString, QVariant> > filter = d->m_modelFilter->filter();
	QString textFilter = ui->dbFastFilter->text();
	QString textFilter2 = ui->dbFastFilter2->text();
	QLineEdit *le = qobject_cast<QLineEdit *>(QObject::sender());
	if ( le == NULL ) return;
	QString modifiedText = le->text();

	/* Si no hay filtro establecido no se hace nada */
	if ( filter.isEmpty() && textFilter.isEmpty() && !le->property(FILTER_HISTORY).isValid() ) {
		return;
	}
	if ( ui->cbOperators->currentIndex() == CB_OPERATOR_BETWEEN && (textFilter.isEmpty() || textFilter2.isEmpty()) ) {
		return;
	}
	if ( ui->cbFastFilter->currentIndex() == -1 ) {
		QMessageBox::warning(this, trUtf8(APP_NAME),
							 trUtf8(MSG_NO_COLUMN_SELECCIONADA), QMessageBox::Ok);
	} else {
		int rowCount = d->m_model->rowCount();
		bool strongFilter = d->filterTypeToApply(QObject::sender(), modifiedText, rowCount, filter.count());
		QString dbFieldName = ui->cbFastFilter->itemData(ui->cbFastFilter->currentIndex()).toString();
		if ( strongFilter || ui->cbOperators->currentIndex() == CB_OPERATOR_BETWEEN ) {
			d->m_modelFilter->removeFilterKeyColumn(dbFieldName);
			filterWithFastUserFilter();
		} else {
			double v1 = configuracion.getLocale()->toDouble(ui->dbFastFilter->text());
			double v2 = configuracion.getLocale()->toDouble(ui->dbFastFilter2->text());
			if ( ui->cbOperators->currentIndex() != CB_OPERATOR_BETWEEN && !textFilter.isEmpty() && !textFilter2.isEmpty() ) {
				d->m_modelFilter->setFilterKeyColumnBetween(dbFieldName, v1, v2);
			} else {
				QString op;
				if ( ui->cbOperators->currentIndex() == CB_OPERATOR_EQUAL ) {
					op = "=";
				} else if ( ui->cbOperators->currentIndex() == CB_OPERATOR_LESS ) {
					op = "<";
				} else if ( ui->cbOperators->currentIndex() == CB_OPERATOR_MORE ) {
					op = ">";
				} else if ( ui->cbOperators->currentIndex() == CB_OPERATOR_DISTINCT ) {
					op = "!=";
				}
				d->m_modelFilter->setFilterKeyColumn( dbFieldName, v1, op );
			}
			d->m_modelFilter->invalidate();
		}
	}
}

/*!
  Se filtra por el valor del combo
  */
void DBFilterTableView::fastFilterByCombo(int index)
{
	QVariant data = ui->cbFastFilterValue->itemData(index);
	if ( data.isValid() && ui->cbFastFilter->currentIndex() != -1 ) {
		QString dbFieldName = ui->cbFastFilter->itemData(ui->cbFastFilter->currentIndex()).toString();
		if ( d->m_largeResultSets ) {
			filterWithFastUserFilter();
			d->m_modelFilter->removeFilterKeyColumn(dbFieldName);
		} else {
			d->m_modelFilter->setFilterKeyColumn( dbFieldName, data );
			d->m_modelFilter->invalidate();
		}
	}
}

/*!
	Slot que realiza un filtrado rápido de los registros presentados, en función de lo que el
	usuario haya introducido en los campos de fecha. Este filtrado se realiza a partir del objeto Filter definido.
*/
void DBFilterTableView::fastFilterByDate()
{
	if ( ui->dateEdit1->date().isNull() || !ui->dateEdit1->date().isValid() ) {
		return;
	} else {
		QString dbFieldName = ui->cbFastFilter->itemData(ui->cbFastFilter->currentIndex()).toString();
		if ( d->m_largeResultSets ) {
			d->m_modelFilter->removeFilterKeyColumn(dbFieldName);
			filterWithFastUserFilter();
		} else {
			QDate date1 = ui->dateEdit1->date();
			QDate date2 = ui->dateEdit2->date();
			if ( date2.isNull() || !date2.isValid() ) {
				date2 = date1;
			}
			d->m_modelFilter->setFilterKeyColumnBetween( dbFieldName, date1, date2 );
			d->m_modelFilter->invalidate();
		}
	}
}

void DBFilterTableView::clearFilter()
{
	QString dbFieldName = ui->cbFastFilter->itemData(ui->cbFastFilter->currentIndex()).toString();
	d->m_modelFilter->removeFilterKeyColumn(dbFieldName);
	ui->dbFastFilter->clear();
	ui->dbFastFilter->setProperty(FILTER_HISTORY, QVariant());
	ui->dbFastFilter2->clear();
	ui->dbFastFilter2->setProperty(FILTER_HISTORY, QVariant());
	ui->txtFastFilter->clear();
	ui->txtFastFilter->setProperty(FILTER_HISTORY, QVariant());
}

/*!
  Este filtro es fuerte. Se realiza con un where en base de datos
  */
void DBFilterTableView::filter()
{
	QComboBox *cb = qobject_cast<QComboBox *>(QObject::sender());
	QString key = QString("%1%2").arg(d->m_tableName).arg(cb->objectName());
	configuracion.saveRegistryValue(key, cb->itemData(cb->currentIndex()));

	QString oldWhere = d->m_whereFilter;
	buildFilterWhere();
	if ( oldWhere != d->m_whereFilter ) {
		QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
		d->m_model->setWhere(d->m_whereFilter);
		d->m_modelFilter->invalidate();
		QApplication::restoreOverrideCursor();
	}
}

/*!
  Realiza un filtro fuerte sobre el modelo teniendo en cuenta, no sólo los filtros fuertes
  que hubiese, si no además la información que el usuario ha metido ya que se supone
  que la búsqueda sería muy pesada
  */
void DBFilterTableView::filterWithFastUserFilter()
{
	QString aditionalSql;
	DBFieldMetadata *fld = dbFieldSelectedOnCombo();
	if ( fld != NULL ) {
		if ( !fld->optionsList().isEmpty() ) {
			QString val = ui->cbFastFilterValue->itemData(ui->cbFastFilterValue->currentIndex()).toString();
			if ( !val.isEmpty() ) {
				aditionalSql = fld->sqlWhere("=", val);
			}
		} else {
			if ( fld->type() == QVariant::String ) {
				if ( !ui->txtFastFilter->text().isEmpty() ) {
					aditionalSql = QString("UPPER(%1) LIKE UPPER('\%%2\%')").arg(fld->dbFieldName()).arg(ui->txtFastFilter->text());
				}
			} else if ( fld->type() == QVariant::Date ) {
				aditionalSql = QString("%1 BETWEEN %2 AND %3").arg(fld->dbFieldName()).
						arg(fld->sqlValue(ui->dateEdit1->date())).arg(fld->sqlValue(ui->dateEdit2->date()));
			} else if ( fld->type() == QVariant::Int || fld->type() == QVariant::Double ) {
				double v1 = configuracion.getLocale()->toDouble(ui->dbFastFilter->text());
				double v2 = configuracion.getLocale()->toDouble(ui->dbFastFilter2->text());
				if ( !(v1 == 0 && v2 == 0) ) {
					if ( ui->cbOperators->currentIndex() == CB_OPERATOR_EQUAL ) {
						aditionalSql = fld->sqlWhere("=", v1);
					} else if ( ui->cbOperators->currentIndex() == CB_OPERATOR_BETWEEN ) {
						aditionalSql = QString("%1 BETWEEN %2 AND %3").arg(fld->dbFieldName()).
								arg(fld->sqlValue(v1)).arg(fld->sqlValue(v2));
					} else if ( ui->cbOperators->currentIndex() == CB_OPERATOR_LESS ) {
						aditionalSql = fld->sqlWhere("<", v1);
					} else if ( ui->cbOperators->currentIndex() == CB_OPERATOR_MORE ) {
						aditionalSql = fld->sqlWhere(">", v1);
					} else if ( ui->cbOperators->currentIndex() == CB_OPERATOR_DISTINCT ) {
						aditionalSql = fld->sqlWhere("<>", v1);
					}
				}
			}
		}
	}
	buildFilterWhere(aditionalSql);
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	d->m_model->setWhere(d->m_whereFilter);
	d->m_modelFilter->invalidate();
	QApplication::restoreOverrideCursor();
}

/*!
	Cuando el combo de filtro rápido cambia, se actualizan los controles que se presenta en el toolbar, se aplican
	las máscaras adecuados y se reordena el table view poniendo como primera columna la seleccionada
	por el usuario y ordenando por esa columna...
 */
void DBFilterTableView::changedComboField(int index)
{
	if ( index == -1 ) {
		return;
	}
	int modelIndex = -1;
	QList<DBFieldMetadata *> flds = d->m_modelFilter->visibleFields();
	foreach (DBFieldMetadata *fld, flds) {
		modelIndex++;
		if ( fld->dbFieldName() == ui->cbFastFilter->itemData(index).toString() ) {
			break;
		}
	}
	int visibleIndex = ui->tvListView->horizontalHeader()->visualIndex(modelIndex);
	ui->tvListView->horizontalHeader()->moveSection(visibleIndex, 0);
	clearFilter();
	d->m_modelFilter->resetFilter();
	filterWithFastUserFilter();
	prepareFilterControls();
	ui->tvListView->QTableView::sortByColumn(modelIndex, Qt::AscendingOrder);
	ui->tvListView->horizontalHeader()->setSortIndicator(modelIndex, Qt::AscendingOrder);
}

/*!
  Devuelve los metadatos del field seleccionado en el combo de filtrado
  */
DBFieldMetadata * DBFilterTableView::dbFieldSelectedOnCombo() {
	QString dbFieldName = ui->cbFastFilter->itemData(ui->cbFastFilter->currentIndex()).toString();
	DBFieldMetadata *fld = d->m_metadata->field(dbFieldName);
	return fld;
}

/*!
  Prepara los controles que se utilizarán en el filtro
  */
void DBFilterTableView::prepareFilterControls()
{
	if ( d->m_model == NULL ) {
		return;
	}
	DBFieldMetadata *fld = dbFieldSelectedOnCombo();

	if ( fld != NULL ) {
		if ( !fld->optionsList().isEmpty() ) {
			addOptionsCombo(fld);
			ui->cbFastFilterValue->setVisible(true);
            ui->dateEdit1->setVisible(false);
            ui->dateEdit2->setVisible(false);
            ui->lblDate1->setVisible(false);
            ui->lblDate2->setVisible(false);
            ui->txtFastFilter->setVisible(false);
			ui->lblBetween->setVisible(false);
			ui->lblAnd->setVisible(false);
			ui->cbOperators->setVisible(false);
			ui->dbFastFilter->setVisible(false);
			ui->dbFastFilter2->setVisible(false);
			fastFilterByCombo(ui->cbFastFilterValue->currentIndex());
        } else {
			ui->cbFastFilterValue->setVisible(false);
			if ( fld->type() == QVariant::Date ) {
				ui->dateEdit1->setVisible(true);
				ui->dateEdit2->setVisible(true);
				ui->lblDate1->setVisible(true);
				ui->lblDate2->setVisible(true);
				ui->txtFastFilter->setVisible(false);
				ui->lblBetween->setVisible(false);
				ui->lblAnd->setVisible(false);
				ui->cbOperators->setVisible(false);
				ui->dbFastFilter->setVisible(false);
				ui->dbFastFilter2->setVisible(false);
				fastFilterByDate();
			} else {
				ui->txtFastFilter->setInputMask("");
				ui->dateEdit1->setVisible(false);
				ui->dateEdit2->setVisible(false);
				ui->lblDate1->setVisible(false);
				ui->lblDate2->setVisible(false);
				if ( fld->type() == QVariant::String ) {
					ui->txtFastFilter->setVisible(true);
					ui->lblBetween->setVisible(false);
					ui->lblAnd->setVisible(false);
					ui->cbOperators->setVisible(false);
					ui->dbFastFilter->setVisible(false);
					ui->dbFastFilter2->setVisible(false);
				} else {
					ui->txtFastFilter->setVisible(false);
					ui->cbOperators->setVisible(true);
					ui->cbOperators->setCurrentIndex(0);
					prepareFilterControlsByOperator();
				}
                fastFilterByText();
			}
		}
	}
}

/*!
  Según la opción seleccionada en el combo de operadores (entre, mayor, menor, igual...)
  se visualizarán algunos controles
  */
void DBFilterTableView::prepareFilterControlsByOperator()
{
	clearFilter();
	if ( ui->cbOperators->currentText() == trUtf8("Entre") ) {
		ui->lblBetween->setVisible(true);
		ui->lblAnd->setVisible(true);
		ui->dbFastFilter2->setVisible(true);
	} else {
		ui->lblBetween->setVisible(false);
		ui->lblAnd->setVisible(false);
		ui->dbFastFilter2->setVisible(false);
	}
	ui->dbFastFilter->setVisible(true);
	filterWithFastUserFilter();
}

/*!
  Esta función devuelve el bean actualmente seleccionado en el modelo.
  */
QSharedPointer<BaseBean> DBFilterTableView::selectedBean() {
	QSharedPointer<BaseBean> bean;
	QModelIndex indice = d->m_modelFilter->mapToSource(ui->tvListView->currentIndex());

	if ( indice.isValid () ) {
		bean = d->m_model->bean(indice);
	}
	return bean;
}

void DBFilterTableView::setSelectedBean(const QSharedPointer<BaseBean> &bean)
{
	if ( bean.isNull() || d->m_model == NULL || d->m_modelFilter == NULL ) {
		return;
	}
	QModelIndex sourceIdx = d->m_model->indexByPk(bean->pkValue());
	if ( sourceIdx.isValid() ) {
		QModelIndex filterIdx = d->m_modelFilter->mapFromSource(sourceIdx);
		QModelIndex firstIdx = d->m_modelFilter->index(filterIdx.row(), 0);
		QModelIndex lastIdx = d->m_modelFilter->index(filterIdx.row(), bean->fieldCount() - 1);
		QItemSelection selection(firstIdx, lastIdx);
		ui->tvListView->selectionModel()->select(selection, QItemSelectionModel::Select);
		ui->tvListView->setCurrentIndex(filterIdx);
		ui->tvListView->scrollTo(filterIdx, QAbstractItemView::EnsureVisible);
	}
}

void DBFilterTableView::refresh()
{
	if ( d->m_model != NULL ) {
		d->m_model->refresh();
		d->m_modelFilter->invalidate();
	}
}

void DBFilterTableView::resizeRowsToContents ()
{
	ui->tvListView->resizeRowsToContents();
}

void DBFilterTableView::sortForm()
{
	QList<QPair<QString, QString> > result;
	DBTableViewColumnOrderForm *dlg = new DBTableViewColumnOrderForm(d->m_modelFilter, ui->tvListView->horizontalHeader(),
																	 &result, this);
	dlg->setModal(true);
	dlg->setAttribute(Qt::WA_DeleteOnClose);
	dlg->exec();
	if ( result.size() > 0 ) {
		QStringList order;
		QStringList orderSort;
		for ( int i = 0 ; i < result.size() ; i++ ) {
			order << result[i].first;
			orderSort << result[i].second;
		}
		ui->tvListView->orderColumns(order);
		ui->tvListView->saveTableViewColumnOrder(order, orderSort);
		delete d->m_modelFilter;
		delete d->m_model;
		init(false);
	}
}

/*!
  Tenemos que decirle al motor de scripts, que DBFormDlg se convierte de esta forma a un valor script
  */
QScriptValue DBFilterTableView::toScriptValue(QScriptEngine *engine, DBFilterTableView * const &in)
{
	return engine->newQObject(in);
}

void DBFilterTableView::fromScriptValue(const QScriptValue &object, DBFilterTableView * &out)
{
	out = qobject_cast<DBFilterTableView *>(object.toQObject());
}

void DBFilterTableView::stopReloadingModel()
{
	if ( d->m_model != NULL ) {
		d->m_model->stopReloading();
	}
}

void DBFilterTableView::startReloadingModel()
{
	if ( d->m_model != NULL ) {
		d->m_model->startReloading();
	}
}

/*!
  Mientras el usuario tenga el foco en los objetos de filtrado, no se actualiza dinámicamente
  el modelo. Así se evitan recargas innecesarias
  */
bool DBFilterTableView::eventFilter(QObject *obj, QEvent *ev)
{
	if ( ev->type() == QEvent::FocusIn ) {
		if ( obj->objectName() == ui->cbFastFilterValue->objectName() ||
			 obj->objectName() == ui->txtFastFilter->objectName() ||
			 obj->objectName() == ui->dateEdit1->objectName() ||
			 obj->objectName() == ui->dateEdit1->objectName() ||
			 obj->objectName() == ui->dbFastFilter->objectName() ||
			 obj->objectName() == ui->dbFastFilter2->objectName() ||
			 obj->objectName() == ui->cbFastFilter->objectName() ||
			 obj->objectName() == ui->cbOperators->objectName() ) {
			d->m_model->setStaticModel(true);
		}
	} else if ( ev->type() == QEvent::FocusOut ) {
		if ( obj->objectName() == ui->cbFastFilterValue->objectName() ||
			 obj->objectName() == ui->txtFastFilter->objectName() ||
			 obj->objectName() == ui->dateEdit1->objectName() ||
			 obj->objectName() == ui->dateEdit1->objectName() ||
			 obj->objectName() == ui->dbFastFilter->objectName() ||
			 obj->objectName() == ui->dbFastFilter2->objectName() ||
			 obj->objectName() == ui->cbFastFilter->objectName() ||
			 obj->objectName() == ui->cbOperators->objectName() ) {
			d->m_model->setStaticModel(false);
		}
	} else if ( ev->type() == QEvent::Close ) {
		if ( obj == this ) {
			return true;
		}
	}
	return QWidget::eventFilter(obj, ev);
}
