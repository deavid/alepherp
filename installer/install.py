#!/usr/bin/python
# -*- coding: utf-8 -*-
import sys
from PyQt4 import QtGui, QtCore
import psycopg2

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
KEY_TIPO_CONEXION			= "db/TipoConexion"
KEY_DSN_ODBC				= "db/DSN_ODBC"
KEY_CODIFICACION_BBDD		= "db/codificacion"
KEY_ESQUEMA_BBDD			= "db/esquema_bbdd"
KEY_SYSTEM_TABLE_PREFIX		= "db/system_table_prefix"
KEY_FILESYSTEM_ENCODING		= "db/filesystem_encoding"


settings = QtCore.QSettings(COMPANY,APP_NAME)
 
class WPageIntroduccion(QtGui.QWizardPage):
    def __init__(self, *args, **kwargs):
        QtGui.QWizardPage.__init__(self, *args, **kwargs)
        self.setTitle(u"Bienvenido")
        textlines = []
        textlines += [u"Este asistente le ayudará a configurar AlephERP"]
        textlines += [u""]
        textlines += [u"Realizaremos los siguientes pasos en orden:"]
        textlines += [u"  1 -> Conectar a base de datos"]
        textlines += [u"  2 -> Configurar la conexión del cliente AlephERP"]
        label = QtGui.QLabel("\n".join(textlines))
        label.setWordWrap(True)

        layout = QtGui.QVBoxLayout()
        layout.addWidget(label)
        self.setLayout(layout)

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
        self.l = HBox(self.t,self.w, halign="lr")
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
            
        


class WPageDBConnect(QtGui.QWizardPage):
    def __init__(self, *args, **kwargs):
        QtGui.QWizardPage.__init__(self, *args, **kwargs)
        self.setTitle(u"Configurar la aplicación")

        layout = QtGui.QVBoxLayout()
        dbHost = LabelAndControl(u"&Host:", "string")
        dbPort = LabelAndControl(u"&Port:", "string", maxwidth=100)
        
        layout.addLayout(HBox(dbHost.l,dbPort.l))
        
        dbUser = LabelAndControl(u"&Usuario:", "string")
        dbPassword = LabelAndControl(u"&Password:", "password")
        
        layout.addLayout(HBox(dbUser.l,dbPassword.l))

        dbTipoConn = LabelAndControl(u"&Tipo Conexión:", "optionlist", items = ["NATIVA","ODBC","SQLITE"])
        dbCodificacion = LabelAndControl(u"&Encoding:", "optionlist", items = ["UTF-8","ISO-8859-15"])
        
        layout.addLayout(HBox(dbTipoConn.l,dbCodificacion.l))
        
        dbHost.readSetting(KEY_SERVIDORDB,"localhost")
        dbPort.readSetting(KEY_PORTDB,"5432")

        dbUser.readSetting(KEY_USUARIODB,"postgres")
        dbPassword.readSetting(KEY_PASSWORDDB,"postgres")
        
        dbTipoConn.readSetting(KEY_TIPO_CONEXION,"NATIVA")
        dbCodificacion.readSetting(KEY_CODIFICACION_BBDD,"UTF-8")
        
        self.setLayout(layout)
    
class WizardConfiguradorAlephERP(QtGui.QWizard):
    def __init__(self, *args, **kwargs):
        QtGui.QWizard.__init__(self, *args, **kwargs)
        self.setWindowTitle(u"Instalador/Configurador de AlephERP")
        
        self.pg_intro = WPageIntroduccion()
        self.pg_dbconn = WPageDBConnect()
        self.addPage(self.pg_intro)
        self.addPage(self.pg_dbconn)

def main():
    
    app = QtGui.QApplication(sys.argv)
    wizard = WizardConfiguradorAlephERP()
    wizard.show()
    
    sys.exit(app.exec_())


if __name__ == '__main__':
    main()
