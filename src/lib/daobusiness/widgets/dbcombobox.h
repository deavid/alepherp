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
#ifndef DBComboBox_H
#define DBComboBox_H

#include <QComboBox>
#include <QString>
#include <QStringList>
#include <QList>
#include <QVariant>
#include <QDebug>
#include <QListView>
#include <QScriptValue>
#include <alepherpglobal.h>
#include "dao/beans/basebean.h"
#include "dao/observerfactory.h"
#include "models/dbbasebeanmodel.h"
#include "widgets/dbbasewidget.h"

class DBComboBoxPrivate;

/**
	@author David Pinelo <david.pinelo@alephsistemas.es>
	Este combo será el utilizado para presentar datos asociados a los items. La idea es
	que cada item, lleve asociado un objeto de la clase BaseBean. Para ello, este combo,
	tiene un método setup, por el que es posible pasarle un objeto de tipo ComboModel, que
	devuelve un conjunto de objetos BaseBean, que son los que poblarán de información
	el combo.

	Notas sobre el despliegue automatico en
	http://aicastell.blogspot.com/2009/08/qt4-qcomboboxes-y-el-foco-del-teclado.html
*/
class Q_ALEPHERP_EXPORT DBComboBox : public QComboBox, public DBBaseWidget
{
	Q_OBJECT
	/** Campo en el que se guardará el valor seleccionado por el usuario. Si se especifica
	  un relationName, entonces este será el fieldName del que depende esa relación, y el valor
	  se almacenará en el bean hijo, en el field relationFieldName y en el bean hijo que pase
	  el filtro relationFilter */
	Q_PROPERTY (QString fieldName READ fieldName WRITE setFieldName)
	Q_PROPERTY (QString relationFilter READ relationFilter WRITE setRelationFilter)
	/** Indica qué tabla poblará de items el combobox */
	Q_PROPERTY (QString listTableModel READ listTableModel WRITE setListTableModel)
	/** Esta será la columna que se visualizará y que el usuario podrá escoger */
	Q_PROPERTY (QString listColumnName READ listColumnName WRITE setListColumnName)
    /** Filtro para el modelo que presenta los Items.
        TODO: OJO, Hay un bug documentado en 4.7.X
          https://bugreports.qt.nokia.com/browse/QTBUG-18215
          que provoca que este mapFromSource pueda no funcionar. Por eso
          hago este workaround cutre, que deja sin posibilidad de filtrar
          los datos de la lista. El mapFromSource deja de funcionar por utilizar
          filterAcceptColumn
    */
    /* Q_PROPERTY (QString listFilter READ listFilter WRITE setListFilter) */
	/** Esta será la columna que se almacenará en el fieldName del bean que da soporte al formulario */
	Q_PROPERTY (QString listColumnToSave READ listColumnToSave WRITE setListColumnToSave)
	Q_PROPERTY (bool dataEditable READ dataEditable WRITE setDataEditable)
	Q_PROPERTY (bool perpControl READ perpControl)
	Q_PROPERTY (bool userModified READ userModified WRITE setUserModified)
	Q_PROPERTY (QVariant value READ value WRITE setValue)
	Q_PROPERTY (int currentIndex READ currentIndex WRITE setCurrentIndex)
	Q_PROPERTY (bool dataFromParentDialog READ dataFromParentDialog WRITE setDataFromParentDialog)

private:
	DBComboBoxPrivate *d;
	Q_DECLARE_PRIVATE(DBComboBox)

	void conexiones();
	void desconexiones();
	void createCompleter();
	void setSelectionModel();
	void init();
	void setModelColumn();

public:
	DBComboBox(QWidget * parent = 0 );

	~DBComboBox();

	int observerType() { return OBSERVER_DBFIELD; }

	QString listTableModel();
	void setListTableModel(const QString &name);
	void setListColumnName (const QString &visibleColumn);
	QString listColumnName ();
	void setListColumnToSave ( const QString &dbColumn );
	QString listColumnToSave ();
	void setListFilter ( const QString &filter );
	QString listFilter ();

	Q_INVOKABLE QSharedPointer<BaseBean> bean(int index);

	// TODO Si utilizamos directamente un QComboBox, QtScript no reconoce directamente estas funciones
	// Hago este truquito mientras
	Q_INVOKABLE void addItem ( const QString & text, const QVariant & userData = QVariant() );
	Q_INVOKABLE QVariant itemData ( int index, int role = Qt::UserRole ) const;
	Q_INVOKABLE QSharedPointer<BaseBean> selectedBean();

	void applyFieldProperties();

	QVariant value();

	int currentIndex() const;
	void setCurrentIndex ( int index );

	static QScriptValue toScriptValue(QScriptEngine *engine, DBComboBox * const &in);
	static void fromScriptValue(const QScriptValue &object, DBComboBox * &out);

protected:
	bool eventFilter(QObject * obj, QEvent * event);
	void focusInEvent(QFocusEvent * event);
	void showEvent(QShowEvent *event);
	void hideEvent(QHideEvent *event) { DBBaseWidget::hideEvent(event); QWidget::hideEvent(event); }

public slots:
	void setCurrentIndexByDbColumn (const QString &dbModelColumn, const QVariant &value);
	void setValue(const QVariant &value);
	void refresh();
	void observerUnregistered();

private slots:
	void itemChanged(int index);
	void itemChanged(const QString &text);
	void userModifiedCombo(int index);

signals:
	/** Esta señal propia, es igual que currentIndexChanged, pero emite el id del bean del item. El id hace referencia
	  a la primary key */
	void valueEdited(const QVariant &value);
	void destroyed(QWidget *widget);
};

Q_DECLARE_METATYPE(DBComboBox*)
Q_SCRIPT_DECLARE_QMETAOBJECT(DBComboBox, DBComboBox*)

#endif
