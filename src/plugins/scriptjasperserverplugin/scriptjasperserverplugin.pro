include (../../../config.pri)

CONFIG += plugin

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

TARGET = scriptjasperserverplugin
TEMPLATE = lib

QT += script \
	scripttools

INCLUDEPATH += ../../ ../../lib/config ../../lib/jasperserver
LIBS += -ljasperserver

HEADERS = scriptjasperserverplugin.h

SOURCES = scriptjasperserverplugin.cpp

FORMS = 

RESOURCES = 

