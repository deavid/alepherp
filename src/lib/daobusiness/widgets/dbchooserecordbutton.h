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

#ifndef DBCHOOSERECORDBUTTON_H
#define DBCHOOSERECORDBUTTON_H

#include <QObject>
#include <QPushButton>
#include <QScriptValue>
#include <alepherpglobal.h>

class DBChooseRecordButtonPrivate;

class Q_ALEPHERP_EXPORT DBChooseRecordButton : public QPushButton
{
    Q_OBJECT
	/** Tabla en la que buscar el registro */
	Q_PROPERTY(QString tableName READ tableName WRITE setTableName)
	Q_PROPERTY(QString fieldsToGet READ fieldsToGet WRITE setFieldsToGet)
	Q_PROPERTY(QString controlsToAssignValue READ controlsToAssignValue WRITE setControlsToAssignValue)
	Q_PROPERTY(QString searchFilter READ searchFilter WRITE setSearchFilter)

private:
	DBChooseRecordButtonPrivate *d;
	Q_DECLARE_PRIVATE(DBChooseRecordButton)

	QDialog * parentDialog();

public:
	explicit DBChooseRecordButton(QWidget *parent = 0);
	~DBChooseRecordButton();

	QString tableName();
	void setTableName(const QString &value);
	QString fieldsToGet ();
	void setFieldsToGet(const QString &value);
	QString controlsToAssignValue();
	void setControlsToAssignValue(const QString &value);
	QString searchFilter();
	void setSearchFilter(const QString &value);

	static QScriptValue toScriptValue(QScriptEngine *engine, DBChooseRecordButton * const &in);
	static void fromScriptValue(const QScriptValue &object, DBChooseRecordButton * &out);

signals:

private slots:
	void buttonClicked();

};

//Q_DECLARE_METATYPE(DBChooseRecordButton *)
//Q_SCRIPT_DECLARE_QMETAOBJECT(DBChooseRecordButton, DBChooseRecordButton*)

#endif // DBCHOOSERECORDBUTTON_H
