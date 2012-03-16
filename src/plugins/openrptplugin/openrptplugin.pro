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

TARGET = openrptplugin
TEMPLATE = lib

QT += script \
	scripttools \
	sql \
	xml

CONFIG(release, debug|release) {
	INCLUDEPATH += $$BUILDPATH/tmp/$$APPNAME/release/ui
}

CONFIG(debug, debug|release) {
	INCLUDEPATH += $$BUILDPATH/tmp/$$APPNAME/debug/ui
}

INCLUDEPATH += ../../ ../../lib/config ../../lib/daobusiness
INCLUDEPATH += openrpt/common openrpt/OpenRPT/common openrpt/OpenRPT/renderer

LIBS += -L./openrpt/common/ -L./openrpt/OpenRPT/renderer
LIBS += -lconfig -ldaobusiness -lrenderer -lcommon

HEADERS = openrptplugin.h \
		  openrptscriptobject.h

SOURCES = openrptplugin.cpp \
		  openrptscriptobject.cpp

FORMS = 

RESOURCES = 

