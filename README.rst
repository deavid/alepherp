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

Instalación Base de datos
-----------------------------

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

Inicialización de un proyecto
-------------------------------    

Debe definirse como mínimo un UI de tipo QMainDlg (pantalla principal)(*1) , y 
una tabla en un fichero .mtd y con eso debería funcionar.

Cuando arranque, el sistema buscará un archivo en la tabla "alepherp_system" 
que tenga como valores::

    * nombre: main.qmaindlg.ui
    * contenido: El contenido XML de una ventana generada con QtDesigner
    * type: ui
    * on_init_debug y debug a false
    * version: 1

También puedes asociarle un archivo .qs con código Qs, con las mismas columnas 
que antes, y nombre main.qmaindlg.qs y type: qs

*1: en AlephERP la pantalla principal no es fija, se lee de base de datos

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

Q: Qmake me devuelve dos avisos sobre que no localiza algún elemento

A: config.pri, ALEPHERPPATH debería valer $$PWD. Está solucionado en la última versión.

--

Q: No encuentra libconfig.so y/o libdaobusiness.so

A: config.pri, ALEPHERPPATH debería valer $$PWD. Está solucionado en la última versión.

--

Q: /usr/bin/ld: cannot find -lqcodeedit

A: Está solucionado en la última versión. Falta que copie libqcodeedit a la carpeta
lib de la compilación. Se puede copiar a mano.

--

Q: libhtmleditor.so / libqcodeedit.so.1, needed by (..)/libdaobusiness.so, not found 

A: Falta que en la compilación final enlace también con estas dos librerías. 
Está solucionado en la última versión.

--

Errores conocidos al iniciar el programa
-----------------------------------------

Q: No encuentra las librerías ".so" de la carpeta lib y no arranca.

A: Comprueba que las librerías estén en "../lib/" desde la ruta del ejecutable.
Si es así, hay un problema con rpath, que está solucionado en la última versión.

--

Q: Me da un montón de errores de SQL en "EXECUTE".

A: Es un bug del programa corregido en la última versión. Si tienes la última
versión, avisa al autor de cómo reproducir el problema. También es indicativo
de que estás usando una versión anticuada de la estructura de base de datos.

--

Últimos cambios en la estructura de sistema
----------------------------------------------

 * La tabla alepherp_system ahora tiene una nueva columna "version" integer.
 * La tabla printingerp_envvars ahora se llama alepherp_envvars.


Preguntas frecuentes
------------------------

Q: ¿Cómo distingue AlephERP el tipo de base de datos, si es MySQL, PostgreSQL o SQLLite, es por el puerto?

A: ????

--




