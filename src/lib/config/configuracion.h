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
#ifndef CONFIGURACION_H
#define CONFIGURACION_H

#define COMPANY							"Aleph Sistemas de Informacion"
#define APP_NAME						"AlephERP"

#define TIPO_CONEXION_NATIVA			"NATIVA"
#define TIPO_CONEXION_ODBC				"ODBC"
#define TIPO_CONEXION_SQLITE			"SQLITE"

#define HEIGHT_POR_DEFECTO					400
#define WIDTH_POR_DEFECTO					400

#include <QString>
#include <QStringList>
#include <QTextStream>
#include <QSettings>
#include <QMap>
#include <QSize>
#include <QLocale>
#include <QTableWidget>
#include <QDebug>
#include <QTextCodec>
#include <QMutex>
#include <alepherpglobal.h>

#define EUR QChar(8364)
#define GBP QChar(163)
#define JPY QChar(165)
#define DOL QChar(36)

/**
	Esta clase servirá de wrapper para las opciones de configuración del programa.
	@author David Pinelo <david.pinelo@alephsistemas.es>
*/
class Q_ALEPHERP_EXPORT Configuracion : public QObject {
	Q_OBJECT

	private:
		// Variables propias de la conexión a la BBDD
		QString m_dbServer;
		QString m_userDb;
		QString m_passwordDb;
        int m_portDb;
		QString m_dbName;
		QString m_dbSchema;
		QString m_dbSchemaErp;
		QString m_tipoConexion;
		QString m_dsnODBC;
		QString m_BBDDEncoding;
		QString m_fileSystemEncoding;

		// Nombre de las tablas de sistema
		QString m_systemTablePrefix;

		// Tiempos de espera antes de recargar los datos
		int m_timeBetweenReloads;
		/** Número de registros en un modelo que se supone elevado según el tráfico de red */
		int m_strongFilterRowCountLimit;

		// Variables relativas al aspecto de la aplicación
		QMap<QString, QByteArray> m_dimensionesForms;
		QMap<QString, QPoint> m_posForms;
		
		bool m_debuggerEnabled;

		// Imágenes a usar
		QString m_externalResource;

		mutable QMutex m_mutex;

		// Estado de la ventana principal
		QByteArray m_mainWindowState;

		/** Último usuario logado */
		QString m_lastLoggedUser;

        QString m_check;

		// Variables para el envío de correos electrónicos
		QString mailServer;
		QString mailResponderA;
		QString mailCopiaA;
		QString mailFrom;
		
		QString m_tempPath;
		
		QString m_externalPDFViewer;
		bool m_internalPDFViewer;
		
		bool m_mdiTabView;

		int m_lookAndFeel;
		
		int m_fontSizeReferenceReports;
		
		// Variables para el acceso a la API de configuración.
		QSettings *settings;
		QLocale *locale;
		QTextCodec *m_codec;
		QTextCodec *m_fileSystemCodec;

		// Primer día de la semana para los calendarios
		QString m_firstDayOfWeek;

		// Frecuencia de refresco de los datos
		int m_modelRefreshTimeout;

		// Procedimientos para cargar y almacenar valores.
		void init (void);
		void almacenar (void);

	public:
		Configuracion(QObject *parent = 0);
		~Configuracion();
		
		Q_INVOKABLE QString key(const QString &value);
		Q_INVOKABLE QString scriptKey(const QString &value);
		Q_INVOKABLE void setScriptKey(const QString &key, const QVariant &value);

		void setDbServer ( const QString& theValue );
		QString dbServer() const;
		void setUserDb ( const QString& theValue );
		QString userDb() const;
		void setPasswordDb ( const QString& theValue );
		QString passwordDb() const;
		void setDbName ( const QString& theValue );
		QString dbName() const;
        int portDb() const;
        void setPortDb ( int theValue );
        void setDbSchema ( const QString& theValue );
		QString dbSchema() const;
		void setConnectionType ( const QString& theValue );
		QString getConnectionType() const;
		void setDsnODBC ( const QString& theValue );
		QString dsnODBC() const;
		QString systemTablePrefix() const;
		void setSystemTablePrevix (const QString &value);
		QString fileSystemEncoding() const;
		void setSileSystemEncoding (const QString &value);

		int timeBetweenReloads();
		void setTimeBetweenReloads(int time);
		int strongFilterRowCountLimit();
		void setStrongFilterRowCountLimit(int limit);

		void applyDimensionForm (QWidget *formulario) const;
		void saveDimensionForm (QWidget *formulario);
		void applyPosForm(QWidget *formulario) const;
		void savePosForm(QWidget *formulario);

		bool debuggerEnabled();
		void setDebuggerEnabled(bool value);

		bool mdiTabView();

		QString externalResource();
		void setExternalResource(const QString &temp);

		QString firstDayOfWeek();
		void setFirstDayOfWeek(const QString &temp);

		QStringList tableViewColumnOrder(QTableView *tw);
		QStringList tableViewColumnSort(QTableView *tw);
		void saveTableViewColumnOrder(QTableView *tw, QStringList columnNames, QStringList columnOrder);
		void applyTableViewState(QTableView *tw);
		void saveTableViewState(QTableView *tw);

		QString tempPath() const;
		QLocale* getLocale() const;
        QString check();

		QDate minimunDate();

		void saveRegistryValue(const QString &key, const QVariant &value);
		QVariant loadRegistryValue(const QString &key);

		QString lastLoggedUser();
		void setLastLoggerUser(const QString &user);

		void setLookAndFeel ( int theValue );
		int lookAndFeel() const;

		void setBBDDEnconding ( const QString& theValue );
		QString bbddEncoding() const;
		
		QTextCodec* bbddCodec() const;
		QTextCodec* fileSystemCodec() const;

		void setInternalPDFViewer ( bool theValue );
		bool internalPDFViewer() const;

		int fontSizeReferenceReports();
		void setFontSizeReferenceReports( int theValue );

		int modelRefreshTimeout();
		void setModelRefreshTimeout(int value);

		void setMailServer ( const QString& theValue );
		QString getMailServer() const;
		void setMailResponderA ( const QString& theValue );
		QString getMailResponderA() const;
		void setMailCopiaA ( const QString& theValue );
		QString getMailCopiaA() const;
		void setMailFrom ( const QString& theValue );
		QString getMailFrom() const;

		void setExternalPDFViewer ( const QString& theValue );
		QString externalPDFViewer() const;
	
		Q_INVOKABLE QString formatToMoney(const QString &value);

		Q_INVOKABLE int uniqueId();
};

Q_ALEPHERP_EXPORT extern Configuracion configuracion;

Q_DECLARE_METATYPE(Configuracion*)

#endif
