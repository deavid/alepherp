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

#include "dbnumbereditplugin.h"
#include "widgets/dbnumberedit.h"
#include <QtPlugin>

DBNumberEditPlugin::DBNumberEditPlugin(QObject *parent) : QObject (parent)
{
	m_initialized = false;
}

void DBNumberEditPlugin::initialize(QDesignerFormEditorInterface * /* core */)
{
	if ( m_initialized ) {
		return;
	}
	m_initialized = true;
 }

bool DBNumberEditPlugin::isInitialized() const
{
	return m_initialized;
}

QWidget *DBNumberEditPlugin::createWidget(QWidget *parent)
{
	return new DBNumberEdit(parent);
}

QString DBNumberEditPlugin::name() const
{
	return "DBNumberEdit";
}

QString DBNumberEditPlugin::group() const
{
	return "PrintingERP";
}

QIcon DBNumberEditPlugin::icon() const
{
	return QIcon(":/images/dbnumberedit.png");
}

QString DBNumberEditPlugin::toolTip() const
{
	return trUtf8("Editor numérico");
}

QString DBNumberEditPlugin::whatsThis() const
{
	return trUtf8("Editor numérico");
}

bool DBNumberEditPlugin::isContainer() const
{
	return false;
}

QString DBNumberEditPlugin::domXml() const
{
	return "<ui language=\"c++\">\n"
			" <widget class=\"DBNumberEdit\" name=\"dbNumberEdit\">\n"
			"  <property name=\"geometry\">\n"
			"   <rect>\n"
			"    <x>0</x>\n"
			"    <y>0</y>\n"
			"    <width>100</width>\n"
			"    <height>25</height>\n"
			"   </rect>\n"
			"  </property>\n"
			" </widget>\n"
			"</ui>";
}

QString DBNumberEditPlugin::includeFile() const
{
	return "widgets/dbnumberedit.h";
}
