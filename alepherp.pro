include( config.pri )

CONFIG += ordered

# Este orden es MUY importante ya que marca el orden de dependencias
# en compilación
SUBDIRS += src/lib/config \
	src/lib/htmleditor \
	src/plugins/qwwrichtextedit \
	src/plugins/qcodeedit \
	src/lib/daobusiness \
	src/lib/jasperserver \
	src/plugins/qcodeedit/designer-plugin \
	src/plugins/dbcommons \
	src/plugins/htmleditorplugin \
	src/plugins/scriptjasperserverplugin

contains (OPENRPTSUPPORT, "Y") {
    SUBDIRS += src/plugins/openrptplugin/openrpt/common \
               src/plugins/openrptplugin/openrpt/OpenRPT/renderer \
               src/plugins/openrptplugin
}

win32 {
    system(SubWCRev . nsis-wininstaller.template.nsi nsis-wininstaller.nsi)
}

SUBDIRS += src

TEMPLATE = subdirs 
CONFIG += warn_on \
          qt \
          thread \
          ordered

