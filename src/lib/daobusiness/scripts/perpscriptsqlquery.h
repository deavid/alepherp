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
#ifndef PERPSCRIPTSQLQUERY_H
#define PERPSCRIPTSQLQUERY_H

#include <QSqlQuery>
#include <QObject>
#include <QSharedPointer>
#include <QVariant>
#include <QScriptValue>
#include <QScriptEngine>
#include <QScriptable>

class PERPScriptSqlQuery;

/*!
  Permite lanzar una consulta a base de datos desde el motor Javascript. La forma de utilizarlo
  es como sigue:

  var qry = new PERPSqlQuery;
  var sql = "SELECT max(codcliente) FROM clientes WHERE codgrupo='" + bean.defaultFieldValue("codgrupo") + "'";
  qry.prepare(sql);
  var result = qry.exec();
  if ( result == true && qry.first() ) {
	  var ultimo = qry.value(0);
  }
  while (qry.next()) {
	do something...
  }
*/
class PERPScriptSqlQuery : public QObject, public QScriptable
{
	Q_OBJECT
private:
	QSqlQuery *m_query;

public:
	explicit PERPScriptSqlQuery(QObject *parent = 0);
	~PERPScriptSqlQuery();

	Q_INVOKABLE void addBindValue ( const QVariant & val, QSql::ParamType paramType = QSql::In );
	Q_INVOKABLE void bindValue ( const QString & placeholder, const QVariant & val, QSql::ParamType paramType = QSql::In );
	Q_INVOKABLE void bindValue ( int pos, const QVariant & val, QSql::ParamType paramType = QSql::In );
	Q_INVOKABLE bool exec ( const QString & query );
	Q_INVOKABLE bool exec ();
	Q_INVOKABLE bool prepare ( const QString & query );
	Q_INVOKABLE QVariant value (int index) const;
	Q_INVOKABLE bool first ();
	Q_INVOKABLE bool next ();
	Q_INVOKABLE int size ();
	static QScriptValue specialPERPScriptSqlQueryConstructor(QScriptContext *context, QScriptEngine *engine);

signals:

public slots:

};

Q_DECLARE_METATYPE(PERPScriptSqlQuery*)

#endif // PERPSCRIPTSQLQUERY_H
