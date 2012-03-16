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
#include "dbfileupload.h"
#include "dao/dbfieldobserver.h"
#include "ui_dbfileupload.h"
#include "configuracion.h"
#include <QFileDialog>
#include <QMessageBox>

class DBFileUploadPrivate {
	Q_DECLARE_PUBLIC(DBFileUpload)
public:
	QString m_groupName;
	DBFileUpload *q_ptr;

	DBFileUploadPrivate(DBFileUpload *qq) : q_ptr(qq)  {
	}
};

DBFileUpload::DBFileUpload(QWidget *parent) :
    QWidget(parent),
	ui(new Ui::DBFileUpload), d(new DBFileUploadPrivate(this))
{
    ui->setupUi(this);
	connect(ui->pbUpload, SIGNAL(clicked()), this, SLOT(pbUploadClicked()));
	connect(ui->pbDelete, SIGNAL(clicked()), this, SLOT(pbDeleteClicked()));
}

DBFileUpload::~DBFileUpload()
{
	emit destroyed(this);
	delete ui;
	delete d;
}

void DBFileUpload::setGroupName(const QString &name)
{
	d->m_groupName = name;
	ui->groupBox->setTitle(name);
}

QString DBFileUpload::groupName()
{
	return d->m_groupName;
}

void DBFileUpload::setValue(const QVariant &value)
{
    if ( value.isValid() ) {
        QByteArray ba = value.toByteArray();
        QPixmap pixmap;
        if ( pixmap.loadFromData(ba) ) {
			// Evitamos que la imágen muy grande destroce el formulario
			QSize sz = ui->lblImage->size();
			QPixmap resizedPixmap = pixmap.scaled(sz, Qt::KeepAspectRatio);
			ui->lblImage->setPixmap(resizedPixmap);
        }
	}
}

QVariant DBFileUpload::value()
{
	QVariant v(ui->lblImage->pixmap());
	return v;
}

/**
	Limpieza absoluta del control
*/
void DBFileUpload::clear()
{
	ui->lblImage->clear();
	emit valueEdited(QVariant());
}

/*!
  Ajusta los parámetros de visualización de este Widget en función de lo definido en DBField d->m_field
  */
void DBFileUpload::applyFieldProperties()
{
	DBFieldObserver *obs= qobject_cast<DBFieldObserver *>(observer());
	if ( obs != NULL ) {
		if ( !dataEditable() ) {
			ui->pbUpload->setVisible(false);
			ui->pbDelete->setVisible(false);
		}
	}
}

void DBFileUpload::observerUnregistered()
{
	DBBaseWidget::observerUnregistered();
	blockSignals(true);
	this->clear();
	blockSignals(false);
}

/*!
  Pide un nuevo observador si es necesario
  */
void DBFileUpload::refresh()
{
	observer();
	if ( m_observer != NULL ) {
		m_observer->sync();
	}
}

void DBFileUpload::pbUploadClicked()
{
	// trUtf8("Imágenes (*.png *.xpm *.jpg, *.bmp, *.gif)")
	QString fileName = QFileDialog::getOpenFileName(this, trUtf8("Seleccione el fichero con la imágen que desea agregar"));
	if ( fileName.isNull() ) {
		return;
	}
    QFile file(fileName);
    if (  !file.open(QIODevice::ReadOnly) ) {
        QMessageBox::warning(this, trUtf8(APP_NAME), trUtf8("No se pudo abrir el archivo de imágen."), QMessageBox::Ok);
        return;
    }
	QPixmap pixmap(fileName);
	if ( pixmap.isNull() ) {
        QMessageBox::warning(this, trUtf8(APP_NAME), trUtf8("No se ha reconocido el formato de imágen seleccionado. No es posible agregar la imágen."), QMessageBox::Ok);
		return;
	}
	ui->lblImage->setPixmap(pixmap);
    QVariant v = file.readAll();
    m_userModified = true;
    emit valueEdited(v);
}

void DBFileUpload::pbDeleteClicked()
{
	int ret = QMessageBox::warning(this, trUtf8(APP_NAME), trUtf8("La imágen se eliminará y no podra recuperarse. ¿Está seguro de querer continuar?"), QMessageBox::Yes | QMessageBox::No);
	if ( ret == QMessageBox::Yes ) {
		ui->lblImage->clear();
        m_userModified = true;
        emit valueEdited(QVariant());
	}
}

/*!
  Tenemos que decirle al motor de scripts, que DBFormDlg se convierte de esta forma a un valor script
  */
QScriptValue DBFileUpload::toScriptValue(QScriptEngine *engine, DBFileUpload * const &in)
{
	return engine->newQObject(in, QScriptEngine::QtOwnership, QScriptEngine::PreferExistingWrapperObject);
}

void DBFileUpload::fromScriptValue(const QScriptValue &object, DBFileUpload * &out)
{
	out = qobject_cast<DBFileUpload *>(object.toQObject());
}
