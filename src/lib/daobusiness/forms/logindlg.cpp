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
#include "logindlg.h"
#include "ui_logindlg.h"
#include "dao/userdao.h"
#include "dao/basedao.h"
#include "configuracion.h"
#include <QMessageBox>

LoginDlg::LoginDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDlg)
{
    ui->setupUi(this);
	m_loginOk = NOT_LOGIN;
	connect (ui->pbOk, SIGNAL(clicked()), this, SLOT(okClicked()));
	connect (ui->pbCancel, SIGNAL(clicked()), this, SLOT(close()));

	// Puede que haya variables, como userNameCaseInsensitive que pueden ser definidas en el sistema, y necesarias
	// para el login. Por eso, se necesita previamente las variables de entorno.
	BaseDAO::loadEnvVars();

	if ( !configuracion.lastLoggedUser().isEmpty() ) {
		ui->txtUserName->setText(configuracion.lastLoggedUser());
		ui->txtPassword->setFocus();
	}
}

LoginDlg::~LoginDlg()
{
    delete ui;
}

LoginDlg::CloseTypes LoginDlg::loginOk()
{
	return m_loginOk;
}

void LoginDlg::okClicked()
{
	QString userName = ui->txtUserName->text();
	QString password = ui->txtPassword->text();
	QString errorMessage;
	bool passwordIsEmpty = false;

	if ( UserDAO::login(userName, password, passwordIsEmpty, errorMessage) ) {
		m_userName = userName;
		if ( passwordIsEmpty ) {
			m_loginOk = EMPTY_PASSWORD;
		} else {
			m_loginOk = LOGIN_OK;
		}
		close();
	} else {
		if ( errorMessage.trimmed().isEmpty() ) {
			QMessageBox::warning(this, trUtf8(APP_NAME), trUtf8("Usuario y/o contraseña incorrectos."), QMessageBox::Ok);
		} else {
			QMessageBox::warning(this, trUtf8(APP_NAME),
								 trUtf8("Ha ocurrido un error en la conexión a la base de datos: \r\nERROR: %1.").arg(errorMessage), QMessageBox::Ok);
		}
		ui->txtPassword->setText("");
	}
}

QString LoginDlg::userName()
{
	return m_userName;
}
