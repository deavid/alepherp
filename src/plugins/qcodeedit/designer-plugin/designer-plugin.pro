
include (../../../../config.pri)

TARGET = qcodeedit-plugin
TEMPLATE = lib

CONFIG += designer plugin

win32 {
	CONFIG(release) {
		DESTDIR = $$BUILDPATH/release/plugins/designer
	}
	CONFIG (debug) {
		DESTDIR = $$BUILDPATH/debug/plugins/designer
	}
}
unix {
	target.path = $$BUILDPATH/plugins/designer
	INSTALLS += target
	DESTDIR = $$BUILDPATH/plugins/designer
}

INCLUDEPATH += ../lib ../lib/widgets ../lib/document \
    ../../../lib/daobusiness
LIBS += -ldaobusiness -lqcodeedit

HEADERS += collection.h \
	editorplugin.h \
	colorpickerplugin.h \
	editorconfigplugin.h \
	formatconfigplugin.h

SOURCES += collection.cpp \
	editorplugin.cpp \
	colorpickerplugin.cpp \
	editorconfigplugin.cpp \
	formatconfigplugin.cpp

