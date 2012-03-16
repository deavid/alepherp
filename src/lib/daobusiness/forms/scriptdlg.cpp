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
#include "scriptdlg.h"
#include "configuracion.h"
#include "scripts/perpscriptwidget.h"
#include "forms/perpbasedialog_p.h"
#include "dao/beans/beansfactory.h"
#include <QDir>
#include <QUiLoader>
#include <QMessageBox>
#include <QCloseEvent>
#include <QCoreApplication>

class ScriptDlgPrivate
{
//	Q_DECLARE_PUBLIC(ScriptDlg)
public:
	/** Widget principal */
	QWidget *m_widget;
	/** Nombre del UI de base de datos */
	QString m_ui;
	/** Nombre del QS en base de datos */
	QString m_qs;

	ScriptDlgPrivate() {
		m_widget = NULL;
	}
};

ScriptDlg::ScriptDlg(const QString &uiName, const QString &qsName, QWidget* parent, Qt::WFlags fl) :
	PERPBaseDialog(parent, fl), d(new ScriptDlgPrivate)
{
	d->m_qs = qsName;
	d->m_ui = uiName;
	if ( !init() ) {
		close();
		setOpenSuccess(false);
		return;
	}
	setOpenSuccess(true);
}

bool ScriptDlg::init()
{
	// Nombre único para identificar las propiedades de este formulario
	setObjectName(QString("%1%2").arg(objectName()).arg(d->m_ui));

	setWindowFlags(windowFlags() | Qt::WindowMinMaxButtonsHint |
			Qt::WindowSystemMenuHint | Qt::WindowContextHelpButtonHint);
	// Leemos y establecemos de base de datos los widgets de este form
	setupMainWidget();

	// Código propio del formulario
	execQs();
	return true;
}

ScriptDlg::~ScriptDlg()
{
	delete d;
}

void ScriptDlg::showEvent(QShowEvent * event)
{
	Q_UNUSED(event)
	PERPBaseDialog::showEvent(event);
}

void ScriptDlg::closeEvent(QCloseEvent * event)
{
	PERPBaseDialog::closeEvent(event);
	event->accept();
}

/*!
  Carga el formulario ui definido en base de datos, y que define la interfaz de usuario. Puede haber
  dos formularios: nombre_tabla.new.dbrecord.ui que se utilza para insertar un nuevo registro
  o nombre_tabla.dbrecord.ui que se utiliza para editar y para insertar un nuevo registro
  si nombre_tabla.new.dbrecord.ui no existe
  */
void ScriptDlg::setupMainWidget()
{
	QString fileName = QString("%1/%2").
			arg(QDir::fromNativeSeparators(configuracion.tempPath())).
			 arg(d->m_ui);
	QUiLoader uiLoader;
    QString pluginDir = QString("%1/plugins/designer").arg(qApp->applicationDirPath());

	if ( QFile::exists(fileName) ) {
		QFile file (fileName);
		uiLoader.addPluginPath(pluginDir);
		file.open( QFile::ReadOnly );
		d->m_widget = uiLoader.load(&file, 0);
		if ( d->m_widget != NULL ) {
			d->m_widget->setParent(this);
			QVBoxLayout *lay = new QVBoxLayout(this);
			lay->addWidget(d->m_widget);
			this->setLayout(lay);
		} else {
			QMessageBox::warning(this, trUtf8(APP_NAME), trUtf8("No se ha podido cargar la interfaz de usuario de este formulario. Existe un problema en la definición de las tablas de sistema de su programa."),
								 QMessageBox::Ok);
			close();
		}
		file.close();
	}
}

/*!
  Este formulario puede contener cierto código script a ejecutar en su inicio. Esta función lo lanza
  inmediatamente. El código script está en presupuestos_system, con el nombre de la tabla principal
  acabado en dbform.qs
  */
void ScriptDlg::execQs()
{
	QString qsName = d->m_qs;

	/** Ejecutamos el script asociado. La filosofía fundamental de ese script es proporcionar
	  algo de código básico que justifique este formulario de edición de registros */
	if ( !BeansFactory::tableWidgetsScripts.contains(qsName) ) {
		return;
	}
    engine()->setScriptName(qsName);
    engine()->setDebug(BeansFactory::tableWidgetsScriptsDebug.value(qsName));
    engine()->setOnInitDebug(BeansFactory::tableWidgetsScriptsDebugOnInit.value(qsName));
    engine()->setScriptObject("ScriptDlg");
    engine()->setUi(d->m_widget);
    engine()->addAvailableObject("thisForm", this);
    if ( !engine()->createQsObject() ) {
		QMessageBox::warning(this, trUtf8(APP_NAME), trUtf8("Ha ocurrido un error al cargar el script asociado a este "
															"formulario. Es posible que algunas funciones no est\303\251n disponibles."),
							 QMessageBox::Ok);
#if !defined(QT_NO_SCRIPTTOOLS)
		int ret = QMessageBox::information(this, trUtf8(APP_NAME), trUtf8("El script ejecutado contiene errores. ¿Desea editarlo?"),
										   QMessageBox::Yes | QMessageBox::No);
		if ( ret == QMessageBox::Yes ) {
            engine()->editScript(this);
		}
#endif
	}
}

void ScriptDlg::keyPressEvent (QKeyEvent * e)
{
	bool accept = true;
	if ( e->modifiers() == Qt::ControlModifier ) {
	} else if ( e->key() == Qt::Key_Escape ) {
		close();
	} else {
		accept = false;
	}
	if ( accept ) {
		e->accept();
	}
}
