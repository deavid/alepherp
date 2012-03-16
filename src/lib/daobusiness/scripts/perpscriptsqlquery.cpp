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
#include "perpscriptsqlquery.h"
#include "dao/database.h"
#include <QCoreApplication>

PERPScriptSqlQuery::PERPScriptSqlQuery(QObject *parent) : QObject(parent)
{
	m_query = new QSqlQuery(Database::getQDatabase());
}

PERPScriptSqlQuery::~PERPScriptSqlQuery()
{
	delete m_query;
}

QScriptValue PERPScriptSqlQuery::specialPERPScriptSqlQueryConstructor(QScriptContext *context, QScriptEngine *engine)
{
	Q_UNUSED(context)
	PERPScriptSqlQuery *object = new PERPScriptSqlQuery();
	return engine->newQObject(object, QScriptEngine::ScriptOwnership);
}

void PERPScriptSqlQuery::addBindValue ( const QVariant & val, QSql::ParamType paramType )
{
	m_query->addBindValue(val, paramType);
}

void PERPScriptSqlQuery::bindValue ( const QString & placeholder, const QVariant & val, QSql::ParamType paramType )
{
	m_query->bindValue(placeholder, val, paramType);
}

void PERPScriptSqlQuery::bindValue ( int pos, const QVariant & val, QSql::ParamType paramType )
{
	m_query->bindValue(pos, val, paramType);
}

bool PERPScriptSqlQuery::exec ( const QString & query )
{
	bool result = m_query->exec(query);
	qDebug() << "PERPScriptSqlQuery:exec(): SQL EJECUTADA: [ " << m_query->lastQuery() << " ]";
	qDebug() << "PERPScriptSqlQuery:exec(): Numero de filas devueltas: [ " << m_query->size() << " ]";
	if ( result ) {
		m_query->first();
	} else {
		qDebug() << "PERPScriptSqlQuery:exec(): ERROR: [ " << m_query->lastError().databaseText() << " ]";
	}
	return result;
}

bool PERPScriptSqlQuery::exec ()
{
	bool result = m_query->exec();
	qDebug() << "PERPScriptSqlQuery:exec(): SQL EJECUTADA: [ " << m_query->lastQuery() << " ]";
	qDebug() << "PERPScriptSqlQuery:exec(): Numero de filas devueltas: [ " << m_query->size() << " ]";
	if ( result ) {
		m_query->first();
	} else {
		qDebug() << "PERPScriptSqlQuery:exec(): ERROR: [ " << m_query->lastError().databaseText() << " ]";
	}
	return result;
}

bool PERPScriptSqlQuery::prepare ( const QString & query )
{
	return m_query->prepare(query);
}

QVariant PERPScriptSqlQuery::value (int index) const
{
	return m_query->value(index);
}

bool PERPScriptSqlQuery::first ()
{
	return m_query->first();
}

bool PERPScriptSqlQuery::next ()
{
	return m_query->next();
}

int PERPScriptSqlQuery::size ()
{
	return m_query->size();
}

