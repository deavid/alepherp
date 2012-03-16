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
#ifndef PERPBASEDIALOG_P_H
#define PERPBASEDIALOG_P_H

#include <QString>
#include <QLayout>
#include <QComboBox>
#include "scripts/perpscript.h"

class WaitWidget;
class PERPBaseDialog;
class BaseBeanMetadata;

class PERPBaseDialogPrivate
{
public:
	Q_DECLARE_PUBLIC(PERPBaseDialog)
	/** Nombre del bean que edita este formulario, si edita alguno */
	QString m_tableName;
	/** Widget para simular la espera */
	WaitWidget *m_waitWidget;
	/** Indicará si el formulario se ha abierto correctamente */
	bool m_openSuccess;
    /** Motor de script para las funciones */
    PERPScript m_engine;

	PERPBaseDialog *q_ptr;

	PERPBaseDialogPrivate(PERPBaseDialog * qq) : q_ptr(qq) {
		m_waitWidget = 0;
	}

	void setupDBRecordDlg(BaseBeanMetadata *metadata, QLayout *layoutDestiny,
						  bool showVisibleGridOnly);
	void setupDBSearchDlg(BaseBeanMetadata *metadata, QLayout *layoutDestiny,
						  bool showVisibleGridOnly);
	QComboBox * createComboOperators(const QString &fld);

};


#endif // PERPBASEDIALOG_P_H
