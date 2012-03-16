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
#include "perpscriptcommon.h"
#include "configuracion.h"
#include "dao/basedao.h"
#include "dao/beans/beansfactory.h"
#include "dao/database.h"
#include "dao/userdao.h"
#include "models/envvars.h"
#include "forms/registereddialogs.h"
#include "forms/perpbasedialog.h"
#include "forms/logindlg.h"
#include "forms/changepassworddlg.h"
#include "globales.h"
#include <QFile>
#include <QTemporaryFile>
#include <QTextStream>
#include <QScriptEngine>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QBuffer>
#include <QString>
#include <QXmlQuery>
#include <QXmlResultItems>
#include <QApplication>
#include <QCryptographicHash>
#include <QPointer>

class PERPScriptCommonPrivate
{
public:

	PERPScriptCommonPrivate() {
	}
};

PERPScriptCommon::PERPScriptCommon(QObject *parent) :
	QObject(parent), d_ptr(new PERPScriptCommonPrivate)
{
}

PERPScriptCommon::~PERPScriptCommon()
{
	delete d_ptr;
}

QScriptValue PERPScriptCommon::toScriptValue(QScriptEngine *engine, PERPScriptCommon * &in)
{
	return engine->newQObject(in);
}

void PERPScriptCommon::fromScriptValue(const QScriptValue &object, PERPScriptCommon * &out)
{
	out = qobject_cast<PERPScriptCommon *>(object.toQObject());
}

/*!
  Guarda el contenido pasado en content, en el archivo fileName. Si se pone overwrite a true
  sobreescribe el fichero que hubiere
  */
bool PERPScriptCommon::saveToFile(const QString &fileName, const QString &content, bool overwrite)
{
	bool r;
	QFile f(fileName);
	if ( f.exists() && !overwrite ) {
		return false;
	} else if ( f.exists() && overwrite ) {
		r = f.open(QIODevice::WriteOnly | QIODevice::Truncate);
	} else {
		r = f.open(QIODevice::WriteOnly);
	}
	if ( !r ) {
		return false;
	}
	QTextStream t(&f);
	t.setCodec("UTF-8");
//	t.setCodec(configuracion.fileSystemCodec());
	t << content;
	f.flush();
	f.close();
	return true;
}

/*!
  Lee el contenido del archivo fileName. Devuelve el contenido
  */
QString PERPScriptCommon::readFromFile(const QString &fileName)
{
	QString content;
	QFile f(fileName);
	if ( !f.exists() ) {
		return content;
	}
	if ( f.open(QIODevice::ReadOnly ) ) {
		QTextStream t(&f);
		t.setCodec("UTF-8");
//		t.setCodec(configuracion.fileSystemCodec());
		content = t.readAll();
		f.close();
	}
	return content;
}

QString PERPScriptCommon::saveToTempFile(const QString &content)
{
	QTemporaryFile file;
	QString fileName;
		if ( file.open() ) {
		QTextStream t(&file);
		file.setAutoRemove(false);
		t.setCodec("UTF-8");
		t << content;
		fileName = file.fileName();
		file.flush();
		file.close();
	}
	return fileName;
}

QString PERPScriptCommon::readFromTempFile(const QString &fileName, bool remove)
{
	QFile file(fileName);
	QString content;
	if ( file.exists() && file.open(QIODevice::ReadOnly) ) {
		QTextStream t(&file);
//		t.setCodec(configuracion.fileSystemCodec());
		t.setCodec("UTF-8");
		content = t.readAll();
		file.close();
	}
	if ( remove ) {
		QFile::remove(fileName);
	}
	return content;
}

/*!
  Crea un bean con los metadatos de la tabla adecuada. Muy útil cuando por ejemplo, se quieren
  insertar nuevas líneas en base de datos
  var bean = PERPScriptCommon.createBean("alepherp_cliente");
  bean.setFieldValue("valor");
  bean.save();
  */
QScriptValue PERPScriptCommon::createBean(const QString &tableName)
{
	BaseBean *bean = BeansFactory::instance()->newBaseBean(tableName);
	QScriptValue result = engine()->newQObject(bean, QScriptEngine::ScriptOwnership);
	return result;
}

/*!
  Función disponible en el motor Javascript para obtener un objeto bean de base de datos. Un objeto
  bean representa un registro de una tabla de base de datos. El modo de utilización es como sigue:

   var where = "id_papel = " + idPapel;
   beanPapel = PERPScriptCommon.bean("papeles", where);

   \a tableName es el nombre de la tabla en base de datos. En AlephERP un bean representa un registro de una
   tabla en base de datos. \a where es la claúsula para obtener ese bean
   \sa beans.
   El objeto resultante tiene siempre una propiedad: empty, que indica si ha habido
   éxito obteniendo el resultado:
   if ( beanPapel.empty ) {... }
*/
QScriptValue PERPScriptCommon::bean(const QString &tableName, const QString &where)
{
	QScriptValue result;
	BaseBean *bean = BeansFactory::instance()->newBaseBean(tableName);
	if ( !BaseDAO::selectFirst(bean, where) ) {
		// TODO Esto no funciona bien, porque result no es un objeto valido
		result.setProperty("empty", true, QScriptValue::ReadOnly);
	} else {
		if ( engine() != NULL ) {
			result = engine()->newQObject(bean, QScriptEngine::ScriptOwnership);
			result.setProperty("empty", false, QScriptValue::ReadOnly);
		}
	}
	return result;
}

/*!
  Función como \sa bean pero que permite obtener más registros de una tabla de base de datos.
  Es necesario indicar qué beans se obtendrán: los indicados en \a tableName, con qué claúsula
  \a where se obtendrán, y en qué orden \a order.
  */
QScriptValue PERPScriptCommon::beans(const QString &tableName, const QString &where, const QString &order)
{
	QList<BaseBean*> list;
	QScriptValue array = engine()->newArray();
	if ( BaseDAO::select(list, tableName, where, order) ) {
		for ( int i = 0; i < list.size(); i++ ) {
			array.setProperty(i, engine()->newQObject(list.at(i), QScriptEngine::ScriptOwnership));
		}
	}
	return array;
}

/*!
  Habilita a los scripts para realizar consultas de manera rápida a base de datos. También tienen
  la posibilidad de utilizar PERPSqlQuery para consultas más complejas. Esta función
  devuelve el primer registro que cumple con la condición where
  */
QVariant PERPScriptCommon::sqlSelect(const QString &fields, const QString &from, const QString &where, const QString &sort)
{
	QList<QVariant> result;
	QScopedPointer<QSqlQuery> qry (new QSqlQuery(Database::getQDatabase()));
	QString sql = QString("SELECT %1 FROM %2 WHERE %3").arg(fields).arg(from).arg(where);
	if ( !sort.isEmpty() ) {
		sql = QString ("%1 ORDER BY %2").arg(sort);
	}
	qry->prepare(sql);
	if ( qry->exec() && qry->first() ) {
		QSqlRecord rec = qry->record();
		for ( int i = 0 ; i < rec.count() ; i++ ) {
			result.append(rec.value(i));
		}
	}
	qDebug() << "PERPScriptCommon: sqlSelect: " << qry->lastQuery();
	return result;
}

/*!
  Permite desde Javascript invocar una sentencia SQL arbitraria
  */
bool PERPScriptCommon::sqlExecute(const QString &sql)
{
	return BaseDAO::execute(sql);
}

/*!
  Permite ejecutar una consulta XQuery sobre la variable xml. El resultado
  es la salida en string de esa consulta
  */
QString PERPScriptCommon::xmlQuery(const QString &xml, const QString &query)
{
	QBuffer device;
	QXmlQuery xmlQuery;
	QString xpath = QString("doc($inputDocument)%1").arg(query);
	QString result;

	device.setData(xml.toUtf8());
	device.open(QIODevice::ReadOnly);
	xmlQuery.bindVariable("inputDocument", &device);
	xmlQuery.setQuery(xpath);
	if ( xmlQuery.isValid() ) {
		xmlQuery.evaluateTo(&result);
	}
	return result;
}

/*!
  Permite actualizar un objeto de sistema (archivo .ui o archivo .qs). Las tablas
  requieren de reinicialización del programa, salvo que se cambie un script, que
  será también actualizado por este objeto.
  */
bool PERPScriptCommon::updateQsSystemObject(const QString &objectName, const QString &content)
{
	bool r = false;
	if ( BeansFactory::tableWidgetsScripts.contains(objectName) ) {
		BeansFactory::tableWidgetsScripts[objectName] = content;
		r = true;
	}
	return r;
}

/*!
  Da formato según lo configurado en la aplicación a un número
  */
QString PERPScriptCommon::formatNumber(const QVariant &number, int numDecimals)
{
	bool ok;
	QString result;
	if ( number.type() == QVariant::String ) {
		double value = configuracion.getLocale()->toDouble(number.toString(), &ok);
		if ( ok ) {
			result = configuracion.getLocale()->toString(value, 'f', numDecimals);
		}
	} else if ( number.type() == QVariant::Int || number.type() == QVariant::Double ) {
		double value = number.toDouble(&ok);
		if ( ok ) {
			result = configuracion.getLocale()->toString(value, 'f', numDecimals);
		}
	}
	return result;
}

double PERPScriptCommon::parseDouble(const QString &number)
{
	bool ok;
	double value = configuracion.getLocale()->toDouble(number, &ok);
	if ( !ok ) {
		value = number.toDouble(&ok);
		if ( !ok ) {
			value = 0;
		}
	}
	return value;
}

/*!
  Permite establecer variables de entorno. Esta variable de entorno pueden hacer
  referencia a filtros establecidos en todas las visualizaciones. Por ejemplo:
  sólo se muestran los datos referentes a un centro de trabajo
  */
void PERPScriptCommon::setEnvVar(const QString &varName, const QVariant &v)
{
	EnvVars::setVar(varName, v);
}

QVariant PERPScriptCommon::envVar(const QString &name)
{
	return EnvVars::var(name);
}

/*!
  Proporciona el usuario que se ha logado en el sistema
  var userName = PERPScriptCommon.username();
  */
QString PERPScriptCommon::username()
{
	return qApp->property("userName").toString();
}

/*!
  Lee del registro de configuración
  var valorDelRegistro = PERPScriptCommon.registryValue("tienda");
  */
QVariant PERPScriptCommon::registryValue(const QString &key)
{
	return configuracion.scriptKey(key);
}

/*!
  Guarda un valor en el registro de configuración
  PERPScriptCommon.setRegistryValue("tienda", 2);
  */
void PERPScriptCommon::setRegistryValue(const QString &key, const QString &value)
{
	configuracion.setScriptKey(key, value);
}

/*!
  Devuelve una referencia al formulario abierto cuyo nombre es name
  var form = PERPScriptCommon.getOpenForm("alepherp_cliente.dbrecorddlg.ui");
  if ( form != null ) {
		... el formulario existe ...
  }
  */
QScriptValue PERPScriptCommon::getOpenForm(const QString &name)
{
	QScriptValue result(QScriptValue::NullValue);
	PERPBaseDialog *dlg = RegisteredDialogs::dialog(name);
	if ( dlg != NULL ) {
		result = engine()->newQObject(dlg, QScriptEngine::QtOwnership);
	}
	return result;
}

/*!
  Presenta una ventana de login para que un usuario se identifique. Su uso es
  var username = PERPScriptCommon.login();
  if ( username == null ) {
	no se logo
  } else {
	se logo  y username contiene el login del usuario logado
  }
*/
QScriptValue PERPScriptCommon::login()
{
	QPointer<LoginDlg> loginDlg = new LoginDlg;
	loginDlg->setModal(true);
	loginDlg->exec();
	if ( loginDlg->loginOk() == LoginDlg::NOT_LOGIN ) {
		return QScriptValue (QScriptValue::NullValue);
	} else if ( loginDlg->loginOk() == LoginDlg::EMPTY_PASSWORD ) {
		QPointer<ChangePasswordDlg> passDlg = new ChangePasswordDlg(true);
		passDlg->setModal(true);
		passDlg->exec();
		delete passDlg;
	}
	QString user = loginDlg->userName();
	delete loginDlg;
	return QScriptValue(user);
}

/*!
  Realiza un cambio de usuario logado en el sistema. A todos los efectos es como cerrar la aplicación
  y volverla a abrir con un nuevo usuario
  */
QScriptValue PERPScriptCommon::newLoginUser()
{
	QScriptValue result = login();
	if ( !result.isNull() ) {
		qApp->setProperty("userName", result.toString());
		configuracion.setLastLoggerUser(result.toString());
	}
	return result;
}

/*!
  Permite invocar al formulario para el cambio de password
  */
bool PERPScriptCommon::changeUserPassword()
{
	QPointer<ChangePasswordDlg> passDlg = new ChangePasswordDlg();
	passDlg->setModal(true);
	passDlg->exec();
	delete passDlg;
	return true;
}

/*!
  valueToCheck es una clave sin codificar. md5Hash es la hash codificada. Comprueba
  si son iguales
  */
bool PERPScriptCommon::checkMd5(const QString &valueToCheck, const QString &md5Hash)
{
	QByteArray valueMd5 = QCryptographicHash::hash(valueToCheck.toLatin1(), QCryptographicHash::Md5).toHex();
	if ( valueMd5 == md5Hash ) {
		return true;
	}
	return false;
}

/*!
  Comprueba si cif corresponde a un Código de Identificación Fiscal español valido
  */
bool PERPScriptCommon::validateCIF(const QString &cif)
{
    return CommonsFunctions::cifValid(cif);
}

/*!
  Comprueba si NIF corresponde a un número de identificación fiscal español valido
  */
bool PERPScriptCommon::validateNIF(const QString &nif)
{
    return CommonsFunctions::nifValid(nif);
}

/*!
  Permite crear un usuario de sistema
  */
bool PERPScriptCommon::createSystemUser(const QString &userName, const QString &password)
{
	return UserDAO::createUser(userName, password);
}
