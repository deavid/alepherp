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
#include "dbtableviewcolumnorderform.h"
#include "ui_dbtableviewcolumnorderform.h"
#include "dao/beans/basebeanmetadata.h"
#include "dao/beans/dbfieldmetadata.h"
#include "models/filterbasebeanmodel.h"
#include <QComboBox>
#include <QHeaderView>
#include <QDebug>

#define TEXT_ORDER_ASC " - Orden Ascendente"
#define TEXT_ORDER_DESC " - Orden Descendente"

class DBTableViewColumnOrderFormPrivate
{
public:
	QList<QPair<QString, QString> > *m_order;
	FilterBaseBeanModel *m_model;
	QHeaderView *m_header;
	QIcon m_iconAscending;
	QIcon m_iconDescending;
	QList<QListWidgetItem *> m_items;

	DBTableViewColumnOrderFormPrivate() {}
};

DBTableViewColumnOrderForm::DBTableViewColumnOrderForm(FilterBaseBeanModel *model, QHeaderView *header,
													   QList<QPair<QString, QString> > *order, QWidget *parent) :
    QDialog(parent),
	ui(new Ui::DBTableViewColumnOrderForm),
	d(new DBTableViewColumnOrderFormPrivate)
{
    ui->setupUi(this);
	d->m_order = order;
	d->m_model = model;
	d->m_iconAscending = QIcon(":/generales/images/sort_ascend.png");
	d->m_iconDescending = QIcon(":/generales/images/sort_descend.png");
	d->m_header = header;

	ui->listWidget->setMovement(QListView::Free);
	ui->listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	ui->listWidget->setDragEnabled(true);
	ui->listWidget->viewport()->setAcceptDrops(true);
	ui->listWidget->setDropIndicatorShown(true);
	ui->listWidget->setDragDropMode(QAbstractItemView::InternalMove);

	ui->listWidget->installEventFilter(this);
	init();
	connect(ui->pbOk, SIGNAL(clicked()), this, SLOT(okClicked()));
	connect(ui->pbCancel, SIGNAL(clicked()), this, SLOT(close()));
	connect(ui->listWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(itemHasBeenDoubleClicked(QListWidgetItem*)));
	connect(ui->pbOrder, SIGNAL(clicked()), this, SLOT(changeOrder()));
	connect(ui->listWidget, SIGNAL(itemSelectionChanged()), this, SLOT(showOrderIconOnButton()));
	connect(ui->pbUp, SIGNAL(clicked()), this, SLOT(itemUp()));
	connect(ui->pbDown, SIGNAL(clicked()), this, SLOT(itemDown()));
}

DBTableViewColumnOrderForm::~DBTableViewColumnOrderForm()
{
    delete ui;
	delete d;
}

void DBTableViewColumnOrderForm::init()
{
	for ( int i = 0 ; i < d->m_header->count() ; i++ ) {
		QListWidgetItem *item = new QListWidgetItem (ui->listWidget);
		QString name = d->m_model->headerData(d->m_header->logicalIndex(i), Qt::Horizontal).toString();
		QVariant value = d->m_model->headerData(d->m_header->logicalIndex(i), Qt::Horizontal, Qt::UserRole);
		item->setText(QString("%1%2").arg(name).arg(trUtf8(TEXT_ORDER_ASC)));
		item->setIcon(d->m_iconAscending);
		item->setData(Qt::UserRole, value);
		d->m_items << item;
		ui->listWidget->addItem(item);
	}
}

bool DBTableViewColumnOrderForm::eventFilter (QObject *target, QEvent *event)
{
	if ( target->objectName() == "listWidget" ) {
		if ( event->spontaneous() ) {
			if ( event->type() == QEvent::Drop ) {
				itemsMoved();
			}
		}
	}
	return QDialog::eventFilter(target, event);
}
void DBTableViewColumnOrderForm::okClicked()
{
	for ( int i = 0 ; i < ui->listWidget->count() ; i++ ) {
		QListWidgetItem *item = ui->listWidget->item(i);
		QPair<QString, QString> pair;
		pair.first = item->data(Qt::UserRole).toString();
		pair.second = ( ui->listWidget->item(i)->text().contains(trUtf8(TEXT_ORDER_ASC))
					   ? QString("ASC") : QString("DESC") );
		(*d->m_order).append(pair);
	}
	close();
}

void DBTableViewColumnOrderForm::itemHasBeenDoubleClicked(QListWidgetItem *item)
{
	if ( item == NULL ) {
		return;
	}
	int iSection = item->data(Qt::UserRole).toInt();
	QString name = d->m_model->headerData(iSection, Qt::Horizontal).toString();
	if ( item->text().contains(trUtf8(TEXT_ORDER_ASC)) ) {
		item->setIcon(d->m_iconDescending);
		QString text = item->text();
		text.replace(trUtf8(TEXT_ORDER_ASC), trUtf8(TEXT_ORDER_DESC));
		item->setText(text);
	} else {
		item->setIcon(d->m_iconAscending);
		QString text = item->text();
		text.replace(trUtf8(TEXT_ORDER_DESC), trUtf8(TEXT_ORDER_ASC));
		item->setText(text);
	}
	showOrderIconOnButton();
}

void DBTableViewColumnOrderForm::showOrderIconOnButton()
{
	QListWidgetItem *item = ui->listWidget->currentItem();
	if ( item == NULL ) {
		return;
	}
	ui->pbOrder->setIcon(item->icon());
}

void DBTableViewColumnOrderForm::changeOrder()
{
	QListWidgetItem *item = ui->listWidget->currentItem();
	if ( item == NULL ) {
		return;
	}
	itemHasBeenDoubleClicked(item);
}

void DBTableViewColumnOrderForm::itemUp()
{
	QListWidgetItem *item = ui->listWidget->currentItem();
	if ( item == NULL ) {
		return;
	}
	int row = d->m_items.indexOf(item);
	if ( row <= 0 ) {
		return;
	}
	QListWidgetItem *movementItem = ui->listWidget->takeItem(row);
	d->m_items.removeAt(row);
	ui->listWidget->insertItem(row-1, movementItem);
	d->m_items.insert(row-1, movementItem);
	ui->listWidget->setCurrentItem(movementItem);
}

void DBTableViewColumnOrderForm::itemDown()
{
	QListWidgetItem *item = ui->listWidget->currentItem();
	if ( item == NULL ) {
		return;
	}
	int row = d->m_items.indexOf(item);
	if ( row == ui->listWidget->count() ) {
		return;
	}
	QListWidgetItem *movementItem = ui->listWidget->takeItem(row);
	d->m_items.removeAt(row);
	ui->listWidget->insertItem(row+1, movementItem);
	d->m_items.insert(row+1, movementItem);
	ui->listWidget->setCurrentItem(movementItem);
}

void DBTableViewColumnOrderForm::itemsMoved()
{
	d->m_items.clear();
	for ( int i = 0 ; i < ui->listWidget->count() ; i++ ) {
		QListWidgetItem *item = ui->listWidget->item(i);
		if ( item->text().isEmpty() ) {
			ui->listWidget->removeItemWidget(item);
		} else {
			d->m_items << item;
		}
	}
}
