include (../../../config.pri)

CONFIG += designer plugin

win32 {
	QMAKESPEC = win32-g++
	CONFIG(release, debug|release) {
		DESTDIR = $$BUILDPATH/release/plugins/script
		target.path = $$BUILDPATH/release/plugins/script
	}
	CONFIG(debug, debug|release) {
		DESTDIR = $$BUILDPATH/debug/plugins/script
		target.path = $$BUILDPATH/debug/plugins/script
	}
	# install
	INSTALLS += target
}

unix {
	# install
	target.path = $$BUILDPATH/plugins/script
	INSTALLS += target
	DESTDIR = $$BUILDPATH/plugins/script
}

TARGET = perpscriptextensionplugin
TEMPLATE = lib

INCLUDEPATH += ../../lib/config ../../lib/printing
LIBS += -lconfig -lprinting

HEADERS = perpscriptextensionplugin.h

SOURCES = perpscriptextensionplugin.cpp

FORMS = 

RESOURCES = 

