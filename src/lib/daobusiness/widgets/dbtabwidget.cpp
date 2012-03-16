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
#include "dbtabwidget.h"
#include "dao/beans/basebean.h"
#include "dao/beans/beansfactory.h"
#include "dao/beans/basebeanmetadata.h"
#include "dao/beans/dbfield.h"
#include "dao/basedao.h"

DBTabWidget::DBTabWidget(QWidget * parent)
 : QTabWidget(parent)
{
	connect (this->tabBar(), SIGNAL(currentChanged(int)), this, SLOT(tabCambiada(int)));
}

DBTabWidget::DBTabWidget(const QString &dbField, QWidget * parent)
 : QTabWidget(parent)
{
	m_fieldView = dbField;
	connect (this->tabBar(), SIGNAL(currentChanged(int)), this, SLOT(tabCambiada(int)));
}


DBTabWidget::~DBTabWidget()
{
}

QString DBTabWidget::tableName()
{
	return m_tableName;
}

void DBTabWidget::setTableName (const QString &value)
{
	m_tableName = value;
	// Si la lista no estaba vacía es que se han cambiado datos. Reiniciamos
	if ( !m_list.isEmpty() ) {
		clear();
		init();
	}
}

QString DBTabWidget::fieldView()
{
	return m_fieldView;
}

void DBTabWidget::setFieldView(const QString &field)
{
	m_fieldView = field;
	// Si la lista no estaba vacía es que se han cambiado datos. Reiniciamos
	if ( !m_list.isEmpty() ) {
		clear();
		init();
	}
}

QString DBTabWidget::filter()
{
	return m_filter;
}

void DBTabWidget::setFilter(const QString &value)
{
	m_filter = value;
	// Si la lista no estaba vacía es que se han cambiado datos. Reiniciamos
	if ( !m_list.isEmpty() ) {
		clear();
		init();
	}
}

QString DBTabWidget::order()
{
	return m_order;
}

void DBTabWidget::setOrder(const QString &value)
{
	m_order = value;
	// Si la lista no estaba vacía es que se han cambiado datos. Reiniciamos
	if ( !m_list.isEmpty() ) {
		clear();
		init();
	}
}

void DBTabWidget::init()
{
	if ( m_tableName.isEmpty() || m_fieldView.isEmpty() ) {
		return;
	}
	if ( BaseDAO::select(m_list, m_tableName, m_filter, m_order ) ) {
		QTabBar *barra = tabBar();
		if ( m_list.size () > 0 ) {
			// La primera ficha está ya añadida.
			barra->setTabText(0, m_list.at(0)->fieldValue(m_fieldView).toString());
			for ( int i = 1 ; i < m_list.size() ; i++ ) {
				barra->addTab(m_list.at(i)->fieldValue(m_fieldView).toString());
			}
		}
	}
}

void DBTabWidget::clear()
{
	QTabWidget::clear();
	m_list.clear();
}

void DBTabWidget::showEvent ( QShowEvent * event )
{
	Q_UNUSED(event)
	if ( m_list.isEmpty() ) {
		init();
	}
}

/*!
Este slot interno emite un señal indicando qué tab ha cambiado el usuario y
lanzando el ID asociado a ese tab.
 */
void DBTabWidget::tabCambiada(int index)
{
	QVariant id;
	
	if ( index > -1 && !m_list.isEmpty() ) {
		id = m_list.at(index)->pkValue();
	}
	emit (tabChanged(id));
	emit tabChanged();
}

/*!
	Devuelve el id del tab seleccionado.
 */
QVariant DBTabWidget::idCurrentTab(void)
{
	QVariant id;
	if ( !m_list.isEmpty() && currentIndex() > -1 && currentIndex() < m_list.size() ) {
		QSharedPointer<BaseBean> bean = m_list.at(currentIndex());
		id = bean->pkValue();
	}
	return id;
}

/*!
  Permite añadir un tab que no está en la tabla. Por ejemplo, para añadir un
  tab "Todos"
  */
void DBTabWidget::addTab(const QVariant &id, const QString & nombre)
{
	QSharedPointer<BaseBean> temp = m_list.at(0);
	QTabBar *barra = tabBar();
		
	m_list.append(BeansFactory::instance()->newQBaseBean(temp->metadata()->tableName()));
	m_list.last()->setPkValue(id);
	m_list.last()->field(m_fieldView)->setValue(nombre);

	barra->addTab(nombre);	
}

/*!
  Tenemos que decirle al motor de scripts, que DBFormDlg se convierte de esta forma a un valor script
  */
QScriptValue DBTabWidget::toScriptValue(QScriptEngine *engine, DBTabWidget * const &in)
{
	return engine->newQObject(in, QScriptEngine::QtOwnership, QScriptEngine::PreferExistingWrapperObject);
}

void DBTabWidget::fromScriptValue(const QScriptValue &object, DBTabWidget * &out)
{
	out = qobject_cast<DBTabWidget *>(object.toQObject());
}
