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
#ifndef HISTORYVIEWDLG_H
#define HISTORYVIEWDLG_H

#include <QDialog>
#include <QModelIndex>
#include <alepherpglobal.h>

namespace Ui {
    class HistoryViewDlg;
}

/**
  Este formulario presenta la información fiscalizada de cambios realizados sobre un registro
  */
class Q_ALEPHERP_EXPORT HistoryViewDlg : public QDialog
{
    Q_OBJECT

public:
	explicit HistoryViewDlg(const QString &tableName, const QString &pKey, QWidget *parent = 0);
    ~HistoryViewDlg();

private:
    Ui::HistoryViewDlg *ui;
	QString m_tableName;

private slots:
	void showInfo(const QModelIndex &idx);

};

#endif // HISTORYVIEWDLG_H
