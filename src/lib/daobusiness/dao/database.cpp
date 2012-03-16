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
#include "dao/database.h"
#include "dao/beans/beansfactory.h"
#include "configuracion.h"
#include <QCoreApplication>
#include <QDir>
#include <QSqlDriver>
#include <QThread>

#define MSG_DRIVER_NO_DISPONIBLE QT_TR_NOOP("El driver de conexión a la base de datos no está disponible. No se puede iniciar la aplicación.\r\nListado de drivers disponibles: %1")

QString Database::openMessage;


// OJO: En los archivos .cpp, en las definiciones no se debe poner la palabra static. La razón
/*
cannot declare member function 'static int Foo::bar()'' to have static linkage

if you declare a method to be static in your .cc file.

The reason is that static means something different inside .cc files than in class declarations It is really stupid, but the keyword static has three different meanings. In the .cc file, the static keyword means that the function isn't visible to any code outside of that particular file.

This means that you shouldn't use static in a .cc file to define one-per-class methods and variables. Fortunately, you don't need it. In C++, you are not allowed to have static variables or static methods with the same name(s) as instance variables or instance methods. Therefore if you declare a variable or method as static in the class declaration, you don't need the static keyword in the definition. The compiler still knows that the variable/method is part of the class and not the instance.

WRONG

 Foo.h:
 class Foo 
 {
   public: 
     static int bar();
 };
 Foo.cc:
 static int Foo::bar() 
 {
   // stuff
 }

WORKS

 Foo.h:
 class Foo 
 {
   public: 
     static int bar();
 };
 Foo.cc:
 int Foo::bar() 
 {
   // stuff
 }

*/

QMutex Database::mutex;

class DatabasePrivate
{
public:
	DatabasePrivate() {}
};

Database::Database(QObject *parent) : QObject(parent), d(new DatabasePrivate)
{
}

Database::~Database()
{
	delete d;
}

/*!
  Inicia toda la estructura de beans y datos comunes del sistema
  */
bool Database::initSystemsBeans()
{
	if ( BeansFactory::checkTableSystem() ) {
		return BeansFactory::init();
	}
	return false;
}

Database * Database::instance()
{
	static Database* singleton = 0;
	if ( singleton == 0 ) {
		singleton = new Database();
		// Esto garantiza que el objeto se borra al cerrar la aplicación
		singleton->setParent(qApp);
	}
	return singleton;
}

QString Database::systemDatabaseName()
{
	return QString("AlephERPSystem");
}

/*!
  Crea la conexión a la base de datos SQLite de sistema, donde se contiene el código
  a ejecutar por el sistema
  */
bool Database::createSystemConnection()
{
	bool result = false, emptyDatabase = true;
	QString errorDriverNotAvailable = QObject::trUtf8(MSG_DRIVER_NO_DISPONIBLE);
	if ( !QSqlDatabase::isDriverAvailable("QSQLITE") ) {
		QMessageBox::critical(0, APP_NAME, errorDriverNotAvailable, QMessageBox::Ok);
		return false;
	}
	result = openSQLite(Database::systemDatabaseName(), emptyDatabase);
	if ( result && emptyDatabase ) {
		result = Database::createSystemTablesSQLite();
		if ( !result ) {
			QMessageBox::critical(0, APP_NAME, QObject::trUtf8("Se ha producido un error creando los archivos de sistema. No es posible abrir la aplicación."), QMessageBox::Ok);
		}
	}
	return result;
}

bool Database::createConnection(const QString &name)
{
	// Sacamos por salida estándar la lista de Drivers disponibles
    bool result = false;
    QStringList list = QSqlDatabase::drivers();
	qDebug() << "Lista de drivers BBDD disponible:";
	foreach ( QString driver, list ) {
		qDebug() << driver;
	}
    QString errorDriverNotAvailable = QObject::trUtf8(MSG_DRIVER_NO_DISPONIBLE).arg(list.join(" - "));
    if ( configuracion.getConnectionType() == TIPO_CONEXION_NATIVA ) {
        if ( !QSqlDatabase::isDriverAvailable("QPSQL") ) {
            QMessageBox::critical (0, APP_NAME, errorDriverNotAvailable, QMessageBox::Ok);
            return false;
        }
        result = openPostgreSQL(name);
    } else if ( configuracion.getConnectionType() == TIPO_CONEXION_ODBC ) {
        if ( !QSqlDatabase::isDriverAvailable("QODBC") ) {
            QMessageBox::critical (0, APP_NAME, errorDriverNotAvailable, QMessageBox::Ok);
            return false;
        }
        result = openODBC(name);
    } else if ( configuracion.getConnectionType() == TIPO_CONEXION_SQLITE ) {
        if ( !QSqlDatabase::isDriverAvailable("QSQLITE") ) {
            QMessageBox::critical(0, APP_NAME, errorDriverNotAvailable, QMessageBox::Ok);
            return false;
        }
		bool emptyDatabase;
		result = openSQLite(name, emptyDatabase);
		if ( emptyDatabase ) {
			result = Database::createTablesSQLite();
		}
    }
    return result;
}

bool Database::openPostgreSQL(const QString &connectionName)
{
	QSqlDatabase db;

	qDebug() << "Database:openPostgreSQL: Conexion a la BBDD con los siguientes parametros: ";
	qDebug() << "Database:openPostgreSQL: Tipo de conexion: NATIVA";
	qDebug() << "Database:openPostgreSQL: Servidor: [ " << configuracion.dbServer() << " ]";
	qDebug() << "Database:openPostgreSQL: Nombre de la base de datos: [ " << configuracion.dbName() << " ]";
	qDebug() << "Database:openPostgreSQL: Usuario: [ " << configuracion.userDb() << " ]";
	qDebug() << "Database:openPostgreSQL: Nombre interno de la conexion: [ " << connectionName << " ]";
	db = QSqlDatabase::addDatabase("QPSQL", connectionName);
	db.setHostName(configuracion.dbServer());
	db.setDatabaseName(configuracion.dbName());
	db.setUserName(configuracion.userDb());
    db.setPort(configuracion.portDb());
	if ( !configuracion.passwordDb().isEmpty() ) {
		db.setPassword(configuracion.passwordDb());
	}
	db.open();
	QSqlError ultimoerror = db.lastError();
	if ( ultimoerror.isValid() && ultimoerror.type() != QSqlError::NoError ) {
        Database::openMessage = ultimoerror.text();
		qCritical() << ultimoerror.text();
		return false;
	}
	// Una vez abierta la base de datos, indicamos la ruta de esquemas por las que el usuario debe pasar
	QString searchPath = QString("SET search_path TO %1;").arg(configuracion.dbSchema());
	db.exec(searchPath);
	qDebug() << "Database:openPostgreSQL: Estableciendo ruta a las tablas: " << searchPath;
	// Nos conectamos a las noticias de la base de datos
	if ( db.driver()->hasFeature(QSqlDriver::EventNotifications) ) {
		if ( !db.driver()->subscribeToNotification(QString("breaklock")) ) {
			qDebug() << "Database:openPostgreSQL: No pudo establecerse la suscripcion a la notificacion breakLock";
		}
	}
	return true;
}

bool Database::openODBC(const QString &connectionName)
{
	QSqlDatabase db;
	qDebug() << "Database:openODBC: Realizando conexion a la BBDD con los siguientes parametros: ";
	qDebug() << "Database:openODBC: Tipo de conexion, ODBC";
	qDebug() << "Database:openODBC: DSN: [ " << configuracion.dsnODBC() << " ]";
	db = QSqlDatabase::addDatabase("QODBC", connectionName);
	db.setDatabaseName(configuracion.dsnODBC());
	db.open();
	QSqlError ultimoerror = db.lastError();
	if ( ultimoerror.isValid() && ultimoerror.type() != QSqlError::NoError ) {
        Database::openMessage = ultimoerror.text();
        qCritical() << ultimoerror.text();
		return false;
	}
	return true;
}

bool Database::openSQLite(const QString &connectionName, bool &emptyDatabase)
{
	QSqlDatabase db;
	QString dbPath(configuracion.tempPath());
	dbPath.append(QDir::separator()).append(QString("%1.db.sqlite").arg(connectionName));
	// ¿Existe la base de datos? Si no existiera, habría que crearla con todos sus avíos
	QFile path(dbPath);
	if ( path.exists() ) {
		emptyDatabase = false;
	} else {
		emptyDatabase = true;
	}
	qDebug() << "Database:openSQLite: Realizando conexion a la BBDD con los siguientes parametros: ";
	qDebug() << "Database:openSQLite: Tipo de conexion, SQLITE";
	qDebug() << "Database:openSQLite: File: [ " << dbPath << " ]";
	db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
	db.setDatabaseName(dbPath);
	db.open();
	QSqlError ultimoerror = db.lastError();
	if ( ultimoerror.isValid() && ultimoerror.type() != QSqlError::NoError ) {
        Database::openMessage = ultimoerror.text();
        qCritical() << ultimoerror.text();
		return false;
	}
	return true;
}

bool Database::createTablesSQLite()
{
	return true;
}

/*!
  Crea las tablas de sistema en la base de datos SQLite que contiene el código QtScript, las
  definiciones de los formularios de usuario..
  */
bool Database::createSystemTablesSQLite()
{
	bool result;
	QSqlDatabase db = QSqlDatabase::database(Database::systemDatabaseName());
	if ( !db.open() ) {
		qDebug() << db.lastError();
		return false;
	}
	QScopedPointer<QSqlQuery> qry (new QSqlQuery(db));
	QString sql = QString("CREATE TABLE alepherp_system(id INTEGER PRIMARY KEY, nombre VARCHAR(250), "
						  "contenido TEXT, type VARCHAR(10), version INTEGER, debug BOOLEAN, on_init_debug BOOLEAN);");
	result = qry->exec(sql);
	qDebug() << "Database::createSystemTablesSQLite: [ " << qry->lastQuery() << " ]";
	if ( !result ) {
		qDebug() << "Database::createSystemTablesSQLite: [ " << qry->lastError().text() << " ]";
	}
	return result;
}

const QSqlDatabase Database::getQDatabase(const QString &connection)
{
	QString dbConn;
	if ( connection == "" ) {
		dbConn = BASE_CONNECTION;
	} else {
		dbConn = connection;
	}

	QSqlDatabase db = QSqlDatabase::database(dbConn);
	if ( !db.isValid() ) {
		Database::createConnection(dbConn);
	}
	if ( db.isOpen() ) {
		return QSqlDatabase::database(dbConn);
	} else {
		return QSqlDatabase();
	}
}

const QSqlDatabase Database::getSystemDatabase()
{
	QSqlDatabase db = QSqlDatabase::database(Database::systemDatabaseName());
	if ( !db.isValid() ) {
		Database::createSystemConnection();
	}
	db = QSqlDatabase::database(Database::systemDatabaseName());
	if ( !db.isValid() ) {
		return QSqlDatabase();
	}
	if ( !db.isOpen() ) {
		if ( !db.open() ) {
			return QSqlDatabase();
		}
	}
	return db;
}
