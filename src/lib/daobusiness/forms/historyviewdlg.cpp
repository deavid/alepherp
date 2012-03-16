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
#include "historyviewdlg.h"
#include "ui_historyviewdlg.h"
#include "configuracion.h"
#include "dao/basedao.h"
#include "dao/database.h"
#include "dao/beans/beansfactory.h"
#include "dao/beans/dbfieldmetadata.h"
#include "dao/beans/basebeanmetadata.h"
#include <QSqlQueryModel>
#include <QDomDocument>

HistoryViewDlg::HistoryViewDlg(const QString &tableName, const QString &pKey, QWidget *parent) :
    QDialog(parent),
	ui(new Ui::HistoryViewDlg), m_tableName(tableName)
{
    ui->setupUi(this);
	QSqlQueryModel *model = new QSqlQueryModel(this);
	QString userColumn = trUtf8("Usuario");
	QString actionColumn = trUtf8("Accion");
	QString timeStampColumn = trUtf8("Fecha/Hora");
	QString actionInsert = trUtf8("Creación");
	QString actionUpdate = trUtf8("Modificación");
	QString actionDelete = trUtf8("Borrado");
	QString actionNoAction = trUtf8("");

	QString query = QString("SELECT username AS \"%1\", "
							"CASE action WHEN 'INSERT' THEN '%2' WHEN 'UPDATE' THEN '%3' WHEN 'DELETE' THEN '%4' ELSE '%5' END AS \"%6\", "
							"timestamp as \"%7\", changed_data FROM %8_history WHERE tablename = '%9' and pkey = '%10'").
			arg(userColumn).arg(actionInsert).arg(actionUpdate).arg(actionDelete).arg(actionNoAction).arg(actionColumn).
			arg(timeStampColumn).arg(configuracion.systemTablePrefix()).
			arg(tableName).arg(pKey);
	model->setQuery(query, Database::getQDatabase());
	ui->tvHistoryView->setModel(model);
	ui->tvHistoryView->horizontalHeader()->hideSection(3);
	connect(ui->pbOk, SIGNAL(clicked()), this, SLOT(close()));
	connect(ui->tvHistoryView, SIGNAL(activated(QModelIndex)), this, SLOT(showInfo(QModelIndex)));
    connect(ui->tvHistoryView, SIGNAL(clicked(QModelIndex)), this, SLOT(showInfo(QModelIndex)));
}

HistoryViewDlg::~HistoryViewDlg()
{
    delete ui;
}

void HistoryViewDlg::showInfo(const QModelIndex &idx)
{
	QSqlQueryModel *mdl = qobject_cast<QSqlQueryModel *>(ui->tvHistoryView->model());
	QModelIndex index = mdl->index(idx.row(), 3);
	QString data = mdl->data(index).toString();
	BaseBeanMetadata *metadata = BeansFactory::metadataBean(m_tableName);
	QDomDocument domDocument;
	QString html;

	if ( data.isEmpty() ) {
		return;
	}
	html = QString("<html><body>");
	html = html + "<table width='100%' border='0' bgcolor='#000000' cellpadding='1' cellspacing='1'>";
	html = html + "<tr><th bgcolor='#CCCCCC' width='50%' align='left'><b>CAMPO</b></th>";
	html = html + "<th bgcolor='#CCCCCC' width='50%' align='left'></b>VALOR</b></th></tr>";
	if ( domDocument.setContent( data ) )  {
		QDomElement root = domDocument.documentElement();
		// Iteramos sobre todos los campos
		QDomNodeList fieldList = root.elementsByTagName("field");
		for ( int i = 0 ; i < fieldList.size() ; i++ ) {
			QDomElement element = fieldList.at(i).toElement();
			if ( element.hasAttribute("name") ) {
				DBFieldMetadata *fld = metadata->field(element.attribute("name"));
				QString align;
				if ( fld->type() == QVariant::String ) {
					align = "left";
				} else {
					align = "right";
				}
				html = html + QString("<tr>"
					"<td bgcolor='#FFFFFF' width='50%' align='left'>&nbsp;<i>%1</i>&nbsp;</td>").arg(fld->fieldName());
				html = html + QString("<td bgcolor='#FFFFFF' width='50%' align='%1' >&nbsp;%2&nbsp;</td>"
									  "</tr>").arg(align).arg(fld->displayValue(element.text()));
			}
		}
	}
	html = html + "</table></html>";
	ui->txtData->setHtml(html);
}
