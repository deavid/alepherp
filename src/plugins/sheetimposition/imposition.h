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
#ifndef IMPOSITION_H
#define IMPOSITION_H

#include <QList>
#include <QRectF>
#include <QScriptValue>

class ImpositionPrivate;

class Imposition : public QObject
{
	Q_OBJECT
	Q_ENUMS(ImpositionTypes)
	Q_ENUMS(ImpositionMode)
	Q_PROPERTY(ImpositionTypes itemType READ itemType WRITE setItemType)
	Q_PROPERTY(ImpositionMode impositionMode READ impositionMode)
	Q_PROPERTY(double pinzas READ pinzas WRITE setPinzas)
	Q_PROPERTY(double sangre READ sangre WRITE setSangre)
	Q_PROPERTY(double ancho READ ancho WRITE setAncho)
	Q_PROPERTY(double largo READ largo WRITE setLargo)
	Q_PROPERTY(double pliegoAncho READ pliegoAncho WRITE setPliegoAncho)
	Q_PROPERTY(double pliegoLargo READ pliegoLargo WRITE setPliegoLargo)
	Q_PROPERTY(int numEjemplares READ numEjemplares)

private:
	ImpositionPrivate *d;
	Q_DECLARE_PRIVATE(Imposition)

public:
	Imposition(QObject *parent = NULL);
	~Imposition();

	enum ImpositionTypes { SINGLE = 1, BOOK = 2 };
	enum ImpositionMode { WIDHT_HEIGHT = 1, HEIGHT_WIDTH = 2 };

	void setItemType(ImpositionTypes itemType);
	ImpositionTypes itemType();
	ImpositionMode impositionMode();
	void setPinzas(double value);
	double pinzas();
	void setSangre(double value);
	double sangre();
	void setAncho(double value);
	double ancho();
	void setLargo(double value);
	double largo();
	void setPliegoAncho(double value);
	double pliegoAncho();
	void setPliegoLargo(double value);
	double pliegoLargo();
	int numEjemplares();

	Q_INVOKABLE void imposition (QList<QRectF> &positions, int &rows, int &cols);
	Q_INVOKABLE int numItemsPerSheetFace();

	static QScriptValue toScriptValueSharedPointer(QScriptEngine *engine, const QSharedPointer<Imposition> &in);
	static void fromScriptValueSharedPointer(const QScriptValue &object, QSharedPointer<Imposition> &out);

};

#endif // IMPOSITION_H
