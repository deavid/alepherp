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
#ifndef PERPGRAPHICSVIEW_H
#define PERPGRAPHICSVIEW_H

#include <QGraphicsView>

class PERPGraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
	explicit PERPGraphicsView(QWidget *parent = 0);

signals:

public slots:

protected:
	// Holds the current centerpoint for the view, used for panning and zooming
	QPointF CurrentCenterPoint;

	// From panning the view
	QPoint LastPanPoint;

	// Set the current centerpoint in the
	void SetCenter(const QPointF& centerPoint);
	QPointF GetCenter() { return CurrentCenterPoint; }

	//Take over the interaction
	virtual void mousePressEvent(QMouseEvent* event);
	virtual void mouseReleaseEvent(QMouseEvent* event);
	virtual void mouseMoveEvent(QMouseEvent* event);
	virtual void wheelEvent(QWheelEvent* event);
	virtual void resizeEvent(QResizeEvent* event);
};

#endif // PERPGRAPHICSVIEW_H
