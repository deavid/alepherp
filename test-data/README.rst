Creación de aplicación inicial
==================================

AlephERP solo es un motor y no funcionará a menos que la base de datos donde
conectemos contenga una aplicación previamente. Esta sección describe los pasos
para realizar manualmente una aplicación básica que sirva como plantilla inicial.


Ficheros de la aplicación
----------------------------

En esta carpeta hemos puesto distintos ficheros, todos ellos hay que cargarlos
a la tabla "alepherp_system", del siguiente modo::

    * id: es autonumérico, dejamos que el sistema asigne un número secuencial.
    * nombre: el nombre del fichero en test-data
    * contenido: el contenido del fichero
    * type: puede ser "ui" o "table", según el tipo de fichero
    * debug: FALSE
    * on_init_debug: FALSE
    * version: 1
    
De este modo, cargaremos los tres ficheros a la tabla y quedará así::

    | id |    nombre           |    contenido    |  type  | debug | on_init_debug | version |
    |  1 | main.qmaindlg.ui    | <?xml version...|     ui | FALSE |         FALSE |       1 |
    |  2 | alepherp_concepto   | <table>\n<nam...|  table | FALSE |         FALSE |       1 |
    |  3 | alepherp_ejercicios | <table>\n<nam...|  table | FALSE |         FALSE |       1 |
    

Tablas de la aplicación
--------------------------

Los ficheros "table" que subimos corresponden a definiciones de tablas, pero 
el sistema no crea ni adapta las tablas existentes. Así que las tendremos que
crear y actualizar manualmente.

Hemos preparado en "structure.sql" la consulta que crea estas dos tablas. Podemos
ejecutarla con PgAdmin3 o con psql.

Permisos
----------------

Para que nos de acceso a todas las tablas, tenemos que crear un registro en la tabla
alepherp_permissions. Asumiendo que el usuario es "root", el registro sería::

    | id | username | tablename | permissions | id_rol
    |  1 |     root |         * |          rw | NULL
    
Si queremos especificar con más detalle, necesitaremos un registro por tabla y usuario.



Problemas conocidos
------------------------

La aplicación buscará los ficheros en la carpeta temporal que se especificó en la
configuración, pero parece que por el momento no los copia allí. Es necesario
copiar manualmente los ficheros.

**IMPORTANTE**: Si modificamos la tabla alepherp_system, debemos aumentar la versión
de los ficheros, al estilo revisión SVN. Un cliente sólo actualizará los ficheros con
versión superior a la última versión global que haya descargado alguna vez. Si no 
sincroniza, podemos borrar la basede datos sqlite que se genera en la carpeta temporal.


