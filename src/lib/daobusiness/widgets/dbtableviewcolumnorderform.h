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
#ifndef DBTABLEVIEWCOLUMNORDERFORM_H
#define DBTABLEVIEWCOLUMNORDERFORM_H

#include <QDialog>
#include <QListWidgetItem>
#include <QHeaderView>
#include <alepherpglobal.h>

class DBTableViewColumnOrderFormPrivate;
class FilterBaseBeanModel;

namespace Ui {
    class DBTableViewColumnOrderForm;
}

class Q_ALEPHERP_EXPORT DBTableViewColumnOrderForm : public QDialog
{
    Q_OBJECT
private:
	Ui::DBTableViewColumnOrderForm *ui;
	DBTableViewColumnOrderFormPrivate *d;
	Q_DECLARE_PRIVATE(DBTableViewColumnOrderForm)

	void init();

protected:
	bool eventFilter (QObject *target, QEvent *event);

public:
	explicit DBTableViewColumnOrderForm(FilterBaseBeanModel *model, QHeaderView *header, QList<QPair<QString, QString> > *order,
										QWidget *parent = 0);
    ~DBTableViewColumnOrderForm();

private slots:
	void okClicked();
	void itemHasBeenDoubleClicked(QListWidgetItem*);
	void changeOrder();
	void showOrderIconOnButton();
	void itemUp();
	void itemDown();
	void itemsMoved();
};

#endif // DBTABLEVIEWCOLUMNORDERFORM_H
