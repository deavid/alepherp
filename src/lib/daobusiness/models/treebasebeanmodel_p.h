/***************************************************************************
 *   Copyright (C) 2010 by David Pinelo   *
 *   david.pinelo@alephsistemas.es   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef TREEBASEBEANMODEL_P_H
#define TREEBASEBEANMODEL_P_H

#include <QString>
#include <QStringList>


class TreeBaseBeanModelPrivate
{
public:
	/**
	Conjunto de tablas que se mostrarn anidadas: Ejemplo: Papel depende de Subfamilia,
	  Subfamilia depende de Familia. m_tableNames debe ser igual a = {"familias_papel",
	  "subfamilias_papel", "papeles"}. OBLIGATORIAMENTE: m_tableNames debe tener dos elemetos
	como minimo: padre e hijo.
	*/
	QStringList m_tableNames;
	/** Qu campos sern visibles en el widget que presente estos datos */
	QStringList m_fieldsView;
	/**
	Adems, se incluyen posibles filtrados a aplicar en cada subnivel ADICIONALES. Si existen
	deben indicar ENTERAMENTE la clusula FROM y WHERE de la select a obtener
	Tendremos una estructura de datos as:
	familiaspapel = ""
	subfamiliaspapel = "subfamiliaspapel as t0, tipossubfamiliaspapel as t1 WHERE t1.generica = false"
	papeles = ""
	La relación PADRE-HIJO se mantiene siempre, y se busca entre las dos tablas
	*/
	QStringList m_filterLevels;
	/** Nombre que se pondrá al elemento raíz */
	QString m_rootName;
	/** Los nodos intermedios sin hijos, no se visualizan */
	bool m_viewIntermediateNodesWithoutChilds;

	TreeBaseBeanModelPrivate() {
		m_viewIntermediateNodesWithoutChilds = false;
	}
};

#endif // TREEBASEBEANMODEL_P_H
