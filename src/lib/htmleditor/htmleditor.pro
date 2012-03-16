include (../../../config.pri)

TEMPLATE = lib

win32 {
	CONFIG += windows dll
	DEFINES += WINDOWS ALEPHERP_BUILD_LIBS
	QMAKESPEC = win32-g++
}

unix {
	CONFIG += designer plugin
	DESTDIR = $$BUILDPATH/lib
}

TARGET = htmleditor
TEMPLATE = lib

QT += webkit

INCLUDEPATH += ../config ../../
LIBS += -lconfig

HEADERS =	htmleditor.h \
	highlighter.h

SOURCES = htmleditor.cpp \
	highlighter.cpp

FORMS = htmleditor.ui inserthtmldialog.ui

RESOURCES = htmleditor.qrc
