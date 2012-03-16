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
#include "envvars.h"

QHash<QString, QVariant> EnvVars::m_vars;

EnvVars::EnvVars()
{
}

QVariant EnvVars::var(const QString &var)
{
	return EnvVars::m_vars.value(var, QVariant());
}

void EnvVars::setVar(const QString &var, const QVariant &v)
{
	EnvVars::m_vars[var] = v;
}

QHash<QString, QVariant> EnvVars::vars()
{
	return EnvVars::m_vars;
}

void EnvVars::clear()
{
	EnvVars::m_vars.clear();
}
