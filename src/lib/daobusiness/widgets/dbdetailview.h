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
#ifndef DBDETAILVIEW_H
#define DBDETAILVIEW_H

#include <QWidget>
#include <QSignalMapper>
#include <QItemSelectionModel>
#include <QAbstractItemView>
#include <QScriptValue>
#include <alepherpglobal.h>
#include "widgets/dbabstractview.h"
#include "dao/observerfactory.h"

class DBRelation;
class RelationBaseBeanModel;
class RelationChildsFilterModel;
class AbstractObserver;
class DBDetailViewPrivate;

namespace Ui {
    class DBDetailView;
}

/**
  Presenta y permite modificar los datos de una relación DBRelation contenida en un
  BaseBean. Es ideal para editar los registros descendientes. Presenta tres botones
  que abren un diálogo si lo hubiera, y permiten agregar, modificar o eliminar
  registros.

  De por sí este componente no ejecuta ninguna acción en base de datos. El almacenamiento
  o no en base de datos, dependen del procesamiento que se realice en el BaseBean Padre.

  Este plugin, y esta clase, junto con las otras clases utilizadas están escritas MUY bien,
  siguiendo todos los paradigmas de POO y de patrones de diseño. Es por ejemplo, muy destacable
  la utilización de los filtros y QItemSelectionModel para todas las interacciones con el modelo.

  @see BaseBean
  @see DBRelation
  @author David Pinelo <david.pinelo@alephsistemas.es>
  */

class Q_ALEPHERP_EXPORT DBDetailView : public QWidget, public DBAbstractViewInterface
{
    Q_OBJECT
	Q_PROPERTY (QString relationName READ relationName WRITE setRelationName)
	Q_PROPERTY (QString relationFilter READ relationFilter WRITE setRelationFilter)
	Q_PROPERTY (bool dataEditable READ dataEditable WRITE setDataEditable)
	Q_PROPERTY (bool perpControl READ perpControl)
	Q_PROPERTY (QString order READ order WRITE setOrder)
	Q_PROPERTY (QAbstractItemView::EditTriggers editTriggers READ editTriggers WRITE setEditTriggers)
	/** La edición de registros se realizará directamente sobre los DBDetailView de tal modo
	  que no aparecerá el botón Editar */
	Q_PROPERTY (bool inlineEdit READ inlineEdit WRITE setInlineEdit)

protected:
	void showEvent(QShowEvent *event);
	void hideEvent(QHideEvent *event) { DBBaseWidget::hideEvent(event); QWidget::hideEvent(event); }

private:
	Ui::DBDetailView *ui;
	DBDetailViewPrivate *d;
	Q_DECLARE_PRIVATE(DBDetailView)

	void setupInternalModel();

public:
    explicit DBDetailView(QWidget *parent = 0);
    ~DBDetailView();

	int observerType() { return OBSERVER_DBRELATION; }
	void applyFieldProperties();
	QVariant value() { return QVariant(); }

	QAbstractItemView::EditTriggers editTriggers () const;
	void setEditTriggers (QAbstractItemView::EditTriggers triggers);
	bool inlineEdit () const;
	void setInlineEdit(bool value);

	static QScriptValue toScriptValue(QScriptEngine *engine, DBDetailView * const &in);
	static void fromScriptValue(const QScriptValue &object, DBDetailView * &out);

public slots:
	void setValue(const QVariant &value);
	void refresh();
	void observerUnregistered();

private slots:
	void editRecord(const QString &insert = "false");
	void deleteRecord();

signals:
	void valueEdited(const QVariant &value);
	void destroyed(QWidget *widget);
};

Q_DECLARE_METATYPE(DBDetailView*)
Q_SCRIPT_DECLARE_QMETAOBJECT(DBDetailView, DBDetailView*)

#endif // DBDETAILVIEW_H
