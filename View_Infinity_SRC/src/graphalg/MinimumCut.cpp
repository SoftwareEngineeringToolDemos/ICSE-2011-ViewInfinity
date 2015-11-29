/*
 * $Revision: $
 * 
 * last checkin:
 *   $Author: klein $ 
 *   $Date: 2007-11-14 16:18:10 +0100 (Mi, 14 Nov 2007) $ 
 ***************************************************************/
 
/** \file
 * \brief Declares & Implements a minimum-cut algorithmn according 
 * to an approach of Stoer and Wagner 1997
 * 
 * \author Mathias Jansen
 * 
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
 
#include <ogdf/graphalg/MinimumCut.h>

using namespace ogdf;


MinCut::MinCut(Graph &G, EdgeArray<double> &w) : m_GC(0) {
	
	// Due to the node contraction (which destroys the Graph step by step),
	// we have to create a GraphCopy.
	m_GC = new GraphCopy(G);
	
	// Edge weights are initialized.
	edge e;
	m_w.init(*m_GC);
	forall_edges(e,*m_GC) {
		m_w[e] = w[(*m_GC).original(e)];
	}
	m_contractedNodes.init(*m_GC);
	m_minCut = 1e20;
}


MinCut::~MinCut() {}


void MinCut::contraction(node t, node s) {
	
	/*
	 * The contraction of the two nodes \as and \a t is performed as follows:
	 * in the first step, all edges between \a s and \a t are deleted, and all edges incident
	 * to \a s are redirected to \a t. Then, node \a s is deleted and the adjacency list of \a t 
	 * is checked for parallel edges. If k parallel edges are found, k-1 of them are deleted 
	 * and their weights are added to the edge that is left.
	 */
	
	// Step 1: redirecting edges and deleting node \a s
	adjEntry adj = s->firstAdj();
	while (adj != 0)
	{
		adjEntry succ = adj->succ();
		edge e = adj->theEdge();
		if (e->source() == t || e->target() == t) {
			m_GC->delEdge(e);
		}
		else if (e->source() == s) {
			m_GC->moveSource(e,t);
		}
		else {
			m_GC->moveTarget(e,t);
		}
		adj = succ;
	}
	m_GC->delNode(s);
	
	/*
	 * Because of technical problems that occur when deleting edges and thus adjacency entries in a loop,
	 * a NodeArray is filled with the edges incident to node \a t.
	 * This NodeArray is checked for entries with more than one edge, which corresponds
	 * to parallel edges.
	 */
	 
	// NodeArray containing parallel edges
	NodeArray<List<edge> > adjNodes(*m_GC);
	
	forall_adj(adj,t) {
		adjNodes[adj->twin()->theNode()].pushBack(adj->theEdge());
	}
	
	// Step 2: deleting parallel edges and adding their weights
	node v = m_GC->firstNode();
	while (v!=0) {
		if (adjNodes[v].size() > 1) {
			edge e = adjNodes[v].front();
			adjNodes[v].popFront();
			ListConstIterator<edge> it;
			for (it=adjNodes[v].begin(); it.valid(); ++it) {
				
				// Add weight of current edge to \a e.
				m_w[e] += m_w[(*it)];
				m_GC->delEdge(*it);
			}
		}
		v = v->succ();
	}
}


double MinCut::minimumCutPhase() {
	
	/*
	 * This function computes the mincut of the current phase.
	 * First, nodes are added successively in descending order of the sum of
	 * their incident edge weights to the list \a markedNodes.
	 * Afterwards, the current mincut value \a cutOfThePhase is computed, which corresponds to the
	 * sum of the weights of those edges incident to node \a t, which is the node that has been
	 * added to list \a markedNodes at last.
	 * At the end, the two last added nodes (\a s and \a t) are contracted and the \cutOfThePhase
	 * is returned.
	 */
	
	// Contains the mincut value according to the current phase.
	double cutOfThePhase;
	
	List<node> markedNodes;
	List<node> leftoverNodes;
	
	// Contains for each node the sum of the edge weights of those edges
	// incident to nodes in list \a markedNodes
	NodeArray<double> nodePrio(*m_GC);
	
	// The two nodes that have been added last to the list \a markedNodes.
	// These are the two nodes that have to be contracted at the end of the function.
	node s,t;
	
	// Initialization of data structures
	node v;
	forall_nodes(v,*m_GC) {
		leftoverNodes.pushBack(v);
	}
	nodePrio.fill(0.0);
	
	// The start-node can be chosen arbitrarily. It has no effect on the correctness of the algorithm.
	// Here, always the first node in the list \a leftoverNodes is chosen.
	v = leftoverNodes.front(); markedNodes.pushBack(v); leftoverNodes.popFront();
	adjEntry adj;
	forall_adj(adj,v) {
		nodePrio[adj->twin()->theNode()] = m_w[adj->theEdge()];
	}

	
	// Temporary variables
	ListIterator<node> it1;
	node maxWeightNode; ListIterator<node> maxWeightNodeIt;
	double mostTightly; bool inside;
	
	// Successively adding the most tightly connected node.
	while (markedNodes.size() != m_GC->numberOfNodes()) {
		
		mostTightly = 0.0; inside = false;
		maxWeightNode = NULL;
		
		// The loop computing the most tightly connected node to the current set \a markedNodes.
		// For better performance, this should be done using PriorityQueues! Since this algorithmn
		// is only used for the Cut-separation within the Branch&Cut-algorithmn for MCPSP, only small
		// and moderate Graph sizes are considered. Thus, the total running time is hardly affected.
		for(it1=leftoverNodes.begin(); it1.valid(); ++it1) {
			
			if(nodePrio[(*it1)] > mostTightly) {
				inside = true;
				maxWeightNode = (*it1);
				maxWeightNodeIt = it1;
				mostTightly = nodePrio[(*it1)];
			}
		}
		
		// If the Graph is not connected, maxWeightNode might not be updated in each iteration.
		// Hence, in this case we simply choose one of the leftoverNodes (the first one).
		if (maxWeightNode == NULL) {
			maxWeightNode = leftoverNodes.front();
			maxWeightNodeIt = leftoverNodes.begin();
		}
		
		// Adding \a maxWeightNode to the list \a markedNodes
		markedNodes.pushBack(maxWeightNode);

		// Deleting \a maxWeightNode from list \a leftoverNodes
		leftoverNodes.del(maxWeightNodeIt);
		
		// Updating the node priorities
		adjEntry a;
		forall_adj(a,maxWeightNode) {
			nodePrio[a->twin()->theNode()] += m_w[a->theEdge()];
		}
		
	}// end of loop while(markedNodes.size()...)
	
	// Computing value \a cutOfThePhase
	cutOfThePhase = 0.0;
	ListConstIterator<node> last = markedNodes.rbegin();
	t = (*last); s = *(last.pred());
	adjEntry t_adj;
	forall_adj(t_adj,t) {
		cutOfThePhase += m_w[t_adj->theEdge()];
	}
	
	// If the current \a cutOfThePhase is strictly smaller than the global mincut value,
	// the partition defining the mincut has to be updated.
	if(cutOfThePhase < m_minCut) {
		m_partition.clear();
		m_partition.pushBack(m_GC->original(t));
		for(ListConstIterator<node> it = m_contractedNodes[t].begin(); it.valid(); ++it) {
			m_partition.pushBack(*it);
		}
	}

	// Since nodes in \a m_GC correspond to sets of nodes (due to the node contraction), 
	// the NodeArray \a m_contractedNodes has to be updated.
	m_contractedNodes[t].pushBack(m_GC->original(s));
	ListConstIterator<node> contractIt;
	for (contractIt = m_contractedNodes[s].begin(); contractIt.valid(); ++contractIt) {
		m_contractedNodes[t].pushBack((*contractIt));
	}
	
	// Performing the node contraction of nodes \a s and \a t.
	contraction(t,s);

 	return cutOfThePhase;
}


double MinCut::minimumCut() {
	
	/*
	 * Main loop of the algorithm
	 * As long as GraphCopy \a m_GC contains at least two nodes, 
	 * function minimumCutPhase() is invoked and \a m_minCut is updated
	 */

	for (int i=m_GC->numberOfNodes(); i>1; --i) {
		m_minCut = min(m_minCut,minimumCutPhase());
		if (m_minCut == 0.0) return m_minCut;
	}
	return m_minCut;
}


void MinCut::partition(List<node> &nodes) {

	nodes.clear();
	ListConstIterator<node> it;
	for (it=m_partition.begin(); it.valid(); ++it) {
		nodes.pushBack(*it);
	}
}


void MinCut::cutEdges(List<edge> &edges, Graph &G) {
	
	edges.clear();
	NodeArray<bool> inPartition(G);
	inPartition.fill(false);
	ListConstIterator<node> nodeIt;
	
	for (nodeIt=m_partition.begin(); nodeIt.valid(); ++nodeIt) {
		inPartition[*nodeIt] = true;
	}
	
	for (nodeIt=m_partition.begin(); nodeIt.valid(); ++nodeIt) {
		edge e;
		forall_adj_edges(e,(*nodeIt)) {
			if(e->source() == (*nodeIt)) {
				if(inPartition[e->target()] == false) {
					edges.pushBack(e);
				}
			} else {
				if(inPartition[e->source()] == false) {
					edges.pushBack(e);
				}
			}
		}
	}
}
