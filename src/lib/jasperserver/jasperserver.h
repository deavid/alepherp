/***************************************************************************
 *   Copyright (C) 2007 by David Pinelo   *
 *   david@pinelo.com   *
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
#ifndef JASPERSERVER_H
#define JASPERSERVER_H

#include <QObject>
#include <QScriptValue>
#include <QScriptEngine>
#include <QFile>
#include "proxy/stdsoap2.h"
#include "alepherpglobal.h"

class JasperServerWSPrivate;

/**
En esta clase se implementan métodos necesarios para el trabajo con conexiones HTTP. Así en esta clase podemos encontrar métodos útiles para bajar un archivo de una determinada url.
Esta clase provee por ejemplo de un método para obtener una copia local de un archivo
en http y abrirlo después

	@author David Pinelo <david@pinelo.com>
*/
class Q_ALEPHERP_EXPORT JasperServerWS : public QObject {
Q_OBJECT

private:
	JasperServerWSPrivate *d;

	void saveDocument (const struct soap_multipart &attachment, const QString &path);

public:
	JasperServerWS(QObject * parent = 0);

	~JasperServerWS();

	Q_INVOKABLE QScriptValue downloadJasperServerFile (const QString &xmlRequest, bool isTempFile = false, const QString &destinyPath = "", QFile *fileTemp = NULL);
	Q_INVOKABLE QScriptValue downloadAndOpenJasperServerFile (const QString &xmlRequest, const QString &fileSuffix);

    static QScriptValue toScriptValueSharedPointer(QScriptEngine *engine, const QSharedPointer<JasperServerWS> &in);
	static void fromScriptValueSharedPointer(const QScriptValue &object, QSharedPointer<JasperServerWS> &out);

protected slots:

signals:
};

#endif
