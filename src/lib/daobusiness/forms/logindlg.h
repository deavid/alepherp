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
#ifndef LOGINDLG_H
#define LOGINDLG_H

#include <QDialog>
#include <alepherpglobal.h>

namespace Ui {
    class LoginDlg;
}

/*!
  Formulario que realiza login a la base de datos. Devuelve, en loginOk tres posibles
  valores: LOGIN_OK (usuario logado correctamente, EMPTY_PASSWORD: usuario con clave vacía,
  NOT_LOGIN: usuario no se ha logado
  */
class Q_ALEPHERP_EXPORT LoginDlg : public QDialog
{
    Q_OBJECT
	Q_ENUMS(CloseTypes)

public:
    explicit LoginDlg(QWidget *parent = 0);
    ~LoginDlg();

	enum CloseTypes { LOGIN_OK = 1, EMPTY_PASSWORD = 2, NOT_LOGIN = 4 };

	CloseTypes loginOk();
	QString userName();

protected slots:
	void okClicked();

private:
	CloseTypes m_loginOk;
	QString m_userName;
	Ui::LoginDlg *ui;
};

#endif // LOGINDLG_H
