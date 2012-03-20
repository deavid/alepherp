#!/usr/bin/python
# -*- coding: utf-8 -*-
import sys, socket
from PyQt4 import QtGui, QtCore
import psycopg2
import traceback
import threading

class ConnectionError(Exception):
    """ Error relacionado con las conexiones
    """

Qt = QtCore.Qt
# Defines copiados desde src/lib/config/configuracion.h
COMPANY	= "Aleph Sistemas de Informacion"
APP_NAME = "AlephERP"

# Defines copiados desde src/lib/config/configuracion.cpp
KEY_SERVIDORDB				= "db/servidor"
KEY_PORTDB                  = "db/port"
KEY_USUARIODB				= "db/usuario"
KEY_PASSWORDDB				= "db/password"
KEY_NOMBREDB				= "db/nombre"
KEY_SYSTEM_TABLE_PREFIX		= "db/system_table_prefix"
KEY_TIPO_CONEXION			= "db/TipoConexion"
KEY_DSN_ODBC				= "db/DSN_ODBC"
KEY_CODIFICACION_BBDD		= "db/codificacion"
KEY_ESQUEMA_BBDD			= "db/esquema_bbdd"
KEY_FILESYSTEM_ENCODING		= "db/filesystem_encoding"


settings = QtCore.QSettings(COMPANY,APP_NAME)

def HBox(*widgets,**kwargs):
    layout = QtGui.QHBoxLayout()
    haligment = list(kwargs.get("halign","")+ "0" * len(widgets))[:len(widgets)]
        
    for w,ha in zip(widgets,haligment):
        if isinstance(w, QtGui.QLayout):
            layout.addLayout(w)
        else:
            align = 0
            stretch = 0
            if ha == "l": align += Qt.AlignLeft
            if ha == "c": align += Qt.AlignHCenter
            if ha == "r": align += Qt.AlignRight
            if ha == "j": align += Qt.AlignJustify
            layout.addWidget(w, stretch, Qt.Alignment(align))
    return layout
    
class LabelAndControl(object):
    def __init__(self, title, fieldtype, **kwargs):
        self.fieldtype = fieldtype
        if self.fieldtype in ['string','password']:
            widget = QtGui.QLineEdit()
            
        if self.fieldtype in ['optionlist']:
            widget = QtGui.QComboBox()
            if "items" in kwargs:
                widget.addItems(kwargs['items'])
            
        self.t = QtGui.QLabel(title)
        self.w = widget
        self.t.setBuddy(self.w)
        self.l = HBox(self.t,self.w, halign="l0")
        maxwidth = kwargs.get("maxwidth", None)
        if maxwidth: self.w.setMaximumWidth(maxwidth)
        if self.fieldtype == "password": 
            self.w.setEchoMode(QtGui.QLineEdit.Password)
    
    def readSetting(self, key, defaultvalue=None):
        self.settings_key = key
        variant = settings.value(key,defaultvalue)
        if isinstance(self.w, QtGui.QLineEdit):
            value = variant.toString()
            self.w.setText(value)
        elif isinstance(self.w, QtGui.QComboBox):
            value = variant.toString()
            idx = self.w.findText(value)
            if idx == -1:
                self.w.addItem(value)
                idx = self.w.findText(value)
            if idx == -1: raise AssertionError
            self.w.setCurrentIndex(idx)
                
        else:
            raise NotImplementedError("No puedo asignar a un tipo de control %s" % (self.w.__class__.__name__))

    def writeSetting(self, key=None):
        if key is None: key = self.settings_key
        value = None
        if isinstance(self.w, QtGui.QLineEdit):
            value = self.w.text()
        elif isinstance(self.w, QtGui.QComboBox):
            value = self.w.currentText()
        else:
            raise NotImplementedError("No puedo leer de un tipo de control %s" % (self.w.__class__.__name__))
        
        settings.setValue(key,value)
            
def gui_exception_handling(fn):
    def myfn(self, *args,**kwargs):
        ret = None
        try:
            ret = fn(self, *args,**kwargs)
        except Exception, e:
            print traceback.format_exc()
            try: stre = unicode(e)
            except Exception: 
                try: stre = unicode(e,"UTF-8","replace")
                except Exception: stre = repr(e)
            
            QtGui.QMessageBox.warning(self, e.__class__.__name__,
                                stre,
                                QtGui.QMessageBox.Ok,
                                QtGui.QMessageBox.NoButton
                                )
        return ret
    return myfn
        
 
class WizardPage(QtGui.QWizardPage):
    def __init__(self, *args, **kwargs):
        self.parent = kwargs['parent']
        del kwargs['parent']
        QtGui.QWizardPage.__init__(self, *args, **kwargs)
        self.setup()
        self.page_id = self.parent.addPage(self)
        
        
    def setup(self):
        pass
 
    def validatePage(self):
        try:
            self.validate()
            return True
        except Exception, e:
            print traceback.format_exc()
            try: stre = unicode(e)
            except Exception: 
                try: stre = unicode(e,"UTF-8","replace")
                except Exception: stre = repr(e)
            
            QtGui.QMessageBox.warning(self, e.__class__.__name__,
                                stre,
                                QtGui.QMessageBox.Ok,
                                QtGui.QMessageBox.NoButton
                                )
            return False
            
    def validate(self):
        pass
    
    def do_something_slow(self, function_ptr, title = u"Trabajando", label = u"Por favor, espere . . ."):
        result = {}
        def do_the_slow(function_ptr = function_ptr, result = result):
            try: 
                ret = function_ptr()
                result['value'] = ret
                result['success'] = True
                result['error'] = False
            except Exception, e:
                result['exception'] = e
                result['success'] = False
                result['error'] = True
                
        progress = QtGui.QProgressDialog(label, QtCore.QString(), 0,  100, self)
        progress.setWindowTitle(title)
        progress.setWindowModality(Qt.WindowModal)
        number = 0

        thread1 = threading.Thread(target=do_the_slow)
        thread1.start()
        thread1.join(0.20)
        if thread1.isAlive(): progress.show()
        while thread1.isAlive():
            number += 1
            if number >= 100: number -= 20
            progress.setValue(number)
            for i in range(number):
                thread1.join(20.0 / (101 - number) / number)
                QtGui.QApplication.processEvents()
        progress.reset()
        
        if result['success']: return result['value']
        else: raise result['exception']
        
    @gui_exception_handling
    def button_create_database_clicked(self, checked):
        for control in self.controls:
            control.writeSetting()
        
        dbname = str(settings.value(KEY_NOMBREDB).toString())
        
        self.do_something_slow(lambda: self.test_current_parameters("template1"), 
            title = u"Conectando", 
            label = u"Conectando en modo mantenimiento (db: template1) . . ."
            )
        
        cur = self.parent.cur
        self.parent.cur = None
        self.parent.conn = None
        def create_db():
            try:
                cur.execute("""CREATE DATABASE "%s" """ % dbname)
            except psycopg2.Error, e:
                if e.pgcode == "42P04":
                    raise ConnectionError(u"La base de datos %s ya existe." % (repr(dbname)))
                else:
                    raise ConnectionError(u"Error al crear la base de datos %s. Motivo: %s (%s)" % (repr(dbname), unicode(e.pgerror.strip(),"UTF-8", "replace"), e.pgcode))
        self.do_something_slow(create_db, 
            title = u"Creando Base de Datos", 
            label = u"Se está procediendo a crear la base de datos %s . . ." % (repr(dbname))
            )
            
        QtGui.QMessageBox.information(self, u"Creación de base de datos",
                            u"La creación de la base de datos %s finalizó correctamente." % (repr(dbname)),
                            QtGui.QMessageBox.Ok,
                            QtGui.QMessageBox.NoButton
                            )
                            
    def test_current_parameters(self, dbname = None):
        if dbname is None: dbname = str(settings.value(KEY_NOMBREDB).toString())
        username = str(settings.value(KEY_USUARIODB).toString())
        password = str(settings.value(KEY_PASSWORDDB).toString())
        host = str(settings.value(KEY_SERVIDORDB).toString())
        port = int(settings.value(KEY_PORTDB).toString())
        try:
            conn = psycopg2.connect(database=dbname, user=username, host=host, port=port, password=password, connect_timeout=3)
        except psycopg2.Error, e:
            if e.pgerror is None: e.pgerror = str(e)
            raise ConnectionError(u"Error al conectar a la base de datos %s. Motivo: %s (%s)" % (repr(dbname), unicode(e.pgerror.strip(),"UTF-8", "replace"), e.pgcode))
        
        conn.set_isolation_level(0)
        cur = conn.cursor()
        self.parent.conn = conn
        self.parent.cur = cur
                            
    

class WPageIntroduccion(WizardPage):
    def setup(self):
        self.setTitle(u"Bienvenido")
        textlines = []
        textlines += [u"Este asistente le ayudará a configurar AlephERP"]
        textlines += [u""]
        textlines += [u"Elija el modo de configurar la conexión:"]
        label = QtGui.QLabel("\n".join(textlines))
        label.setWordWrap(True)

        layout = QtGui.QVBoxLayout()
        layout.addWidget(label)
        
        self.opt_asistente = QtGui.QRadioButton(u"Iniciar el asistente de conexión")
        self.opt_avanzado = QtGui.QRadioButton(u"Configuración manual")
        self.opt_omitircfg = QtGui.QRadioButton(u"La conexión ya está correctamente configurada")
        self.opt_omitircfg.hide()
        self.opt_asistente.setChecked(True)
        layout.addWidget(self.opt_asistente)
        layout.addWidget(self.opt_avanzado)
        layout.addWidget(self.opt_omitircfg)
        
        self.setLayout(layout)
        
    def initializePage(self):
        QtCore.QTimer.singleShot(100, self.postInitPage)
        
    def postInitPage(self):
        try:
            self.do_something_slow(self.test_current_parameters, 
                title = u"Comprobando conexión", 
                label = u"Se está probando la configuración actual . . ."
                )
        except Exception, e:
            return None
        self.opt_omitircfg.show()
        self.opt_omitircfg.setChecked(True)
            
    def nextId(self):
        if self.opt_asistente.isChecked():
            return self.parent.pg_asist_conn1.page_id
        if self.opt_avanzado.isChecked():
            return self.parent.pg_dbconn.page_id
        if self.opt_omitircfg.isChecked():
            return self.parent.pg_conn_completa.page_id
            
class WPageAsistenteConexion1(WizardPage):
    def setup(self):
        self.setTitle(u"Asistente de conexión (1 de 3)")

        layout = QtGui.QVBoxLayout()
        textlines = []
        textlines += [u"Escoja el tipo de conexión y la codificación de la conexión"]
        textlines += [u""]
        textlines += [u"Los valores recomendados son 'NATIVA' y 'UTF-8'."]
        textlines += [u"El tipo de conexión establece si es PostgreSQL, driver nativo"
            + u", a través de ODBC o si en cambio, la base de datos es SQLITE"]
        textlines += [u"NOTA: Este asistente sólo soporta NATIVA y UTF-8"]
        textlines += [u""]
        label = QtGui.QLabel("\n".join(textlines))
        label.setWordWrap(True)
        layout.addWidget(label)

        dbTipoConn = LabelAndControl(u"&Tipo Conexión:", "optionlist", items = ["NATIVA","ODBC","SQLITE"])
        dbCodificacion = LabelAndControl(u"&Encoding:", "optionlist", items = ["UTF-8","ISO-8859-15"])
        
        layout.addLayout(HBox(dbTipoConn.l,dbCodificacion.l))
        
        dbTipoConn.readSetting(KEY_TIPO_CONEXION,"NATIVA")
        dbCodificacion.readSetting(KEY_CODIFICACION_BBDD,"UTF-8")
        
        self.setLayout(layout)
        
        self.controls = [
            dbTipoConn, dbCodificacion,
        ]
            
    def validate(self):
        for control in self.controls:
            control.writeSetting()
        tipo_conexion = settings.value(KEY_TIPO_CONEXION).toString()
        encoding = settings.value(KEY_CODIFICACION_BBDD).toString()
        if tipo_conexion != "NATIVA": raise ValueError(u"Este asistente sólo soporta conexión NATIVA.")
        if encoding != "UTF-8": raise ValueError(u"Este asistente sólo soporta codificación UTF-8.")

class WPageAsistenteConexion2(WizardPage):
    def setup(self):
        self.setTitle(u"Asistente de conexión (2 de 3)")

        layout = QtGui.QVBoxLayout()
        textlines = []
        textlines += [u"Determine el Hostname y el puerto al que conectarse."]
        textlines += [u""]
        textlines += [u"AlephERP se conecta a un servidor PostgreSQL para la base de datos. " + 
                u"Necesita especificar la dirección IP (o hostname) del equipo donde está alojada la base de datos" + 
                u", así como el número de puerto en el que se puede acceder a la base de datos."]
        textlines += [u""]
        textlines += [u"Para PostgreSQL el puerto por defecto es el 5432. Si el servidor está funcionando en " + 
                u"este mismo equipo, puede especificar 'localhost' como Hostname."]
        textlines += [u""]
        label = QtGui.QLabel("\n".join(textlines))
        label.setWordWrap(True)
        layout.addWidget(label)

        dbHost = LabelAndControl(u"&Host:", "string")
        dbPort = LabelAndControl(u"&Port:", "string")
        
        layout.addLayout(HBox(dbHost.l,dbPort.l))
        
        dbHost.readSetting(KEY_SERVIDORDB,"localhost")
        dbPort.readSetting(KEY_PORTDB,"5432")
        
        self.setLayout(layout)
        
        self.controls = [
            dbHost, dbPort,
        ]
    
    def validate(self):
        for control in self.controls:
            control.writeSetting()
        try: host = str(settings.value(KEY_SERVIDORDB).toString())
        except ValueError: raise ValueError(u"El host no es válido")
        try: port = int(settings.value(KEY_PORTDB).toString())
        except ValueError: raise ValueError(u"El número de puerto no es válido, introduzca sólo números")
        if port <= 0 or port > 65535: raise ValueError(u"El número de puerto no es válido, debe estar entre 1 y 65535")
        def test_conn():
            try:
                sck = socket.create_connection( (host, port) , 3)
            except socket.error, e:
                errno, text = e
                raise ConnectionError(u"Error %d al conectar a %s:%d (motivo: %s)" % (errno, host, port, unicode(text,"UTF-8","replace")))
            except socket.timeout, e:
                text = e
                raise ConnectionError(u"Timeout al conectar a %s:%d (motivo: %s)" % (host, port, unicode(text,"UTF-8","replace")))
            except socket.herror, e:
                errno, text = e
                raise ConnectionError(u"Error %d al interpretar el host %s (motivo: %s)" % (errno, host, unicode(text,"UTF-8","replace")))
            except socket.gaierror, e:
                errno, text = e
                raise ConnectionError(u"Error %d al obtener dirección del host %s (motivo: %s)" % (errno, host, unicode(text,"UTF-8","replace")))
            try: sck.shutdown(socket.SHUT_RDWR)
            except Exception: pass
            sck.close()

        self.do_something_slow(test_conn, 
            title = u"Probando la conexión", 
            label = u"Se está procediendo a conectar por TCP/IP a %s:%d . . ." % (host,port)
            )

            
class WPageAsistenteConexion3(WizardPage):
    def setup(self):
        self.setTitle(u"Asistente de conexión (3 de 3)")

        layout = QtGui.QVBoxLayout()
        label = QtGui.QLabel(u"Vamos a realizar la conexión con la base de datos. "
            + u"Indique el usuario y contraseña para autenticarse en el servidor. "
            + u"Es necesario que tenga permisos de administrador.")
        label.setWordWrap(True)
        layout.addWidget(label)

        dbUser = LabelAndControl(u"&Usuario:", "string")
        dbPassword = LabelAndControl(u"&Password:", "password")
               
        layout.addLayout(HBox(dbUser.l,dbPassword.l))

        label = QtGui.QLabel(u"Especifique el nombre de la base de datos para AlephERP. "
            + u"Si aún no está creada, el botón <CREATE> creará una vacía. ")
        label.setWordWrap(True)
        layout.addWidget(label)

        self.btnCrearDB = QtGui.QPushButton(u"CREATE")
        
        dbName = LabelAndControl(u"&Base de Datos:", "string")
        self.btnCrearDB.clicked.connect(self.button_create_database_clicked)
        layout.addLayout(HBox(dbName.l,self.btnCrearDB))

        label = QtGui.QLabel(u"Especifique el prefijo que desea para las tablas de sistema. "
            + u"Todas las tablas internas de AlephERP quedarán como PREFIJO_nombretabla.")
        label.setWordWrap(True)
        layout.addWidget(label)

        dbTablePrefix = LabelAndControl(u"&Prefijo de Tablas de Sistema:", "string")
        layout.addLayout(dbTablePrefix.l)

        dbUser.readSetting(KEY_USUARIODB,"postgres")
        dbPassword.readSetting(KEY_PASSWORDDB,"postgres")
        dbName.readSetting(KEY_NOMBREDB,"alepherp")
        dbTablePrefix.readSetting(KEY_SYSTEM_TABLE_PREFIX,"alepherp")
        
        self.setLayout(layout)
        
        self.controls = [
            dbUser, dbPassword, 
            dbName, dbTablePrefix
        ]
        self.setCommitPage(True)

    
    def validate(self):
        for control in self.controls:
            control.writeSetting()

        self.do_something_slow(self.test_current_parameters, 
            title = u"Conectando", 
            label = u"Se está conectando a la base de datos . . ."
            )

class WPageDBConnect(WizardPage):
    def setup(self):
        self.setTitle(u"Configurar manualmente la aplicación")

        layout = QtGui.QVBoxLayout()
        dbHost = LabelAndControl(u"&Host:", "string")
        dbPort = LabelAndControl(u"&Port:", "string")
        
        layout.addLayout(HBox(dbHost.l,dbPort.l))
        
        dbUser = LabelAndControl(u"&Usuario:", "string")
        dbPassword = LabelAndControl(u"&Password:", "password")
        
        layout.addLayout(HBox(dbUser.l,dbPassword.l))

        dbTipoConn = LabelAndControl(u"&Tipo Conexión:", "optionlist", items = ["NATIVA","ODBC","SQLITE"])
        dbCodificacion = LabelAndControl(u"&Encoding:", "optionlist", items = ["UTF-8","ISO-8859-15"])
        
        layout.addLayout(HBox(dbTipoConn.l,dbCodificacion.l))
        
        dbTablePrefix = LabelAndControl(u"&Prefijo de Tablas de sistema:", "string")
        
        layout.addLayout(dbTablePrefix.l)

        label = QtGui.QLabel(u"Especifique el nombre de la base de datos para AlephERP. "
            + u"Si aún no está creada, el botón <CREATE> creará una vacía. ")
        label.setWordWrap(True)
        layout.addWidget(label)

        self.btnCrearDB = QtGui.QPushButton(u"CREATE")
        
        dbName = LabelAndControl(u"&Base de Datos:", "string")
        self.btnCrearDB.clicked.connect(self.button_create_database_clicked)
        layout.addLayout(HBox(dbName.l,self.btnCrearDB))

        
        dbHost.readSetting(KEY_SERVIDORDB,"localhost")
        dbPort.readSetting(KEY_PORTDB,"5432")

        dbUser.readSetting(KEY_USUARIODB,"postgres")
        dbPassword.readSetting(KEY_PASSWORDDB,"postgres")
        
        dbTipoConn.readSetting(KEY_TIPO_CONEXION,"NATIVA")
        dbCodificacion.readSetting(KEY_CODIFICACION_BBDD,"UTF-8")
        
        dbName.readSetting(KEY_NOMBREDB,"alepherp")
        dbTablePrefix.readSetting(KEY_SYSTEM_TABLE_PREFIX,"alepherp")
        
        self.setLayout(layout)
        
        self.controls = [
            dbHost, dbPort,
            dbUser, dbPassword,
            dbTipoConn, dbCodificacion,
            dbName, dbTablePrefix
        ]
        self.setCommitPage(True)

    
    def nextId(self):
        return self.parent.pg_conn_completa.page_id

    def validate(self):
        for control in self.controls:
            control.writeSetting()
            
        self.do_something_slow(self.test_current_parameters, 
            title = u"Conectando", 
            label = u"Se está conectando a la base de datos . . ."
            )
        
class WPageConexionCompletada(WizardPage):
    def setup(self):
        self.setTitle(u"Conexión comletada")
        textlines = []
        textlines += [u"Se ha podido establecer conexión a la base de datos."]
        textlines += [u""]
        textlines += [u"Indique qué acción quiere realizar ahora:"]
        label = QtGui.QLabel("\n".join(textlines))
        label.setWordWrap(True)

        layout = QtGui.QVBoxLayout()
        layout.addWidget(label)
        # TODO : Agregar aquí el resto de opciones que son interesantes/convenientes en AlephERP...
        
        # - - - - - - - - - 
        self.opt_creartablas = QtGui.QRadioButton(u"Crear las tablas de sistema de AlephERP")
        self.opt_crearusuario = QtGui.QRadioButton(u"Crear un usuario de administración")
        self.opt_cargarproyecto = QtGui.QRadioButton(u"Cargar un proyecto inicial")
        self.opt_creartablas.setChecked(True)
        layout.addWidget(self.opt_creartablas)
        layout.addWidget(self.opt_crearusuario)
        layout.addWidget(self.opt_cargarproyecto)
        
        self.setLayout(layout)
    
    
class WizardConfiguradorAlephERP(QtGui.QWizard):
    def __init__(self, *args, **kwargs):
        QtGui.QWizard.__init__(self, *args, **kwargs)
        self.setWindowTitle(u"Instalador/Configurador de AlephERP")
        
        self.pg_intro = WPageIntroduccion(parent=self)
        self.pg_dbconn = WPageDBConnect(parent=self)
        self.pg_asist_conn1 = WPageAsistenteConexion1(parent=self)
        self.pg_asist_conn2 = WPageAsistenteConexion2(parent=self)
        self.pg_asist_conn3 = WPageAsistenteConexion3(parent=self)
        self.pg_conn_completa = WPageConexionCompletada(parent=self)
        
    

def main():
    
    app = QtGui.QApplication(sys.argv)
    wizard = WizardConfiguradorAlephERP()
    wizard.show()
    
    sys.exit(app.exec_())


if __name__ == '__main__':
    main()
