/***************************************************************************
 *   Copyright (C) 2011 by David Pinelo   *
 *   david.pinelo@alephsistemas.es   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "item.h"
#include "sheetimposition.h"
#include "imposition.h"

#include <QtGui>

class ItemPrivate
{
public:
	int m_width;
	int m_height;
	QImage m_image;
	Imposition::ImpositionTypes m_itemType;

	ItemPrivate() {}
};

Item::Item(int width, int height, Imposition::ImpositionTypes itemType, QGraphicsItem *parent)
	: QGraphicsItem(parent), d(new ItemPrivate)
{
	d->m_width = width;
	d->m_height = height;
	d->m_itemType = itemType;
	d->m_image = QImage (":/images/bookpage.jpg");

	if ( d->m_height > d->m_width ) {
		QTransform trans;
		trans.rotate(270);
		d->m_image = d->m_image.transformed(trans);
	}
	setZValue((width + height) % 2);
	setFlags(ItemSendsGeometryChanges);
    setAcceptsHoverEvents(true);
}

Item::~Item()
{
	delete d;
}

QRectF Item::boundingRect() const
{
	return QRectF(0, 0, d->m_width, d->m_height);
}

QPainterPath Item::shape() const
{
    QPainterPath path;
	path.addRect(0, 0, d->m_width, d->m_height);
    return path;
}

void Item::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);

    const qreal lod = option->levelOfDetailFromTransform(painter->worldTransform());
	painter->setPen(QPen(Qt::darkGray, 0.1));
	if ( lod < 0.125 ) {
		painter->fillRect(QRectF(0, 0, d->m_width, d->m_height), Qt::gray);
		return;
	}
	QBrush b = painter->brush();
	painter->setBrush(Qt::white);
	painter->drawRect(0, 0, d->m_width, d->m_height);
	painter->setBrush(b);
	if ( lod >= 1 ) {
		painter->drawImage(QRectF(0.1, 0.1, d->m_width - 0.1, d->m_height - 0.1), d->m_image);
	}
}

void Item::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mousePressEvent(event);
    update();
}

void Item::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mouseMoveEvent(event);
}

void Item::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mouseReleaseEvent(event);
    update();
}

