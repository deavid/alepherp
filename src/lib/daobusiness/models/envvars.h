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
#ifndef ENVVARS_H
#define ENVVARS_H

#include <QHash>
#include <QVariant>
#include <QString>

/*!
  Existirán algunas variables de entorno que, por ejemplo marcarán la visualización
  de datos. El ejemplo típico es cuando se quieran mostrar datos según un centro de trabajo.
  Esta clase, que será un singleton, contendrá esas variables. Lo hará en forma de pares:
  nombre_variable:valor
  Estas variables de entorno además, podrán guardarse entre sesión y sesión, y se asignan
  a un usuario.
  */
class EnvVars
{
private:
	static QHash<QString, QVariant> m_vars;

public:
	EnvVars();
	static QVariant var(const QString &var);
	static void setVar(const QString &var, const QVariant &v);
	static QHash<QString, QVariant> vars();
	static void clear();
};

#endif // ENVVARS_H
