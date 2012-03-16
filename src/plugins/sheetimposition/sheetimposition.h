/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef SHEETIMPOSITION_H
#define SHEETIMPOSITION_H

#include <QWidget>
#include "imposition.h"

class SheetImpositionPrivate;

class SheetImposition : public QWidget
{
	Q_OBJECT
	Q_PROPERTY(Imposition::ImpositionTypes itemType READ itemType WRITE setItemType)
	Q_PROPERTY(Imposition::ImpositionMode impositionMode READ impositionMode)
	Q_PROPERTY(double pinzas READ pinzas WRITE setPinzas)
	Q_PROPERTY(double sangre READ sangre WRITE setSangre)
	Q_PROPERTY(double ancho READ ancho WRITE setAncho)
	Q_PROPERTY(double largo READ largo WRITE setLargo)
	Q_PROPERTY(double pliegoAncho READ pliegoAncho WRITE setPliegoAncho)
	Q_PROPERTY(double pliegoLargo READ pliegoLargo WRITE setPliegoLargo)
	Q_PROPERTY(int numEjemplares READ numEjemplares)

private:
	SheetImpositionPrivate *d;
	Q_DECLARE_PRIVATE(SheetImposition)

public:
	SheetImposition(QWidget *parent = 0);
	~SheetImposition();

	void setItemType(Imposition::ImpositionTypes itemType);
	Imposition::ImpositionTypes itemType();
	Imposition::ImpositionMode impositionMode();
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

protected:
	void showEvent(QShowEvent *);
	void mouseDoubleClickEvent ( QMouseEvent * event );

private:

public slots:
	void refresh();

private slots:
	void setupMatrix();
	void zoomIn();
	void zoomOut();

signals:
};

#endif // SHEETIMPOSITION_H
