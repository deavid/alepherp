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
#include "beansfactory.h"
#include "configuracion.h"
#include "dao/basedao.h"
#include "dao/systemdao.h"
#include "dao/beans/basebean.h"
#include "dao/beans/basebeanmetadata.h"
#include "dao/beans/dbrelation.h"
#include "dao/beans/dbrelationmetadata.h"
#include "dao/database.h"
#include "models/envvars.h"
#include <QApplication>
#include <QMessageBox>
#include <QDir>
#include <QFile>

QList<BaseBeanMetadata *> BeansFactory::metadatasBeans;
QStringList BeansFactory::tableWidgets;
QStringList BeansFactory::tableReports;
QHash<QString, QString> BeansFactory::tableWidgetsScripts;
QHash<QString, bool> BeansFactory::tableWidgetsScriptsDebug;
QHash<QString, bool> BeansFactory::tableWidgetsScriptsDebugOnInit;

#define MSG_NO_EXISTE_UI QT_TR_NOOP("No existe un fichero UI en base de datos con la defición del formulario de búsqueda")


BeansFactory::BeansFactory(QObject *parent) : QObject(parent)
{
}

BeansFactory::~BeansFactory()
{
}

/*!
  Esta función crea una estructura con una copia maestra de todos los metadatas de beans existentes
  en la aplicación
  */
bool BeansFactory::buildMetadatasBeans()
{
	QStringList nombres, xml;
	bool result = false;

	if ( SystemDAO::getAllXMLSystemTable(nombres, xml) ) {
		for ( int i = 0 ; i < nombres.size() ; i++ ) {
			BaseBeanMetadata *metadata;
			metadata = new BaseBeanMetadata();
			metadata->setParent(qApp);
			metadata->setTableName(nombres.at(i));
			metadata->setXml(xml.at(i));
			BeansFactory::metadatasBeans.append(metadata);
		}
		result = true;
	}
	return result;
}

/*!
  Esta función almacena en el directorio temporal todos los widgets que se crean desde fuera
  */
bool BeansFactory::buildTableWidgets()
{
	QStringList uis, names;
	bool result = false;

	if ( SystemDAO::getAllUISystem(names, uis) ) {
		for ( int i = 0 ; i < names.size() ; i++ ) {
			QString fileName = QString("%1/%2").
							   arg(QDir::fromNativeSeparators(configuracion.tempPath())).
								arg(names.at(i));
			QFile file (fileName);
			if ( !file.open( QFile::ReadWrite | QFile::Truncate ) ) {
				QMessageBox::warning(NULL, trUtf8(APP_NAME), trUtf8(MSG_NO_EXISTE_UI),
									 QMessageBox::Ok);
				return false;
			}
			QTextStream out (&file);
			//out.setCodec(configuracion.fileSystemCodec());
			out.setCodec("UTF-8");
			out << uis.at(i);
			file.flush();
			file.close();
			// Ahora comprobamos que el archivo se ha guardado correctamente
			if ( file.open(QFile::ReadOnly) ) {
				BeansFactory::tableWidgets << names.at(i);
			} else {
				qDebug() << "BeansFactory::buildTableWidgets: No se ha creado el fichero: " << fileName;
				QString message = trUtf8("No se ha podido crear el fichero: %1").arg(fileName);
				QMessageBox::warning(NULL, trUtf8(APP_NAME), message, QMessageBox::Ok);
				return false;
			}
			file.close();
		}
		result = true;
	}
	return result;
}

bool BeansFactory::buildScripts()
{
	QStringList scripts, names;
	QList<bool> debugFlags, onInitDebugFlags;

	if ( SystemDAO::getAllQSSystem(names, scripts, debugFlags, onInitDebugFlags) ) {
		for ( int i = 0 ; i < names.size() ; i++ ) {
			if ( names.at(i) == "__init__.js" || names.at(i).contains("/") ) {
				QString fileName, dir, fullDir, fullFileName;
				if ( names.at(i) == "__init__.js" ) {
					dir = "script/";
					fileName = "__init__.js";
				} else {
					QStringList tree = names.at(i).split("/");
					for (int i = 0 ; i < tree.size() - 1 ; i++ ) {
						if ( dir.isEmpty() ) {
							dir = QString("script/%1").arg(tree.at(i));
						} else {
							dir = QString("%1/%2").arg(dir).arg(tree.at(i));
						}
					}
					fileName = tree.at(tree.size()-1);
				}
				fullDir = QString("%1/%2").arg(configuracion.tempPath()).arg(dir);
				QDir initDir(fullDir);
				if ( !initDir.exists() ) {
					initDir.setPath(configuracion.tempPath());
					if ( !initDir.mkpath(dir) ) {
						QMessageBox::warning(NULL, trUtf8(APP_NAME), trUtf8("No se pudo crear el subdirectorio: ").arg(fullDir), QMessageBox::Ok);
						return false;
					}
				}
				fullFileName = QString("%1%2").arg(fullDir).arg(fileName);
				QFile file (fullFileName);
				if ( !file.open( QFile::ReadWrite | QFile::Truncate ) ) {
					QMessageBox::warning(NULL, trUtf8(APP_NAME), trUtf8("No se pudo crear el archivo: ").arg(fullDir),
										 QMessageBox::Ok);
					return false;
				}
				QTextStream out (&file);
				//out.setCodec(configuracion.fileSystemCodec());
				out.setCodec("UTF-8");
				out << scripts.at(i);
				file.flush();
				file.close();
			} else {
				BeansFactory::tableWidgetsScripts[names.at(i)] = scripts.at(i);
				BeansFactory::tableWidgetsScriptsDebug[names.at(i)] = debugFlags.at(i);
				BeansFactory::tableWidgetsScriptsDebugOnInit[names.at(i)] = onInitDebugFlags.at(i);
			}
		}
	}
	return true;
}

/*!
  Cargamos y guardamos los informes que se hayan almacenado en base de datos
  */
bool BeansFactory::buildTableReports()
{
	QStringList reports, names;
	bool result = false;

	if ( SystemDAO::getAllReportSystem(names, reports) ) {
		for ( int i = 0 ; i < names.size() ; i++ ) {
			QString fileName = QString("%1/%2").
							   arg(QDir::fromNativeSeparators(configuracion.tempPath())).
								arg(names.at(i));
			QFile file (fileName);
			if ( !file.open( QFile::ReadWrite | QFile::Truncate ) ) {
				QMessageBox::warning(NULL, trUtf8(APP_NAME), trUtf8(MSG_NO_EXISTE_UI),
									 QMessageBox::Ok);
				return false;
			}
			QTextStream out (&file);
			out.setCodec("UTF-8");
			out << reports.at(i);
			file.flush();
			file.close();
			// Ahora comprobamos que el archivo se ha guardado correctamente
			if ( file.open(QFile::ReadOnly) ) {
				BeansFactory::tableReports << names.at(i);
			} else {
				qDebug() << "BeansFactory::buildTableWidgets: No se ha creado el fichero: " << fileName;
				QString message = trUtf8("No se ha podido crear el fichero: %1").arg(fileName);
				QMessageBox::warning(NULL, trUtf8(APP_NAME), message, QMessageBox::Ok);
				return false;
			}
			file.close();
		}
		result = true;
	}
	return result;
}

BeansFactory * BeansFactory::instance()
{
	static BeansFactory* singleton = 0;
	if ( singleton == 0 ) {
		singleton = new BeansFactory();
		// Esto garantiza que el objeto se borra al cerrar la aplicación
		singleton->setParent(qApp);
	}
	return singleton;
}

/*!
  Función de la factoría que crea los objetos necesarios. Es posible especificar
  si se crearan los valores por defecto
  */
QSharedPointer<BaseBean> BeansFactory::newQBaseBean(const QString & tableName, bool setDefaultValue)
{
	// Los punteros QSharedPointer no se les establece el padre, ya que se produciría una duplicidad
	// de responsables en el borrado. Ver ejemplo en
	// http://blog.codef00.com/2011/12/15/not-so-much-fun-with-qsharedpointer/
	QSharedPointer<BaseBean> bean = QSharedPointer<BaseBean> (newBaseBean(tableName, setDefaultValue, false));
	if ( !bean.isNull() ) {
		QList<DBRelation *> rels = bean->relations();
		foreach ( DBRelation *rel, rels ) {
			if ( rel->metadata()->type() == DBRelationMetadata::ONE_TO_MANY ) {
				rel->setFatherForM1Relation(bean);
			}
		}
	}
	return bean;
}

/*!
  Función de la factoría que crea los objetos necesarios
  */
BaseBean * BeansFactory::newBaseBean(const QString & tableName, bool setDefaultValue, bool setDefaultParent)
{
	BaseBean *obj = 0;

	foreach (BaseBeanMetadata * metadata, BeansFactory::metadatasBeans) {
		if (metadata->tableName() == tableName) {
			obj = new BaseBean();
			if ( setDefaultParent ) {
				obj->setParent(this);
			}
			obj->setMetadata(metadata, setDefaultValue);
			// Asignamos valores únicos a los campos seriales, por si hay que encontrar
			// e identificar a un bean y no ha pasado por base de datos
			obj->setSerialUniqueId();
		}
	}
	return obj;
}

bool BeansFactory::init()
{
	if ( !Database::createSystemConnection() ) {
		return false;
	}

	// Necesitamos una copia de beans maestros de la aplicación
	if ( BeansFactory::metadatasBeans.isEmpty() ) {
		if ( !buildMetadatasBeans() ) {
			return false;
		}
	}
	if ( BeansFactory::tableWidgets.isEmpty() ) {
		if ( !buildTableWidgets() ) {
			return false;
		}
	}
	if ( BeansFactory::tableWidgetsScripts.isEmpty() ) {
		if ( !buildScripts() ) {
			return false;
		}
	}
	if ( BeansFactory::tableReports.isEmpty() ) {
		if ( !buildTableReports() ) {
			return false;
		}
	}
	EnvVars::clear();
    BaseDAO::loadEnvVars();
	return true;
}

/*!
  Comprueba que todas las definiciones de beans y de tablas de sistema existen en la base
  de datos. Se descarga además las ultimas versiones del código de negocio.
  */
bool BeansFactory::checkTableSystem()
{
	if ( !SystemDAO::checkSystemTable() ) {
		return false;
	}
	return true;
}

/*!
  Obtiene el master bean de la tabla name
  */
BaseBeanMetadata * BeansFactory::metadataBean(const QString &name)
{
	foreach (BaseBeanMetadata *metadata, BeansFactory::metadatasBeans ) {
		if ( metadata->tableName() == name ) {
			return metadata;
		}
	}
	return NULL;
}
