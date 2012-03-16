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
#include "perpscriptwidget.h"
#include "configuracion.h"
#include "dao/beans/beansfactory.h"
#include "dao/beans/basebean.h"
#include "dao/observerfactory.h"
#include "dao/beans/dbrelation.h"
#include "forms/dbrecorddlg.h"
#include <QDir>
#include <QUiLoader>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QTimer>
#include <QCoreApplication>

class PERPScriptWidgetPrivate
{
public:
	Q_DECLARE_PUBLIC(PERPScriptWidget)
	/** Nombre del widget para buscar en base de datos el .ui y el .qs que le darán funcionalidad.
	  Si m_name está vacío se utilizará objectName() */
	QString m_name;
	/** Bean cuyo contenido mostrará o sobre el que trabajará este widget */
	QSharedPointer<BaseBean> m_bean;
	/** Motor para los scripts */
	PERPScript m_engine;
	/** QWidget que contendrá el objeto creado por el UI */
	QWidget *m_widget;
	/** Indica si se ha iniciado la ejecución del script */
	bool m_init;
	/** Nombre de la clase en QS que dará la funcionalidad */
	QString m_qsClassName;
	PERPScriptWidget *q_ptr;

	PERPScriptWidgetPrivate(PERPScriptWidget * qq) : q_ptr(qq) {
		m_widget = NULL;
		m_init = false;
	}

	bool setupWidget();
	QDialog *parentDialog();
};

PERPScriptWidget::PERPScriptWidget(QWidget *parent) :
	QWidget(parent), d(new PERPScriptWidgetPrivate(this))
{
}

PERPScriptWidget::~PERPScriptWidget()
{
	emit destroyed(this);
	delete d;
}

void PERPScriptWidget::emitSignal(const QString &signalName, const QVariant &value)
{
	emit signalEmitted(signalName, value);
}

void PERPScriptWidget::showEvent(QShowEvent *event)
{
	if ( d->m_init ) {
		refresh();
	} else {
		// Se utiliza el evento espontáneo que se emite cuando ya es visible el widget
		QTimer::singleShot(20, this, SLOT(init()));
	}
	if ( event->spontaneous() ) {
		// Cargamos las dimensiones guardadas de la ventana
		configuracion.applyDimensionForm(this);
	}
	QWidget::showEvent(event);
}

BaseBean * PERPScriptWidget::bean()
{
    if ( m_observer == NULL ) {
        return NULL;
    }
    BaseBean *bean = qobject_cast<BaseBean *>(m_observer->entity());
    return bean;
}

void PERPScriptWidget::setName(const QString &value)
{
	d->m_name = value;
}

QString PERPScriptWidget::name()
{
	if ( d->m_name.isEmpty() ) {
		return objectName();
	}
	return d->m_name;
}

QString PERPScriptWidget::qsClassName()
{
	return d->m_qsClassName;
}

void PERPScriptWidget::setQsClassName(const QString &value)
{
	d->m_qsClassName = value;
}

AbstractObserver * PERPScriptWidget::observer()
{
    if ( m_observer == NULL ) {
        BaseBean *containerBean = getBeanFromContainer();
        if ( containerBean == NULL ) {
            m_observer = NULL;
        } else {
            DBRelation *rel = containerBean->relation(this->m_relationName);
            if ( rel == NULL ) {
                m_observer = ObserverFactory::instance()->registerBaseWidget(this, containerBean);
            } else {
                QSharedPointer<BaseBean> childBean = rel->childByFilter(m_relationFilter);
                m_observer = ObserverFactory::instance()->registerBaseWidget(this, childBean.data());
            }
            if ( m_observer != NULL ) {
                applyFieldProperties();
                m_observer->sync();
            }
        }
    }
    return m_observer;
}

/*!
  Esta función incia la funcionalidad de este widget
  */
bool PERPScriptWidget::init(bool setupWidget)
{
	QString qsName = QString ("%1.widget.qs").arg(name());
	QString mensaje = trUtf8("Ha ocurrido un error al cargar el script asociado al widget "
							 "%1. Es posible que algunas funciones no estén disponibles.").arg(name());
	QDialog *dlg = d->parentDialog();
	if ( dlg == NULL ) {
		return false;
	}

	observer();
	if ( setupWidget ) {
		if ( !d->setupWidget() ) {
			return false;
		}
		/** Ejecutamos el script asociado. La filosofía fundamental de ese script es proporcionar
		  algo de código básico que justifique el funcionamiento de este widget */
		if ( !BeansFactory::tableWidgetsScripts.contains(qsName) ) {
			return false;
		}

		d->m_engine.setScriptName(qsName);
		d->m_engine.setDebug(BeansFactory::tableWidgetsScriptsDebug.value(qsName));
		d->m_engine.setOnInitDebug(BeansFactory::tableWidgetsScriptsDebugOnInit.value(qsName));

		if ( d->m_qsClassName.isEmpty() ) {
			d->m_engine.setScriptObject("Widget");
		} else {
			d->m_engine.setScriptObject(d->m_qsClassName);
		}
		d->m_engine.setUi(d->m_widget);
		d->m_engine.addAvailableObject("thisWidget", this);
        if ( m_observer != NULL ) {
            BaseBean *bean = qobject_cast<BaseBean *>(m_observer->entity());
            d->m_engine.addAvailableObject("bean", bean);
        }
		if ( dlg != NULL ) {
			d->m_engine.addAvailableObject("thisForm", dlg);
		}

		if ( !d->m_engine.createQsObject() ) {
			QMessageBox::warning(this, trUtf8(APP_NAME), mensaje, QMessageBox::Ok);
	#if !defined(QT_NO_SCRIPTTOOLS)
			int ret = QMessageBox::information(this, trUtf8(APP_NAME), trUtf8("El script ejecutado contiene errores. ¿Desea editarlo?"),
											   QMessageBox::Yes | QMessageBox::No);
			if ( ret == QMessageBox::Yes ) {
				d->m_engine.editScript(this);
			}
	#endif
			return false;
		}
	}
	d->m_init = true;
	emit ready();
	return true;
}

QDialog * PERPScriptWidgetPrivate::parentDialog()
{
	QDialog *dlg = NULL;
	QObject *tmp = q_ptr->parent();
	while ( dlg == NULL && tmp != NULL ) {
		dlg = qobject_cast<QDialog *> (tmp);
		tmp = tmp->parent();
	}
	return dlg;
}

/*!
  Lee el widget de base de datos (el UI) y lo establece
  */
bool PERPScriptWidgetPrivate::setupWidget()
{
	QUiLoader uiLoader;
    QString pluginDir = QString("%1/plugins/designer").arg(qApp->applicationDirPath());
	QString fileName = QString("%1/%2.widget.ui").
					   arg(QDir::fromNativeSeparators(configuracion.tempPath())).
						arg(q_ptr->name());
	QFile file (fileName);
	QString mensaje = QObject::trUtf8("No se ha podido cargar la interfaz de usuario del objeto %1. "
									  "Existe un problema en la definición de las tablas de sistema de su programa.").
			arg(q_ptr->name());
	bool result = true;

	if ( file.exists() ) {
		uiLoader.addPluginPath(pluginDir);
		file.open( QFile::ReadOnly );
		m_widget = uiLoader.load(&file, q_ptr);
		if ( m_widget != NULL ) {
			m_widget->setParent(q_ptr);
			QVBoxLayout *layout = new QVBoxLayout;
			layout->setContentsMargins(0, 0, 0, 0);
			layout->setSpacing(0);
			layout->addWidget(m_widget);
			q_ptr->setLayout(layout);
			// Ahora instalamos los filtros que el diálogo principal necesita
			// para controlar a estos widgets (y la modificación por parte del usuario)
			QDialog *dlg = parentDialog();
			if ( dlg != NULL ) {
				QList<QWidget *> childs = m_widget->findChildren<QWidget *>();
				foreach ( QWidget *child, childs ) {
					if ( child->property("perpControl").toBool() ) {
						child->installEventFilter(dlg);
					}
				}
			}
		} else {
			QMessageBox::warning(q_ptr, QObject::trUtf8(APP_NAME), mensaje, QMessageBox::Ok);
			result = false;
		}
	} else {
		QMessageBox::warning(q_ptr, QObject::trUtf8(APP_NAME), mensaje, QMessageBox::Ok);
		result = false;
	}
	file.close();
	return result;
}

/*!
  Establece el valor a mostrar en el control
*/
void PERPScriptWidget::setValue(const QVariant &value)
{
	Q_UNUSED(value)
}

/*!
 Devuelve el valor mostrado o introducido en el control
*/
QVariant PERPScriptWidget::value()
{
	return QVariant();
}

/*!
 Ajusta el control y sus propiedades a lo definido en el field
*/
void PERPScriptWidget::applyFieldProperties()
{
}

/*!
	Para refrescar los controles: Piden nuevo observador si es necesario
*/
void PERPScriptWidget::refresh()
{
	if ( d->m_init ) {
		observer();
		if ( m_observer != NULL ) {
			BaseBean *bean = qobject_cast<BaseBean *>(m_observer->entity());
			d->m_engine.replaceAvailabelObject("bean", bean);
		}
		callMethod("refresh");
	}
}

/*!
  Permite llamar a un método de la clase que controla al widget. Muy interesante
  para obtener valores determinados del widget.
  */
QScriptValue PERPScriptWidget::callMethod(const QString &method)
{
	QScriptValue result;
	if ( !d->m_init ) {
		init();
	}
	d->m_engine.callQsObjectFunction(result, method);
	return result;
}

void PERPScriptWidget::closeEvent ( QCloseEvent * event )
{
	// Guardamos las dimensiones del usuario
	configuracion.saveDimensionForm(this);
	event->accept();

}

void PERPScriptWidget::observerUnregistered()
{
	DBBaseWidget::observerUnregistered();
	blockSignals(true);
	d->m_engine.replaceAvailabelObject("bean", NULL);
	blockSignals(false);
}

void PERPScriptWidget::setRelationName(const QString &name)
{
	if ( m_relationName != name ) {
		DBBaseWidget::setRelationName(name);
		init(false);
	}
}

void PERPScriptWidget::setRelationFilter(const QString &name)
{
	if ( m_relationFilter != name ) {
		DBBaseWidget::setRelationFilter(name);
		init(false);
	}
}

/**
	Cuando el texto recibe el foco, se le eliminan todos los caracteres menos números y punto.
	Es decir, mostramos en el texto, el numerito almacenado internamente, pero sin locale.
 */
void PERPScriptWidget::focusInEvent(QFocusEvent * event)
{
	if ( event->reason() == Qt::TabFocusReason || event->reason() == Qt::ShortcutFocusReason ) {
		if ( d->m_widget != NULL ) {
			QObjectList list = d->m_widget->children();
			foreach (QObject *obj, list) {
				QWidget *wid = qobject_cast<QWidget *>(obj);
				if ( wid != NULL && wid->focusPolicy() != Qt::NoFocus ) {
					qDebug() << obj->objectName();
					qDebug() << obj->metaObject()->className();
					wid->setFocus();
					return;
				}
			}
		}
	}
}
