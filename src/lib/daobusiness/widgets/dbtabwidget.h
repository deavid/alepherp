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
#ifndef DBTABWIDGET_H
#define DBTABWIDGET_H

#include <QTabWidget>
#include <QList>
#include <QTabBar>
#include <QVariant>
#include <QShowEvent>
#include <QScriptValue>
#include <alepherpglobal.h>
#include "dao/basedao.h"

/**
  Este control presentará tantas tabs como registros tenga la tabla tableName pasada a el mismo.
  El campo que se visualizará será el indicado en fieldView. Cuando además se produzca un cambio
  de tab, se generará una señal, tabChanged que emitirá la primary key lanzada
	@author David Pinelo <david.pinelo@alephsistemas.es>
*/
class Q_ALEPHERP_EXPORT DBTabWidget : public QTabWidget
{
	Q_OBJECT
	Q_PROPERTY (QString tableName READ tableName WRITE setTableName)
	Q_PROPERTY (QString fieldView READ fieldView WRITE setFieldView)
	Q_PROPERTY (QString filter READ filter WRITE setFilter)
	Q_PROPERTY (QString order READ order WRITE setOrder)

private:
	/** Tabla de cuyos registros se tomarán los datos */
	QString m_tableName;
	/** Campo que se visualizará en el tab text */
	QString m_fieldView;
	/** Filtro que se aplica para obtener los registros de la tabla */
	QString m_filter;
	/** Columna que marca el orden con el que se presentan los datos */
	QString m_order;
	/** Contiene los beans que soportan la visualización */
	BaseBeanPointerList m_list;

protected:
	void showEvent (QShowEvent * event);

public:
	DBTabWidget(QWidget * parent);
	DBTabWidget(const QString &dbField, QWidget * parent);

	~DBTabWidget();

	QString tableName();
	void setTableName (const QString &value);
	QString fieldView();
	void setFieldView(const QString &field);
	QString filter();
	void setFilter(const QString &value);
	QString order();
	void setOrder(const QString &value);

	void addTab(const QVariant &id, const QString & nombre);

	Q_INVOKABLE QVariant idCurrentTab(void);

	static QScriptValue toScriptValue(QScriptEngine *engine, DBTabWidget * const &in);
	static void fromScriptValue(const QScriptValue &object, DBTabWidget * &out);

public slots:
	void init();
	void clear();

protected slots:
	void tabCambiada(int);

signals:
	/** Cuando se cambia de tab, devuelve el primary key del tab */
	void tabChanged(QVariant);
	void tabChanged();
};

Q_DECLARE_METATYPE(DBTabWidget*)
Q_SCRIPT_DECLARE_QMETAOBJECT(DBTabWidget, DBTabWidget*)

#endif
