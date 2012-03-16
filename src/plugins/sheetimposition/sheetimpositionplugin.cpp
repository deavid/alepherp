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
#include "sheetimpositionplugin.h"
#include "sheetimposition.h"
#include <QtPlugin>

SheetImpositionPlugin::SheetImpositionPlugin(QObject *parent) : QObject (parent)
{
	m_initialized = false;
}

void SheetImpositionPlugin::initialize(QDesignerFormEditorInterface * /* core */)
{
	if ( m_initialized ) {
		return;
	}
	m_initialized = true;
 }

bool SheetImpositionPlugin::isInitialized() const
{
	return m_initialized;
}

QWidget *SheetImpositionPlugin::createWidget(QWidget *parent)
{
	return new SheetImposition(parent);
}

QString SheetImpositionPlugin::name() const
{
	return "SheetImposition";
}

QString SheetImpositionPlugin::group() const
{
	return "PrintingERP";
}

QIcon SheetImpositionPlugin::icon() const
{
	return QIcon(":/images/sheetimposition.png");
}

 QString SheetImpositionPlugin::toolTip() const
 {
	 return trUtf8("Presenta un esquema de imposición");
 }

 QString SheetImpositionPlugin::whatsThis() const
 {
	 return trUtf8("Presenta un esquema de imposición");
 }

 bool SheetImpositionPlugin::isContainer() const
 {
	 return false;
 }

 QString SheetImpositionPlugin::domXml() const
 {
	 return "<ui language=\"c++\">\n"
			" <widget class=\"SheetImposition\" name=\"sheetImposition\">\n"
			"  <property name=\"geometry\">\n"
			"   <rect>\n"
			"    <x>0</x>\n"
			"    <y>0</y>\n"
			"    <width>100</width>\n"
			"    <height>100</height>\n"
			"   </rect>\n"
			"  </property>\n"
			" </widget>\n"
			"</ui>";
 }

 QString SheetImpositionPlugin::includeFile() const
 {
	 return "sheetimposition.h";
 }

 Q_EXPORT_PLUGIN2(sheetimpositionplugin, SheetImpositionPlugin)
