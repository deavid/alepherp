TEMPLATE = subdirs

DEFINES += _QCODE_EDIT_BUILD_

win32 {
	CONFIG += designer plugin
}
unix {
	CONFIG += designer plugin
}

SUBDIRS += lib

CONFIG(debug, debug|release) {
	# placeholder
	QCODE_EDIT_EXTRA_DEFINES += _DEBUG_BUILD_
} else {
	# placeholder
	QCODE_EDIT_EXTRA_DEFINES += _RELEASE_BUILD_
}

include(install.pri)
