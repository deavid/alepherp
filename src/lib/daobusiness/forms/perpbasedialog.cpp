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
#include "perpbasedialog.h"
#include "perpbasedialog_p.h"
#include "configuracion.h"
#include "dao/beans/dbfieldmetadata.h"
#include "dao/beans/basebeanmetadata.h"
#include "dao/beans/dbrelationmetadata.h"
#include "dao/beans/beansfactory.h"
#include "forms/registereddialogs.h"
#include "widgets/dbbasewidget.h"
#include "widgets/waitwidget.h"
#include "widgets/dbnumberedit.h"
#include "widgets/dblineedit.h"
#include "widgets/dbcombobox.h"
#include "widgets/dbtextedit.h"
#include "widgets/dbdatetimeedit.h"
#include "widgets/dblistview.h"
#include "widgets/dbfileupload.h"
#include <QCloseEvent>
#include <QtGui>
#include <QGraphicsLinearLayout>

#define TRANSITION_TIME_MS 250

PERPBaseDialog::PERPBaseDialog(QWidget* parent, Qt::WFlags fl) : QDialog(parent, fl), d(new PERPBaseDialogPrivate(this))
{
	RegisteredDialogs::registerDialog(this);
	QMainWindow *main = (QMainWindow *) qApp->property("MainWindowPointer").value<void *>();
	setWindowIcon(main->windowIcon());
	setWindowTitle(main->windowTitle());
}

PERPBaseDialog::~PERPBaseDialog()
{
	RegisteredDialogs::unRegisterDialog(this);
	delete d;
}

PERPScript *PERPBaseDialog::engine()
{
    return &(d->m_engine);
}

QString PERPBaseDialog::tableName()
{
	return d->m_tableName;
}

void PERPBaseDialog::setTableName(const QString &value)
{
	d->m_tableName = value;
}

bool PERPBaseDialog::openSuccess()
{
	return d->m_openSuccess;
}

void PERPBaseDialog::setOpenSuccess(bool value)
{
	d->m_openSuccess = value;
}

void PERPBaseDialog::closeEvent ( QCloseEvent * event )
{
	// Guardamos las dimensiones del usuario
	configuracion.savePosForm(this);
	configuracion.saveDimensionForm(this);
	event->accept();
}

void PERPBaseDialog::showEvent ( QShowEvent * event )
{
	if ( event->spontaneous() ) {
		// Cargamos las dimensiones guardadas de la ventana
		configuracion.applyPosForm(this);
	}
	configuracion.applyDimensionForm(this);
	event->accept();
}

void PERPBaseDialog::show ()
{
	QDialog::show();
	configuracion.applyDimensionForm(this);
}

/*!
  Slot que redimensiona a los valores guardados por la última acción del usuario
  */
void PERPBaseDialog::resizeToSavedDimension()
{
	configuracion.applyPosForm(this);
	configuracion.applyDimensionForm(this);
}

void PERPBaseDialog::hideEvent ( QHideEvent * event )
{
	// Guardamos las dimensiones del usuario
	configuracion.savePosForm(this);
	configuracion.saveDimensionForm(this);
	event->accept();
}

void PERPBaseDialog::showWaitAnimation(bool value, const QString message)
{
	if ( value && d->m_waitWidget == 0 ) {
		d->m_waitWidget = new WaitWidget(message, this);
		d->m_waitWidget->move(rect().center() - (d->m_waitWidget->rect().center() * 2));
		d->m_waitWidget->setParent(this);
		d->m_waitWidget->show();
		setEnabled(false);
	} else if ( !value && d->m_waitWidget != 0 ) {
		setEnabled(true);
		d->m_waitWidget->closeAnimation();
		d->m_waitWidget = 0;
	}
}

/*!
  Para formularios (de búsqueda o edición de datos) crea controles automáticamente para la edición.
  Esta función se utilizará para todas aquellas tablas a editar que no posean ningún archivo .ui
  asociado. Caso de ser un formulario de búsqueda, se indicará mediante searchDlg
  */
void PERPBaseDialog::setupWidgetFromBaseBeanMetadata(BaseBeanMetadata *metadata, QLayout *layoutDestiny,
													 bool showVisibleGridOnly, bool searchDlg)
{
	if ( searchDlg ) {
		d->setupDBSearchDlg(metadata, layoutDestiny, showVisibleGridOnly);
	} else {
		d->setupDBRecordDlg(metadata, layoutDestiny, showVisibleGridOnly);
	}
}

void PERPBaseDialogPrivate::setupDBRecordDlg(BaseBeanMetadata *metadata, QLayout *layoutDestiny,
											 bool showVisibleGridOnly)
{
	foreach (DBFieldMetadata *fld, metadata->fields())	{
		if ( !showVisibleGridOnly || fld->visibleGrid() ) {
			QWidget *wid = NULL;
			if ( !fld->optionsList().isEmpty() ) {
				DBComboBox *cb = new DBComboBox(q_ptr);
				cb->setFieldName(fld->dbFieldName());
				wid = qobject_cast<DBComboBox *>(cb);
			} else {
				if ( fld->type() == QVariant::Int || fld->type() == QVariant::Double ) {
					DBNumberEdit *ne = new DBNumberEdit(q_ptr);
					ne->setDecimalNumbers(fld->partD());
					wid = qobject_cast<QWidget *>(ne);
				} else if ( fld->type() == QVariant::String && !fld->memo() ) {
					DBLineEdit *le = new DBLineEdit(q_ptr);
					wid = qobject_cast<QWidget *>(le);
				} else if ( fld->type() == QVariant::String && fld->memo() ) {
					DBTextEdit *te = new DBTextEdit(q_ptr);
					wid = qobject_cast<QWidget *>(te);
				} else if ( fld->type() == QVariant::Date ) {
					DBDateTimeEdit *de = new DBDateTimeEdit(q_ptr);
					de->setCalendarPopup(true);
					wid = qobject_cast<QWidget *>(de);
                } else if ( fld->type() == QVariant::Pixmap ) {
                    DBFileUpload *fu = new DBFileUpload(q_ptr);
                    wid = qobject_cast<QWidget *>(fu);
                }
			}
			if ( wid != NULL ) {
				QLabel *lbl = new QLabel(q_ptr);
				lbl->setText(fld->fieldName());
				DBBaseWidget *baseWid = dynamic_cast<DBBaseWidget *>(wid);
				baseWid->setFieldName(fld->dbFieldName());
				if ( !fld->readOnly() ) {
					baseWid->setDataEditable(true);
				}
				wid->setObjectName(QString("db_%1").arg(fld->dbFieldName()));
				QHBoxLayout *lay = new QHBoxLayout;
				lay->setSpacing(4);
				lay->addWidget(lbl);
				lay->addWidget(wid);
				layoutDestiny->addItem(lay);
			}
		}
	}
}

/*!
  Construye un formulario de búsqueda automático
  */
void PERPBaseDialogPrivate::setupDBSearchDlg(BaseBeanMetadata *metadata, QLayout *layoutDestiny,
											 bool showVisibleGridOnly)
{
	foreach (DBFieldMetadata *fld, metadata->fields())	{
		if ( !showVisibleGridOnly || fld->visibleGrid() ) {
			QWidgetList widList;
			QList<DBRelationMetadata *> relations = fld->relations();
			DBRelationMetadata *fatherRelation = NULL;
			foreach ( DBRelationMetadata *tmp, relations ) {
				if ( tmp->type() == DBRelationMetadata::MANY_TO_ONE ) {
					fatherRelation = tmp;
				}
			}
			if ( fatherRelation != NULL ) {
				DBListView	*lv = new DBListView(q_ptr);
				lv->setItemCheckBox(true);
				lv->setKeyField(fld->dbFieldName());
				BaseBeanMetadata *fatherMetadata = BeansFactory::metadataBean(fatherRelation->tableName());
				lv->setVisibleField(fatherMetadata->defaultVisualizationField());
				widList << qobject_cast<DBListView *>(lv);
			} else {
				if ( fld->type() == QVariant::Int || fld->type() == QVariant::Double ) {
					widList << createComboOperators(fld->dbFieldName());
					DBNumberEdit *ne = new DBNumberEdit(q_ptr);
					ne->setDecimalNumbers(fld->partD());
					ne->setObjectName(QString("db_%1_1").arg(fld->dbFieldName()));
					ne->setProperty("fieldName", fld->dbFieldName());
					ne->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
					widList << qobject_cast<QWidget *>(ne);
					ne = new DBNumberEdit(q_ptr);
					ne->setDecimalNumbers(fld->partD());
					ne->setObjectName(QString("db_%1_2").arg(fld->dbFieldName()));
					ne->setVisible(false);
					ne->setProperty("fieldName", fld->dbFieldName());
					ne->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
					widList << qobject_cast<QWidget *>(ne);
				} else if ( fld->type() == QVariant::String ) {
					DBLineEdit *le = new DBLineEdit(q_ptr);
					le->setObjectName(QString("db_%1").arg(fld->dbFieldName()));
					le->setProperty("fieldName", fld->dbFieldName());
					le->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
					widList << qobject_cast<QWidget *>(le);
				} else if ( fld->type() == QVariant::Date ) {
					widList << createComboOperators(fld->dbFieldName());
					DBDateTimeEdit *de = new DBDateTimeEdit(q_ptr);
					de->setCalendarPopup(true);
					de->setObjectName(QString("db_%1_1").arg(fld->dbFieldName()));
					de->setDisplayFormat("dd/MM/yyyy");
					de->setProperty("fieldName", fld->dbFieldName());
					de->setDate(configuracion.minimunDate());
					de->setSpecialValueText(QObject::trUtf8("Seleccione fecha"));
					de->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
					widList << qobject_cast<QWidget *>(de);
					de = new DBDateTimeEdit(q_ptr);
					de->setCalendarPopup(true);
					de->setObjectName(QString("db_%1_2").arg(fld->dbFieldName()));
					de->setDisplayFormat("dd/MM/yyyy");
					de->setVisible(false);
					de->setProperty("fieldName", fld->dbFieldName());
					de->setDate(configuracion.minimunDate());
					de->setSpecialValueText(QObject::trUtf8("Seleccione fecha"));
					de->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
					widList << qobject_cast<QWidget *>(de);
				}
			}
			if ( widList.size() > 0 ) {
				QHBoxLayout *lay = new QHBoxLayout;
				lay->setSpacing(4);
				QLabel *lbl = new QLabel(q_ptr);
				lbl->setText(fld->fieldName());
				lay->addWidget(lbl);
				foreach ( QWidget *w, widList ) {
					lay->addWidget(w);
				}
				lay->addSpacerItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
				layoutDestiny->addItem(lay);
			}
		}
	}
}

QComboBox * PERPBaseDialogPrivate::createComboOperators(const QString &fld)
{
	QComboBox *cb = new QComboBox(q_ptr);
	QStringList list;
	list << QObject::trUtf8("Menor que") << QObject::trUtf8("Igual") <<
			QObject::trUtf8("Mayor que") << QObject::trUtf8("Entre los valores");
	cb->addItems(list);
	cb->setObjectName(QString("cb_%1").arg(fld));
	cb->setProperty("fieldName", fld);
	QObject::connect(cb, SIGNAL(currentIndexChanged(int)), q_ptr, SLOT(searchComboChanged(int)));
	return cb;
}

/*!
  Cuando el combobox cambia, puede ser necesario que algunos controles aparezcan o desaparezcan
  */
void PERPBaseDialog::searchComboChanged(int index)
{
	QString fieldName = QObject::sender()->property("fieldName").toString();
	QString controlName = QString("db_%1_2").arg(fieldName);
	QWidget *secondWidget = findChild<QWidget *>(controlName);
	if ( secondWidget != NULL ) {
		if ( index != BETWEEN_ELECTION_INDEX ) {
			secondWidget->setVisible(false);
		} else {
			secondWidget->setVisible(true);
		}
	}
}

/*!
  Comprueba si el usuario tiene permisos para siquiera abrir este formulario
*/
bool PERPBaseDialog::checkPermissionsToOpen()
{
	QHash<QString, QVariant> permissions = qApp->property("permissions").toHash();
	QString p = permissions.value(d->m_tableName).toString();
	if ( p.isEmpty() || ( !p.contains("r") && !p.contains("w") ) ) {
		QMessageBox::warning(this, trUtf8(APP_NAME), trUtf8("No tiene permisos para acceder a estos datos."), QMessageBox::Ok);
		return false;
	}
	return true;
}

/*!
  Permite llamar a un método de la clase que controla al formulario. Muy interesante
  para obtener valores determinados del formulario.
  */
QScriptValue PERPBaseDialog::callMethod(const QString &method)
{
    QScriptValue result;
    d->m_engine.callQsObjectFunction(result, method);
    return result;
}
