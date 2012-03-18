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
#ifndef DBTABLEVIEW_H
#define DBTABLEVIEW_H

#include <QTableView>
#include <QKeyEvent>
#include <QSharedPointer>
#include <QItemEditorFactory>
#include <QAbstractItemModel>
#include <QList>
#include <QScriptValue>
#include <alepherpglobal.h>
#include "widgets/dbabstractview.h"
#include "dao/observerfactory.h"
#include "dao/beans/basebean.h"

class BaseBean;
class RelationBaseBeanModel;
class PERPHtmlDelegate;
class DBTableViewPrivate;
class BaseBeanMetadata;
class DBFieldMetadata;

/**
  Es la extensión básica de QTableView, que añade características especiales para
  poder tratar con BaseBean. Es una tabla usada por otros componentes como
  DBDetailView. Puede presentar datos internos (si internalData está a true, a partir
  del bean del observador adecuado) o externos.

  @see BaseBean
  @see DBDetailView
  @author David Pinelo <david.pinelo@alephsistemas.es>
*/
class Q_ALEPHERP_EXPORT DBTableView : public QTableView, public DBAbstractViewInterface
{
	Q_OBJECT
	/** ¿Se leen los datos de base de dato o del Bean del formulario que contiene este control? */
	Q_PROPERTY(bool internalData READ internalData WRITE setInternalData)

	/** Si los datos se leen de base de datos, entonces se introduce este tableName */
	Q_PROPERTY (QString tableName READ tableName WRITE setTableName DESIGNABLE externalDataPropertyVisible)
	/** Este filtro aplica a los datos que se leen de base de datos */
	Q_PROPERTY (QString filter READ filter WRITE setFilter DESIGNABLE externalDataPropertyVisible)
	Q_PROPERTY (QString order READ order WRITE setOrder DESIGNABLE externalDataPropertyVisible)

	/** El relationName puede venir en la forma: presupuestos_ejemplares.presupuestos_actividades ...
	En ese caso, debemos iterar hasta dar con la relación adecuada. Esa iteración se realiza en
	combinación con el filtro establecido. El filtro en ese caso debe mostrar primero
	la primary key de presupuestos_ejemplares, y después el filtro que determina presupuestos_actividades. En ese
	caso, por ejemplo una combinación válida es:
	relationName: presupuestos_cabecera.presupuestos_ejemplares.presupuestos_actividades
	relationFilter: id_presupuesto=2595;id_numejemplares=24442;id_categoria=2;y más filtros para presupuestos_actividades */
	Q_PROPERTY(QString relationName READ relationName WRITE setRelationName DESIGNABLE internalDataPropertyVisible)
	Q_PROPERTY (QString relationFilter READ relationFilter WRITE setRelationFilter DESIGNABLE internalDataPropertyVisible)

	Q_PROPERTY (bool dataEditable READ dataEditable WRITE setDataEditable)
	Q_PROPERTY (bool perpControl READ perpControl)

	/** Devuelve el objeto bean que se encuentra seleccionado */
	Q_PROPERTY (BaseBeanPointerList selectedBeans READ selectedBeans)

    /** Puede definirse que una columna del listado, tenga un boton de checkbox. Después
      será posible obtener los beans que se han marcado con checkedBeans. Esta propiedad
      define esa columna */
    Q_PROPERTY (QString fieldCheckBox READ fieldCheckBox WRITE setFieldCheckBox)

	/** Por defecto, los DBDetailView tienen un nombre generado automáticamente. Aquí se puede
	  habilitar o desactivar esa característica */
	Q_PROPERTY (bool automaticName READ automaticName WRITE setAutomaticName)
	/** Permite navegar usando el ENTER. Cuando el usuario pincha enter, se va pasando de celda
	  en celda */
	Q_PROPERTY (bool navigateOnEnter READ navigateOnEnter WRITE setNavigateOnEnter)
	/** Navegando con enter, al final de los registros, ¿inserta una nueva fila?*/
	Q_PROPERTY (bool atRowsEndNewRow READ atRowsEndNewRow WRITE setAtRowsEndNewRow)

private:
	DBTableViewPrivate *d;
	Q_DECLARE_PRIVATE(DBTableView)

	void prepareColumns();
	void setupInternalModel();
	void setupExternalModel();

protected:
	void showEvent (QShowEvent * event);
	void keyPressEvent (QKeyEvent * event);
	void closeEvent (QCloseEvent * event);
	void mouseDoubleClickEvent (QMouseEvent * event);

public:
	DBTableView( QWidget * parent = 0 );
	~DBTableView();

	void setRelationName(const QString &name);
	void setTableName(const QString &value);
    void setFilter(const QString &value);

    void setFieldCheckBox(const QString &name);
    QString fieldCheckBox();
	bool navigateOnEnter() const;
	void setNavigateOnEnter(bool value);
	bool atRowsEndNewRow() const;
	void setAtRowsEndNewRow(bool value);

	void setModel(QAbstractItemModel *model);

	/** Devuelve el valor mostrado o introducido en el control */
	QVariant value();
	/** Ajusta el control y sus propiedades a lo definido en el field */
	void applyFieldProperties();

	Q_INVOKABLE QSharedPointer<BaseBean> addBean();
	Q_INVOKABLE void deleteSelectedsBean();
    Q_INVOKABLE BaseBeanPointerList checkedBeans();
	Q_INVOKABLE void setCheckedBeans(BaseBeanPointerList list, bool checked = true);
	Q_INVOKABLE void setCheckedBeansByPk(QVariantList list, bool checked = true);

	void orderColumns(const QStringList &order);
	void sortByColumn(const QString &field, Qt::SortOrder order = Qt::DescendingOrder);

	void setAutomaticName(bool value);
	bool automaticName();

	static QScriptValue toScriptValue(QScriptEngine *engine, DBTableView * const &in);
	static void fromScriptValue(const QScriptValue &object, DBTableView * &out);

protected slots:
	void saveHeaderSize(int logicalSection, int oldSize, int newSize);
	void nextCellOnEnter(const QModelIndex &actualCell);

public slots:
	/** Para refrescar los controles: Piden nuevo observador si es necesario */
	void refresh();
	/** Establece el valor a mostrar en el control */
	void setValue(const QVariant &value);
	void observerUnregistered();
	void saveTableViewColumnOrder(const QStringList &order, const QStringList &sort);
	void checkAllItems(bool checked = true);

signals:
	void valueEdited(const QVariant &value);
	void recordChangedByKey (const QModelIndex &index);
	/** Esta señal indicará cuándo se borra un widget. No se puede usar destroyed(QObject *)
	  ya que cuando ésta se emite, se ha ejecutado ya el destructor de QWidget */
	void destroyed(QWidget *widget);
	void enterPressedOnValidIndex(const QModelIndex &index);
	void doubleClickOnValidIndex(const QModelIndex &index);
};

Q_DECLARE_METATYPE(DBTableView*)
Q_SCRIPT_DECLARE_QMETAOBJECT(DBTableView, DBTableView*)

#endif
