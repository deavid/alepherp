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
#include "dblistview.h"
#include "dao/beans/basebeanmetadata.h"
#include "dao/beans/beansfactory.h"
#include "dao/beans/dbrelation.h"
#include "dao/beans/dbfieldmetadata.h"
#include "models/filterbasebeanmodel.h"
#include "models/dbbasebeanmodel.h"
#include "models/relationbasebeanmodel.h"
#include "dao/dbrelationobserver.h"

class DBListViewPrivate
{
public:
	/** Indica qué field del BaseBean es visible */
    QString m_visibleField;
	/** Indica qué fields del BaseBean se devolverán con value */
    QString m_keyField;
	/** Indica si se presenta un checkbox al lado de cada item */
	bool m_itemCheckBox;
	/** Se pueden asignar beans a ser checkeados, antes de que el control se haya iniciado.
	  En ese caso, se guardan previamente. Como los beans pueden venir del motor javascript,
	mejor guardos los PK */
	QVariantList m_initedCheckedBeansPk;

	DBListViewPrivate();
};

DBListViewPrivate::DBListViewPrivate()
{
	m_itemCheckBox = false;
}

DBListView::DBListView(QWidget *parent) : QListView(parent), DBAbstractViewInterface(), d_ptr(new DBListViewPrivate)
{
}

DBListView::~DBListView()
{
	emit destroyed(this);
	delete d_ptr;
}

void DBListView::setItemCheckBox(bool value)
{
	d_ptr->m_itemCheckBox = value;
	if ( value ) {
		BaseBeanModel *mdl = qobject_cast<BaseBeanModel *>(m_model);
		if ( mdl != NULL ) {
			QStringList fields;
			fields << d_ptr->m_visibleField;
			mdl->setCheckColumns(fields);
		}
	}
}

bool DBListView::itemCheckBox()
{
	return d_ptr->m_itemCheckBox;
}

QString DBListView::visibleField()
{
	return d_ptr->m_visibleField;
}

void DBListView::setVisibleField(const QString &value)
{
	d_ptr->m_visibleField = value;
	if ( d_ptr->m_itemCheckBox ) {
		setItemCheckBox(true);
	}
}

QString DBListView::keyField()
{
	if ( d_ptr->m_keyField.isEmpty() ) {
		BaseBeanModel *mdl = qobject_cast<BaseBeanModel *>(m_model);
		if ( mdl != NULL ) {
			BaseBeanMetadata *metadata = mdl->metadata();
			QList<DBFieldMetadata *> pkFields = metadata->pkFields();
			foreach ( DBFieldMetadata *fld, pkFields ) {
				if ( d_ptr->m_keyField.isEmpty() ) {
					d_ptr->m_keyField = fld->dbFieldName();
				} else {
					d_ptr->m_keyField = QString("%1;%2").arg(d_ptr->m_keyField).arg(fld->dbFieldName());
				}
			}
		}
	}
	return d_ptr->m_keyField;
}

void DBListView::setKeyField(const QString &value)
{
	d_ptr->m_keyField = value;
}


/** Establece el valor a mostrar en el control. El valor será un array  */
void DBListView::setValue(const QVariant &value)
{
	Q_UNUSED(value)
/*	QList<QVariant> list = value.toList();
	BaseBeanModel *mdl = qobject_cast<BaseBeanModel *> (m_model);
	if ( mdl != NULL ) {
		mdl-
	}*/
}

/** Devuelve el valor mostrado o introducido en el control. Los valores
que devuelve dependen de keyField, y serán, por ejemplo, para
keyField="username, password":
value te devolverá un array con datos de los beans asociados a los checks marcados.
¿Como? Así:
	var v = listView.value;
	entonces, v es
	v[0] = "david,mipassword"
	v[1] = "jose,mipassword"
	*/
QVariant DBListView::value()
{
	QVariant v;
	QStringList keyFields = d_ptr->m_keyField.split(";");
	BaseBeanModel *mdl = qobject_cast<BaseBeanModel *>(m_model);
	if ( mdl != NULL ) {
		QModelIndexList indexes;
		if ( d_ptr->m_itemCheckBox ) {
			indexes = mdl->checkedItems();
		} else {
			indexes = QListView::selectedIndexes();
		}
		if ( indexes.size() > 0 ) {
			QList<QVariant> list;
			foreach ( QModelIndex idx, indexes ) {
				QSharedPointer<BaseBean> bean = mdl->bean(idx);
				if ( !bean.isNull() ) {
					QString result;
                    foreach ( QString key, keyFields) {
						if ( result.isEmpty() ) {
							result = bean->fieldValue(key).toString();
						} else {
							result = QString("%1,%2").arg(result).arg(bean->fieldValue(key).toString());
						}
					}
					list.append(result);
				}
			}
			v = QVariant(list);
		}
	}
	return v;
}

/*!
 Ajusta el control y sus propiedades a lo definido en el field
*/
void DBListView::applyFieldProperties()
{
}

/*!
  Para refrescar los controles: Piden nuevo observador si es necesario
*/
void DBListView::refresh()
{
	if ( m_model != NULL ) {
		if ( m_filterModel != NULL ) {
			m_filterModel->invalidate();
		}
	}
}

/*!
  Al mostrarse el control es cuando se crean los modelos y demás
  */
void DBListView::showEvent ( QShowEvent * event )
{
	DBBaseWidget::showEvent(event);
	init(true);
	setItemCheckBox(d_ptr->m_itemCheckBox);
	if ( d_ptr->m_initedCheckedBeansPk.size() > 0 ) {
		setCheckedBeansByPk(d_ptr->m_initedCheckedBeansPk);
		d_ptr->m_initedCheckedBeansPk.clear();
	}
}

void DBListView::setupInternalModel()
{
	DBAbstractViewInterface::setupInternalModel();
	if ( !m_externalModel && m_metadata != NULL ) {
		DBFieldMetadata *fld = m_metadata->field(d_ptr->m_visibleField);
		if (  fld != NULL ) {
			QModelIndex sourceColumn = m_model->index(0, fld->index());
			QModelIndex proxyColumn = m_filterModel->mapFromSource(sourceColumn);
            int visibleField = proxyColumn.column();
            QListView::setModelColumn(visibleField);
		}
		if ( m_filterModel != NULL ) {
			m_filterModel->setDbStates(BaseBean::INSERT | BaseBean::UPDATE);
		}
	}
}

/*!
  Devuelve un listado de los beans que han sido checkeados por el usuario
  */
BaseBeanPointerList DBListView::checkedBeans()
{
	BaseBeanPointerList list;
	BaseBeanModel *mdl = NULL;
	FilterBaseBeanModel *filterModel = qobject_cast<FilterBaseBeanModel *>(model());
	if ( filterModel != NULL ) {
		mdl = qobject_cast<BaseBeanModel *>(filterModel->sourceModel());
	} else {
		mdl = qobject_cast<BaseBeanModel *>(model());
	}
	if ( mdl == NULL ) {
		return list;
	}
	QModelIndexList checkedItems = mdl->checkedItems();
	for ( int i = 0 ; i < checkedItems.size() ; i++ ) {
		QSharedPointer<BaseBean> bean = mdl->bean(checkedItems.at(i));
		if ( !bean.isNull() ) {
			list.append(bean);
		}
	}
	return list;
}

void DBListView::setCheckedBeansByPk(QVariantList list, bool checked)
{
	BaseBeanModel *mdl = NULL;
	FilterBaseBeanModel *filterModel = qobject_cast<FilterBaseBeanModel *>(model());
	if ( filterModel != NULL ) {
		mdl = qobject_cast<BaseBeanModel *>(filterModel->sourceModel());
	} else {
		mdl = qobject_cast<BaseBeanModel *>(model());
	}
	if ( mdl == NULL ) {
		d_ptr->m_initedCheckedBeansPk = list;
		return;
	}
	QModelIndexList checkedItems ;
	for ( int i = 0 ; i < list.size() ; i++ ) {
		for ( int row = 0 ; row < mdl->rowCount() ; row++ ) {
			QSharedPointer<BaseBean> bean = mdl->bean(row);
			if ( bean->pkEqual(list.at(i)) ) {
				checkedItems.append(mdl->index(row, 0));
			}
		}
	}
	if ( checkedItems.size() > 0 ) {
		mdl->setCheckedItems(checkedItems, checked);
	}
}

/*!
  Devuelve un listado de los beans que han sido checkeados por el usuario.
  Esta función puede ser llamada desde Javascript, en el constructor del widget. En ese
  caso los modelos aún no se han creado, por lo que se guardarán en una estructura
  intermedia las primary keys de los beans pasados. No se guardan los beans, porque
  estos pueden haber sido borrados previamente por el motor de javascript
  */
void DBListView::setCheckedBeans(BaseBeanPointerList list, bool checked)
{
	BaseBeanModel *mdl = NULL;
	FilterBaseBeanModel *filterModel = qobject_cast<FilterBaseBeanModel *>(model());
	if ( filterModel != NULL ) {
		mdl = qobject_cast<BaseBeanModel *>(filterModel->sourceModel());
	} else {
		mdl = qobject_cast<BaseBeanModel *>(model());
	}
	for ( int i = 0 ; i < list.size() ; i++ ) {
		if ( !list.at(i).isNull() ) {
			d_ptr->m_initedCheckedBeansPk.append(list.at(i)->pkValue());
		}
	}
	if ( mdl != NULL ) {
		setCheckedBeansByPk(d_ptr->m_initedCheckedBeansPk, checked);
	}
}

/*!
  El observador que alimenta de datos a este control se ha borrado. Actuamos reseteándolo
  */
void DBListView::observerUnregistered()
{
	DBAbstractViewInterface::observerUnregistered();
	blockSignals(true);
	reset();
	blockSignals(false);
}

/*!
  Tenemos que decirle al motor de scripts, que DBFormDlg se convierte de esta forma a un valor script
  */
QScriptValue DBListView::toScriptValue(QScriptEngine *engine, DBListView * const &in)
{
	return engine->newQObject(in, QScriptEngine::QtOwnership, QScriptEngine::PreferExistingWrapperObject);
}

void DBListView::fromScriptValue(const QScriptValue &object, DBListView * &out)
{
	out = qobject_cast<DBListView *>(object.toQObject());
}
