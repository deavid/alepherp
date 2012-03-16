TEMPLATE = lib

include( ../../../config.pri )

LIB_CONFIG = ../config
LIB_HTMLEDITOR = ../htmleditor
LIB_QCODEEDIT = ../../plugins/qcodeedit/lib

win32 {
	QMAKESPEC = win32-g++
	CONFIG += windows dll
	DEFINES += WINDOWS XML_LIBRARY ALEPHERP_BUILD_LIBS
	RC_FILE = win32info.rc
}

unix {
	DESTDIR = $$BUILDPATH/lib
}

TARGET = daobusiness
DEFINES += _QCODE_EDIT_ _QCODE_EDIT_GENERIC_

INCLUDEPATH += ./ ../../ $$LIB_CONFIG $$LIB_HTMLEDITOR $$LIB_QCODEEDIT \
	$$LIB_QCODEEDIT/snippets $$LIB_QCODEEDIT/document $$LIB_QCODEEDIT/snippets $$LIB_QCODEEDIT/widgets
LIBS += -lconfig -lhtmleditor -lqcodeedit

CONFIG += warn_on \
	thread \
	qt \
	exceptions \
	uitools

QT += sql \
	xml \
	xmlpatterns \
	network \
	script \
	scripttools

VERSTR = '\\"$${VERSION}\\"'
DEFINES += VER=\"$${VERSTR}\"

TRANSLATIONS    = daobusiness_english.ts \
				  daobusiness_spanish.ts \
				  daobusiness_french.ts \
				  daobusiness_german.ts \
				  daobusiness_portuges.ts


SOURCES += dao/basedao.cpp \
 dao/database.cpp \
 dao/beans/basebean.cpp \
 dao/beans/dbfield.cpp \
 dao/beans/beansfactory.cpp \
 dao/beans/dbrelation.cpp \
 dao/beans/basebeanvalidator.cpp \
 dao/dbfieldobserver.cpp \
 dao/basebeanobserver.cpp \
 dao/observerfactory.cpp \
 dao/dbobject.cpp \
 dao/dbrelationobserver.cpp \
 dao/beans/basebeanmetadata.cpp \
 dao/beans/dbfieldmetadata.cpp \
 dao/beans/dbrelationmetadata.cpp \
 dao/userdao.cpp \
 dao/historydao.cpp \
 dao/systemdao.cpp \
 models/treeitem.cpp \
 models/treeviewmodel.cpp \
 models/perpquerymodel.cpp \
 models/perphtmldelegate.cpp \
 models/relationbasebeanmodel.cpp \
 models/treebasebeanmodel.cpp \
 models/dbbasebeanmodel.cpp \
 models/filterbasebeanmodel.cpp \
 models/fieldbeanitem.cpp \
 models/basebeanmodel.cpp \
 models/envvars.cpp \
 widgets/dbcombobox.cpp \
 widgets/dbdetailview.cpp \
 widgets/dbtableview.cpp \
 widgets/fademessage.cpp \
 widgets/dblineedit.cpp \
 widgets/dbbasewidget.cpp \
 widgets/dblabel.cpp \
 widgets/dbcheckbox.cpp \
 widgets/dbtextedit.cpp \
 widgets/dbdatetimeedit.cpp \
 widgets/dbnumberedit.cpp \
 widgets/dbhtmleditor.cpp \
 widgets/dbtabwidget.cpp \
 widgets/dbfiltertableview.cpp \
 widgets/dbframebuttons.cpp \
 widgets/dbtreeview.cpp \
 widgets/dblistview.cpp \
 widgets/dbabstractview.cpp \
 widgets/graphicsstackedwidget.cpp \
 widgets/waitwidget.cpp \
 widgets/dbcodeedit.cpp \
 widgets/dbtableviewcolumnorderform.cpp \
 widgets/menutreewidget.cpp \
 widgets/dbchooserecordbutton.cpp \
 widgets/dbfileupload.cpp \
 forms/perpbasedialog.cpp \
 forms/dbsearchdlg.cpp \
 forms/dbrecorddlg.cpp  \
 forms/perpscripteditdlg.cpp \
 forms/dbformdlg.cpp \
 forms/scriptdlg.cpp \
 forms/qdlgacercade.cpp \
 forms/seleccionestilodlg.cpp \
 forms/logindlg.cpp \
 forms/changepassworddlg.cpp \
 forms/historyviewdlg.cpp \
 forms/registereddialogs.cpp \
 forms/perpmainwindow.cpp \
 scripts/perpscriptsqlquery.cpp \
 scripts/perpscriptwidget.cpp \
 scripts/perpscriptdialog.cpp \
 scripts/perpscriptengine.cpp \
 scripts/perpscriptcommon.cpp \
 scripts/perpscript.cpp \
 scripts/dbcommonsscriptprototypes.cpp

HEADERS += dao/basedao.h \
 dao/database.h \
 dao/beans/basebean.h \
 dao/beans/dbfield.h \
 dao/beans/beansfactory.h \
 dao/beans/dbrelation.h \
 dao/beans/basebeanvalidator.h \
 dao/dbfieldobserver.h \
 dao/basebeanobserver.h \
 dao/observerfactory.h \
 dao/dbobject.h \
 dao/dbrelationobserver.h \
 dao/beans/basebeanmetadata.h \
 dao/beans/dbfieldmetadata.h \
 dao/beans/dbrelationmetadata.h \
 dao/userdao.h \
 dao/historydao.h \
 dao/systemdao.h \
 models/treeitem.h \
 models/treeviewmodel.h \
 models/perpquerymodel.h \
 models/perphtmldelegate.h \
 models/relationbasebeanmodel.h \
 models/treebasebeanmodel.h \
 models/dbbasebeanmodel.h \
 models/filterbasebeanmodel.h  \
 models/fieldbeanitem.h \
 models/treebasebeanmodel_p.h \
 models/basebeanmodel.h \
 models/envvars.h \
 widgets/dbcombobox.h \
 widgets/dbdetailview.h \
 widgets/dbtableview.h \
 widgets/fademessage.h \
 widgets/dblineedit.h \
 widgets/dbbasewidget.h \
 widgets/dblabel.h \
 widgets/dbcheckbox.h \
 widgets/dbtextedit.h \
 widgets/dbdatetimeedit.h \
 widgets/dbnumberedit.h \
 widgets/dbhtmleditor.h \
 widgets/dbtabwidget.h \
 widgets/dbfiltertableview.h \
 widgets/dbframebuttons.h \
 widgets/dbtreeview.h \
 widgets/dblistview.h \
 widgets/dbabstractview.h \
 widgets/graphicsstackedwidget.h \
 widgets/waitwidget.h \
 widgets/dbcodeedit.h \
 widgets/dbtableviewcolumnorderform.h \
 widgets/menutreewidget.h \
 widgets/dbchooserecordbutton.h \
 widgets/dbfileupload.h \
 forms/perpbasedialog.h \
 forms/dbsearchdlg.h \
 forms/dbrecorddlg.h \
 forms/perpscripteditdlg.h \
 forms/perpbasedialog_p.h \
 forms/dbformdlg.h \
 forms/scriptdlg.h \
 forms/qdlgacercade.h \
 forms/seleccionestilodlg.h \
 forms/logindlg.h \
 forms/changepassworddlg.h \
 forms/historyviewdlg.h \
 forms/registereddialogs.h \
 forms/perpmainwindow.h \
 scripts/perpscript.h \
 scripts/perpscriptsqlquery.h \
 scripts/perpscriptwidget.h \
 scripts/perpscriptdialog.h \
 scripts/perpscriptengine.h \
 scripts/perpscriptcommon.h \
 scripts/dbcommonsscriptprototypes.h

FORMS += widgets/dbfiltertableview.ui \
 widgets/dbdetailview.ui \
 widgets/fademessage.ui \
 widgets/waitwidget.ui \
 widgets/dbtableviewcolumnorderform.ui \
 widgets/dbfileupload.ui \
 forms/dbsearchdlg.ui \
 forms/dbrecorddlg.ui \
 forms/perpscripteditdlg.ui \
 forms/dbformdlg.ui \
 forms/acercaDe.ui \
 forms/seleccionEstilo.ui \
 forms/logindlg.ui \
 forms/changepassworddlg.ui \
 forms/historyviewdlg.ui

RESOURCES += \
	widgets/images/codeedit/dbcodeedit.qrc \
    ../../resources/resources.qrc
