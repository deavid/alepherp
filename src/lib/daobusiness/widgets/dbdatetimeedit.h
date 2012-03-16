/***************************************************************************
 *   Copyright (C) 2011 by David Pinelo   *
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
#ifndef DBDATETIMEEDIT_H
#define DBDATETIMEEDIT_H

#include <QDateTimeEdit>
#include <QScriptValue>
#include <alepherpglobal.h>
#include "widgets/dbbasewidget.h"
#include "dao/observerfactory.h"

/**
  Clase para la edición de campos de tiempo/hora DBField de un BaseBean
  @author David Pinelo <david.pinelo@alephsistemas.es>
  @see QDateTimeEdit
  @see DBBaseWidget
  */
class Q_ALEPHERP_EXPORT DBDateTimeEdit : public QDateTimeEdit, public DBBaseWidget
{
    Q_OBJECT
	Q_PROPERTY (QString fieldName READ fieldName WRITE setFieldName)
	Q_PROPERTY (QString relationFilter READ relationFilter WRITE setRelationFilter)
	Q_PROPERTY (bool dataEditable READ dataEditable WRITE setDataEditable)
	Q_PROPERTY (bool perpControl READ perpControl)
	Q_PROPERTY (bool userModified READ userModified WRITE setUserModified)
	Q_PROPERTY (QVariant value READ value WRITE setValue)
	Q_PROPERTY (bool dataFromParentDialog READ dataFromParentDialog WRITE setDataFromParentDialog)

protected:
	void showEvent(QShowEvent *event);
	void hideEvent(QHideEvent *event);
	void focusInEvent(QFocusEvent * event);
	void focusOutEvent(QFocusEvent * event);

public:
	explicit DBDateTimeEdit(QWidget *parent = 0);
	~DBDateTimeEdit();

	int observerType() { return OBSERVER_DBFIELD; }
	void applyFieldProperties();
	QVariant value();

	static QScriptValue toScriptValue(QScriptEngine *engine, DBDateTimeEdit * const &in);
	static void fromScriptValue(const QScriptValue &object, DBDateTimeEdit * &out);

signals:
	void valueEdited(const QVariant &value);
	void destroyed(QWidget *widget);

public slots:
	void setValue(const QVariant &value);
	void refresh();
	void observerUnregistered();

private slots:
	void emitValueEdited();
};

Q_DECLARE_METATYPE(DBDateTimeEdit*)
Q_SCRIPT_DECLARE_QMETAOBJECT(DBDateTimeEdit, DBDateTimeEdit*)

#endif // DBDATETIMEEDIT_H
