include (../../../config.pri)

CONFIG += designer plugin

win32 {
	CONFIG(debug, debug|release){
		DESTDIR = $$BUILDPATH/debug/plugins/designer
	}
	CONFIG (release, debug|release) {
		DESTDIR = $$BUILDPATH/release/plugins/designer
	}
	QMAKESPEC = win32-g++
}

unix {
	DESTDIR = $$BUILDPATH/plugins/designer
}

TARGET = sheetimpositionplugin
TEMPLATE = lib

INCLUDEPATH += ../../lib/config ../../ ./
LIBS += -lconfig -lprinting

HEADERS = sheetimposition.h \
 sheetimpositionplugin.h \
 item.h \
 sheet.h \
 detailimpositiondlg.h \
 cota.h

SOURCES = sheetimposition.cpp \
 sheetimpositionplugin.cpp \
 item.cpp \
 sheet.cpp \
 detailimpositiondlg.cpp \
 cota.cpp

FORMS = detailimpositiondlg.ui

RESOURCES = sheetimposition.qrc

