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
#include "perpscriptdialog.h"
#include "configuracion.h"
#include "forms/dbsearchdlg.h"
#include "forms/dbrecorddlg.h"
#include "forms/scriptdlg.h"
#include "dao/beans/basebean.h"
#include "dao/beans/basebeanmetadata.h"
#include "dao/beans/beansfactory.h"
#include "dao/basedao.h"
#include <QMessageBox>
#include <QPointer>

class PERPScriptDialogPrivate
{
public:
	/** Tipo del diálogo */
	QString m_type;
	/** Tabla de la que editará los datos */
	QString m_tableName;
	/** Si abrimos un formulario de búsqueda, este será el field que podrá leerse a través
	  de la propiedad fieldSearched tras la ejecución del diálogo */
	QString m_fieldToSearch;
	/** Dato buscado */
	QVariant m_fieldSearched;
	/** Filtro de búsqueda con el que se abrirá el formulario de búsqueda */
	QString m_searchFilter;
	/** Indica si el usuario, en el formulario abierto ejecutó la acción o no */
	bool m_userClickOk;
	/** Valores de la primary key del bean que se editará, si se utiliza el tipo "record" */
	QVariantMap m_pkValues;
	/** Widget que será el padre de los formularios */
	QWidget *m_parentWidget;
	/** Bean seleccionado por el usuario en una búsqueda */
	QSharedPointer<BaseBean> m_selectedBean;
	/** Engine de Javascript */
	QScriptEngine *m_engine;
	/** Código Qs que ejecutará un formulario de tipo Script */
	QString m_qsName;
	/** Código Ui que se creará para el formulario */
	QString m_uiName;

	PERPScriptDialogPrivate() {
		m_engine = NULL;
	}
};


PERPScriptDialog::PERPScriptDialog(QObject *parent) :
	QObject(parent), d(new PERPScriptDialogPrivate)
{
	d->m_parentWidget = NULL;
}

PERPScriptDialog::~PERPScriptDialog()
{
	delete d;
}

/*!
  Tenemos que decirle al motor de scripts, que DBSearchDlg se convierte de esta forma a un valor script
  */
QScriptValue PERPScriptDialog::toScriptValue(QScriptEngine *engine, const PERPScriptDialogPointer &in)
{
	in->setEngine(engine);
	return engine->newQObject(in.data());
}

void PERPScriptDialog::fromScriptValue(const QScriptValue &object, PERPScriptDialogPointer &out)
{
	out = PERPScriptDialogPointer(qobject_cast<PERPScriptDialog *>(object.toQObject()));
}

/*!
  type indicará qué tipo de diálogo se crea:
  "search" para un diálogo de búsqueda
  "record" para la edición de datos
  */
void PERPScriptDialog::setType(const QString &type)
{
	d->m_type = type;
}

QString PERPScriptDialog::type()
{
	return d->m_type;
}

void PERPScriptDialog::setTableName(const QString &tableName)
{
	d->m_tableName = tableName;
}

QString PERPScriptDialog::tableName()
{
	return d->m_tableName;
}

void PERPScriptDialog::setQsName(const QString &name)
{
	d->m_qsName = name;
}

QString PERPScriptDialog::qsName()
{
	return d->m_qsName;
}

void PERPScriptDialog::setUiName(const QString &name)
{
	d->m_uiName = name;
}

QString PERPScriptDialog::uiName()
{
	return d->m_uiName;
}

QString PERPScriptDialog::fieldToSearch()
{
	return d->m_fieldToSearch;
}

void PERPScriptDialog::setFieldToSearch(const QString &value)
{
	d->m_fieldToSearch = value;
}

QVariant PERPScriptDialog::fieldSearched()
{
	return d->m_fieldSearched;
}

void PERPScriptDialog::setFieldSearched(const QVariant &value)
{
	d->m_fieldSearched = value;
}

QString PERPScriptDialog::searchFilter()
{
	return d->m_searchFilter;
}

void PERPScriptDialog::setSearchFilter(const QString &value)
{
	d->m_searchFilter = value;
}

QScriptEngine *PERPScriptDialog::engine()
{
	return d->m_engine;
}

void PERPScriptDialog::setEngine (QScriptEngine *engine)
{
	d->m_engine = engine;
}

bool PERPScriptDialog::userClickOk()
{
	return d->m_userClickOk;
}

void PERPScriptDialog::addPkValue(const QString &field, const QVariant &data)
{
	d->m_pkValues[field] = data;
}

void PERPScriptDialog::setParentWidget(QWidget *parent)
{
	d->m_parentWidget = parent;
}

void PERPScriptDialog::show()
{
	if ( d->m_type == "search" ) {
		newSearchDlg();
	} else if ( d->m_type == "record" ) {
		newRecordDlg();
	} else if ( d->m_type == "script" ) {
		newScriptDlg();
	}
}

void PERPScriptDialog::newSearchDlg()
{
	QPointer<DBSearchDlg> dialog = new DBSearchDlg(d->m_tableName, d->m_parentWidget);
	// No vamos a establecer el flag de borrado automático
	if ( dialog->openSuccess() ) {
		dialog->setModal(true);
		dialog->setCanApplyfilter(false);
		dialog->setFilterData(d->m_searchFilter);
		dialog->init();
		dialog->exec();
		d->m_userClickOk = dialog->userClickOk();
		// Recogemos el campo buscado si lo hay
		QSharedPointer<BaseBean> bean = dialog->selectedBean();
		if ( !bean.isNull() ) {
			d->m_selectedBean = bean;
			d->m_fieldSearched = bean->fieldValue(d->m_fieldToSearch);
		}
	}
	delete dialog;
}

void PERPScriptDialog::newRecordDlg()
{
	bool userSaveData;
	QSharedPointer<BaseBean> bean = BaseDAO::selectByPk(d->m_pkValues, d->m_tableName);
	if ( bean.isNull() ) {
		QMessageBox::warning(d->m_parentWidget, QString::fromUtf8(APP_NAME),
							 trUtf8("Ha ocurrido un error leyendo todos los datos a modificar. No es posible la modificac\303\263n del registro."),
							 QMessageBox::Ok);
		return;
	}
	QPointer<DBRecordDlg> dialog = new DBRecordDlg(bean, &userSaveData, DbRecordOpenType::INSERT, d->m_parentWidget);
	if ( dialog->openSuccess() )  {
		dialog->setAttribute(Qt::WA_DeleteOnClose);
		dialog->setModal(true);
		dialog->exec();
		d->m_userClickOk = userSaveData;
		if ( userSaveData ) {
			if ( !BaseDAO::update(bean, true) ) {
				QMessageBox::warning(0, QString::fromUtf8(APP_NAME),
									 trUtf8("Ha ocurrido un error. No se ha podido guardar los datos que ha modificado."), QMessageBox::Ok);
			}
		}
	} else {
		delete dialog;
	}
}

void PERPScriptDialog::newScriptDlg()
{
	QPointer<ScriptDlg> dialog = new ScriptDlg(d->m_uiName, d->m_qsName, d->m_parentWidget);
	if ( dialog->openSuccess() )  {
		dialog->setAttribute(Qt::WA_DeleteOnClose);
		dialog->setModal(true);
		dialog->exec();
	}
	delete dialog;
}

QScriptValue PERPScriptDialog::selectedBean()
{
	QScriptValue result;
	if ( !d->m_selectedBean.isNull() && d->m_engine != NULL ) {
		BaseBean *bean = BeansFactory::instance()->newBaseBean(d->m_selectedBean->metadata()->tableName());
		if ( BaseDAO::selectByPk(d->m_selectedBean->pkValue(), bean) ) {
			result = d->m_engine->newQObject(bean, QScriptEngine::ScriptOwnership);
		}
	}
	return result;
}
