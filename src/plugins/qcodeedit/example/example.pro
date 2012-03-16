
include (../../../../config.pri)

TEMPLATE = app
TARGET = example
CONFIG += debug console
CONFIG -= app_bundle

UI_DIR = $$BUILDPATH/tmp/$$APPNAME/ui
MOC_DIR = $$BUILDPATH/tmp/$$APPNAME/moc
OBJECTS_DIR = $$BUILDPATH/tmp/$$APPNAME/objetos
DESTDIR = .

LIBS += -L$$BUILDPATH/lib -lqcodeedit

DEFINES += _QCODE_EDIT_ _QCODE_EDIT_GENERIC_

INCLUDEPATH += ../lib ../lib/document ../lib/snippets ../lib/widgets ../lib/.build

HEADERS += window.h
SOURCES += main.cpp window.cpp

FORMS += window.ui

RESOURCES += example.qrc
