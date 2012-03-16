/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the demonstration applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
** $QT_END_LICENSE$
**
****************************************************************************/

#include "sheet.h"
#include <QtGui>

#define HEIGHT_COTA			4
#define BORDERLINE			0.5
#define SAFETY_DISTANCE		5

class SheetPrivate
{
public:
	int m_width;
	int m_height;
	double m_pinzas;
	QList<QPointF> m_stuff;

	SheetPrivate() {}
};

Sheet::Sheet( int width, int height, double pinzas )
	: QGraphicsItem(), d(new SheetPrivate)
{
	d->m_width = width;
	d->m_height = height;
	d->m_pinzas = pinzas;
	setZValue((d->m_width + d->m_height) % 2);

    setAcceptsHoverEvents(true);
}

Sheet::~Sheet()
{
	delete d;
}

QRectF Sheet::boundingRect() const
{
	return QRectF(0, 0, d->m_width + SAFETY_DISTANCE + BORDERLINE, d->m_height + SAFETY_DISTANCE + BORDERLINE);
}

QPainterPath Sheet::shape() const
{
    QPainterPath path;
	path.addRect(0, 0, d->m_width + SAFETY_DISTANCE + BORDERLINE, d->m_height + SAFETY_DISTANCE + BORDERLINE);
    return path;
}

void Sheet::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);

	QColor fillColor = Qt::white;

	// Nos proporcionará la constante de visualización, el nivel de detalle, que nos
	// permitirá decidir qué datos se ofrecen y cuáles no
    const qreal lod = option->levelOfDetailFromTransform(painter->worldTransform());
	painter->save();

	if ( lod < 0.2 ) {
		if ( lod < 0.125 ) {
			painter->fillRect(QRectF(0, 0, d->m_width, d->m_height), fillColor);
			painter->restore();
			return;
        }
        painter->setBrush(fillColor);
		painter->drawRect(0, 0, d->m_width, d->m_height);
		painter->restore();
        return;
    }
	painter->setBrush(QBrush(fillColor));
	painter->setPen(QPen(Qt::black, BORDERLINE));
	painter->drawRect(QRectF(-BORDERLINE, -BORDERLINE, d->m_width+(BORDERLINE*2), d->m_height+(BORDERLINE*2)));

	// Esto dibujará la sombra del pliego
	if ( lod >= 1 ) {
		painter->setPen(QPen(Qt::gray, BORDERLINE));
		painter->drawLine(QPointF(BORDERLINE, d->m_height+(BORDERLINE*2)), QPointF(d->m_width+BORDERLINE, d->m_height+(BORDERLINE*2)));
		painter->drawLine(QPointF(d->m_width+(BORDERLINE*2), BORDERLINE), QPointF(d->m_width+(BORDERLINE*2), d->m_height+BORDERLINE));
		// Dibujamos la zona de pinzas ahora
		painter->setPen(QPen(Qt::red, 0.1));
		painter->setBrush(QBrush(Qt::red));
		painter->drawRect(QRectF(0, 0, d->m_width, d->m_pinzas));
	}

	painter->restore();
}

void Sheet::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	QGraphicsItem::mousePressEvent(event);
}

void Sheet::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
	QGraphicsItem::mouseMoveEvent(event);
}

void Sheet::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
	QGraphicsItem::mouseReleaseEvent(event);
}
