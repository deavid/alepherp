//---------------------------------------------------------------------------
#include "globales.h"
#include "configuracion.h"
#include <iostream>
#include <cstdlib>
#include <QRegExp>
#include <QProcess>
#include <QDir>
#include <QDesktopWidget>
#include <QApplication>

CommonsFunctions::CommonsFunctions(QObject *parent) : QObject(parent)
{

}

/*!
Descripcion: Calcula el valor de una recta de regresión del tipo
Y = A + BX o Y = cortecero + pendienteX
*/
double CommonsFunctions::ValorRectaRegresion (double pendiente, double cortecero, double X)
{
    return ( cortecero + (pendiente*X) );
}


void CommonsFunctions::DesglosarTiempo (double iMinutos, stTIEMPO *tiempo)
{
    DesglosarTiempo ((int) iMinutos, tiempo);
}


void CommonsFunctions::DesglosarTiempo (int iMinutos, stTIEMPO *tiempo)
{
    tiempo->iMinutos =  iMinutos % 60;
    tiempo->iHoras = (int) ( iMinutos / 60 );
}


double CommonsFunctions::redondeaArriba (double number)
{
    double fraccion, entero;

    fraccion = modf(number, &entero);
    if ( fraccion > 0 ) {
    	entero = entero + 1;
    }
    return entero;
}


double CommonsFunctions::round( double Value, int Digits )
{
	if( Value > 0.0 ) {
		return ( (long)( Value * pow( 10.0, Digits) + 0.5 ) ) / pow( 10.0,
				  Digits);
	}
 
	return ( (long)( Value * pow( 10.0, Digits) - 0.5 ) ) / pow( 10.0,
			  Digits);
}


double CommonsFunctions::getPrimerElemento(QString formato, char separador)
{
	double formatoAncho = 0;
	bool ok;
	QStringList cadenas = formato.split(separador);
	
	if ( cadenas.size() == 2 ) {
		formatoAncho = configuracion.getLocale()->toDouble(cadenas.at(0), &ok);
		if ( !ok ) {
			formatoAncho = 0;
		}
	}
	return formatoAncho;
}


double CommonsFunctions::getSegundoElemento(QString formato, char separador)
{
	double formatoLargo = 0;
	bool ok;
	QStringList cadenas = formato.split(separador);
	
	if ( cadenas.size() == 2 ) {
		formatoLargo = configuracion.getLocale()->toDouble(cadenas.at(1), &ok);
		if ( !ok ) {
			formatoLargo = 0;
		}
	}
	return formatoLargo;
}


/**
	Esta función se encarga de validar el CIF. Para ello, seguimos lo siguiente:
	El CIF consta de 9 dígitos alfanumémericos con la siguiente estructura:
	T  P  P  N  N  N  N  N  C
	Siendo: 
	T: Letra de tipo de Organización, una de las siguientes: A,B,C,D,E,F,G,H,K,L,M,N,P,Q,S.
	P: Código provincial.
	N: Númeración secuenial dentro de la provincia.
	C: Dígito de control, un número o letra: A-1,B-2,C-3,D-4,E-5,F-6,G-7,H-8,I-9,J-0.

	El último es el dígito de control que puede ser un número ó una letra.
	Según la información que me habeis hecho llegar la forma en que se asigna número
	o una letra es la siguiente:
<ul>
<li>Será una LETRA si la clave de entidad es K, P, Q ó S</li>
<li>Será un NUMERO si la entidad es A, B, E ó H.</li>
<li>Para otras claves de entidad: el dígito podrá ser tanto número como letra.</li>
</ul>
Las operaciones para calcular este dígito de control se realizan sobre los siete dígitos centrales y son las siguientes:
<ul>
<li>Sumar los dígitos de la posiciones pares. Suma = A </li>
<li>Para cada uno de los dígitos de la posiciones impares, multiplicarlo por 2 y
sumar los dígitos del resultado.</li>
<li>Ej.: ( 8 * 2 = 16 --> 1 + 6 = 7 ). Acumular el resultado. Suma = B</li>
<li>Calcular la suma A + B = C </li>
<li>Tomar sólo el dígito de las unidades de C y restárselo a 10. Esta resta nos da D.</li>
<li>A partir de D ya se obtiene el dígito de control. Si ha de ser numérico es
directamente D y si se trata de una letra se corresponde con la relación:
 A = 1, B = 2, C = 3, D = 4, E = 5, F = 6, G = 7, H = 8, I = 9, J = 10</li>
*/
bool CommonsFunctions::cifValid(const QString &cif)
{
	int A = 0;
	int posImpar;
	int B = 0;
	int C;
	QString temp;
	int D;
	QString comprobacionLetras = "ABCDEFGHIJ";
	
	// El CIF tiene 9 digitos. Lo que no tenga 9 digitos, no es válido.
	if ( cif.length() != 9 ) {
		return false;
	}
	// Si es un CIF, el primer dígito debe ser una letra, y después 8 números
	QRegExp regexp ("[A-Za-z][0-9]{8}");
	if ( !regexp.exactMatch (cif) ) {
		return false;
	}
	
	// Sumamos los dígitos de las posiciones pares. Suma = A.
	// Ejemplo: A58818501. Utilizamos los dígitos centrales 58818501
	// Sumamos los dígitos pares: A = 8 + 1 + 5 = 14
	for ( int i = 2 ; i < 8 ; i = i + 2 ) {
		A = A + cif.at(i).digitValue();
	}
	// Para cada uno de los dígitos de la posiciones impares, multiplicarlo por 2 y sumar
	// los dígitos del resultado.
	// Ej.: Posiciones impares:
	//	5 * 2 = 10 -> 1 + 0 = 1
	//	8 * 2 = 16 -> 1 + 6 = 7
	//	8 * 2 = 16 -> 1 + 6 = 7
	//	0 * 2 = 0 -> = 0
	//	Sumamos los resultados: B = 1 + 7 + 7 + 0 = 15
	for ( int i = 1 ; i < 8 ; i = i + 2 ) {
		posImpar = cif.at(i).digitValue() * 2;
		if ( posImpar > 9 ) {
			temp = QString("%1").arg(posImpar);
			B = B + temp.at(0).digitValue() + temp.at(1).digitValue();
		} else {
			B = B + posImpar;
		}
	}
	// Calcular la suma A + B = C 
	// Ej: Suma parcial: C = A + B = 14 + 15 = 29
	C = A + B;
	// Tomar sólo el dígito de las unidades de C y restárselo a 10. Esta resta nos da D.
	// Ej: El dígito de las unidades de C es 9.
	// Se lo restamos a 10 y nos da: D = 10 - 9 = 1
	if ( C > 10 ) {
		temp = QString("%1").arg(C);
		D = 10 - temp.at(1).digitValue();
	} else {
		D = 10 - C;
	}
	if ( D == 10 ) {
		D = 0;
	}
	
	// A partir de D ya se obtiene el dígito de control. Si ha de ser numérico es
	// directamente D y si se trata de una letra se corresponde con la relación:
	// Será una LETRA si la clave de entidad es K, P, Q ó S<
	// Será un NUMERO si la entidad es A, B, E ó H.
	if ( cif.at(0) == 'K' || cif.at(0) == 'P' || cif.at(0) == 'Q' || cif.at(0) == 'S' ) {
		if ( cif.at(8) == comprobacionLetras.at(D-1) ) {
			return true;
		} else {
			return false;
		}
	} else if ( cif.at(0) == 'A' || cif.at(0) == 'B' || cif.at(0) == 'E' || cif.at(0) == 'H' ) {
		if ( D == cif.at(8).digitValue() ) {
			return true;
		} else {
			return false;
		}
	} else {
		return true;
	}
}

/**
	Comprueba la validez de un NIF. El nif debe venir en el formato
	NNNNNNNNL
	donde N son numeros y L una letra. Es decir, 8 números y una letra.
*/
bool CommonsFunctions::nifValid(const QString &nifOriginal)
{
    // Eliminamos los "-" que el usuario haya podido meter
    QString nif = nifOriginal;
    nif = nif.remove(QChar('-'));

	const char letra[] = "TRWAGMYFPDXBNJZSQVHLCKE";
	const int kTAM = 9; // numero de cifras para el DNI
	int dni;
	bool ok;
	
	if ( nif.length() != kTAM ) {
		return false;
	} 
	// Si es un NIF, tenemos 8 dígitos, y después una letra.
	QRegExp regexp ("[0-9]{8}[A-Za-z]");
	if ( !regexp.exactMatch (nif) ) {
		return false;
	}
	dni = nif.left(8).toInt(&ok);
	if ( !ok ) {
		return false;
	}
	dni %= 23;
	if ( letra[dni] == nif.at(8) ) {
		return true;
	} else {
		return false;
	}
}

bool CommonsFunctions::openPDF(QUrl urlFile)
{
	// Si hay definido un visor externo de PDF utilizamos ese
	if ( !configuracion.externalPDFViewer().isEmpty() ) {
		QString linea;
	
		linea = QString("%1 %2").
				arg( QDir::fromNativeSeparators ( configuracion.externalPDFViewer() ) ).
				arg( urlFile.toLocalFile() );
		QProcess::execute ( linea );
	} else {
		if ( !configuracion.internalPDFViewer() ) {
			QDesktopServices::openUrl(urlFile);
		}
	}
	return true;
/*	QString filename;

	Poppler::Document* document = Poppler::Document::load(urlFile.path());
	if (!document || document->isLocked()) {
		delete document;
		return false;
	}
	
	// Access page of the PDF file
	Poppler::Page* pdfPage = document->page(0);  // Document starts at page 0
	if (pdfPage == 0) {
		delete document;
		return false;
	}

	// Generate a QImage of the rendered page
	QImage image = pdfPage->renderToImage();
	if (image.isNull()) {
		delete document;
		return false;
	}

// ... use image ...
	VisorPDFForm *visor = new VisorPDFForm;
	visor->show();
	visor->setImage(image);

// after the usage, the page must be deleted
	delete pdfPage;
	
	delete document;
	return true;*/
}

void CommonsFunctions::centerWindow (QDialog *dlg)
{
	QDesktopWidget *desktop = QApplication::desktop();
	QRect rectDesktop = desktop->availableGeometry();
	QRect rectWindow = dlg->frameGeometry();
	QPoint posFinal((rectDesktop.width() - rectWindow.width()) / 2, (rectDesktop.height() - rectWindow.height()) / 2);
	dlg->move(posFinal);
}

QDialog * CommonsFunctions::parentDialog(QWidget *wid)
{
	QDialog *dlg = NULL;
	QObject *tmp = wid->parent();
	while ( dlg == NULL && tmp != NULL ) {
		dlg = qobject_cast<QDialog *> (tmp);
		tmp = tmp->parent();
	}
	return dlg;
}
