include (../../../config.pri)

CONFIG += designer plugin

win32 {
	QMAKESPEC = win32-g++
	CONFIG(release) {
		DESTDIR = $$BUILDPATH/release/plugins/designer/
	}
	CONFIG(debug) {
		DESTDIR = $$BUILDPATH/debug/plugins/designer/
	}
	DEFINES += ALEPHERP_BUILD_LIBS
}

unix {
	DESTDIR = $$BUILDPATH/plugins/designer
}

TARGET = htmleditorplugin
TEMPLATE = lib

QT += xml \
	webkit

INCLUDEPATH += ../../lib/config ../../ ../../lib/htmleditor
LIBS += -lhtmleditor -lconfig

HEADERS = htmleditorplugin.h

SOURCES = htmleditorplugin.cpp

FORMS = htmleditor.ui
