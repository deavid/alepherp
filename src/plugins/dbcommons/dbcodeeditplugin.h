 
#ifndef _DBCODEEDIT_PLUGIN_H_
#define _DBCODEEDIT_PLUGIN_H_

#include <QDesignerCustomWidgetInterface>

class DBCodeEditPlugin : public QObject, public QDesignerCustomWidgetInterface
{
	Q_OBJECT
	Q_INTERFACES(QDesignerCustomWidgetInterface)
	
	public:
		DBCodeEditPlugin(QObject *p = 0);
		
		bool isContainer() const;
		bool isInitialized() const;
		QIcon icon() const;
		QString domXml() const;
		QString group() const;
		QString includeFile() const;
		QString name() const;
		QString toolTip() const;
		QString whatsThis() const;
		QWidget *createWidget(QWidget *p);
		void initialize(QDesignerFormEditorInterface *core);
		
	private:
		bool m_initialized;
};

#endif // _DBCODEEDIT_PLUGIN_H_
