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

#include "dbchooserecordbutton.h"
#include "forms/dbsearchdlg.h"
#include "widgets/dbbasewidget.h"
#include <QPointer>

class DBChooseRecordButtonPrivate
{
public:
	QString m_tableName;
	QString m_fieldsToGet;
	QString m_controlsToAssignValue;
	QString m_searchFilter;

	DBChooseRecordButtonPrivate() {}
};

DBChooseRecordButton::DBChooseRecordButton(QWidget *parent) :
	QPushButton(parent), d(new DBChooseRecordButtonPrivate)
{
	connect(this, SIGNAL(clicked()), this, SLOT(buttonClicked()));
	setEnabled(false);
}

DBChooseRecordButton::~DBChooseRecordButton()
{
	delete d;
}

QString DBChooseRecordButton::tableName()
{
	return d->m_tableName;
}

void DBChooseRecordButton::setTableName(const QString &value)
{
	d->m_tableName = value;
	if ( !d->m_tableName.isEmpty() ) {
		setEnabled(true);
	}
}

QString DBChooseRecordButton::fieldsToGet ()
{
	return d->m_fieldsToGet;
}

void DBChooseRecordButton::setFieldsToGet(const QString &value)
{
	d->m_fieldsToGet = value;
}

QString DBChooseRecordButton::controlsToAssignValue()
{
	return d->m_controlsToAssignValue;
}

void DBChooseRecordButton::setControlsToAssignValue(const QString &value)
{
	d->m_controlsToAssignValue = value;
}

void DBChooseRecordButton::setSearchFilter(const QString &value)
{
	d->m_searchFilter = value;
}

QString DBChooseRecordButton::searchFilter()
{
	return d->m_searchFilter;
}

void DBChooseRecordButton::buttonClicked()
{
	if ( d->m_tableName.isEmpty() ) {
		return;
	}
	QPointer<DBSearchDlg> dlg = new DBSearchDlg(d->m_tableName);
	if ( dlg->openSuccess() ) {
		dlg->setModal(true);
		dlg->setCanApplyfilter(false);
		dlg->setFilterData(d->m_searchFilter);
		dlg->init();
		dlg->exec();
		if ( dlg->userClickOk() ) {
			// Recogemos el campo buscado si lo hay
			QSharedPointer<BaseBean> bean = dlg->selectedBean();
			if ( !bean.isNull() ) {
				QDialog *dlg = parentDialog();
				QStringList fields = d->m_fieldsToGet.split(";");
				QStringList controls = d->m_controlsToAssignValue.split(";");
				if ( dlg != NULL && controls.size() >= fields.size() ) {
					int i = 0;
					foreach ( QString field, fields ) {
						QWidget *wid = dlg->findChild<QWidget *>(controls.at(i));
						if ( wid->property("perpControl").toBool() ) {
							DBBaseWidget *base = dynamic_cast<DBBaseWidget *>(wid);
							base->setValue(bean->fieldValue(field));
						}
					}
				}
			}
		}
	} else {
		delete dlg;
	}
}

/*!
  Devuelve el diálogo padre en el que está este control
  */
QDialog *DBChooseRecordButton::parentDialog()
{
	QObject *tmp = parent();
	QDialog *dlg = NULL;
	// Buscamos ahora el dialogo padre
	while ( dlg == NULL && tmp != NULL ) {
		dlg = qobject_cast<QDialog *> (tmp);
		tmp = tmp->parent();
	}
	return dlg;
}

/*!
  Tenemos que decirle al motor de scripts, que DBFormDlg se convierte de esta forma a un valor script
  */
QScriptValue DBChooseRecordButton::toScriptValue(QScriptEngine *engine, DBChooseRecordButton * const &in)
{
	return engine->newQObject(in, QScriptEngine::QtOwnership, QScriptEngine::PreferExistingWrapperObject);
}

void DBChooseRecordButton::fromScriptValue(const QScriptValue &object, DBChooseRecordButton * &out)
{
	out = qobject_cast<DBChooseRecordButton *>(object.toQObject());
}
