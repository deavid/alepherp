TEMPLATE = lib

include( ../../../config.pri )

LIB_CONFIG = ../config
LIB_DAOBUSINESS = ../daobusiness

INCLUDEPATH += ../../ $$LIB_CONFIG $$LIB_DAOBUSINESS

DEFINES += WITH_NONAMESPACES
DEFINES += WITH_COOKIES

win32 {
	QMAKESPEC = win32-g++
	CONFIG += windows dll
	RC_FILE = win32info.rc

	DEFINES += WINDOWS ALEPHERP_BUILD_LIBS
	LIBS += -lws2_32
}

unix {
	LIBS += -L/usr/lib
	DESTDIR = $$BUILDPATH/lib/
}

TARGET = jasperserver

LIBS += -lconfig -ldaobusiness

CONFIG += warn_on \
	thread \
	qt \
	exceptions

QT += sql \
	xml \
	network \
	script

VERSTR = '\\"$${VERSION}\\"'
DEFINES += VER=\"$${VERSTR}\"

SOURCES += jasperserver.cpp \
 proxy/stdsoap2.cpp \
 proxy/JasperServerC.cpp \
 proxy/JasperServerrepositorySoapBindingProxy.cpp \
 proxy/envC.cpp

HEADERS += jasperserver.h \
 proxy/stdsoap2.h \
 proxy/JasperServer.nsmap \
 proxy/JasperServerrepositorySoapBindingProxy.h \
 proxy/JasperServerH.h \
 proxy/JasperServerStub.h
