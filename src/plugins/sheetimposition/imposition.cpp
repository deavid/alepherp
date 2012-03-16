/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the Graphics Dojo project on Qt Labs.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 or 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#include "imposition.h"
#include <math.h>
#include <QScriptEngine>

class ImpositionPrivate
{
public:
	Imposition::ImpositionTypes m_itemType;
	Imposition::ImpositionMode m_impositionMode;
	double m_pinzas;
	double m_sangre;
	double m_largo;
	double m_ancho;
	double m_pliegoAncho;
	double m_pliegoLargo;
	int m_numEjemplares;

	ImpositionPrivate(){
		m_pinzas = 0;
		m_sangre = 0;
		m_largo = 0;
		m_ancho = 0;
		m_pliegoAncho = 0;
		m_pliegoLargo = 0;
		m_itemType = Imposition::SINGLE;
		m_impositionMode = Imposition::WIDHT_HEIGHT;
	}
};

Imposition::Imposition(QObject *parent) : QObject(parent), d(new ImpositionPrivate)
{
}

Imposition::~Imposition()
{
	delete d;
}

void Imposition::setItemType(Imposition::ImpositionTypes itemType)
{
	d->m_itemType = itemType;
}

Imposition::ImpositionTypes Imposition::itemType()
{
	return d->m_itemType;
}

Imposition::ImpositionMode Imposition::impositionMode()
{
	return d->m_impositionMode;
}

void Imposition::setPinzas(double value)
{
	d->m_pinzas = value;
}

double Imposition::pinzas()
{
	return d->m_pinzas;
}

void Imposition::setSangre(double value)
{
	d->m_sangre = value;
}

double Imposition::sangre()
{
	return d->m_sangre;
}

void Imposition::setAncho(double value)
{
	d->m_ancho = value;
}

double Imposition::ancho()
{
	return d->m_ancho;
}

void Imposition::setLargo(double value)
{
	d->m_largo = value;
}

double Imposition::largo()
{
	return d->m_largo;
}

void Imposition::setPliegoAncho(double value)
{
	d->m_pliegoAncho = value;
}

double Imposition::pliegoAncho()
{
	return d->m_pliegoAncho;
}

void Imposition::setPliegoLargo(double value)
{
	d->m_pliegoLargo = value;
}

double Imposition::pliegoLargo()
{
	return d->m_pliegoLargo;
}

QScriptValue Imposition::toScriptValueSharedPointer(QScriptEngine *engine, const QSharedPointer<Imposition> &in)
{
	return engine->newQObject(in.data(), QScriptEngine::AutoOwnership);
}

void Imposition::fromScriptValueSharedPointer(const QScriptValue &object, QSharedPointer<Imposition> &out)
{
	out = QSharedPointer<Imposition>(qobject_cast<Imposition *>(object.toQObject()));
}

/*!
El cálculo del número de páginas por cara, no deja de ser un cálculo de superficies
Debemos tener en cuenta, la superficie del trabajo a imprimir, la final. A eso
hay que añadirle superficies del papel que no pueden ser impresas, como la zona
de pinzas de máquinas, la tira de control, o los márgenes del trabajo para
el corte (sangre). Ahora bien, es un cálculo de superficies particular. Pongamos un ejemplo
Tenemos un trabajo en 20x21, que debe entrar en un pliego 50x70. 20x21 = 420 cm2. 50x70 = 3500cm2.
Si dividimos 3500 / 420 = 8,33. Nos diría que entran 8 ejemplares por pliego.
Eso es un error. Si nos fijamos en las medidas, es imposible que entren 8.
El error están en que debemos suponer estructuras compactas, es decir, la división
no es de superficies, sino de cuadrados INDIVISIBLES de 20x21 que deben entrar en 50x70.
La idea del algoritmo es situar los rectángulos finales dentro del pliego total.
*/
void Imposition::imposition (QList<QRectF> &positions, int &rows, int &cols)
{

	// El ancho del pliego es la parte que consideraremos, entra a la máquina
	double anchoPliego, largoPliego, anchoTrabajo, largoTrabajo;
	int opcion1, opcion2;
	int paginasPorCara = 0;
	rows = 0;
	cols = 0;

	// Aquí lo que hacemos es que, llamaremos ancho del pliego a la parte más grande
	// del pliego. Las pinzas cogen el ancho del pliego para arrastrar el papel, pero eso
	// afecta al largo del pliego
	if ( d->m_pliegoAncho > d->m_pliegoLargo ) {
		anchoPliego = d->m_pliegoAncho;
		largoPliego = d->m_pliegoLargo;
	} else {
		anchoPliego = d->m_pliegoLargo;
		largoPliego = d->m_pliegoAncho;
	}

	// Obtengamos la superficie útil del pliego, es decir la imprimible menos la pinza de máquina
	largoPliego = largoPliego - d->m_pinzas;

	if ( d->m_ancho > 0 && d->m_largo > 0 ) {
		// OJO: Si la sección es de un libro, ¡¡no lleva sangre entre las páginas!!
		if ( d->m_itemType != Imposition::BOOK ) {
			anchoTrabajo = d->m_ancho + ( d->m_sangre * 2 );
			largoTrabajo = d->m_largo + ( d->m_sangre * 2 );
		} else {
			anchoTrabajo = d->m_ancho;
			largoTrabajo = d->m_largo;
			// En el caso de un libro, el truquito es disminuir un poquito el tamaño del pliego
			// para así tener la superficie de impresión real
			largoPliego = largoPliego - d->m_sangre;
			anchoPliego = anchoPliego - d->m_sangre;
		}
		// El trabajo es rectangular siempre. De dimensión A x B. El pliego es de dimensión C x D.
		/*
			A
		-------------
		|			|
	B	|			|
		|			|
		|			|
		-------------
		Como es rectángulo debe entrar en otro de dimensiones C x D, veremos para las dos posibles
		combinaciones cuántas veces entra ese rectángulo en el pliego.
		*/
		opcion1 = floor ( anchoPliego / anchoTrabajo ) * floor ( largoPliego / largoTrabajo );
		opcion2 = floor ( anchoPliego / largoTrabajo ) * floor ( largoPliego / anchoTrabajo );
		if ( opcion1 > opcion2 ) {
			paginasPorCara = opcion1;
		} else {
			paginasPorCara = opcion2;
		}
	}

	// El número de páginas por cara, debe ser par, y no sólo eso, debe ser además múltiplo de
	// 4, 8 o 16... en el caso de estar calculando pliegos para un libro
	if ( d->m_itemType == Imposition::BOOK ) {
		QList<int> posibilidades;
		int pos = 0;
		posibilidades << 2 << 4 << 8 << 16 << 32 << 64 << 128 << 256;
		// Si el número de páginas que nos da no es alguno de los números de la lista, aproximamos
		// al menor de las páginas por cara inferior a posibilidades
		if ( !posibilidades.contains ( paginasPorCara ) ) {
			for ( int i = 0 ; i < posibilidades.size() ; i++ ) {
				if ( posibilidades.at(i) < paginasPorCara ) {
					pos = i;
				}
			}
			int lowPot = pos / 2;
			rows = pow(2, lowPot);
			cols = pow(2, (pos - lowPot));
		}
	} else {
		if ( opcion1 >= opcion2 ) {
			cols = floor( anchoPliego / anchoTrabajo );
			rows = floor( largoPliego / largoTrabajo );
			double offsetAncho = (anchoPliego - (cols * anchoTrabajo)) / 2;
			double offsetLargo = (largoPliego - (rows * largoTrabajo)) / 2;
			// Este debe ser el orden de los bucles
			for ( int row = 0 ; row < rows ; row++ ) {
				for ( int col = 0 ; col < cols ; col++ ) {
					QRectF rect(offsetAncho + (anchoTrabajo * col), d->m_pinzas + offsetLargo + (largoTrabajo * row),
								anchoTrabajo, largoTrabajo);
					positions.append(rect);
				}
			}
			d->m_impositionMode = Imposition::WIDHT_HEIGHT;
		} else {
			cols = floor ( anchoPliego / largoTrabajo );
			rows = floor ( largoPliego / anchoTrabajo );
			double offsetAncho = (anchoPliego - (cols * largoTrabajo)) / 2;
			double offsetLargo = (largoPliego - (rows * anchoTrabajo)) / 2;
			// Este debe ser el orden de los bucles
			for ( int row = 0 ; row < rows ; row++ ) {
				for ( int col = 0 ; col < cols ; col++ ) {
					QRectF rect(offsetAncho + (largoTrabajo * col), d->m_pinzas + offsetLargo + (anchoTrabajo * row),
								 largoTrabajo, anchoTrabajo);
					positions.append(rect);
				}
			}
			d->m_impositionMode = Imposition::HEIGHT_WIDTH;
		}
	}
}

/*!
El cálculo del número de páginas por cara, no deja de ser un cálculo de superficies
Debemos tener en cuenta, la superficie del trabajo a imprimir, la final. A eso
hay que añadirle superficies del papel que no pueden ser impresas, como la zona
de pinzas de máquinas, la tira de control, o los márgenes del trabajo para
el corte (sangre). Ahora bien, es un cálculo de superficies particular. Pongamos un ejemplo
Tenemos un trabajo en 20x21, que debe entrar en un pliego 50x70. 20x21 = 420 cm2. 50x70 = 3500cm2.
Si dividimos 3500 / 420 = 8,33. Nos diría que entran 8 ejemplares por pliego.
Eso es un error. Si nos fijamos en las medidas, es imposible que entren 8.
El error están en que debemos suponer estructuras compactas, es decir, la división
no es de superficies, sino de cuadrados INDIVISIBLES de 20x21 que deben entrar en 50x70.
La idea del algoritmo es situar los rectángulos finales dentro del pliego total.
*/
int Imposition::numItemsPerSheetFace()
{
	// El ancho del pliego es la parte que consideraremos, entra a la máquina
	double anchoPliego, largoPliego, anchoTrabajo, largoTrabajo;
	int opcion1, opcion2;
	int paginasPorCara = 0;

	// Aquí lo que hacemos es que, llamaremos ancho del pliego a la parte más grande
	// del pliego. Las pinzas cogen el ancho del pliego para arrastrar el papel, pero eso
	// afecta al largo del pliego
	if ( d->m_pliegoAncho > d->m_pliegoLargo ) {
		anchoPliego = d->m_pliegoAncho;
		largoPliego = d->m_pliegoLargo;
	} else {
		anchoPliego = d->m_pliegoLargo;
		largoPliego = d->m_pliegoAncho;
	}

	// Obtengamos la superficie útil del pliego, es decir la imprimible menos la pinza de máquina
	largoPliego = largoPliego - d->m_pinzas;

	if ( d->m_ancho > 0 && d->m_largo > 0 ) {
		// OJO: Si la sección es de un libro, ¡¡no lleva sangre entre las páginas!!
		if ( d->m_itemType != Imposition::BOOK ) {
			anchoTrabajo = d->m_ancho + ( d->m_sangre * 2 );
			largoTrabajo = d->m_largo + ( d->m_sangre * 2 );
		} else {
			anchoTrabajo = d->m_ancho;
			largoTrabajo = d->m_largo;
			// En el caso de un libro, el truquito es disminuir un poquito el tamaño del pliego
			// para así tener la superficie de impresión real
			largoPliego = largoPliego - d->m_sangre;
			anchoPliego = anchoPliego - d->m_sangre;
		}
		// El trabajo es rectangular siempre. De dimensión A x B. El pliego es de dimensión C x D.
		/*
			A
		-------------
		|			|
	B	|			|
		|			|
		|			|
		-------------
		Como es rectángulo debe entrar en otro de dimensiones C x D, veremos para las dos posibles
		combinaciones cuántas veces entra ese rectángulo en el pliego.
		*/
		opcion1 = floor ( anchoPliego / anchoTrabajo ) * floor ( largoPliego / largoTrabajo );
		opcion2 = floor ( anchoPliego / largoTrabajo ) * floor ( largoPliego / anchoTrabajo );
		if ( opcion1 > opcion2 ) {
			paginasPorCara = opcion1;
		} else {
			paginasPorCara = opcion2;
		}
	}

	// El número de páginas por cara, debe ser par, y no sólo eso, debe ser además múltiplo de
	// 4, 8 o 16... en el caso de estar calculando pliegos para un libro
	if ( d->m_itemType == Imposition::BOOK ) {
		QList<int> posibilidades;
		int valor;
		posibilidades << 2 << 4 << 8 << 16 << 32 << 64 << 128 << 256;
		// Si el número de páginas que nos da no es alguno de los números de la lista, aproximamos
		// al menor de las páginas por cara inferior a posibilidades
		if ( !posibilidades.contains ( paginasPorCara ) ) {
			for ( int i = 0 ; i < posibilidades.size() ; i++ ) {
				if ( posibilidades.at(i) < paginasPorCara ) {
					valor = posibilidades.at(i);
				}
			}
			paginasPorCara = valor;
		}
	} else {
		if ( paginasPorCara > 1 && (paginasPorCara % 2) ) {
			paginasPorCara = paginasPorCara - 1;
		}
	}

	return paginasPorCara;
}
