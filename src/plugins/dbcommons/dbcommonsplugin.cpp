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
#include "dbcommonsplugin.h"
#include "dbcomboboxplugin.h"
#include "dbdetailviewplugin.h"
#include "dbfiltertableviewplugin.h"
#include "dbdatetimeeditplugin.h"
#include "dbhtmleditorplugin.h"
#include "dbtableviewplugin.h"
#include "perpscriptwidgetplugin.h"
#include "dblineeditplugin.h"
#include "dbnumbereditplugin.h"
#include "dbtexteditplugin.h"
#include "dbtabwidgetplugin.h"
#include "dbframebuttonsplugin.h"
#include "dbtreeviewplugin.h"
#include "dblistviewplugin.h"
#include "dbcheckboxplugin.h"
#include "graphicsstackedwidgetplugin.h"
#include "dbcodeeditplugin.h"
#include "menutreewidgetplugin.h"
#include "dbchooserecordbuttonplugin.h"
#include "dbfileuploadplugin.h"
#include "mainwindowplugin.h"
#include <QtPlugin>

DBCommonsPlugin::DBCommonsPlugin(QObject *parent) :
    QObject(parent)
{
	m_widgets.append(new DBCheckBoxPlugin(this));
	m_widgets.append(new DBComboBoxPlugin(this));
	m_widgets.append(new DBDateTimeEditPlugin(this));
	m_widgets.append(new DBDetailViewPlugin(this));
	m_widgets.append(new DBFilterTableViewPlugin(this));
	m_widgets.append(new DBFrameButtonsPlugin(this));
	m_widgets.append(new DBHtmlEditorPlugin(this));
	m_widgets.append(new DBLineEditPlugin(this));
	m_widgets.append(new DBListViewPlugin(this));
	m_widgets.append(new DBNumberEditPlugin(this));
	m_widgets.append(new DBTabWidgetPlugin(this));
	m_widgets.append(new DBTableViewPlugin(this));
	m_widgets.append(new DBTextEditPlugin(this));
	m_widgets.append(new DBTreeViewPlugin(this));
	m_widgets.append(new PERPScriptWidgetPlugin(this));
	m_widgets.append(new GraphicsStackedWidgetPlugin(this));
	m_widgets.append(new DBCodeEditPlugin(this));
	m_widgets.append(new MenuTreeWidgetPlugin(this));
	m_widgets.append(new DBChooseRecordButtonPlugin(this));
	m_widgets.append(new DBFileUploadPlugin(this));
	m_widgets.append(new PERPMainWindowPlugin(this));
}

QList<QDesignerCustomWidgetInterface*> DBCommonsPlugin::customWidgets() const
{
	return m_widgets;
}

Q_EXPORT_PLUGIN2(dbcommonsplugin, DBCommonsPlugin)
