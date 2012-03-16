include( ../config.pri ) 

TARGET = $$APPNAME

win32 {
	QMAKESPEC = win32-g++
	CONFIG += windows
	RC_FILE = win32info.rc
	DEFINES += WINDOWS XML_LIBRARY
}

unix {
	DESTDIR = $$BUILDPATH/bin
}


TRANSLATIONS    = printingerp_english.ts \
				  printingerp_spanish.ts \
				  printingerp_french.ts \
				  printingerp_german.ts \
				  printingerp_portuges.ts

CONFIG += warn_on \
	thread \
	qt \
	exceptions \
	uitools

QT += sql network script

VERSTR = '\\"$${VERSION}\\"'
DEFINES += VER=\"$${VERSTR}\"

INCLUDEPATH +=  ./ \
				lib/daobusiness \
				lib/daobusiness/widgets \
				lib/config \
				lib/httputil \
				plugins/dbnumberedit \
				plugins/dbcommons \
				plugins/htmleditor

LIBS += -lconfig \
			-ldaobusiness \
			-lhtmleditor\
			-lqcodeedit
#			-lvmime \
#			-liconv \
#			-lpq
# Las librerias, ws2_32 la requiere gsoap. iconv las requiere vmime

# Necesario por GSOAP
DEFINES += WITH_NONAMESPACES
DEFINES += WITH_COOKIES

SOURCES += main.cpp \
 checkstatus.cpp

HEADERS += checkstatus.h

RESOURCES += resources/resources.qrc
