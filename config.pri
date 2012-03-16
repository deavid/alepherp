ALEPHERPPATH=$$PWD
BUILDPATH=$$ALEPHERPPATH/build
DLLSPATH=$$ALEPHERPPATH/src/dlls
APPNAME=alepherp
XBASELIB=
GSOAPLIB=$$ALEPHERPPATH/gsoap-2.7
VMIMELIB=$$ALEPHERPPATH/libvmime-0.9.0
PRINTINGERPSUPPORT="N"
OPENRPTSUPPORT="Y"
QTSCRIPTGENERATEDCPP=$$ALEPHERPPATH/qtscriptgenerator-src-0.2.0/generated_cpp

CONFIG += debug

CONFIG(release, debug|release) {
	MOC_DIR = $$BUILDPATH/tmp/$$APPNAME/release/moc
	OBJECTS_DIR = $$BUILDPATH/tmp/$$APPNAME/release/objetos
	UI_DIR = $$BUILDPATH/tmp/$$APPNAME/release/ui
	win32 {
		DESTDIR = $$BUILDPATH/release
        LIBS += -L$$DLLSPATH -L$$BUILDPATH/release \
                -L$$BUILDPATH/release/plugins/designer
	}
}

CONFIG(debug, debug|release) {
	MOC_DIR = $$BUILDPATH/tmp/$$APPNAME/debug/moc
	OBJECTS_DIR = $$BUILDPATH/tmp/$$APPNAME/debug/objetos
	UI_DIR = $$BUILDPATH/tmp/$$APPNAME/debug/ui
	win32 {
		DESTDIR = $$BUILDPATH/debug
        LIBS += -L$$DLLSPATH -L$$BUILDPATH/debug \
                -L$$BUILDPATH/debug/plugins/designer
	}
}

unix {
	LIBS += -L/usr/lib -L$$BUILDPATH/lib -L$$BUILDPATH/plugins/designer
}
