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
#ifndef DBCOMMONSSCRIPTPROTOTYPES_H
#define DBCOMMONSSCRIPTPROTOTYPES_H

#include <QObject>
#include <QPointer>
#include "widgets/dbcheckbox.h"
#include "widgets/dbchooserecordbutton.h"
#include "widgets/dbcodeedit.h"
#include "widgets/dbcombobox.h"
#include "widgets/dbdatetimeedit.h"
#include "widgets/dbdetailview.h"
#include "widgets/dbfileupload.h"
#include "widgets/dbframebuttons.h"
#include "widgets/dbhtmleditor.h"
#include "widgets/dblabel.h"
#include "widgets/dblineedit.h"
#include "widgets/dbnumberedit.h"
#include "widgets/dbtableview.h"
#include "widgets/dbtextedit.h"
#include "widgets/dbtreeview.h"
#include "widgets/dbfiltertableview.h"
#include "widgets/dblistview.h"

class DBCommonsScriptPrototypes : public QObject
{
	Q_OBJECT
public:
	static DBCommonsScriptPrototypes *instance();
	~DBCommonsScriptPrototypes();

	DBCheckBox *dbCheckBox();
	DBChooseRecordButton *dbChooseRecordButton();
	DBCodeEdit *dbCodeEdit();
	DBComboBox *dbComboBox();
	DBDateTimeEdit *dbDateTimeEdit();
	DBDetailView *dbDetailView();
	DBFileUpload *dbFileUpload();
	DBFrameButtons *dbFrameButtons();
	DBHtmlEditor *dbHtmlEditor();
	DBLabel *dbLabel();
	DBLineEdit *dbLineEdit();
	DBNumberEdit *dbNumberEdit();
	DBTableView *dbTableView();
	DBTextEdit *dbTextEdit();
	DBFilterTableView *dbFilterTableView();
	DBListView *dbListView();

private:
	static bool m_initialized;
	QPointer<DBCheckBox> m_dbCheckBox;
	QPointer<DBChooseRecordButton> m_dbChooseRecordButton;
	QPointer<DBCodeEdit> m_dbCodeEdit;
	QPointer<DBComboBox> m_dbComboBox;
	QPointer<DBDateTimeEdit> m_dbDateTimeEdit;
	QPointer<DBDetailView> m_dbDetailView;
	QPointer<DBFileUpload> m_dbFileUpload;
	QPointer<DBFrameButtons> m_dbFrameButtons;
	QPointer<DBHtmlEditor> m_dbHtmlEditor;
	QPointer<DBLabel> m_dbLabel;
	QPointer<DBLineEdit> m_dbLineEdit;
	QPointer<DBNumberEdit> m_dbNumberEdit;
	QPointer<DBTableView> m_dbTableView;
	QPointer<DBTextEdit> m_dbTextEdit;
	QPointer<DBFilterTableView> m_dbFilterTableView;
	QPointer<DBListView> m_dbListView;

	DBCommonsScriptPrototypes(QObject *parent = 0);
	void createPrototypes();
	void destroyProtoypes();

};

#endif // DBCOMMONSSCRIPTPROTOTYPES_H
