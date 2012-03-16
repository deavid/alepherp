/***************************************************************************
 *   Copyright (C) 2007 by David Pinelo   *
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
#ifndef BASEBEANVALIDATOR_H
#define BASEBEANVALIDATOR_H

#include <QObject>

class BaseBean;
class DBField;

class BaseBeanValidator : public QObject
{
    Q_OBJECT
private:
	/** Bean sobre el que se realizarán las validaciones */
	BaseBean *m_bean;
	/** Mensaje que se va escribiendo una vez ejecutado validate */
	QString m_message;
	/** Mismo mensaje, pero en HTML */
	QString m_htmlMessage;
	/** Contiene una referencia al widget al que se le aplicará el foco, caso de que
	  el dbfield asociado no pase la validación */
	QWidget *m_widget;

	bool checkNull(DBField *fld);
	bool checkPk();
	bool checkLength(DBField *fld);

public:
    explicit BaseBeanValidator(QObject *parent = 0);

	void setBean(BaseBean *bean);
	BaseBean *bean();
	bool validate();
	QString validateMessages();
	QString validateHtmlMessages();
	QWidget *widgetOnBadValidate();

signals:

public slots:

};

#endif // BASEBEANVALIDATOR_H
