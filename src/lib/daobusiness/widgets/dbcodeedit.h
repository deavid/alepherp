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

#ifndef DBCODEEDIT_H
#define DBCODEEDIT_H

#include <QVariant>
#include <QWidget>
#include <QScriptValue>
#include <alepherpglobal.h>
#include "widgets/dbbasewidget.h"
#include "dao/observerfactory.h"

class DBCodeEditPrivate;
class QLanguageFactory;

class Q_ALEPHERP_EXPORT DBCodeEdit : public QWidget, public DBBaseWidget
{
    Q_OBJECT
	Q_PROPERTY (QString fieldName READ fieldName WRITE setFieldName)
	Q_PROPERTY (QString relationFilter READ relationFilter WRITE setRelationFilter)
	Q_PROPERTY (bool dataEditable READ dataEditable WRITE setDataEditable)
	Q_PROPERTY (bool perpControl READ perpControl)
	Q_PROPERTY (bool userModified READ userModified WRITE setUserModified)
	Q_PROPERTY (QVariant value READ value WRITE setValue)
	Q_PROPERTY (bool dataFromParentDialog READ dataFromParentDialog WRITE setDataFromParentDialog)

	Q_PROPERTY (QString codeLanguage READ codeLanguage WRITE setCodeLanguage)

private:
	DBCodeEditPrivate *d;
	Q_DECLARE_PRIVATE(DBCodeEdit)

	static QLanguageFactory *m_languages;

protected:
	void showEvent(QShowEvent *event) { DBBaseWidget::showEvent(event); QWidget::showEvent(event); }
	void hideEvent(QHideEvent *event) { DBBaseWidget::hideEvent(event); QWidget::hideEvent(event); }

public:
	explicit DBCodeEdit(QWidget *parent = 0);
	~DBCodeEdit();

	int observerType() { return OBSERVER_DBFIELD; }
	void applyFieldProperties();
	QVariant value();

	void setCodeLanguage(const QString &value);
	QString codeLanguage() const;

	static QScriptValue toScriptValue(QScriptEngine *engine, DBCodeEdit * const &in);
	static void fromScriptValue(const QScriptValue &object, DBCodeEdit * &out);

signals:
	void valueEdited(const QVariant &value);
	void destroyed(QWidget *widget);

public slots:
	void setValue(const QVariant &value);
	void refresh();
	void observerUnregistered();

private slots:
	void emitValueEdited();
	void editorLostFocus();
};

class EditorLostFocusEventFilter : public QObject
{
	Q_OBJECT
public:
	EditorLostFocusEventFilter(QObject *parent);

protected:
	 bool eventFilter(QObject *obj, QEvent *event);

signals:
	 void editorLostFocus();
};

Q_DECLARE_METATYPE(DBCodeEdit*)
Q_SCRIPT_DECLARE_QMETAOBJECT(DBCodeEdit, DBCodeEdit*)

#endif // DBCODEEDIT_H
