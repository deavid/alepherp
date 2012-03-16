//---------------------------------------------------------------------------

#ifndef globalesH
#define globalesH

#include <math.h>
#include <vector>
#include <string>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QDesktopServices>
#include <QDialog>
#include <alepherpglobal.h>

// Precisión de los campos DOUBLE en base de datos
#define DB_DOUBLE_PRECISION	5

// Mis definiciones
#define MONEDA_PTAS     1
#define MONEDA_EUROS    2
#define UNEUROSONPTAS   166.386

#define MASK_PTAS       "#,##0"
#define MASK_EUROS      "#,##0.00"
#define ETIQ_PTAS       "Pts"
#define ETIQ_EUROS      "\342\202\254"

#define NUEVA_LINEA		"\r\n"
#define TABULADOR		"\t"

#define FORMATO_FECHA_BBDD	"yyyy-MM-dd"

#define LOOK_CDE		0
#define LOOK_CLEANLOOK	1
#define LOOK_MAC		2
#define LOOK_MOTIF		3
#define LOOK_PLASTIQUE	4
#define LOOK_WINDOWS	5
#define LOOK_VISTA		6
#define LOOK_XP			8
#define LOOK_GTK		9

#define EXTENSION_ARCHIVO_OPENOFFICE 	".odt"
#define EXTENSION_ARCHIVO_PDF			".pdf"

#define MASK_TELEFONO "000 00 00 00 - 000 00 00 00 - 000 00 00 00;"
#define MASK_FAX "000 00 00 00;"

//---------------------------------------------------------------------------
typedef struct _TIEMPO {
    int iHoras;
    int iMinutos;
} stTIEMPO;
//---------------------------------------------------------------------------

class Q_ALEPHERP_EXPORT CommonsFunctions : public QObject {
    Q_OBJECT
public:
    CommonsFunctions(QObject *parent = 0);

    static bool cifValid (const QString &cif);
    static bool nifValid (const QString &nif);
    static bool openPDF (QUrl urlFile);

    static void centerWindow (QDialog *dlg);
    static QDialog * parentDialog(QWidget *wid);

    static double  ValorRectaRegresion (double pendiente, double cortecero, double X);
    static void DesglosarTiempo (int iMinutos, stTIEMPO *tiempo);
    static void DesglosarTiempo (double iMinutos, stTIEMPO *tiempo);
    static double redondeaArriba (double valor);
    static double round( double Value, int Digits );
    static double getPrimerElemento(QString formato, char separador);
    static double getSegundoElemento(QString formato, char separador);

};

#endif
