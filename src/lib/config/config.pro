TEMPLATE = lib

include( ../../../config.pri )

INCLUDEPATH += ../../

win32 {
	QMAKESPEC = win32-g++
	TARGET = config
	LIBS += -L$$DLLSPATH
	CONFIG += windows dll
    RC_FILE = win32info.rc

	DEFINES += WINDOWS ALEPHERP_BUILD_LIBS
}

unix {
	TARGET = config
	LIBS += -L/usr/lib
	DESTDIR = $$BUILDPATH/lib/
}

CONFIG += warn_on \
	thread \
	qt \
	exceptions

QT += gui

VERSTR = '\\"$${VERSION}\\"'
DEFINES += VER=\"$${VERSTR}\"

TRANSLATIONS    = config_english.ts \
				  config_spanish.ts \
				  config_french.ts \
				  config_german.ts \
				  config_portuges.ts

SOURCES += configuracion.cpp \
 globales.cpp

HEADERS += configuracion.h \
 globales.h
