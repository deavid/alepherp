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

#include "seleccionestilodlg.h"
#include "configuracion.h"
#include "globales.h"

SeleccionEstiloDlg::SeleccionEstiloDlg(QWidget* parent, Qt::WFlags fl)
	: QDialog( parent, fl ), Ui::SeleccionarEstiloDlg()
{
	setupUi(this);
	
	rbs = new QButtonGroup;
	rbs->addButton(radioButton_1, LOOK_CDE);
	rbs->addButton(radioButton_2, LOOK_CLEANLOOK);
	rbs->addButton(radioButton_4, LOOK_MOTIF);
	rbs->addButton(radioButton_5, LOOK_PLASTIQUE);
	rbs->addButton(radioButton_6, LOOK_WINDOWS);
	rbs->addButton(radioButton_7, LOOK_VISTA);
	rbs->addButton(radioButton_8, LOOK_XP);
	rbs->addButton(radioButton_9, LOOK_GTK);
	
	iBotonPulsado = configuracion.lookAndFeel();
	seleccionaBoton (configuracion.lookAndFeel());
	connect (rbs, SIGNAL(buttonClicked(int)), this, SLOT(botonPulsado(int)));
	connect (bbBotones, SIGNAL(accepted()), this, SLOT(aceptar()));
	connect (bbBotones, SIGNAL(rejected()), this, SLOT(cancelar()));

}

SeleccionEstiloDlg::~SeleccionEstiloDlg()
{
	delete rbs;
}

/*$SPECIALIZATION$*/

void SeleccionEstiloDlg::seleccionaBoton(int boton)
{
	QRadioButton *rb = static_cast<QRadioButton *> (rbs->button(boton));
	
	if ( rb != 0 ) {
		rb->setChecked(true);
	}

}

void SeleccionEstiloDlg::botonPulsado(int estilo)
{
	QRadioButton *rb = static_cast<QRadioButton *> (rbs->button(estilo));
	
	if ( rb != 0 ) {
		if ( rb->isChecked() ) {
			iBotonPulsado = estilo;
		}
	}
	
}

void SeleccionEstiloDlg::aceptar()
{
	configuracion.setLookAndFeel(iBotonPulsado);
	this->close();
}

void SeleccionEstiloDlg::cancelar()
{
	this->close();
}

void SeleccionEstiloDlg::closeEvent(QCloseEvent * event)
{
	event->accept();
	emit closingWindow(this);
}


