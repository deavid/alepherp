include (../../../config.pri)

CONFIG += designer plugin

win32 {
	QMAKESPEC = win32-g++
	CONFIG(debug) {
		DESTDIR = $$BUILDPATH/debug/plugins/designer
	}
	CONFIG(release) {
		DESTDIR = $$BUILDPATH/release/plugins/designer
	}
	DEFINES += WW_BUILD_WWWIDGETS
}

unix {
	DESTDIR = $$BUILDPATH/plugins/designer
}

TARGET = qwwrichtexteditplugin
TEMPLATE    = lib

#QT += xml \
#	webkit

INCLUDEPATH += ../../lib/config
LIBS += -lconfig

HEADERS = qwwrichtextedit.h \
 qwwcolorbutton.h \
 wwglobal.h \
 wwglobal_p.h \
 colormodel.h \
 qwwrichtexteditplugin.h

SOURCES = qwwrichtextedit.cpp \
 qwwcolorbutton.cpp \
 wwglobal_p.cpp \
 colormodel.cpp \
 qwwrichtexteditplugin.cpp

