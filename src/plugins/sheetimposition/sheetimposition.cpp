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
#include "sheetimposition.h"
#include "sheet.h"
#include "item.h"
#include "configuracion.h"
#include "detailimpositiondlg.h"
#include "perpgraphicsview.h"
#include "cota.h"
#include "imposition.h"
#include <QtGui>
#include <QFrame>
#include <QGraphicsPixmapItem>

#define ZOOM_MAX		500
#define ZOOM_INITIAL	250
#define ZOOM_STEP		50

class SheetImpositionPrivate
{
public:
	Q_DECLARE_PUBLIC(SheetImposition)
	QGraphicsScene *m_scene;
	QGraphicsView *m_graphicsView;
	QSlider *m_zoomSlider;
	Imposition *m_imposition;
	SheetImposition * q_ptr;

	SheetImpositionPrivate(SheetImposition * qq) : q_ptr (qq) {
		m_scene = NULL;
		m_graphicsView = NULL;
		m_zoomSlider = NULL;
	}

	void populateScene();
	void imposition(QList<QRectF> &positions, int &rows, int &cols);
};

SheetImposition::SheetImposition(QWidget *parent)
		: QWidget(parent), d(new SheetImpositionPrivate(this))
{
	d->m_imposition = new Imposition(this);
	d->m_graphicsView = new QGraphicsView(this);
	d->m_graphicsView->setRenderHint(QPainter::Antialiasing, true);
	d->m_graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
	d->m_graphicsView->setOptimizationFlags(QGraphicsView::DontSavePainterState);
	d->m_graphicsView->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);

	QToolButton *zoomInIcon = new QToolButton(this);
	zoomInIcon->setAutoRepeat(true);
	zoomInIcon->setAutoRepeatInterval(33);
	zoomInIcon->setAutoRepeatDelay(0);
	zoomInIcon->setIcon(QPixmap(":/images/zoomin.png"));
	QToolButton *zoomOutIcon = new QToolButton(this);
	zoomOutIcon->setAutoRepeat(true);
	zoomOutIcon->setAutoRepeatInterval(33);
	zoomOutIcon->setAutoRepeatDelay(0);
	zoomOutIcon->setIcon(QPixmap(":/images/zoomout.png"));
	d->m_zoomSlider = new QSlider;
	d->m_zoomSlider->setMinimum(0);
	d->m_zoomSlider->setMaximum(ZOOM_MAX);
	d->m_zoomSlider->setValue(ZOOM_INITIAL);
	d->m_zoomSlider->setTickPosition(QSlider::TicksRight);

	// Zoom slider layout
	QVBoxLayout *zoomSliderLayout = new QVBoxLayout;
	zoomSliderLayout->addWidget(zoomInIcon);
	zoomSliderLayout->addWidget(d->m_zoomSlider);
	zoomSliderLayout->addWidget(zoomOutIcon);

	QHBoxLayout *topLayout = new QHBoxLayout;
	topLayout->addWidget(d->m_graphicsView, 1, 0);
	topLayout->addLayout(zoomSliderLayout);
	topLayout->setContentsMargins(0, 0, 0, 0);
	setLayout(topLayout);

	connect(d->m_zoomSlider, SIGNAL(valueChanged(int)), this, SLOT(setupMatrix()));
}

SheetImposition::~SheetImposition()
{
	delete d;
}

void SheetImposition::setItemType(Imposition::ImpositionTypes itemType)
{
	d->m_imposition->setItemType(itemType);
}

Imposition::ImpositionTypes SheetImposition::itemType()
{
	return d->m_imposition->itemType();
}

Imposition::ImpositionMode SheetImposition::impositionMode()
{
	return d->m_imposition->impositionMode();
}

void SheetImposition::setPinzas(double value)
{
	d->m_imposition->setPinzas(value);
}

double SheetImposition::pinzas()
{
	return d->m_imposition->pinzas();
}

void SheetImposition::setSangre(double value)
{
	d->m_imposition->setSangre(value);
}

double SheetImposition::sangre()
{
	return d->m_imposition->sangre();
}

void SheetImposition::setAncho(double value)
{
	d->m_imposition->setAncho(value);
}

double SheetImposition::ancho()
{
	return d->m_imposition->ancho();
}

void SheetImposition::setLargo(double value)
{
	d->m_imposition->setLargo(value);
}

double SheetImposition::largo()
{
	return d->m_imposition->largo();
}

void SheetImposition::setPliegoAncho(double value)
{
	d->m_imposition->setPliegoAncho(value);
}

double SheetImposition::pliegoAncho()
{
	return d->m_imposition->pliegoAncho();
}

void SheetImposition::setPliegoLargo(double value)
{
	d->m_imposition->setPliegoLargo(value);
}

double SheetImposition::pliegoLargo()
{
	return d->m_imposition->pliegoLargo();
}

int SheetImposition::numEjemplares()
{
	return d->m_imposition->numItemsPerSheetFace();
}

void SheetImposition::showEvent(QShowEvent *event)
{
	QWidget::showEvent(event);
	refresh();
}

void SheetImposition::mouseDoubleClickEvent ( QMouseEvent * event )
{
	Q_UNUSED(event)
	DetailImpositionDlg *dlg = new DetailImpositionDlg(this);
	dlg->setAttribute(Qt::WA_DeleteOnClose);
	dlg->setData(d->m_imposition->itemType(), d->m_imposition->pinzas(),
				 d->m_imposition->sangre(), d->m_imposition->ancho(),
				 d->m_imposition->largo(),d->m_imposition->pliegoAncho(), d->m_imposition->pliegoLargo());
	dlg->setModal(true);
	dlg->show();
}

void SheetImposition::setupMatrix()
{
	QMatrix matrix;
	qreal scale = qPow(qreal(2), (d->m_zoomSlider->value() - ZOOM_INITIAL) / qreal(ZOOM_STEP));
	matrix.scale(scale, scale);
	d->m_graphicsView->setMatrix(matrix);
	d->m_graphicsView->update();
}

void SheetImposition::refresh()
{
	if ( d->m_scene != NULL ) {
		d->m_scene->clear();
		delete d->m_scene;
		d->m_scene = NULL;
	}
	d->populateScene();
	d->m_graphicsView->setScene(d->m_scene);
	d->m_graphicsView->centerOn(QPointF(d->m_imposition->ancho()/2, d->m_imposition->largo()/2));
	d->m_graphicsView->fitInView(d->m_scene->sceneRect());
	QMatrix matrix;
	qreal scale = this->width() / d->m_imposition->pliegoAncho();
	matrix.scale(scale, scale);
	d->m_graphicsView->setMatrix(matrix);
	double sliderValue = ZOOM_STEP * ( log(scale) - log(2) ) + ZOOM_INITIAL;
	d->m_zoomSlider->setValue(sliderValue);
}

void SheetImposition::zoomIn()
{
	d->m_zoomSlider->setValue(d->m_zoomSlider->value() + 1);
}

void SheetImposition::zoomOut()
{
	d->m_zoomSlider->setValue(d->m_zoomSlider->value() - 1);
}

void SheetImpositionPrivate::populateScene()
{
	QString unit;
	int rows, cols;

	if ( configuracion.getLocale()->measurementSystem() == QLocale::MetricSystem ) {
		unit = "cm";
	} else {
		unit = "inches";
	}
	if ( m_scene == NULL ) {
		m_scene = new QGraphicsScene(q_ptr);
	}

	Sheet *sheet = new Sheet(m_imposition->pliegoAncho(), m_imposition->pliegoLargo(), m_imposition->pinzas());
	m_scene->addItem(sheet);
	sheet->setPos(0, 0);
	sheet->setZValue(0);

	Cota *cotaHorizontal = new Cota(m_imposition->pliegoAncho(), unit);
	m_scene->addItem(cotaHorizontal);
	cotaHorizontal->setPos(0, -1 * cotaHorizontal->boundingRect().height());
	cotaHorizontal->setZValue(1);

	Cota *cotaVertical = new Cota(m_imposition->pliegoLargo(), unit);
	m_scene->addItem(cotaVertical);
	QTransform trans;
	trans.rotate(-90);
	trans.translate(-1 * m_imposition->pliegoLargo(), 0);
	cotaVertical->setTransform(trans);
	cotaVertical->setPos(-1 * cotaHorizontal->boundingRect().height(), 0);
	cotaVertical->setZValue(1);

	QList<QRectF> positions;
	m_imposition->imposition(positions, rows, cols);

	for ( int i = 0 ; i < positions.size() ; i++ ) {
		// Hay que ver si el trabajo se ha metido a lo largo o a lo ancho
		Item *item;
		QString toolTip;
		if ( m_imposition->impositionMode() == Imposition::WIDHT_HEIGHT ) {
			item = new Item(m_imposition->ancho(), m_imposition->largo(), m_imposition->itemType());
			toolTip = QObject::trUtf8("%1x%2 %3").arg(m_imposition->ancho()).arg(m_imposition->largo()).arg(unit);
		} else {
			item = new Item(m_imposition->largo(), m_imposition->ancho(), m_imposition->itemType());
			toolTip = QObject::trUtf8("%1x%2 %3").arg(m_imposition->ancho()).arg(m_imposition->largo()).arg(unit);
		}
		item->setPos(positions.at(i).topLeft());
		item->setToolTip(toolTip);
		item->setZValue(2);
		m_scene->addItem(item);
	}
	if ( positions.size() > 0 ) {
		double distance;
		Cota *cotaSangre;
		// Primero la cota de sangre horizontal
		int index = cols - 1;
		distance = m_imposition->pliegoAncho() - positions.at(index).right() - m_imposition->sangre();
		cotaSangre = new Cota(distance, unit);
		m_scene->addItem(cotaSangre);
		cotaSangre->setPos(positions.at(index).right() - m_imposition->sangre(),
						   positions.at(index).top() + (positions.at(index).height()/2));
		cotaSangre->setZValue(3);
		// Ahora la vertical
		index = (cols - 1) * rows;
		distance = m_imposition->pliegoLargo() - positions.at(index).bottom() - m_imposition->sangre();
		cotaSangre = new Cota(distance, unit);
		m_scene->addItem(cotaSangre);
		QTransform transCota;
		transCota.rotate(-90);
		cotaSangre->setTransform(transCota);
		cotaSangre->setPos(positions.at(index).left() + (positions.at(index).width()/2),
						   positions.at(index).bottom() - m_imposition->sangre());
		cotaSangre->setZValue(3);
	}
	// Finalmente, la cota intermedia de sangre
	if ( positions.size() > 1 ) {
		Cota *sangreIntermedia = new Cota((m_imposition->sangre() * 2), unit);
		m_scene->addItem(sangreIntermedia);
		sangreIntermedia->setPos(positions.at(0).right() - (m_imposition->sangre() * 2),
								 positions.at(0).top() + (positions.at(0).height()/2));
		sangreIntermedia->setZValue(3);
	}
	// Y la cota de la pinza
	if ( positions.size() > 0 ) {
		Cota *cotaPinza = new Cota(m_imposition->pinzas(), unit);
		m_scene->addItem(cotaPinza);
		QTransform transCota;
		transCota.rotate(-90);
		transCota.translate(-1 * m_imposition->pinzas(), 0);
		cotaPinza->setTransform(transCota);
		cotaPinza->setPos(m_imposition->pliegoAncho()*0.75, 0);
		cotaPinza->setZValue(3);
	}
}
