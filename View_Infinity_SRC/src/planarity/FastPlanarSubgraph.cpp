/*
 * $Revision: 1.8 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2007-11-08 16:10:37 +0100 (Do, 08 Nov 2007) $ 
 ***************************************************************/
 
/** \file
 * \brief Implementation of the FastPlanarSubgraph.
 * 
 * Class is derived from base class PlanarSubgraphModule
 * and implements the interface for the Planarization algorithm 
 * based on PQ-trees.
 * 
 * \author Sebastian Leipert
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


#include <ogdf/basic/basic.h>
#include <ogdf/basic/Array.h>
#include <ogdf/basic/SList.h>
#include <ogdf/basic/simple_graph_alg.h>
#include <ogdf/basic/extended_graph_alg.h>
#include <ogdf/internal/planarity/PlanarSubgraphPQTree.h>
#include <ogdf/internal/planarity/PlanarLeafKey.h>
#include <ogdf/planarity/FastPlanarSubgraph.h>


namespace ogdf{

// Prepares the planarity test and the planar embedding
Module::ReturnType FastPlanarSubgraph::doCall(const Graph &G,
	const List<edge> &preferedEdges,
	List<edge> &delEdges,
	const EdgeArray<int>  *pCost,
	bool preferedImplyPlanar)
{

	delEdges.clear();

	if (G.numberOfEdges() < 9)
		return retOptimal;

	
	node v;
	NodeArray<node> tableNodes(G,0);
	EdgeArray<edge> tableEdges(G,0);
	NodeArray<bool> mark(G,0);

	EdgeArray<int> componentID(G);


	// Determine Biconnected Components
	int bcCount = biconnectedComponents(G,componentID);

	// Determine edges per biconnected component
	Array<SList<edge> > blockEdges(0,bcCount-1);
	edge e;
	forall_edges(e,G)
	{
		if (!e->isSelfLoop())
			blockEdges[componentID[e]].pushFront(e);
	} 

	// Determine nodes per biconnected component.
	Array<SList<node> > blockNodes(0,bcCount-1);
	int i;
	for (i = 0; i < bcCount; i++)
	{
		SListIterator<edge> it;
		for (it = blockEdges[i].begin(); it.valid(); ++it)
		{
			e = *it;
			if (!mark[e->source()])
			{
				blockNodes[i].pushBack(e->source());
				mark[e->source()] = true;
			}
			if (!mark[e->target()])
			{
				blockNodes[i].pushBack(e->target());
				mark[e->target()] = true;
			}
		}
		SListIterator<node> itn;
		for (itn = blockNodes[i].begin(); itn.valid(); ++itn)
		{
			v = *itn;
			if (mark[v])
				mark[v] = false;
			else
				OGDF_ASSERT(mark[v]); // v has been placed two times on the list.
		}

	}
	
	

	// Perform Planarization for every biconnected component
		
	if (bcCount == 1)
	{
		if (G.numberOfEdges() > 4)
		{
			/*// Compute st-numbering
			NodeArray<int> numbering(G,0);
			int n = stNumber(G,numbering);
			OGDF_ASSERT_IF(dlConsistencyChecks,testSTnumber(G,numbering,n))
			
			planarize(G,numbering,delEdges);*/
			computeDelEdges(G,pCost,0,delEdges);
		}
	}
	else
	{
		for (i = 0; i < bcCount; i++)
		{
			#ifdef OGDF_DEBUG
			if (int(ogdf::debugLevel) >= int(dlHeavyChecks))
			{
				cout <<endl << endl << "-----------------------------------" << endl << endl;
				cout << "Component " << i << endl;
			}
			#endif
			Graph C;
		
			SListIterator<node> itn;
			for (itn = blockNodes[i].begin(); itn.valid(); ++ itn)
			{
				v = *itn;
				node w = C.newNode();
				tableNodes[v] = w;
				#ifdef OGDF_DEBUG
				if (int(ogdf::debugLevel) >= int(dlHeavyChecks))
				{
					cout <<"Original: " << v << " New: " << w<< endl;
				}
				#endif
			}


			SListIterator<edge> it;
			for (it = blockEdges[i].begin(); it.valid(); ++it)
			{
				e = *it;
				edge f = C.newEdge(tableNodes[e->source()],tableNodes[e->target()]);
				tableEdges[e] = f;
			}

			// Construct a translation table for the edges.
			// Necessary, since edges are deleted in a new graph.
			// that represents the biconnectedcomponent of the original graph.
			EdgeArray<edge> backTableEdges(C,0);
			for (it = blockEdges[i].begin(); it.valid(); ++it)
				backTableEdges[tableEdges[*it]] = *it; 

			// gets the deletec Edges of the biconnected component
			List<edge> delEdgesOfBC;	
										

			if (C.numberOfEdges() > 4)
			{
				/*// Compute st-numbering
				NodeArray<int> numbering(C,0);
				int n = stNumber(C,numbering);
				OGDF_ASSERT_IF(dlConsistencyChecks,testSTnumber(C,numbering,n))

				planarize(C,numbering,delEdgesOfBC);*/
				computeDelEdges(C,pCost,&backTableEdges,delEdgesOfBC);
			}

			// get the original edges that are deleted and 
			// put them on the list delEdges.
			while (!delEdgesOfBC.empty())
				delEdges.pushBack(backTableEdges[delEdgesOfBC.popFrontRet()]);

		}

	}

	return retFeasible;
}


void FastPlanarSubgraph::computeDelEdges(const Graph &G,
	const EdgeArray<int>  *pCost,
	const EdgeArray<edge> *backTableEdges,
	List<edge> &delEdges)
{
	if (m_nRuns <= 0)
	{
		// Compute st-numbering
		NodeArray<int> numbering(G,0);
		int n = stNumber(G,numbering);
		OGDF_ASSERT_IF(dlConsistencyChecks,testSTnumber(G,numbering,n))
		
		planarize(G,numbering,delEdges);

	} else {
		int bestSolution = INT_MAX;
		
		for(int i = 1; i <= m_nRuns && bestSolution > 1; ++i)
		{
			List<edge> currentDelEdges;

			// Compute (randomized) st-numbering
			NodeArray<int> numbering(G,0);
			int n = stNumber(G,numbering,0,0,true);
			OGDF_ASSERT_IF(dlConsistencyChecks,testSTnumber(G,numbering,n))
			
			planarize(G,numbering,currentDelEdges);

			if(pCost == 0)
			{
				int currentSolution = currentDelEdges.size();
				
				if(currentSolution < bestSolution) {
					bestSolution = currentSolution;
					delEdges.clear();
					delEdges.conc(currentDelEdges);
				}
	
			} else {
				int currentSolution = 0;
				ListConstIterator<edge> it;
				for(it = currentDelEdges.begin(); it.valid(); ++it)
					if(backTableEdges != 0)
						currentSolution += (*pCost)[(*backTableEdges)[*it]];
					else
						currentSolution += (*pCost)[*it];
				
				if(currentSolution < bestSolution) {
					bestSolution = currentSolution;
					delEdges.clear();
					delEdges.conc(currentDelEdges);
					
				}
			}
		}
	}
}



// Performs a planarity test on a biconnected component
// of G. numbering contains an st-numbering of the component.
void FastPlanarSubgraph::planarize(const Graph &G,
								 NodeArray<int> &numbering,
								 List<edge> &delEdges)
{
	node v;

	NodeArray<SListPure<PlanarLeafKey<whaInfo*>* > > inLeaves(G);
	NodeArray<SListPure<PlanarLeafKey<whaInfo*>* > > outLeaves(G);
	Array<node> table(G.numberOfNodes()+1);

	forall_nodes(v,G)
	{
		edge e;
		forall_adj_edges(e,v)
		{
			if (numbering[e->opposite(v)] > numbering[v])
				// sideeffect: ignores selfloops
			{
				PlanarLeafKey<whaInfo*>* L = OGDF_NEW PlanarLeafKey<whaInfo*>(e);
				inLeaves[v].pushFront(L);
			}
		}
		table[numbering[v]] = v;
	}

	forall_nodes(v,G)
	{
		SListIterator<PlanarLeafKey<whaInfo*>* > it;
		for (it = inLeaves[v].begin(); it.valid(); ++it)
		{
			PlanarLeafKey<whaInfo*>* L = *it;
			outLeaves[L->userStructKey()->opposite(v)].pushFront(L);
		}
	}

	SList<PQLeafKey<edge,whaInfo*,bool>*> totalEliminatedKeys;

	PlanarSubgraphPQTree T;
	T.Initialize(inLeaves[table[1]]);
	for (int i = 2; i < G.numberOfNodes(); i++)
	{
		#ifdef OGDF_DEBUG
			char aa[12];
			ogdf::sprintf(aa,12,"%d.txt",i);
			if (int(ogdf::debugLevel) >= int(dlHeavyChecks))
			{
				cout << i << "th Reduction" << endl;
				T.printOutCurrentTree("Before",aa);
			}
		#endif

		SList<PQLeafKey<edge,whaInfo*,bool>*> eliminatedKeys;
		bool check = T.Reduction(outLeaves[table[i]],eliminatedKeys);

		#ifdef OGDF_DEBUG
			if (int(ogdf::debugLevel) >= int(dlHeavyChecks))
			{
				T.printOutCurrentTree("After",aa);
			}
		#endif

		totalEliminatedKeys.conc(eliminatedKeys);
		T.ReplaceRoot(inLeaves[table[i]]);
		T.emptyAllPertinentNodes();
	}


	SListIterator<PQLeafKey<edge,whaInfo*,bool>* > it;
	for (it = totalEliminatedKeys.begin(); it.valid(); ++it)
	{ 
		edge e = (*it)->userStructKey();
		delEdges.pushBack(e);
	}

	//cleanup
	forall_nodes(v,G)
	{
		while (!inLeaves[v].empty())
		{
			PlanarLeafKey<whaInfo*>* L = inLeaves[v].popFrontRet();
			delete L;	
		}
	}

	T.Cleanup(); // Explicit call for destructor necessary. This allows to 
			     // call virtual funtion CleanNode for freeing node information
				 // class.
}


}
