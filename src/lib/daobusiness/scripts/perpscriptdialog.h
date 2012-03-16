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
#ifndef PERPSCRIPTDIALOG_H
#define PERPSCRIPTDIALOG_H

#include <QObject>
#include <QScriptValue>
#include <QScriptEngine>

class PERPScriptDialog;
class PERPScriptDialogPrivate;

typedef QSharedPointer<PERPScriptDialog> PERPScriptDialogPointer;

/**
  Esta clase permitirá crear o invocar formularios directamente desde código Qs.
  Esto permite programar la interacción con ventanas. Se identifican tres tipos de formularios
  <ul>
  <li>DBRecordDlg: Formulario para la edición de un registro de base de datos</li>
  <li>DBSearchDlg: Formulario que permite realizar una búsqueda en base de datos</li>
  <li>ScriptDlg: Formulario que no está ligado a base de datos, y ejecuta código arbitrario en Qs</li>
  La forma de crear un formulario es la siguiente:

  var dlg = new DBDialog;
  dlg.type = "search";
  dlg.tableName = "clientes";
  dlg.fieldToSearch = "codcliente";
  dlg.show();
  if ( dlg.userClickOk && oldCodCliente != dlg.fieldSearched ) {
	bean.setFieldValue("codcliente", dlg.fieldSearched);
	bean.setFieldValue("iddircliente", -1);
  }
*/

class PERPScriptDialog : public QObject
{
    Q_OBJECT
	Q_PROPERTY(QString type READ type WRITE setType)
	Q_PROPERTY(QString tableName READ tableName WRITE setTableName)
	Q_PROPERTY(QVariant fieldSearched READ fieldSearched)
	Q_PROPERTY(QString fieldToSearch READ fieldToSearch WRITE setFieldToSearch)
	Q_PROPERTY(QString searchFilter READ searchFilter WRITE setSearchFilter)
	Q_PROPERTY(QString qsName READ qsName WRITE setQsName)
	Q_PROPERTY(QString uiName READ uiName WRITE setUiName)
	Q_PROPERTY(bool userClickOk READ userClickOk)

private:
	PERPScriptDialogPrivate *d;
	Q_DECLARE_PRIVATE(PERPScriptDialog)

	void newSearchDlg();
	void newRecordDlg();
	void newScriptDlg();

public:
    explicit PERPScriptDialog(QObject *parent = 0);
	~PERPScriptDialog();

	void setType(const QString &type);
	QString type();
	void setTableName(const QString &type);
	QString tableName();

	void setQsName(const QString &name);
	QString qsName();
	void setUiName(const QString &name);
	QString uiName();

	QString fieldToSearch();
	void setFieldToSearch(const QString &value);
	QVariant fieldSearched();
	void setFieldSearched(const QVariant &value);
	QString searchFilter();
	void setSearchFilter(const QString &value);
	bool userClickOk();
	QScriptEngine *engine();
	void setEngine (QScriptEngine *engine);

	void setParentWidget(QWidget *parent);

	Q_INVOKABLE void addPkValue(const QString &field, const QVariant &data);
	Q_INVOKABLE QScriptValue selectedBean();

	static QScriptValue toScriptValue(QScriptEngine *engine, const PERPScriptDialogPointer &in);
	static void fromScriptValue(const QScriptValue &object, PERPScriptDialogPointer &out);

signals:

public slots:
	void show();

};

Q_DECLARE_METATYPE(PERPScriptDialogPointer)

#endif // PERPSCRIPTDIALOG_H
