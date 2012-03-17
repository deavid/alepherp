AlepERP - Guía Rápida
=============================

Para probar este proyecto actualmente es necesario compilarlo. Dado que es un
proyecto de Qt, se compila con qmake; make;

Dependencias
--------------------

Aún no hemos analizado todos los paquetes necesarios, pero inicialmente serían::

    libqt4-dev
    libqt4-webkit
    libqt4-sql-mysql
    libqt4-sql-pgsql
    libqt4-sql-sqlite

    pthread*


Compilación
--------------------

Primero ejecutamos "qmake" en la carpeta raíz y luego "make". "qmake" no debe
devolver ninguna advertencia::

    $ qmake
    $ make
    cd src/lib/config/ && /usr/bin/qmake /home/deavid/git/alepherp/deavid-alepherp/src/lib/config/config.pro -o Makefile
    cd src/lib/config/ && make -f Makefile 
    make[1]: se ingresa al directorio `/home/deavid/git/alepherp/deavid-alepherp/src/lib/config'
    g++ -c -pipe -g -D_REENTRANT -Wall -W -fPIC -DQT_WEBKIT -DVER="\"\"" -DQT_GUI_LIB -DQT_CORE_LIB -DQT_SHARED -I/usr/share/qt4/mkspecs/linux-g++ -I. -I/usr/include/qt4/QtCore -I/usr/include/qt4/QtGui -I/usr/include/qt4 -I../.. -I../../../build/tmp/alepherp/debug/moc -o ../../../build/tmp/alepherp/debug/objetos/configuracion.o configuracion.cpp
    (...)
    
Si todo termina bien, debería haber creado "build/bin/alepherp" que es el 
ejecutable final. Si no lo crea por algún motivo, vea más abajo la sección de 
Errores conocidos al compilar.

Instalación
--------------------

Necesitará al menos un servidor de base de datos con una base de datos vacía.
Recomendamos usar PostgreSQL. A la base de datos la llamaremos "alepherp". 

Debe cargar las tablas que aparecen en el fichero SQL de la raíz: alepherp.sql
Para ello, puede abrirlo con el PgAdmin3 y ejecutarlo directamente.

Desde consola, también puede hacer::

    $ psql alepherp < alepherp.sql
    
Debería funcionar igualmente cualquiera de los dos métodos.

En la tabla "alepherp_users", debe agregar un usuario, sin contraseña. En este
manual asumiremos que ha creado "root".

Debe cambiar los ajustes de la aplicación manualmente. En Windows están en el 
registro dentro de "AlephERP". En Linux están en::
    
    $HOME/.config/Aleph Sistemas de Informacion/AlephERP.conf

Se recomienda establecer las siguientes opciones::

    [db]                                                                                                                  
    servidor=localhost <- Servidor de base de datos
    port=5432 <- Puerto de base de datos
    usuario=alepherp <- Usuario de base de datos (no es el usuario de aplicación)
    password=alepherp 
    nombre=alepherp <- Nombre de la base de datos
    codificacion=UTF-8 
    esquema_bbdd=public <- Si la base de datos se encuentra ubicada en algún esquema específico
    TipoConexion=NATIVA <- Se conectará utilizando el driver QPSQL de Qt.
    DSN_ODBC= <- Solo si TipoConexion = ODBC
    system_table_prefix=alepherp <- prefijo de las tablas de sistema 

    [generales]
    debuggerEnabled=true <- Modo desarrollador

Iniciando el programa
-------------------------

Ejecute build/bin/alepherp , si algo falla o no es como se describe, vea la 
sección de errores conocidos al iniciar el programa.

En el primer arranque, si no puso contraseña al usuario, dejela en blanco. Al 
arrancar le pedirá que le asigne una contraseña.

Actualmente el programa da error al arrancar porque falta un formulario básico
como parte de la aplicación. Estamos trabajando para documentar esta parte.

Errores conocidos al compilar
------------------------------

Errores conocidos al iniciar el programa
-----------------------------------------

Preguntas frecuentes
------------------------

Q: ¿Cómo distingue AlephERP el tipo de base de datos, si es MySQL, PostgreSQL o SQLLite, es por el puerto?
A: ????





