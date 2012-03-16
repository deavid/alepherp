/***************************************************************************
 *   Copyright (C) 2010 by David Pinelo   *
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
#ifndef FIELDBEANITEM_H
#define FIELDBEANITEM_H

#include <QStandardItem>

class DBField;

class FieldBeanItem : public QStandardItem
{
private:
	DBField * m_field;

public:
	FieldBeanItem(DBField *field);

	// Funciones que deben reimplementarse
	int type() const;
	QVariant data ( int role = Qt::UserRole + 1 ) const;
	void setData ( const QVariant & value, int role = Qt::UserRole + 1 );
	bool operator< ( const FieldBeanItem & other ) const;

	DBField *field();
	void setField(DBField *field);
};

#endif // FIELDBEANITEM_H
