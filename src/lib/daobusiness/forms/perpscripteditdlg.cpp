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
#include "perpscripteditdlg.h"
#include "ui_perpscripteditdlg.h"
#include "configuracion.h"
#include "dao/beans/beansfactory.h"
#include "dao/basedao.h"
#include "dao/database.h"
#include <QSqlQuery>

PERPScriptEditDlg::PERPScriptEditDlg(const QString &scriptName, QWidget *parent) :
	QDialog(parent), ui(new Ui::PERPScriptEditDlg), m_scriptName(scriptName)
{
    ui->setupUi(this);
	ui->txtScript->setValue(BeansFactory::tableWidgetsScripts.value(m_scriptName));
	ui->pbSave->setEnabled(false);
	ui->txtScript->setCodeLanguage("QtScript");

	connect(ui->txtScript, SIGNAL(valueEdited(QVariant)), this, SLOT(scriptChanged()));
	connect(ui->pbSave, SIGNAL(clicked()), this, SLOT(save()));
	connect(ui->pbCancel, SIGNAL(clicked()), this, SLOT(close()));
}

PERPScriptEditDlg::~PERPScriptEditDlg()
{
    delete ui;
}

void PERPScriptEditDlg::closeEvent ( QCloseEvent * event )
{
	event->accept();
}

void PERPScriptEditDlg::scriptChanged()
{
	if ( !ui->pbSave->isEnabled() ) {
		ui->pbSave->setEnabled(true);
	}
}

void PERPScriptEditDlg::save()
{
	bool result;

	QScopedPointer<QSqlQuery> qry (new QSqlQuery(Database::getQDatabase()));
	QString sql = QString("UPDATE %1_system SET contenido = :contenido WHERE type = 'qs' AND nombre = :scriptName").arg(configuracion.systemTablePrefix());
	qry->prepare(sql);
	qry->bindValue(":contenido", ui->txtScript->value().toString());
	qry->bindValue(":scriptName", m_scriptName);
	result = qry->exec();
	if ( !result ) {
		qDebug() << "PERPScriptEditDlg::save: " << BaseDAO::instance()->getLastDbMessage();
		QMessageBox::warning(this, QString::fromUtf8(APP_NAME), trUtf8("No se pudo actualizar el script."), QMessageBox::Ok);
	} else {
		BeansFactory::tableWidgetsScripts[m_scriptName] = ui->txtScript->value().toString();
		close();
	}
}
