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
#ifndef DBTREEVIEW_H
#define DBTREEVIEW_H

#include <QTreeView>
#include <QScriptValue>
#include <alepherpglobal.h>
#include "dbbasewidget.h"
#include "dao/observerfactory.h"
#include "dao/beans/basebean.h"

class DBTreeViewPrivate;

/**
  Esta clase extiende a QTreeView añadiéndole las propiedades de un DBBaseWidget de esta aplicación.
  Presentar datos anidados.

  @author David Pinelo <david.pinelo@alephsistemas.es>
  */
class Q_ALEPHERP_EXPORT DBTreeView : public QTreeView, public DBBaseWidget
{
    Q_OBJECT
	/** Si no está vacío, el TreeView seleccionará el item correspondiente al valor del field de nombre
	  fieldName del bean asignado al DBRecordDlg en el que se encuentra este control */
	Q_PROPERTY (QString fieldName READ fieldName WRITE setFieldName)
	/** Si no está vacío, marcará el bean o la relación sobre la que se buscará el bean que
	  seleccionará el item */
	Q_PROPERTY (QString relationName READ relationName WRITE setRelationName)
	Q_PROPERTY (QString relationFilter READ relationFilter WRITE setRelationFilter)
	Q_PROPERTY (bool dataEditable READ dataEditable WRITE setDataEditable)
	Q_PROPERTY (bool perpControl READ perpControl)
	/** Tablas referenciadas que se utilizarán para mostrar los datos (es
	  decir, deben existir relaciones de padres a hijas definidas en los XML).
	  Se separan por puntos y coma, por ejemplo: familias;subfamilias;articulos
	  */
	Q_PROPERTY (QString tableNames READ tableNames WRITE setTableNames)
	/** De cada tabla que se muestra de forma jerárquica, este campo indica qué campos
	  se muestran al usuario. Por ejemplo: nombre;nombre;descripcion.
	  Deben tener el mismo número de entradas que m_tableNames y en el mismo orden */
	Q_PROPERTY (QString visibleColumns READ visibleColumns WRITE setVisibleColumns)
	Q_PROPERTY (QString keyColumns READ keyColumns WRITE setKeyColumns)
	/** Filtro para cada nivel del árbol. Los filtros se separan por ";", que determinan
	  cada nivel. Los filtros serán de la forma: */
	Q_PROPERTY (QString filter READ filter WRITE setFilter)
	/** Nombre del nodo raíz */
	Q_PROPERTY (QString rootName READ rootName WRITE setRootName)
	/** Indica si se mostrará alguna imagen. La imagen puede ser de un archivo que se indique,
	  o bien de una imagen que se lea de la propia tabla que muestra los datos. El formato sería:
	  file:/home/david/tmp/imagen.png;field:imagen
		  */
	Q_PROPERTY (QString images READ images WRITE setImages)
	/** Devuelve el objeto bean que se encuentra seleccionado */
	Q_PROPERTY (BaseBeanPointerList selectedBeans READ selectedBeans)
	/** Tamaño de las imágenes en el TreeView */
	Q_PROPERTY (QSize imagesSize READ imagesSize WRITE setImagesSize)
	/** Indica de qué niveles del árbol, se pueden seleccionar items. Esto es útil cuando se utiliza
	  este TreeView como selector de items para agregar a otro contenedor (como un table view). Con esta
	  propiedad se indica que sólo se pueden seleccionar items de los niveles indicados y separados
	  por ;
	  */
	Q_PROPERTY (QString selectLevels READ selectLevels WRITE setSelectLevels)
	Q_PROPERTY (QVariant value READ value WRITE setValue)
	/** Un control puede estar dentro de un PERPScriptWidget. ¿De dónde lee los datos? Los puede leer
	  del bean asignado al propio PERPScriptWidget, o bien, leerlos del bean del formulario base
	  que lo contiene. Esta propiedad marca esto */
	Q_PROPERTY (bool dataFromParentDialog READ dataFromParentDialog WRITE setDataFromParentDialog)

private:
	DBTreeViewPrivate *d;
	Q_DECLARE_PRIVATE(DBTreeView)

	void init();

protected:
	void showEvent ( QShowEvent * event );

public:
	explicit DBTreeView(QWidget *parent = 0);
	~DBTreeView();

	QString tableNames();
	void setTableNames(const QString &value);
	QString visibleColumns();
	void setVisibleColumns(const QString &value);
	QString keyColumns();
	void setKeyColumns(const QString &value);
	QString filter();
	void setFilter(const QString &value);
	QString rootName();
	void setRootName(const QString &value);
	QString images();
	void setImages(const QString &value);
	QSize imagesSize();
	void setImagesSize(const QSize &value);
	BaseBeanPointerList selectedBeans();
	void setSelectLevels(const QString &level);
	QString selectLevels();

	int observerType() { return OBSERVER_DBFIELD; }
	void applyFieldProperties();
	QVariant value();

	Q_INVOKABLE QSharedPointer<BaseBean> selectedBean();

	static QScriptValue toScriptValue(QScriptEngine *engine, DBTreeView * const &in);
	static void fromScriptValue(const QScriptValue &object, DBTreeView * &out);

signals:
	void valueEdited(const QVariant &value);
	void destroyed(QWidget *widget);

public slots:
	void setValue(const QVariant &value);
	void refresh();
	void observerUnregistered();

};

Q_DECLARE_METATYPE(DBTreeView*)
Q_SCRIPT_DECLARE_QMETAOBJECT(DBTreeView, DBTreeView*)

#endif // DBTREEVIEW_H
