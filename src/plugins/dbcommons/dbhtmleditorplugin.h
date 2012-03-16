#ifndef DBHTMLEDITORPLUGIN_H
#define DBHTMLEDITORPLUGIN_H

#include <QDesignerCustomWidgetInterface>

class DBHtmlEditorPlugin: public QObject, public QDesignerCustomWidgetInterface
{
	Q_OBJECT
	Q_INTERFACES(QDesignerCustomWidgetInterface)

private:
	bool m_initialized;

public:
	DBHtmlEditorPlugin(QObject *parent = 0);

	bool isContainer() const;
	bool isInitialized() const;
	QIcon icon() const;
	QString domXml() const;
	QString group() const;
	QString includeFile() const;
	QString name() const;
	QString toolTip() const;
	QString whatsThis() const;
	QWidget *createWidget(QWidget *parent);
	void initialize(QDesignerFormEditorInterface *core);


};

#endif // DBHTMLEDITORPLUGIN_H
