CREATE TABLE alepherp_envvars
(
  id serial NOT NULL,
  username character varying(255),
  variable character varying DEFAULT 255,
  data bytea,
  value text,
  CONSTRAINT alepherp_envvars_pkey PRIMARY KEY (id )
)
WITH (
  OIDS=TRUE
);

CREATE INDEX alepherp_envvars_username_idx
  ON alepherp_envvars
  USING btree
  (username );

CREATE INDEX alepherp_envvars_variable_idx
  ON alepherp_envvars
  USING btree
  (variable );

CREATE TABLE alepherp_history
(
  id serial NOT NULL,
  username character varying(255),
  action character varying(10),
  tablename character varying(255),
  pkey character varying(1000),
  changed_data text,
  "timestamp" timestamp without time zone DEFAULT now(),
  CONSTRAINT alepherp_history_pkey PRIMARY KEY (id )
)
WITH (
  OIDS=TRUE
);

CREATE INDEX alepherp_history_tablename_idx
  ON alepherp_history
  USING btree
  (tablename );

CREATE INDEX alepherp_history_username_idx
  ON alepherp_history
  USING btree
  (username );


CREATE TABLE alepherp_locks
(
  id serial NOT NULL,
  tablename character varying(150),
  username character varying(150),
  pk_serialize character varying(500),
  ts timestamp without time zone DEFAULT now(),
  CONSTRAINT alepherp_locks_pkey PRIMARY KEY (id )
)
WITH (
  OIDS=TRUE
);

CREATE INDEX alepherp_locks_pk_serialize_idx
  ON alepherp_locks
  USING btree
  (pk_serialize );

-- Index: alepherp_locks_username_idx

-- DROP INDEX alepherp_locks_username_idx;

CREATE INDEX alepherp_locks_username_idx
  ON alepherp_locks
  USING btree
  (username );


-- Rule: alepherp_locks_notify ON alepherp_locks

-- DROP RULE alepherp_locks_notify ON alepherp_locks;

CREATE OR REPLACE RULE alepherp_locks_notify AS
    ON DELETE TO alepherp_locks DO 
 NOTIFY breaklock;


CREATE TABLE alepherp_persmissions
(
  id serial NOT NULL,
  username character varying(255),
  tablename character varying(255),
  permissions character varying(10),
  id_rol integer,
  CONSTRAINT alepherp_persmissions_pkey PRIMARY KEY (id )
)
WITH (
  OIDS=TRUE
);

CREATE TABLE alepherp_roles
(
  id serial NOT NULL,
  nombre character varying(255),
  CONSTRAINT alepherp_roles_pkey PRIMARY KEY (id )
)
WITH (
  OIDS=TRUE
);

CREATE TABLE alepherp_system
(
  id serial NOT NULL,
  nombre character varying(250),
  contenido text,
  type character varying(10),
  debug boolean DEFAULT false,
  on_init_debug boolean DEFAULT false,
  version integer,
  CONSTRAINT presupuestos_system_pkey PRIMARY KEY (id )
)
WITH (
  OIDS=TRUE
);

CREATE TABLE alepherp_users
(
  username character varying(255) NOT NULL,
  password character varying(255),
  CONSTRAINT alepherp_users_pkey PRIMARY KEY (username )
)
WITH (
  OIDS=TRUE
);

CREATE TABLE alepherp_users_roles
(
  id serial NOT NULL,
  id_rol integer,
  username character varying(255),
  CONSTRAINT alepherp_users_roles_pkey PRIMARY KEY (id )
)
WITH (
  OIDS=TRUE
);

