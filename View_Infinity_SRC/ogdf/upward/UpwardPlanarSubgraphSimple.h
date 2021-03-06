/*
 * $Revision: 1.5 $
 * 
 * last checkin:
 *   $Author: klein $ 
 *   $Date: 2007-11-14 16:15:44 +0100 (Mi, 14 Nov 2007) $ 
 ***************************************************************/
 
/** \file
 * \brief Declaration of class UpwardPlanarSubgraphSimple which
 *        computes an upward planar subgfraph by using UpwardPlanarModule.
 * 
 * \author Carsten Gutwenger
 * 
 * \par License:
 * This file is part of the Open Graph Drawing Framework (OGDF).
 * Copyright (C) 2005-2007
 * 
 * \par
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * Version 2 or 3 as published by the Free Software Foundation
 * and appearing in the files LICENSE_GPL_v2.txt and
 * LICENSE_GPL_v3.txt included in the packaging of this file.
 *
 * \par
 * In addition, as a special exception, you have permission to link
 * this software with the libraries of the COIN-OR Osi project
 * (http://www.coin-or.org/projects/Osi.xml), all libraries required
 * by Osi, and all LP-solver libraries directly supported by the
 * COIN-OR Osi project, and distribute executables, as long as
 * you follow the requirements of the GNU General Public License
 * in regard to all of the software in the executable aside from these
 * third-party libraries.
 * 
 * \par
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * \par
 * You should have received a copy of the GNU General Public 
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 * 
 * \see  http://www.gnu.org/copyleft/gpl.html
 ***************************************************************/


#ifdef _MSC_VER
#pragma once
#endif


#ifndef OGDF_UPWARD_PLANAR_SUBGRAPH_SIMPLE_H
#define OGDF_UPWARD_PLANAR_SUBGRAPH_SIMPLE_H




#include <ogdf/module/UpwardPlanarSubgraphModule.h>
#include <ogdf/basic/tuples.h>


namespace ogdf {

//---------------------------------------------------------
// UpwardPlanarSubgraphSimple
// implements a maximal planar subgraph algorithm using
// planarity testing
//---------------------------------------------------------
class UpwardPlanarSubgraphSimple : public UpwardPlanarSubgraphModule
{
public:
	// construction
	UpwardPlanarSubgraphSimple() { }
	// destruction
	~UpwardPlanarSubgraphSimple() { }

	// computes set of edges delEdges, which have to be deleted
	// in order to get a planar subgraph; edges in preferedEdges
	// should be contained in planar subgraph
	void call(const Graph &G, List<edge> &delEdges);

	void call(GraphCopy &GC, List<edge> &delEdges);


private:
	bool checkAcyclic(
		GraphCopySimple &graphAcyclicTest,
		SList<Tuple2<node,node> > &tmpAugmented);
	
	void dfsBuildSpanningTree(
		node v,
		SListPure<edge> &treeEdges,
		NodeArray<bool> &visited);

};


} // end namespace ogdf

#endif
