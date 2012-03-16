/***************************************************************************
 *   Copyright (C) 2012 by David Pinelo   *
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
#include "dbcommonsscriptprototypes.h"
#include <QCoreApplication>

bool DBCommonsScriptPrototypes::m_initialized = false;

DBCommonsScriptPrototypes::DBCommonsScriptPrototypes(QObject *parent) : QObject(parent)
{
	createPrototypes();
	DBCommonsScriptPrototypes::m_initialized = true;
}

DBCommonsScriptPrototypes::~DBCommonsScriptPrototypes()
{
	if ( DBCommonsScriptPrototypes::m_initialized ) {
		destroyProtoypes();
	}
}

/*!
  Singleton
  */
DBCommonsScriptPrototypes * DBCommonsScriptPrototypes::instance()
{
	static DBCommonsScriptPrototypes *singleton = 0;
	if ( singleton == 0 ) {
		singleton = new DBCommonsScriptPrototypes(qApp);
	}
	return singleton;
}

void DBCommonsScriptPrototypes::createPrototypes()
{
	m_dbListView = new DBListView();
	m_dbCheckBox = new DBCheckBox();
	m_dbChooseRecordButton = new DBChooseRecordButton();
	m_dbCodeEdit = new DBCodeEdit();
	m_dbComboBox = new DBComboBox();
	m_dbDateTimeEdit = new DBDateTimeEdit();
	m_dbDetailView = new DBDetailView();
	m_dbFileUpload = new DBFileUpload();
	m_dbFrameButtons = new DBFrameButtons();
	m_dbHtmlEditor = new DBHtmlEditor();
	m_dbLabel = new DBLabel();
	m_dbLineEdit = new DBLineEdit();
	m_dbNumberEdit = new DBNumberEdit();
	m_dbTableView = new DBTableView();
	m_dbTextEdit = new DBTextEdit();
	m_dbFilterTableView = new DBFilterTableView();
	m_dbTableView = new DBTableView();
	DBCommonsScriptPrototypes::m_initialized = true;
}

void DBCommonsScriptPrototypes::destroyProtoypes() {
	if ( m_dbCheckBox ) {
		delete m_dbCheckBox;
	}
	if ( m_dbChooseRecordButton ) {
		delete m_dbChooseRecordButton;
	}
	if ( m_dbComboBox ) {
		delete m_dbComboBox;
	}
	if ( m_dbDateTimeEdit ) {
		delete m_dbDateTimeEdit;
	}
	if ( m_dbDetailView ) {
		delete m_dbDetailView;
	}
	if ( m_dbFileUpload ) {
		delete m_dbFileUpload;
	}
	if ( m_dbFrameButtons ) {
		delete m_dbFrameButtons;
	}
	if ( m_dbHtmlEditor ) {
		delete m_dbHtmlEditor;
	}
	if ( m_dbLabel ) {
		delete m_dbLabel;
	}
	if ( m_dbLineEdit ) {
		delete m_dbLineEdit;
	}
	if ( m_dbNumberEdit ) {
		delete m_dbNumberEdit;
	}
	if ( m_dbTableView ) {
		delete m_dbTableView;
	}
	if ( m_dbTextEdit ) {
		delete m_dbTextEdit;
	}
	if ( m_dbListView ) {
		delete m_dbListView;
	}
	// TODO Esto da error
	if ( m_dbFilterTableView ) {
//		delete m_dbFilterTableView;
	}
	if ( m_dbCodeEdit ) {
//		delete m_dbCodeEdit;
	}
	DBCommonsScriptPrototypes::m_initialized = false;
}

DBCheckBox * DBCommonsScriptPrototypes::dbCheckBox()
{
	return m_dbCheckBox;
}

DBChooseRecordButton * DBCommonsScriptPrototypes::dbChooseRecordButton()
{
	return m_dbChooseRecordButton;
}

DBCodeEdit * DBCommonsScriptPrototypes::dbCodeEdit()
{
	return m_dbCodeEdit;
}

DBComboBox * DBCommonsScriptPrototypes::dbComboBox()
{
	return m_dbComboBox;
}

DBDateTimeEdit * DBCommonsScriptPrototypes::dbDateTimeEdit()
{
	return m_dbDateTimeEdit;
}

DBDetailView * DBCommonsScriptPrototypes::dbDetailView()
{
	return m_dbDetailView;
}

DBFileUpload * DBCommonsScriptPrototypes::dbFileUpload()
{
	return m_dbFileUpload;
}

DBFrameButtons * DBCommonsScriptPrototypes::dbFrameButtons()
{
	return m_dbFrameButtons;
}

DBHtmlEditor * DBCommonsScriptPrototypes::dbHtmlEditor()
{
	return m_dbHtmlEditor;
}

DBLabel * DBCommonsScriptPrototypes::dbLabel()
{
	return m_dbLabel;
}

DBLineEdit * DBCommonsScriptPrototypes::dbLineEdit()
{
	return m_dbLineEdit;
}

DBNumberEdit * DBCommonsScriptPrototypes::dbNumberEdit()
{
	return m_dbNumberEdit;
}

DBTableView * DBCommonsScriptPrototypes::dbTableView()
{
	return m_dbTableView;
}

DBTextEdit * DBCommonsScriptPrototypes::dbTextEdit()
{
	return m_dbTextEdit;
}

DBFilterTableView * DBCommonsScriptPrototypes::dbFilterTableView()
{
	return m_dbFilterTableView;
}

DBListView * DBCommonsScriptPrototypes::dbListView()
{
	return m_dbListView;
}
