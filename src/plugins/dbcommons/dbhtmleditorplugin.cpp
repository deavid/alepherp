#include "dbhtmleditorplugin.h"
#include "widgets/dbhtmleditor.h"
#include <QtPlugin>

DBHtmlEditorPlugin::DBHtmlEditorPlugin(QObject *parent) : QObject (parent)
{
	m_initialized = false;
}

void DBHtmlEditorPlugin::initialize(QDesignerFormEditorInterface * /* core */)
{
	if ( m_initialized ) {
		return;
	}
	m_initialized = true;
 }

bool DBHtmlEditorPlugin::isInitialized() const
{
	return m_initialized;
}

QWidget *DBHtmlEditorPlugin::createWidget(QWidget *parent)
{
	return new DBHtmlEditor(parent);
}

QString DBHtmlEditorPlugin::name() const
{
	return "DBHtmlEditor";
}

QString DBHtmlEditorPlugin::group() const
{
	return "PrintingERP";
}

QIcon DBHtmlEditorPlugin::icon() const
{
	return QIcon(":/images/dbhtmleditor.png");
}

QString DBHtmlEditorPlugin::toolTip() const
{
	return trUtf8("Editor WYSIWYG de código HTML");
}

QString DBHtmlEditorPlugin::whatsThis() const
{
	return trUtf8("Editor WYSIWYG de código HTML");
}

bool DBHtmlEditorPlugin::isContainer() const
{
	return false;
}

QString DBHtmlEditorPlugin::domXml() const
{
	return "<ui language=\"c++\">\n"
		" <widget class=\"DBHtmlEditor\" name=\"dbHtmlEditor\">\n"
		"  <property name=\"geometry\">\n"
		"   <rect>\n"
		"    <x>0</x>\n"
		"    <y>0</y>\n"
		"    <width>100</width>\n"
		"    <height>100</height>\n"
		"   </rect>\n"
		"  </property>\n"
		" </widget>\n"
		"</ui>";
}

QString DBHtmlEditorPlugin::includeFile() const
{
	return "widgets/dbhtmleditor.h";
}
