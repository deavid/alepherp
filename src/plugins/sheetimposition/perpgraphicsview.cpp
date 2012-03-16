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
#include "perpgraphicsview.h"
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QTextStream>
#include <QScrollBar>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QDebug>

PERPGraphicsView::PERPGraphicsView(QWidget *parent) :
    QGraphicsView(parent)
{
	setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

	//Set-up the scene
	QGraphicsScene* Scene = new QGraphicsScene(this);
	setScene(Scene);

	//Set-up the view
//	setSceneRect(-20, -20, width(), height());
//	SetCenter(QPointF(width() / 2, height() / 2)); //A modified version of centerOn(), handles special cases
	setCursor(Qt::OpenHandCursor);
}

/**
  * Sets the current centerpoint.  Also updates the scene's center point.
  * Unlike centerOn, which has no way of getting the floating point center
  * back, SetCenter() stores the center point.  It also handles the special
  * sidebar case.  This function will claim the centerPoint to sceneRec ie.
  * the centerPoint must be within the sceneRec.
  */
//Set the current centerpoint in the
void PERPGraphicsView::SetCenter(const QPointF& centerPoint) {
	//Get the rectangle of the visible area in scene coords
	QRectF visibleArea = mapToScene(rect()).boundingRect();

	//Get the scene area
	QRectF sceneBounds = sceneRect();

	double boundX = visibleArea.width() / 2.0;
	double boundY = visibleArea.height() / 2.0;
	double boundWidth = sceneBounds.width() - 2.0 * boundX;
	double boundHeight = sceneBounds.height() - 2.0 * boundY;

	//The max boundary that the centerPoint can be to
	QRectF bounds(boundX, boundY, boundWidth, boundHeight);

	if(bounds.contains(centerPoint)) {
		//We are within the bounds
		CurrentCenterPoint = centerPoint;
	} else {
		//We need to clamp or use the center of the screen
		if(visibleArea.contains(sceneBounds)) {
			//Use the center of scene ie. we can see the whole scene
			CurrentCenterPoint = sceneBounds.center();
		} else {

			CurrentCenterPoint = centerPoint;

			//We need to clamp the center. The centerPoint is too large
			if(centerPoint.x() > bounds.x() + bounds.width()) {
				CurrentCenterPoint.setX(bounds.x() + bounds.width());
			} else if(centerPoint.x() < bounds.x()) {
				CurrentCenterPoint.setX(bounds.x());
			}

			if(centerPoint.y() > bounds.y() + bounds.height()) {
				CurrentCenterPoint.setY(bounds.y() + bounds.height());
			} else if(centerPoint.y() < bounds.y()) {
				CurrentCenterPoint.setY(bounds.y());
			}

		}
	}

	//Update the scrollbars
	centerOn(CurrentCenterPoint);
}

/**
  * Handles when the mouse button is pressed
  */
void PERPGraphicsView::mousePressEvent(QMouseEvent* event) {
	//For panning the view
	LastPanPoint = event->pos();
	setCursor(Qt::ClosedHandCursor);
}

/**
  * Handles when the mouse button is released
  */
void PERPGraphicsView::mouseReleaseEvent(QMouseEvent* event) {
	setCursor(Qt::OpenHandCursor);
	LastPanPoint = QPoint();
}

/**
*Handles the mouse move event
*/
void PERPGraphicsView::mouseMoveEvent(QMouseEvent* event) {
	if(!LastPanPoint.isNull()) {
		//Get how much we panned
		QPointF delta = mapToScene(LastPanPoint) - mapToScene(event->pos());
		LastPanPoint = event->pos();

		//Update the center ie. do the pan
		SetCenter(GetCenter() + delta);
	}
}

/**
  * Zoom the view in and out.
  */
void PERPGraphicsView::wheelEvent(QWheelEvent* event) {

	//Get the position of the mouse before scaling, in scene coords
	QPointF pointBeforeScale(mapToScene(event->pos()));

	//Get the original screen centerpoint
	QPointF screenCenter = GetCenter(); //CurrentCenterPoint; //(visRect.center());

	//Scale the view ie. do the zoom
	double scaleFactor = 1.15; //How fast we zoom
	if(event->delta() > 0) {
		//Zoom in
		scale(scaleFactor, scaleFactor);
	} else {
		//Zooming out
		scale(1.0 / scaleFactor, 1.0 / scaleFactor);
	}

	//Get the position after scaling, in scene coords
	QPointF pointAfterScale(mapToScene(event->pos()));

	//Get the offset of how the screen moved
	QPointF offset = pointBeforeScale - pointAfterScale;

	//Adjust to the new center for correct zooming
	QPointF newCenter = screenCenter + offset;
	SetCenter(newCenter);
}

/**
  * Need to update the center so there is no jolt in the
  * interaction after resizing the widget.
  */
void PERPGraphicsView::resizeEvent(QResizeEvent* event) {
	//Get the rectangle of the visible area in scene coords
	QRectF visibleArea = mapToScene(rect()).boundingRect();
	SetCenter(visibleArea.center());

	//Call the subclass resize so the scrollbars are updated correctly
	QGraphicsView::resizeEvent(event);
}

