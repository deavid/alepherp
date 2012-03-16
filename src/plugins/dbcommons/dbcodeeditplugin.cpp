
#include "dbcodeeditplugin.h"

#include "widgets/dbcodeedit.h"

DBCodeEditPlugin::DBCodeEditPlugin(QObject *p)
 : QObject(p), m_initialized(false)
{
	
}

bool DBCodeEditPlugin::isContainer() const
{
	return false;
}

bool DBCodeEditPlugin::isInitialized() const
{
	return m_initialized;
}

QIcon DBCodeEditPlugin::icon() const
{
	return QIcon();
}

QString DBCodeEditPlugin::domXml() const
{
	static const QLatin1String _dom("<ui language=\"c++\">\n"
									" <widget class=\"DBCodeEdit\" name=\"code\">\n"
									"  <property name=\"geometry\">\n"
									"   <rect>\n"
									"    <x>0</x>\n"
									"    <y>0</y>\n"
									"    <width>100</width>\n"
									"    <height>25</height>\n"
									"   </rect>\n"
									"  </property>\n"
									" </widget>\n"
									"</ui>");
	return _dom;
}

QString DBCodeEditPlugin::group() const
{
	static const QLatin1String _group("PrintingERP");
	return _group;
}

QString DBCodeEditPlugin::includeFile() const
{
	static const QString _include("dbcodedit.h");
	return _include;
}

QString DBCodeEditPlugin::name() const
{
	static const QLatin1String _name("DBCodeEdit");
	return _name;
}

QString DBCodeEditPlugin::toolTip() const
{
	static const QLatin1String _tooltip("A powerful source code editor widget. Integrated for PrintingERP.");
	return _tooltip;
}

QString DBCodeEditPlugin::whatsThis() const
{
	static const QLatin1String _whatthis("A powerful source code editor widget. Integrated for PrintingERP.");
	return _whatthis;
}

QWidget* DBCodeEditPlugin::createWidget(QWidget *p)
{
	return new DBCodeEdit(p);
}

void DBCodeEditPlugin::initialize(QDesignerFormEditorInterface *core)
{
	Q_UNUSED(core)
	if ( m_initialized )
		return;
	
	m_initialized = true;
}
