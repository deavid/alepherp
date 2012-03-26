# encoding: UTF-8
from inst_utils import Struct
import re

TYPE_NUMBER = [
    # ---------------- ENTEROS
    "smallint",  # 2 bytes
    "int",       # 4 bytes
    "bigint",    # 8 bytes
    
    # ---------------- PRECISION EXACTA
    "decimal",
    "number",
    "money",

    # ---------------- PRECISION INEXACTA
    "real",             # 4 bytes
    "double", 
    "double precision", # 8 bytes
    
]

TYPE_SERIAL = [
    "serial",     # 4 bytes
    "bigserial",  # 8 bytes
]

TYPE_TEXT = [
    "char",
    "character",
    "varchar",
    "character varying",
    "text",
]

TYPE_BINARY = [
    "bytea",
]

TYPE_DATETIME = [
    "timestamp",
    "date",
    "time",
    "interval",
]

TYPE_BOOLEAN = [
    "boolean",
]

def field2serial(field):
    if field.format_extra and not re.match(r'nextval\(.+\)',field.format_extra): return
    if field.format_type == 'integer':
        field.format_extra = None
        field.format_type = 'serial'
        
    if field.format_type == 'bigint':
        field.format_extra = None
        field.format_type = 'bigserial'
     

def getfamilytype(field_type):
    field_type = field_type.lower()
    idx_paren = field_type.find('(')
    if idx_paren != -1:
        field_type = field_type[:idx_paren]
    
    if field_type in TYPE_NUMBER: return "number"
    if field_type in TYPE_SERIAL: return "serial"
    if field_type in TYPE_TEXT: return "text"
    if field_type in TYPE_BINARY: return "binary"
    if field_type in TYPE_DATETIME: return "datetime"
    if field_type in TYPE_BOOLEAN: return "boolean"
    return field_type # <- desconocido

def default4field(field):
    if field.default: return field.default
    if field.nullable: return None
    return default4type(field.format_type)
        
    
def default4type(field_type):
    familytype = getfamilytype(field_type)
    if familytype == "serial": return None
    if familytype == "number": return 0
    if familytype == "text": return ""
    if familytype == "datetime": return "1990-01-01 00:00:00"
    if familytype == "binary": return ""
    if familytype == "boolean": return False
    

def get_oids(conn):
    cur = conn.cursor()
    cur.execute("""
        SELECT c.oid,
          n.nspname,
          c.relname
        FROM pg_catalog.pg_class c
             LEFT JOIN pg_catalog.pg_namespace n ON n.oid = c.relnamespace
        WHERE n.nspname != 'pg_catalog' AND pg_catalog.pg_table_is_visible(c.oid)
        ORDER BY 2, 3;
        """)
    for oid, namespace, relname in cur:
        obj = Struct()
        obj.oid = oid
        obj.namespace = namespace
        obj.name = relname
        yield obj

def get_relname_oid(conn, relname):
    """
        Obtiene el OID de una relación (tabla, indice, pkey, etc)
    """
    cur = conn.cursor()
    cur.execute("""
        SELECT c.oid,
          n.nspname,
          c.relname
        FROM pg_catalog.pg_class c
             LEFT JOIN pg_catalog.pg_namespace n ON n.oid = c.relnamespace
        WHERE c.relname = %s AND pg_catalog.pg_table_is_visible(c.oid)
        ORDER BY 2, 3;
        """, [relname])
    for oid, namespace, relname in cur:
        obj = Struct()
        obj.oid = oid
        obj.namespace = namespace
        obj.name = relname
        return obj
        
def get_oid_info(conn, obj):
    """
        Obtiene información acerca del OID especificado. 
        (probablemente para saber si es tabla, índice, etc)
    """
    cur = conn.cursor()
    cur.execute("""
        SELECT relchecks, relkind, relhasindex, relhasrules, 
                reltriggers <> 0 as relhastriggers, 
                relhasoids, '' as reserved, reltablespace
        FROM pg_catalog.pg_class WHERE oid = %s
        """, [obj.oid])
    for relchecks, relkind, relhasindex, relhasrules, relhastriggers, relhasoids, reserved, reltablespace in cur:
        obj.relchecks = relchecks
        obj.relkind = relkind
        obj.relhasindex = relhasindex
        obj.relhasrules = relhasrules
        obj.relhastriggers = relhastriggers
        obj.relhasoids = relhasoids
        obj.reserved = reserved
        obj.reltablespace = reltablespace
        return obj
        

def get_table_columns(conn,obj):
    """
        Obtiene las columnas de una tabla
    """
    cur = conn.cursor()
    cur.execute("""
        SELECT a.attname,
          pg_catalog.format_type(a.atttypid, a.atttypmod),
          (SELECT substring(pg_catalog.pg_get_expr(d.adbin, d.adrelid) for 128) 
           FROM pg_catalog.pg_attrdef d
           WHERE d.adrelid = a.attrelid AND d.adnum = a.attnum AND a.atthasdef) as format_extra,
          a.attnotnull, a.attnum
        FROM pg_catalog.pg_attribute a
        WHERE a.attrelid = %s AND a.attnum > 0 AND NOT a.attisdropped
        ORDER BY a.attnum
        """, [obj.oid])
    for name, format_type, format_extra, notnull, number in cur:
        field = Struct()
        field.name = name
        field.format_type = format_type
        field.format_extra = format_extra
        field.default = None # <- probablemente esta en format_extra...
        field.notnull = notnull
        field.nullable = not field.notnull
        field.sql_nullable = "NOT NULL" if notnull else ""
        field.number = number
        yield field

def get_index_columns(conn,obj):
    """
        Obtiene las columnas de una tabla
    """
    cur = conn.cursor()
    cur.execute("""
        SELECT a.attname
        FROM pg_catalog.pg_attribute a
        WHERE a.attrelid = %s AND a.attnum > 0 AND NOT a.attisdropped
        ORDER BY a.attnum
        """, [obj.oid])
    return [ name for (name,)  in cur ] 
        
def get_table_indexes(conn,obj):
    """
        Obtiene los indices de una tabla
    """
    cur = conn.cursor()
    cur.execute("""
        SELECT c2.oid, c2.relname, i.indisprimary, i.indisunique, i.indisclustered, i.indisvalid, pg_catalog.pg_get_indexdef(i.indexrelid, 0, true), c2.reltablespace
        FROM pg_catalog.pg_class c, pg_catalog.pg_class c2, pg_catalog.pg_index i
        WHERE c.oid = %s AND c.oid = i.indrelid AND i.indexrelid = c2.oid
        ORDER BY i.indisprimary DESC, i.indisunique DESC, c2.relname
        """, [obj.oid])
        
    for oid, name, isprimary, isunique, isclustered, isvalid, sql_definition, tablespace_oid in cur:
        index = Struct()
        index.oid = oid
        index.name = name
        index.isprimary = isprimary
        index.isunique = isunique
        index.isclustered = isclustered
        index.isvalid = isvalid
        index.sql_definition = sql_definition
        index.tablespace_oid = tablespace_oid
        index.columns = get_index_columns(conn, index)
        yield index

def get_table_parents(conn,obj):
    """
        Obtiene las tablas padres de una tabla (tablas de las que hereda una tabla)
    """
    cur.execute("""
        SELECT c.oid::pg_catalog.regclass, c.relname, n.nspname
        FROM pg_catalog.pg_class c
            INNER JOIN pg_catalog.pg_inherits i ON c.oid=i.inhparent
            LEFT JOIN pg_catalog.pg_namespace n ON n.oid = c.relnamespace
        WHERE i.inhrelid = %s ORDER BY inhseqno
        """, [obj.oid])
    for oid, relname, namespace in cur:
        obj = Struct()
        obj.oid = oid
        obj.namespace = namespace
        obj.name = relname
        yield obj

