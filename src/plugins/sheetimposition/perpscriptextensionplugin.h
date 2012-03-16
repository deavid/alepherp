/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the Graphics Dojo project on Qt Labs.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 or 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#ifndef PERPSCRIPTEXTENSIONPLUGIN_H
#define PERPSCRIPTEXTENSIONPLUGIN_H

#include <QScriptExtensionPlugin>

class PERPScriptExtensionPlugin : public QScriptExtensionPlugin
{
	Q_OBJECT
public:
	PERPScriptExtensionPlugin(QObject * parent = 0);
	~PERPScriptExtensionPlugin();

	void initialize ( const QString & key, QScriptEngine * engine );
	QStringList	keys () const;

};

Q_EXPORT_PLUGIN2(perpscriptextensionplugin, PERPScriptExtensionPlugin)

#endif // PERPSCRIPTEXTENSIONPLUGIN_H
