/*
*************************************************************************************
**                                                                                  *
**                                View Infinity                                     *
**                                                                                  *
**       Zoomable Views in Feature Diagrams for Software Visualization              *
**                                                                                  *
**                                   LICENSE                                        *
*************************************************************************************
**
**
**	This file is part of the examples of View Infinity.
**
**	View Infinity is free software; you can redistribute it and/or
**	modify it under the terms of the GNU General Public License
**	Version 2 or 3 as published by the Free Software Foundation
**	and appearing in the files LICENSE_GPL_v2.txt and
**	LICENSE_GPL_v3.txt included in the packaging of this file.
**
**	View Infinity is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
**
**	See also: svn://luxator.cs.uni-magdeburg.de/uise/students/FOSD/SMV_Stengel
**
**	Contact:		Michael Stengel
**					virtuellerealitaet@googlemail.com
**			
**					Jun.-Prof. Dr.Ing. Raimund Dachselt
**					dachselt@acm.org
**
**	Postal Adress:	Jun. Prof. Dr.-Ing. Raimund Dachselt
**					Otto-von-Guericke-University of Magdeburg
**					Department of Simulation and Graphic
**					Faculty of Computer Science
**					Universit�tsplatz 2
**					D-39106 Magdeburg
**					Germany
**
************************************************************************************/

#include <QApplication>
#include <QtGui>
#include <iostream>
#include "SMVMain.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
	SMVMain mSMV;
	mSMV.show();
	return app.exec();
}
