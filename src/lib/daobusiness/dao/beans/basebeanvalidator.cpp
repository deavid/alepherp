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
#include "basebeanvalidator.h"
#include "dao/beans/basebean.h"
#include "dao/beans/basebeanmetadata.h"
#include "dao/beans/dbfield.h"
#include "dao/beans/dbfieldmetadata.h"
#include "dao/basedao.h"
#include "dao/dbfieldobserver.h"

BaseBeanValidator::BaseBeanValidator(QObject *parent) :
    QObject(parent)
{
	m_bean = NULL;
	m_widget = NULL;
}


void BaseBeanValidator::setBean(BaseBean *bean)
{
	m_bean = bean;
}

BaseBean *BaseBeanValidator::bean()
{
	return m_bean;
}

/*!
  Realiza la validación de los values de los fields del bean m_bean, a partir de la definición
  de la tabla dada
*/
bool BaseBeanValidator::validate()
{
	QList<DBField *> fields = m_bean->fields();
	bool result = true;

	m_message.clear();
	m_htmlMessage.clear();
	m_widget = NULL;

	// Comprobemos si la primary key es única, en operaciones de inserción
	if ( m_bean->dbState() == BaseBean::INSERT ) {
		result = result & checkPk();
	}
	foreach(DBField *fld, fields) {
		// El campo no puede ser nulo (salvo que sea serial)
		if ( !fld->metadata()->serial() && !fld->metadata()->null() ) {
			result = result & checkNull(fld);
		}
		// Comprobemos que no se excede la longitud establecida
		if ( fld->metadata()->type() == QVariant::String ) {
			result = result & checkLength(fld);
		}
		// Si hay alguna regla de validación en Javascript definida en el XML se comprueba
		if ( !fld->metadata()->validationRuleScript().isEmpty() ) {
			bool temp = false;
			QString message = fld->metadata()->validateRule(fld, temp);
			result = result & temp;
			if ( !temp && !message.isEmpty() ) {
				m_htmlMessage = QString("%1<p><strong>%2</strong>: %3</p>").
						arg(m_htmlMessage).arg(fld->metadata()->fieldName()).arg(message);
			}
		}
	}
	return result;
}

/*!
  Devuelve el mensaje correspondiente a la última llamada a validate, incluyendo
  los datos por los que no se pasó la validación
*/
QString BaseBeanValidator::validateMessages()
{
	return m_message;
}

/*!
  Devuelve el mensaje correspondiente a la última llamada a validate, incluyendo
  los datos por los que no se pasó la validación. Lo hace en HTML
*/
QString BaseBeanValidator::validateHtmlMessages()
{
	return m_htmlMessage;
}

/*!
  De una mala validación, devuelve el primer widgetque tenía problemas
*/
QWidget *BaseBeanValidator::widgetOnBadValidate()
{
	return m_widget;
}

/*!
  Chequea si el valor de fld pasa la validación de NULL
  */
bool BaseBeanValidator::checkNull(DBField *fld)
{
	bool result = true;

	if ( fld->metadata()->type() == QVariant::String ||
		 fld->metadata()->type() == QVariant::Double || fld->metadata()->type() == QVariant::Int ||
		 fld->metadata()->type() == QVariant::Date ) {
		QString temp = fld->value().toString();
		// También puede ocurrir, que el campo sea un combobox.
		if ( temp.isEmpty() || ( fld->metadata()->optionsList().size() > 0 && temp == "-1" )) {
			result = false;
			m_message = QString("%1\r\n%2: No puede estar vac\303\255o.").arg(m_message).arg(fld->metadata()->fieldName());
			m_htmlMessage = QString("%1<p><strong>%2</strong>: No puede estar vac&iacute;o.</p>").
							arg(m_htmlMessage).arg(fld->metadata()->fieldName());
			if ( m_widget == NULL ) {
				// TODO Esto no es muy elegante
				QObjectList widgets = fld->observer()->viewWidgets();
				if ( widgets.size() > 0 ) {
					m_widget = qobject_cast<QWidget *>(widgets.at(0));
				}
			}
		}
	} else if ( fld->metadata()->type() == QVariant::Bool || fld->metadata()->type() == QVariant::Pixmap ) {
		if ( fld->value().isNull() || !fld->value().isValid() ) {
			result = false;
			m_message = QString("%1\r\n%2: No puede estar vac\303\255o.").arg(m_message).arg(fld->metadata()->fieldName());
			m_htmlMessage = QString("%1<p><strong>%2</strong>: No puede estar vac&iacute;o.</p>").
							arg(m_htmlMessage).arg(fld->metadata()->fieldName());
			if ( m_widget == NULL ) {
				// TODO Esto no es muy elegante
				QObjectList widgets = fld->observer()->viewWidgets();
				if ( widgets.size() > 0 ) {
					m_widget = qobject_cast<QWidget *>(widgets.at(0));
				}
			}
		}
	}
	return result;
}

/*!
  Chequea, en operaciones de inserción que la primary key es única
  */
bool BaseBeanValidator::checkPk()
{
	bool result = true;

	QVariant pk = m_bean->pkValue();
	// Si la primary key es o contiene un valor serial, se pasa la validación automáticamente
	QList<DBFieldMetadata *> fldPk = m_bean->metadata()->pkFields();
	foreach (DBFieldMetadata *fld, fldPk) {
		if ( fld->serial() ) {
			return true;
		}
	}
	QSharedPointer<BaseBean> temp = BaseDAO::selectByPk(pk, m_bean->metadata()->tableName());
	if ( !temp.isNull() ) {
		result = false;
		m_message = QString("%1\r\nYa existe un registro con el mismo identificador \303\272nico.").arg(m_message);
		m_htmlMessage = QString("%1<p>Ya existe un registro con el <strong>mismo identificador \303\272nico.</strong></p>").arg(m_htmlMessage);
	}
	return result;
}

/*!
  Comprueba que la longitud del campo no exceda lo indicado
  */
bool BaseBeanValidator::checkLength(DBField *fld)
{
	bool result = true;
	if ( fld->metadata()->length() == -1 ) {
		return true;
	}
	QString temp = fld->value().toString();
	if ( temp.size() > fld->metadata()->length() ) {
		result = false;
		m_message = QString("%1\r\n%2: La longitud del texto introducido sobrepasa la permitida. Acorte el texto.").
					arg(m_message).arg(fld->metadata()->fieldName());
		m_htmlMessage = QString("%1<p><strong>%2</strong>: La longitud del texto introducido sobrepasa la permitida. Acorte el texto.</p>").
						arg(m_htmlMessage).arg(fld->metadata()->fieldName());
		if ( m_widget == NULL ) {
			// TODO esto no es muy elegante
			QObjectList widgets = fld->observer()->viewWidgets();
			if ( widgets.size() > 0 ) {
				m_widget = qobject_cast<QWidget *>(widgets.at(0));
			}
		}
	}
	return result;
}
