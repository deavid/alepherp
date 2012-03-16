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

#ifndef PERPMainWindow_H
#define PERPMainWindow_H

#include <QMainWindow>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QMap>
#include <QSignalMapper>
#include <alepherpglobal.h>

class ThreadLoadInitialData;
class PERPMainWindowPrivate;

class Q_ALEPHERP_EXPORT PERPMainWindow : public QMainWindow
{
Q_OBJECT

private:
	PERPMainWindowPrivate *d;
	Q_DECLARE_PRIVATE(PERPMainWindow)

	bool execQs();

public:
	PERPMainWindow(QWidget* parent = 0, Qt::WFlags fl = 0 );
	~PERPMainWindow();

	void init();

public slots:
	void closeEvent(QCloseEvent * event);

protected:

protected slots:
	void openForm(const QString &action);
	void about();
	void style(void);
	void changePassword(void );
	void closeSubWindow (QWidget *objeto);
	void refreshSubWindowData(QMdiSubWindow *mdiWindow);

signals:

};

#endif

