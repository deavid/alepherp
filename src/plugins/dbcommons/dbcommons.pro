include (../../../config.pri)

CONFIG += designer plugin

LIB_DAOBUSINESS = ../../lib/daobusiness
LIB_CONFIG = ../../lib/config
LIB_HTMLEDITOR = ../../lib/htmleditor
LIB_QCODEEDIT = ../qcodeedit/lib

win32 {
	QMAKESPEC = win32-g++
	CONFIG(release, debug|release) {
		DESTDIR = $$BUILDPATH/release/plugins/designer/
	}
	CONFIG(debug, debug|release) {
		DESTDIR = $$BUILDPATH/debug/plugins/designer/
	}
}

unix {
	DESTDIR = $$BUILDPATH/plugins/designer
}

TARGET = dbcommonsplugin
TEMPLATE    = lib

QT += sql

INCLUDEPATH += ../../ $$LIB_CONFIG $$LIB_DAOBUSINESS $$LIB_HTMLEDITOR $$LIB_QCODEEDIT
LIBS += -lconfig -ldaobusiness -lhtmleditor -lqcodeedit

HEADERS = dbcomboboxplugin.h \
	dbdetailviewplugin.h \
	dbfiltertableviewplugin.h \
	dbtableviewplugin.h \
	dbcommonsplugin.h \
	perpscriptwidgetplugin.h \
	dblineeditplugin.h \
	dblabelplugin.h \
	dbcheckboxplugin.h \
	dbtexteditplugin.h \
	dbdatetimeeditplugin.h \
	dbnumbereditplugin.h \
	dbhtmleditorplugin.h \
	dbtabwidgetplugin.h \
	dbframebuttonsplugin.h \
	dbtreeviewplugin.h \
	dblistviewplugin.h \
    graphicsstackedwidgetplugin.h \
	dbcodeeditplugin.h \
    menutreewidgetplugin.h \
    dbchooserecordbuttonplugin.h \
	dbfileuploadplugin.h \
	mainwindowplugin.h
SOURCES = dbcomboboxplugin.cpp \
	dbdetailviewplugin.cpp \
	dbfiltertableviewplugin.cpp \
	dbtableviewplugin.cpp \
	dbcommonsplugin.cpp \
	perpscriptwidgetplugin.cpp \
	dblineeditplugin.cpp \
	dblabelplugin.cpp \
	dbcheckboxplugin.cpp \
	dbtexteditplugin.cpp \
	dbdatetimeeditplugin.cpp \
	dbnumbereditplugin.cpp \
	dbhtmleditorplugin.cpp \
	dbtabwidgetplugin.cpp \
	dbframebuttonsplugin.cpp \
	dbtreeviewplugin.cpp \
	dblistviewplugin.cpp \
    graphicsstackedwidgetplugin.cpp \
	dbcodeeditplugin.cpp \
    menutreewidgetplugin.cpp \
    dbchooserecordbuttonplugin.cpp \
	dbfileuploadplugin.cpp \
	mainwindowplugin.cpp
FORMS = \
	$$LIB_DAOBUSINESS/widgets/dbdetailview.ui \
	$$LIB_DAOBUSINESS/widgets/dbfiltertableview.ui \
	$$LIB_DAOBUSINESS/widgets/dbfileupload.ui \
	perpscriptwidget.ui \
	dbnumberedit.ui
RESOURCES = dbcommons.qrc



