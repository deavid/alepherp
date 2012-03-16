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
#ifndef BEANSFACTORY_H
#define BEANSFACTORY_H

#include <QObject>
#include <QStringList>
#include <alepherpglobal.h>
#include "dao/basedao.h"

class BaseBean;

/**
  * Patron de diseño Factory para la creacion de todos los beans de la aplicacion
  * @author David Pinelo
  */
class Q_ALEPHERP_EXPORT BeansFactory : public QObject
{
    Q_OBJECT

private:
	static bool buildMetadatasBeans();
	static bool buildTableWidgets();
	static bool buildScripts();
	static bool buildTableReports();


public:
	/** Contiene una copia de todas las posibles definiciones de beans de la aplicación.*/
	static QList<BaseBeanMetadata *> metadatasBeans;
	/** Contiene un puntero a los widgets que definen algunos formularios, y que están
	  en base de datos. Se crea al principio de la aplicación */
	static QStringList tableWidgets;
	/** Los widgets anteriores, tendrán un pequeño código asignado en Qt Script. */
	static QHash<QString, QString> tableWidgetsScripts;
	/** Variables para saber si esos scripts se depuran o no */
	static QHash<QString, bool> tableWidgetsScriptsDebug;
	/** Indica si antes de ejecutar el script, se abre en modo debug */
	static QHash<QString, bool> tableWidgetsScriptsDebugOnInit;
	/** Listado de formularios almacenados en base de datos y disponibles en el sistema */
	static QStringList tableReports;

	static BeansFactory *instance();
	static bool checkTableSystem();
	static bool init();

	QSharedPointer<BaseBean> newQBaseBean(const QString &tableName, bool setDefaultValue = true);
	BaseBean * newBaseBean(const QString &tableName, bool setDefaultValue = true, bool setDefaultParent = true);

	static BaseBeanMetadata * metadataBean(const QString &name);

protected:
	explicit BeansFactory(QObject *parent = 0);
	~BeansFactory();

signals:

public slots:

};

#endif // BEANSFACTORY_H
