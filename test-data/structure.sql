CREATE TABLE alepherp_concepto (
    id serial NOT NULL,
    tipo varchar(250) NOT NULL,
    descripcion varchar(250) NOT NULL,
    CONSTRAINT alepherp_concepto_pkey PRIMARY KEY (id)
);

CREATE TABLE alepherp_ejercicios (
    codejercicio varchar(4) NOT NULL,
    nombre varchar(100) NOT NULL,
    fechainicio date NOT NULL,
    fechafin date NOT NULL,
    estado varchar(15) NOT NULL,
    CONSTRAINT alepherp_ejercicios_pkey PRIMARY KEY (codejercicio)
);

