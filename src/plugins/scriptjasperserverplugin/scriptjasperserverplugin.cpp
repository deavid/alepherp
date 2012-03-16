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
#include "scriptjasperserverplugin.h"
#include "jasperserver.h"
#include <QSharedPointer>
#include <QMetaType>
#include <QScriptEngine>
#include <QtPlugin>
#include <QDebug>
#include <QApplication>

#define KEY_EXTENSION	"alepherp"

Q_SCRIPT_DECLARE_QMETAOBJECT(JasperServerWS, QObject*)
Q_DECLARE_METATYPE(QSharedPointer<JasperServerWS>)

ScriptJasperServerPlugin::ScriptJasperServerPlugin(QObject *parent) :
	QScriptExtensionPlugin(parent)
{
}

ScriptJasperServerPlugin::~ScriptJasperServerPlugin()
{
}

QStringList	ScriptJasperServerPlugin::keys () const
{
	QStringList result;
    result.append(KEY_EXTENSION);
	return result;
}

void ScriptJasperServerPlugin::initialize ( const QString & key, QScriptEngine * engine )
{
    if ( key == KEY_EXTENSION ) {
		QVariant jsPointer = qApp->property("jasperserver");
		JasperServerWS *pointer = qobject_cast<JasperServerWS *>(jsPointer.value<QObject *>());
		if ( pointer == NULL ) {
			pointer = new JasperServerWS(qApp);
			qApp->setProperty("jasperserver", qVariantFromValue(qobject_cast<QObject *>(pointer)));
		}
		QScriptValue jsValue = engine->newQObject(pointer);
		engine->globalObject().setProperty("JasperServerWS", jsValue);
	}
}

Q_EXPORT_PLUGIN2(scriptjasperserverplugin, ScriptJasperServerPlugin)

