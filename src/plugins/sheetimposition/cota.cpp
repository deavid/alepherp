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
#include "cota.h"
#include <QtGui>

#define HEIGHT_TOTAL	12
#define HEIGHT_COTA		4

class CotaPrivate
{
public:
	double m_width;
	double m_textWidth;
	QString m_unit;

	CotaPrivate() {}
};

Cota::Cota(double width, const QString &unit, QGraphicsItem *parent) :
	QGraphicsItem(parent), d(new CotaPrivate)
{
	d->m_width = width;
	d->m_unit = unit;
	d->m_textWidth = 0;
}

Cota::~Cota()
{
	delete d;
}

QRectF Cota::boundingRect() const
{
	return QRectF(0, 0, (d->m_width < d->m_textWidth ? d->m_textWidth : d->m_width), HEIGHT_TOTAL);
}

QPainterPath Cota::shape() const
{
	QPainterPath path;
	path.addRect(0, 0, (d->m_width < d->m_textWidth ? d->m_textWidth : d->m_width), HEIGHT_TOTAL);
	return path;
}

void Cota::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	Q_UNUSED(widget);

	const qreal lod = option->levelOfDetailFromTransform(painter->worldTransform());
	if ( lod > 0.125 ) {
		painter->setPen(QPen(Qt::darkGray, 0.2));
		painter->setBrush(QBrush(Qt::darkGray));
		// Dibujamos la línea de la cota
		qreal yLine = HEIGHT_TOTAL - 3;
		painter->drawLine(QPointF(0, yLine), QPointF(d->m_width, yLine));
		painter->drawLine(QPointF(0, yLine-HEIGHT_COTA), QPointF(0, yLine+HEIGHT_COTA));
		painter->drawLine(QPointF(d->m_width, yLine-HEIGHT_COTA), QPointF(d->m_width, yLine+HEIGHT_COTA));
		// Ahora dibujamos los triángulos
		QPolygonF polygon;
		// Si el tamaño es menor que el ancho de las puntas de flecha, se dibujan al revés
		if ( d->m_width < HEIGHT_COTA ) {
			polygon << QPointF(0, yLine)
					<< QPointF(-1*(HEIGHT_COTA/2), (HEIGHT_COTA/2)+yLine)
					<< QPointF(-1*(HEIGHT_COTA/2), -1 * (HEIGHT_COTA/2)+yLine);
			painter->drawPolygon(polygon);
			polygon.clear();
			polygon << QPointF(d->m_width, yLine)
					<< QPointF(d->m_width + (HEIGHT_COTA/2), -1 * (HEIGHT_COTA/2)+yLine)
					<< QPointF(d->m_width + (HEIGHT_COTA/2), (HEIGHT_COTA/2)+yLine);
			painter->drawPolygon(polygon);
		} else {
			polygon << QPointF(0, yLine)
					<< QPointF((HEIGHT_COTA/2), (HEIGHT_COTA/2)+yLine)
					<< QPointF((HEIGHT_COTA/2), -1 * (HEIGHT_COTA/2)+yLine);
			painter->drawPolygon(polygon);
			polygon.clear();
			polygon << QPointF(d->m_width, HEIGHT_TOTAL - 3)
					<< QPointF(d->m_width - (HEIGHT_COTA/2), -1 * (HEIGHT_COTA/2)+yLine)
					<< QPointF(d->m_width - (HEIGHT_COTA/2), (HEIGHT_COTA/2)+yLine);
			painter->drawPolygon(polygon);
		}

		QFont font("Arial", 8);
		font.setStyleStrategy(QFont::ForceOutline);
		double fontScale = 0.5;
		QString text = QString("%1 %2").arg(d->m_width).arg(d->m_unit);
		QRectF rect;
		painter->setPen(QPen(Qt::black, 1));
		painter->setFont(font);
		painter->scale(fontScale, fontScale);
		d->m_textWidth = painter->fontMetrics().width(text);
		rect = QRectF((d->m_width < d->m_textWidth ? -d->m_textWidth/(2*fontScale) : 0), 0,
					  (d->m_width < d->m_textWidth ? d->m_textWidth/fontScale : d->m_width/fontScale),
						HEIGHT_TOTAL/fontScale);
		painter->drawText(rect, Qt::AlignCenter, text);
	}
}

