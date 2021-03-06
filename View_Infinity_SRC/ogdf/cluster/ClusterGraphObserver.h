/*
 * $Revision: 1.0 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2007-11-08 16:10:37 +0100 (Do, 08 Nov 2007) $ 
 ***************************************************************/
 
/** \file
 * \brief Abstract base class for structures on graphs, that need
 *        to be informed about cluster graph changes.
 *
 * Follows the observer pattern: cluster graphs are observable 
 * objects that can inform observers on changes made to their
 * structure.
 * 
 * \author Martin Gronemann
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

#ifndef OGDF_CLUSTER_GRAPH_OBSERVER_H
#define OGDF_CLUSTER_GRAPH_OBSERVER_H


#include <ogdf/basic/List.h>
#include <ogdf/cluster/ClusterGraph.h>

namespace ogdf {


//----------------------------------------------------------
// GraphObserver
// abstract base class
// derived classes have to overload nodeDeleted, nodeAdded
// edgeDeleted, edgeAdded
// these functions should be called by Graph before (delete)
// and after (add) its structure
//----------------------------------------------------------
class ClusterGraphObserver {
	friend class ClusterGraph;

public:
	ClusterGraphObserver() : m_pClusterGraph(0) {} 
	
	ClusterGraphObserver(const ClusterGraph* CG) : m_pClusterGraph(CG) 
	{
		m_itCGList = CG->registerObserver(this);
	}//constructor
	
	virtual ~ClusterGraphObserver() 
	{
		if (m_pClusterGraph) m_pClusterGraph->unregisterObserver(m_itCGList);
	}//destructor

	// associates structure with different graph
	void reregister(const ClusterGraph *pCG) {
		//small speedup: check if == m_pGraph
		if (m_pClusterGraph) m_pClusterGraph->unregisterObserver(m_itCGList);
		if ((m_pClusterGraph = pCG) != 0) m_itCGList = pCG->registerObserver(this);
	}

	virtual void clusterDeleted(cluster v) = 0;
	virtual void clusterAdded(cluster v)   = 0;
	//virtual void reInit()            	= 0; 
	//virtual void cleared()           	= 0;//Graph cleared

	const ClusterGraph*  getGraph() const {	return m_pClusterGraph;}

protected:
	const ClusterGraph* m_pClusterGraph; //underlying clustergraph
	
	//List entry in cluster graphs list of all registered observers
	ListIterator<ClusterGraphObserver*> m_itCGList;
};

}; // end of namespace

#endif
