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
#include "dbcombobox.h"
#include "models/dbbasebeanmodel.h"
#include "models/filterbasebeanmodel.h"
#include "dao/beans/dbfield.h"
#include "dao/beans/dbfieldmetadata.h"
#include "dao/beans/basebean.h"
#include "dao/beans/beansfactory.h"
#include "dao/beans/basebeanmetadata.h"
#include <QModelIndex>
#include <QEvent>
#include <QKeyEvent>
#include <QCompleter>
#include <QLineEdit>

class DBComboBoxPrivate
{
	Q_DECLARE_PUBLIC(DBComboBox)
public:
	DBBaseBeanModel *m_model;
	FilterBaseBeanModel *m_filterModel;
	QCompleter *m_completer;
	/** Nombre de la tabla o vista cuyos valores poblarán el combo */
	QString m_listTableModel;
	/** Nombre de la columna cuyos datos se mostrarán en el combo */
	QString m_listColumnName;
	/** Nombre de la columna cuyos datos se almacenarán */
	QString m_listColumnToSave;
	/** Filtro para la columna de datos */
	QString m_listFilter;
	/** Indica si el listado de ITEMs del combo se ha iniciado */
	bool m_inited;

	DBComboBox *q_ptr;

	DBComboBoxPrivate(DBComboBox *qq) : q_ptr(qq) {
		m_completer = NULL;
		m_model = NULL;
		m_filterModel = NULL;
		m_inited = false;
	}

	void initFromOptionList();
};

DBComboBox::DBComboBox(QWidget * parent ) : QComboBox(parent), DBBaseWidget(), d (new DBComboBoxPrivate(this))
{
	// Vamos a monitorizar los eventos sobre la vista: La idea es capturar la pulsación de la tecla
	// enter, para seleccionar automáticamente el item y pasar de control.
	// TODO
//	d->m_view->installEventFilter(this);
	// Conectamos para emitir nuestra propia señal.
	conexiones();
}

DBComboBox::~DBComboBox()
{
	emit destroyed(this);
	if ( d->m_filterModel != NULL ) {
		delete d->m_filterModel;
	}
	if ( d->m_model != NULL ) {
		delete d->m_model;
	}
	delete d;
}

void DBComboBox::conexiones()
{
	// currentIndexChanged y editTextChanged se emiten tanto cuando el usuario interacciona
	// o cuando se hace programáticamente. activated sólo cuando el usuario interacciona.
	connect (this, SIGNAL(currentIndexChanged(int)), this, SLOT(itemChanged(int)));
	connect (this, SIGNAL(activated(int)), this, SLOT(userModifiedCombo(int)));
	if ( lineEdit() != NULL ) {
		connect (lineEdit(), SIGNAL(textEdited(QString)), this, SLOT(itemChanged(QString)));
	}
	QComboBox::setInsertPolicy(QComboBox::NoInsert);
}

void DBComboBox::desconexiones()
{
	disconnect(SIGNAL(currentIndexChanged (int)));
	disconnect(SIGNAL(activated(int)));
	if ( lineEdit() != NULL ) {
		disconnect(SIGNAL(editTextChanged(QString)));
	}
}

void DBComboBox::setListTableModel(const QString &name)
{
	d->m_listTableModel = name;
	d->m_inited = false;
	if ( d->m_model != NULL ) {
		delete d->m_model;
		d->m_model = NULL;
	}
}

QString DBComboBox::listTableModel()
{
	return d->m_listTableModel;
}

void DBComboBox::setListColumnName ( const QString &dbVisibleColumn )
{
	d->m_listColumnName = dbVisibleColumn;
	if ( d->m_listColumnToSave.isEmpty() ) {
		d->m_listColumnToSave = d->m_listColumnName;
	}
	// Visualizamos la columna correspondiente
	setModelColumn();
}

QString DBComboBox::listColumnName ()
{
	return d->m_listColumnName;
}

void DBComboBox::setListColumnToSave ( const QString &dbColumn )
{
	d->m_listColumnToSave = dbColumn;
}

QString DBComboBox::listColumnToSave ()
{
	return d->m_listColumnToSave;
}

void DBComboBox::setListFilter ( const QString &filter )
{
	d->m_listFilter = filter;
	if ( d->m_filterModel != NULL ) {
		d->m_filterModel->resetFilter();
		d->m_filterModel->setFilter(filter);
		d->m_filterModel->invalidate();
	}
}

QString DBComboBox::listFilter ()
{
	return d->m_listFilter;
}

void DBComboBox::showEvent(QShowEvent *event)
{
	// Este orden es importante
	if ( !d->m_inited ) {
		init();
	}
	DBBaseWidget::showEvent(event);
}

void DBComboBoxPrivate::initFromOptionList()
{
	DBFieldObserver *obs= qobject_cast<DBFieldObserver *>(q_ptr->observer());
	if ( obs != NULL && !m_inited ) {
		DBField *fld = qobject_cast<DBField *> (obs->entity());
		if ( fld != NULL && !fld->metadata()->optionsList().isEmpty() ) {
			QMap<QString, QString> optionList = fld->metadata()->optionsList();
			QMap<QString, QString> optionIcons = fld->metadata()->optionsIcons();
			QMapIterator<QString, QString> it (optionList);
			q_ptr->blockSignals(true);
			while ( it.hasNext() ) {
				it.next();
				if ( optionIcons.contains(it.key()) ) {
					q_ptr->QComboBox::addItem(QIcon(optionIcons.value(it.key())), it.value(), it.key());
				} else {
					q_ptr->addItem(it.value(), it.key());
				}
			}
			// Importante esto para evitar llamadas recursivas
			m_inited = true;
			q_ptr->setValue(fld->value());
			q_ptr->blockSignals(false);
		}
		m_inited = true;
	}
}

void DBComboBox::setModelColumn()
{
	if ( d->m_model != NULL && d->m_filterModel != NULL ) {
		BaseBeanMetadata *m = d->m_model->metadata();
		if ( m != NULL ) {
			int fieldIndex = m->fieldIndex(d->m_listColumnName);
			if ( fieldIndex != -1 ) {
				QModelIndex sourceColumn = d->m_model->index(0, fieldIndex);
				QModelIndex proxyColumn = d->m_filterModel->mapFromSource(sourceColumn);
				int visibleColumn = proxyColumn.column();
				QComboBox::setModelColumn(visibleColumn);
				if ( isEditable() && d->m_completer != NULL ) {
					d->m_completer->setCompletionColumn(fieldIndex);
				}
			}
		}
	}
}

void DBComboBox::init()
{
	desconexiones();
	if ( d->m_listTableModel.isEmpty() && d->m_listColumnName.isEmpty() ) {
		d->initFromOptionList();
		conexiones();
		return;
	}
	// Al crearse el modelo, y este venir de base de datos, no se permite la carga en segundo plano.
	// No tendría sentido almacenar muchos datos en el combobox
	if ( d->m_model != NULL ) {
		delete d->m_model;
	}
	d->m_model = new DBBaseBeanModel(d->m_listTableModel, "", "", true, this);
	d->m_filterModel = new FilterBaseBeanModel(this);
	d->m_filterModel->setFilter(d->m_listFilter);
	d->m_filterModel->setSourceModel(d->m_model);
	if ( d->m_model != NULL && d->m_filterModel != NULL ) {
		QComboBox::setModel(d->m_filterModel);
		if ( this->isEditable() ) {
			createCompleter();
			d->m_completer->setModel(d->m_filterModel);
			setCompleter(d->m_completer);
		}
		setModelColumn();
		setMaxVisibleItems(15);
		QComboBox::setCurrentIndex(-1);
	}
	conexiones();
	d->m_inited = true;
}

void DBComboBox::setValue(const QVariant &v)
{
	if ( !d->m_inited ) {
		init();
	}
	if ( v != value() ) {
		if ( !d->m_listTableModel.isEmpty() ) {
			setCurrentIndexByDbColumn(d->m_listColumnToSave, v);
		} else {
			int index = findData(v);
			if ( index != currentIndex() ) {
				QComboBox::setCurrentIndex(index);
			}
		}
	}
}

QVariant DBComboBox::value()
{
	QVariant v;
	if ( !d->m_inited ) {
		init();
	}
	if ( !d->m_listTableModel.isEmpty() ) {
		FilterBaseBeanModel *mdl = qobject_cast<FilterBaseBeanModel *>(model());
		if ( mdl != NULL ) {
			QModelIndex index = mdl->index(currentIndex(), 0);
			QSharedPointer<BaseBean> bean = mdl->bean(index);
			if ( !bean.isNull() ) {
				v = bean->fieldValue(d->m_listColumnToSave);
			}
		}
	} else {
		QVariant data = itemData(currentIndex());
		if ( data.isValid() ) {
			v = data;
		} else {
			v = currentText();
		}
	}
	return v;
}

int DBComboBox::currentIndex() const
{
	return QComboBox::currentIndex();
}

void DBComboBox::setCurrentIndex ( int index )
{
	if ( !d->m_inited ) {
		init();
	}
	QComboBox::setCurrentIndex(index);
}

void DBComboBox::createCompleter()
{
	if ( d->m_completer != NULL ) {
		delete d->m_completer;
	}
	d->m_completer = new QCompleter(this);
	d->m_completer->setMaxVisibleItems(10);
	d->m_completer->setCaseSensitivity(Qt::CaseInsensitive);
	// Importante decidir sobre qué rol se hace el completado
	d->m_completer->setCompletionRole(Qt::DisplayRole);
	// Se le indica al completer que el modelo está ordenado
	d->m_completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
	// Se completa inline
	d->m_completer->setCompletionMode(QCompleter::InlineCompletion);
	// Cuando en el modelo se active un item, se selecciona también en la vista
	connect(d->m_completer, SIGNAL(activated(QString)), lineEdit(), SLOT(setText(QString)));
}

/*!
  Selecciona del combo el item indicado, según el valor de la columna
  de base de datos
  */
void DBComboBox::setCurrentIndexByDbColumn ( const QString &dbModelColumn, const QVariant &value )
{
	/* Esta es la manera bonita de hacerlo */
	if ( d->m_model == NULL || d->m_model->metadata() == NULL ) {
		return;
	}
	int column = d->m_model->metadata()->fieldIndex(dbModelColumn);
	QModelIndex start = d->m_model->index(0, column);
    QModelIndexList list = d->m_model->match(start, Qt::UserRole, value, -1, Qt::MatchExactly);
	if ( !list.isEmpty() && list.at(0).isValid() ) {
        /*! TODO: OJO, Hay un bug documentado en 4.7.X
          https://bugreports.qt.nokia.com/browse/QTBUG-18215
          que provoca que este mapFromSource pueda no funcionar. Por eso
          hago este workaround cutre, que deja sin posibilidad de filtrar
          los datos de la lista */
        /*QModelIndex filterIndex = d->m_filterModel->mapFromSource(list.at(0));
        view()->setCurrentIndex(filterIndex);*/
		blockSignals(true);
        QComboBox::setCurrentIndex(list.at(0).row());
		blockSignals(false);
	}
}

/*!
  Para el elemento seleccionado, copia en bean el bean que corresponde a ese elemento.
  bean debe ser un objeto válido
  */
QSharedPointer<BaseBean> DBComboBox::bean(int index)
{
	FilterBaseBeanModel *mdl = qobject_cast<FilterBaseBeanModel *>(model());
	if ( mdl == NULL ) {
		return QSharedPointer<BaseBean>();
	}
	QModelIndex indice = mdl->index(index, 0);
	if ( indice.isValid() ) {
		return mdl->bean(indice);
	}
	return QSharedPointer<BaseBean>();
}

/*!
  Se dispara cuando el usuario cambia el item seleccionado
  */
void DBComboBox::itemChanged(int index)
{
	Q_UNUSED(index)
	// Obtengamos el id interno del bean, y lanzemos la señal
	QVariant v = value();
	if ( v.isValid() ) {
		emit valueEdited(v);
	}
}

/*!
  Se dispara cuando el usuario cambia el item seleccionado
  */
void DBComboBox::itemChanged(const QString &text)
{
	// Veamos si el texto coincide con alguno de los items en la lista
	QVariant v = value();
	if ( !v.isValid() ) {
		int index = findText(text);
		if ( index != -1 ) {
			if ( index != currentIndex() ) {
				QComboBox::setCurrentIndex(index);
				v = value();
			}
		}
	}
	if ( v.isValid() ) {
		emit valueEdited(v);
	}
	m_userModified = true;
}

void DBComboBox::userModifiedCombo(int index)
{
	Q_UNUSED(index)
	// Podemos hacer esto porque viene de la señal activated
	m_userModified = true;
}

bool DBComboBox::eventFilter(QObject * obj, QEvent * event)
{
	QModelIndex index;
	if ( obj == view () && event->type() == QEvent::KeyPress ) {
		QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
		switch (keyEvent->key()) {
		case Qt::Key_Enter:
			// Al pulsar enter, seleccionamos, si lo hubiera, el elemento seleccionado de la vista
			index = view()->currentIndex();
			if ( index.isValid() ) {
				QComboBox::setCurrentIndex(index.row());
			}
			hidePopup();
			focusNextChild();
			return true;
		}
	}
	return QObject::eventFilter(obj, event);
}

void DBComboBox::focusInEvent(QFocusEvent * event)
{
	if ( isEditable() ) {
		lineEdit()->selectAll();
	}
	QComboBox::focusInEvent(event);
}

/*!
  Ajusta los parámetros de visualización de este Widget en función de lo definido en DBField d->m_field
  */
void DBComboBox::applyFieldProperties()
{
}

void DBComboBox::observerUnregistered()
{
	DBBaseWidget::observerUnregistered();
	blockSignals(true);
	this->clear();
	d->m_inited = false;
	blockSignals(false);
}

/*!
  Pide un nuevo observador si es necesario
  */
void DBComboBox::refresh()
{
	observer();
	if ( m_observer != NULL ) {
		m_observer->sync();
	}
}

void DBComboBox::addItem (const QString & text, const QVariant & userData)
{
	QComboBox::addItem(text, userData);
}

QVariant DBComboBox::itemData (int index, int role) const
{
	return QComboBox::itemData(index, role);
}

QSharedPointer<BaseBean> DBComboBox::selectedBean()
{
	return bean(currentIndex());
}

/*!
  Tenemos que decirle al motor de scripts, que DBFormDlg se convierte de esta forma a un valor script
  */
QScriptValue DBComboBox::toScriptValue(QScriptEngine *engine, DBComboBox * const &in)
{
	return engine->newQObject(in, QScriptEngine::QtOwnership, QScriptEngine::PreferExistingWrapperObject);
}

void DBComboBox::fromScriptValue(const QScriptValue &object, DBComboBox * &out)
{
	out = qobject_cast<DBComboBox *>(object.toQObject());
}
