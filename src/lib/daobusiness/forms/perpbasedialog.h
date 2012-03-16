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
#ifndef PERPBASEDIALOG_H
#define PERPBASEDIALOG_H

#include <QDialog>
#include <QScriptValue>
#include <alepherpglobal.h>

class PERPBaseDialogPrivate;
class BaseBeanMetadata;
class PERPScript;

#define MINUS_ELECTION			0
#define EQUAL_ELECTION			1
#define MORE_ELECTION			2
#define BETWEEN_ELECTION_INDEX	3

class Q_ALEPHERP_EXPORT PERPBaseDialog : public QDialog
{
    Q_OBJECT

private:
	PERPBaseDialogPrivate *d;
	Q_DECLARE_PRIVATE(PERPBaseDialog)

protected:
	void setupWidgetFromBaseBeanMetadata(BaseBeanMetadata *metadata, QLayout *layoutDestiny,
										 bool showVisibleGridOnly = false, bool searchDlg = false);
	void setOpenSuccess(bool value);

    PERPScript *engine();

	void closeEvent ( QCloseEvent * event );
	void showEvent ( QShowEvent * event );
	void hideEvent ( QHideEvent * event );

public:
	explicit PERPBaseDialog(QWidget* parent = 0, Qt::WFlags fl = 0);
	virtual ~PERPBaseDialog();

	QString tableName();
	void setTableName(const QString &value);
	bool openSuccess();

	bool checkPermissionsToOpen();

    Q_INVOKABLE QScriptValue callMethod(const QString &method);

signals:

public slots:
	void show ();
	void showWaitAnimation(bool value, const QString message = "");
	void resizeToSavedDimension();

protected slots:
	void searchComboChanged(int index);
};

#endif // PERPBASEDIALOG_H
