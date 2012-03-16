/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
** $QT_END_LICENSE$
**
****************************************************************************/
#include "graphicsstackedwidget.h"
#include <QtGui>
#include <QGraphicsLinearLayout>
#include <QSequentialAnimationGroup>

#define TRANSITION_TIME_MS	300

class GraphicsStackedWidgetPrivate
{
public:
	int m_index;
	bool m_active;

	GraphicsStackedWidgetPrivate() {
		m_active = false;
	}
};


GraphicsStackedWidget::GraphicsStackedWidget(QWidget *parent):
		QStackedWidget(parent),
		d(new GraphicsStackedWidgetPrivate)
{
	d->m_index = 0;
}

GraphicsStackedWidget::~GraphicsStackedWidget()
{
	delete d;
}

int GraphicsStackedWidget::currentIndex() const
{
	return QStackedWidget::currentIndex();
}

void GraphicsStackedWidget::setCurrentIndex ( int index )
{
	d->m_index = index;
	if ( !d->m_active ) {
		animatedChange();
	}
}

int GraphicsStackedWidget::addWidget(QWidget *w)
{
	return QStackedWidget::addWidget(w);
}

void GraphicsStackedWidget::animatedChange()
{
	QSequentialAnimationGroup *group = new QSequentialAnimationGroup(this);
	QWidget *oldWidget = QStackedWidget::widget(QStackedWidget::currentIndex());
	if ( oldWidget != NULL ) {
		QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(oldWidget);
		oldWidget->setGraphicsEffect(effect);
		QPropertyAnimation *animationHide = new QPropertyAnimation(effect, "opacity", this);
		animationHide->setDuration(TRANSITION_TIME_MS/2);
		animationHide->setStartValue(1);
		animationHide->setEndValue(0);
		group->addAnimation(animationHide);
	}
	QWidget *widget = QStackedWidget::widget(d->m_index);
	if ( widget != NULL ) {
		QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(widget);
		widget->setGraphicsEffect(effect);
		QPropertyAnimation *animationShow = new QPropertyAnimation(effect, "opacity", this);
		animationShow->setDuration(TRANSITION_TIME_MS/2);
		animationShow->setStartValue(0);
		animationShow->setEndValue(1);
		group->addAnimation(animationShow);
	}
	QObject::connect(group, SIGNAL(finished()), this, SLOT(animationDoneSlot()));
	d->m_active = true;
	group->start();
}

void GraphicsStackedWidget::animationDoneSlot()
{
	QStackedWidget::setCurrentIndex(d->m_index);
	d->m_active = false;
	QTimer::singleShot(0, this, SLOT(update()));
}

