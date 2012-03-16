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
#include "dbcodeedit.h"
#include "qformatscheme.h"
#include "qlinemarksinfocenter.h"
#include "qlanguagefactory.h"
#include "qeditor.h"
#include "qcodeedit.h"
#include "qeditsession.h"
#include "configuracion.h"
#include <QAction>
#include <QToolBar>
#include <QFile>
#include <QDir>
#include <QVBoxLayout>
#include <QApplication>

QLanguageFactory *DBCodeEdit::m_languages;

class DBCodeEditPrivate {
public:
	QFormatScheme *m_formats;
	QString m_codeLanguage;
	QEditSession *m_session;
	QCodeEdit *m_editControl;
	QToolBar *m_editToolbar;
	EditorLostFocusEventFilter *m_filter;

	DBCodeEditPrivate() {
		m_formats = NULL;
		m_session = NULL;
		m_editControl = NULL;
		m_editToolbar = NULL;
	}
};

DBCodeEdit::DBCodeEdit(QWidget *parent) :
	QWidget(parent), DBBaseWidget(), d(new DBCodeEditPrivate)
{
	QString formats = qApp->applicationDirPath() + "/qcodeedit/qxs/formats.qxf";
    QString marks = qApp->applicationDirPath() + "/qcodeedit/qxs/marks.qxm";

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	d->m_filter = new EditorLostFocusEventFilter(this);

	d->m_session = new QEditSession("session", this);

	d->m_editControl = new QCodeEdit(this);

	if ( QFile::exists(formats) ) {
		d->m_formats = new QFormatScheme(formats, d->m_editControl->editor());
		QDocument::setDefaultFormatScheme(d->m_formats);
		QLineMarksInfoCenter::instance()->loadMarkTypes(marks);
		if ( DBCodeEdit::m_languages == NULL ) {
			DBCodeEdit::m_languages = new QLanguageFactory(d->m_formats, qApp);
            DBCodeEdit::m_languages->addDefinitionPath(qApp->applicationDirPath() + "/qcodeedit/qxs");
		}
	}

	d->m_editControl
		->addPanel("Line Mark Panel", QCodeEdit::West, true)
		->setShortcut(QKeySequence("F6"));
	d->m_editControl
		->addPanel("Line Number Panel", QCodeEdit::West, true)
		->setShortcut(QKeySequence("F11"));
	d->m_editControl
		->addPanel("Fold Panel", QCodeEdit::West, true)
		->setShortcut(QKeySequence("F9"));
	d->m_editControl
		->addPanel("Line Change Panel", QCodeEdit::West, true);
	d->m_editControl
		->addPanel("Status Panel", QCodeEdit::South, true);
	d->m_editControl
		->addPanel("Goto Line Panel", QCodeEdit::South);
	d->m_editControl
		->addPanel("Search Replace Panel", QCodeEdit::South);

	d->m_editControl->editor()->installEventFilter(d->m_filter);

	connect(d->m_editControl->editor(), SIGNAL(contentModified(bool)), this, SLOT(emitValueEdited()));
	connect(d->m_filter, SIGNAL(editorLostFocus()), this, SLOT(editorLostFocus()));

	d->m_session->addEditor(d->m_editControl->editor());

	// create toolbars
	d->m_editToolbar = new QToolBar(tr("Edit"), this);
	d->m_editToolbar->setIconSize(QSize(24, 24));
	d->m_editToolbar->addAction(d->m_editControl->editor()->action("undo"));
	d->m_editToolbar->addAction(d->m_editControl->editor()->action("redo"));
	d->m_editToolbar->addSeparator();
	d->m_editToolbar->addAction(d->m_editControl->editor()->action("cut"));
	d->m_editToolbar->addAction(d->m_editControl->editor()->action("copy"));
	d->m_editToolbar->addAction(d->m_editControl->editor()->action("paste"));
	d->m_editToolbar->addSeparator();
	d->m_editToolbar->addAction(d->m_editControl->editor()->action("indent"));
	d->m_editToolbar->addAction(d->m_editControl->editor()->action("unindent"));
	d->m_editToolbar->addAction(d->m_editControl->editor()->action("comment"));
	d->m_editToolbar->addAction(d->m_editControl->editor()->action("uncomment"));
	d->m_editToolbar->addSeparator();
	d->m_editToolbar->addAction(d->m_editControl->editor()->action("find"));
	d->m_editToolbar->addAction(d->m_editControl->editor()->action("findNext"));
	d->m_editToolbar->addAction(d->m_editControl->editor()->action("replace"));
	d->m_editToolbar->addAction(d->m_editControl->editor()->action("goto"));
	layout->insertWidget(0, d->m_editToolbar);

	layout->insertWidget(2, d->m_editControl->editor());
	setLayout(layout);

	int flags = QEditor::defaultFlags();
	flags |= QEditor::LineWrap;
	flags |= QEditor::CursorJumpPastWrap;
	flags |= QEditor::AutoIndent;
	QEditor::setDefaultFlags(flags);
	d->m_session->restore();
}

DBCodeEdit::~DBCodeEdit()
{
	emit destroyed(this);
	if ( d->m_editControl != NULL ) {
		delete d->m_editControl;
	}
	delete d;
}

void DBCodeEdit::setCodeLanguage(const QString &value)
{
	d->m_codeLanguage = value;
	if ( DBCodeEdit::m_languages != NULL ) {
		DBCodeEdit::m_languages->setLanguage(d->m_editControl->editor(), value);
	}
}

QString DBCodeEdit::codeLanguage() const
{
	return d->m_codeLanguage;
}

void DBCodeEdit::setValue(const QVariant &value)
{
	if ( d->m_editControl->editor()->text() != value.toString() ) {
		d->m_editControl->editor()->setText(value.toString());
	}
}

void DBCodeEdit::emitValueEdited()
{
	QVariant v (d->m_editControl->editor()->text());
	emit valueEdited(v);
}

void DBCodeEdit::editorLostFocus ()
{
	if ( d->m_editControl->editor()->isContentModified() ) {
		emitValueEdited();
	}
}

/*!
  Ajusta los parámetros de visualización de este Widget en función de lo definido en DBField m_field
  */
void DBCodeEdit::applyFieldProperties()
{
//	setReadOnly(!dataEditable());
}

QVariant DBCodeEdit::value()
{
	QVariant v;
	if ( d->m_editControl->editor()->text().isEmpty() ) {
		return v;
	}
	return QVariant(d->m_editControl->editor()->text());
}

void DBCodeEdit::observerUnregistered()
{
	DBBaseWidget::observerUnregistered();
	blockSignals(true);
	d->m_editControl->editor()->document()->clear();
	blockSignals(false);
}

/*!
  Pide un nuevo observador si es necesario
  */
void DBCodeEdit::refresh()
{
	observer();
	if ( m_observer != NULL ) {
		m_observer->sync();
	}
}

EditorLostFocusEventFilter::EditorLostFocusEventFilter(QObject *parent) : QObject(parent)
{
}

bool EditorLostFocusEventFilter::eventFilter(QObject *obj, QEvent *event)
{
	if ( event->type() == QEvent::FocusOut ) {
		emit editorLostFocus();
	}
	return QObject::eventFilter(obj, event);
}

/*!
  Tenemos que decirle al motor de scripts, que DBFormDlg se convierte de esta forma a un valor script
  */
QScriptValue DBCodeEdit::toScriptValue(QScriptEngine *engine, DBCodeEdit * const &in)
{
	return engine->newQObject(in, QScriptEngine::QtOwnership, QScriptEngine::PreferExistingWrapperObject);
}

void DBCodeEdit::fromScriptValue(const QScriptValue &object, DBCodeEdit * &out)
{
	out = qobject_cast<DBCodeEdit *>(object.toQObject());
}
