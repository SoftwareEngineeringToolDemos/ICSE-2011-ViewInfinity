/*
 * $Revision: 1.1 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2007-11-08 16:10:37 +0100 (Do, 08 Nov 2007) $ 
 ***************************************************************/
/** \file
 * \brief Implementation of Tutte's Algorithm
 * 
 * \author David Alberts \and Andrea Wagner
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

#ifdef USE_COIN

#include <ogdf/energybased/CoinTutteLayout.h>
#include <ogdf/basic/GraphCopyAttributes.h>
#include <ogdf/planarity/PlanarModule.h>

namespace ogdf {

bool solveLP(int rows, int cols, const CoinPackedMatrix &Matrix,\
	     const Array<double> &rightHandSide, Array<double> &x)
// solves a system of linear equations with a linear solver for optimization problems.
// I'm sorry but there is no Gauss-Algorithm (or some numerical stuff) in OGDF...
{
  int i;

  OsiSolverInterface *osi = CoinManager::createCorrectOsiSolverInterface();

  OGDF_ASSERT(osi);

// constructs a dummy optimization problem. 
// The given system of equations is used as constraint.
  osi->setObjSense(-1);                        // maximize...
  Array<double> obj(0,cols-1,1);               // ...the sum of variables
  Array<double> lowerBound(0,cols-1,-1*(osi->getInfinity()));
  Array<double> upperBound(0,cols-1,osi->getInfinity());

// loads the problem to Coin-Osi
  osi->loadProblem(Matrix, &lowerBound[0], &upperBound[0], &obj[0],\
		   &rightHandSide[0], &rightHandSide[0]);

// solves the linear program
  osi->initialSolve();

// gets the solution and returns true if it is optimal
  const double *sol = osi->getColSolution();
  for(i=0; i<cols; i++) x[i] = sol[i];

  if(osi->isProvenOptimal()) {
    delete osi;
    return true;
  }
  else {
    delete osi;
    return false;
  }
}
  
TutteLayout::TutteLayout()
{
  m_bbox = DRect (0.0, 0.0, 250.0, 250.0);
}



void TutteLayout::setFixedNodes(const Graph &G, List<node>& nodes,\
                                  List<DPoint>& pos, double radius)
// sets the positions of the nodes in a largest face of G in the form
// of a regular k-gon. The corresponding nodes and their positions are
// stored in nodes and pos, respectively.
{
  // compute faces of a copy of G
  GraphCopy GC(G);
  PlanarModule pm;

  // compute a planar embedding if \a G is planar
  if(pm.planarityTest(G)) pm.planarEmbed(GC);

  CombinatorialEmbedding E(GC);
  E.computeFaces();
  
  // search for largest face
  const face &maxFace = E.maximalFace();
  
  OGDF_ASSERT(maxFace);
  
  // delete possible old entries in nodes and pos
  nodes.clear();
  pos.clear();
  
  // set nodes and pos
  NodeArray<bool> addMe(GC,true);
  adjEntry adj;
  
  List<node> maxNodes;
  forall_face_adj(adj,maxFace) {
    maxNodes.pushBack(adj->theNode());
  } 
  
  forall_nonconst_listiterators(node, it, maxNodes) {
    node &w = *it;
    if(addMe[w]) { 
      nodes.pushBack(w);
      addMe[w] = false;
    }
  }
  
  double step  = 2.0 * pi / (double)(nodes.size());
  double alpha = 0.0;
  forall_listiterators(node, it, nodes) {
    pos.pushBack(DPoint(radius * cos(alpha), radius * sin(alpha)));
    alpha += step;
  }
}

void TutteLayout::setFixedNodes(const Graph &G, List<node>& nodes,\
				const List<node>& givenNodes, List<DPoint>& pos,\
				double radius)
// Overload setFixedNodes for given nodes
{
  GraphCopy GC(G);

  // delete possible old entries in nodes and pos
  nodes.clear();
  pos.clear();
  
  // set nodes and pos
  
  forall_listiterators(node, it, givenNodes) {
    node theOrig = *it;
    node theCopy = GC.copy(theOrig);
    nodes.pushBack(theCopy);
  }
  
  double step  = 2.0 * pi / (double)(nodes.size());
  double alpha = 0.0;
  forall_listiterators(node, it, nodes) {
    pos.pushBack(DPoint(radius * cos(alpha), radius * sin(alpha)));
    alpha += step;
  }
}

void TutteLayout::call(GraphAttributes &AG, const List<node> &givenNodes)
{
  const Graph &G = AG.constGraph();
  
  List<node> fixedNodes;
  List<DPoint> positions;
    
  double diam =
    sqrt((m_bbox.width()) * (m_bbox.width())
         + (m_bbox.height()) * (m_bbox.height()));
  
  // handle graphs with less than two nodes
  switch (G.numberOfNodes()) {
  case 0:
    return;
  case 1:
    node v = G.firstNode();
    
    DPoint center(0.5 * m_bbox.width(),0.5 * m_bbox.height());
    center = center + m_bbox.p1();
    
    AG.x(v) = center.m_x;
    AG.y(v) = center.m_y;
    
    return;
  }

  // increase radius to have no overlap on the outer circle
  node v = G.firstNode();

  double r        = diam/2.8284271;
  int    n        = G.numberOfNodes();
  double nodeDiam = 2.0*sqrt((AG.width(v)) * (AG.width(v)) 
			 + (AG.height(v)) * (AG.height(v)));

  if(r<nodeDiam/(2*sin(2*pi/n))) {
    r=nodeDiam/(2*sin(2*pi/n));
    m_bbox = DRect (0.0, 0.0, 2*r, 2*r);
  }

  setFixedNodes(G,fixedNodes,givenNodes,positions,r);
  
  doCall(AG,fixedNodes,positions);
}

void TutteLayout::call(GraphAttributes &AG)
{
  const Graph &G = AG.constGraph();
  
  List<node> fixedNodes;
  List<DPoint> positions;
    
  double diam =
    sqrt((m_bbox.width()) * (m_bbox.width())
         + (m_bbox.height()) * (m_bbox.height()));
  
  // handle graphs with less than two nodes
  switch (G.numberOfNodes()) {
  case 0:
    return;
  case 1:
    node v = G.firstNode();
    
    DPoint center(0.5 * m_bbox.width(),0.5 * m_bbox.height());
    center = center + m_bbox.p1();
    
    AG.x(v) = center.m_x;
    AG.y(v) = center.m_y;
    
    return;
  }

  // increase radius to have no overlap on the outer circle
  node v = G.firstNode();

  double r        = diam/2.8284271;
  int n           = G.numberOfNodes();
  double nodeDiam = 2.0*sqrt((AG.width(v)) * (AG.width(v)) 
			 + (AG.height(v)) * (AG.height(v)));

  if(r<nodeDiam/(2*sin(2*pi/n))) {
    r=nodeDiam/(2*sin(2*pi/n));
    m_bbox = DRect (0.0, 0.0, 2*r, 2*r);
  }

  setFixedNodes(G,fixedNodes,positions,r);
  
  doCall(AG,fixedNodes,positions);
}



bool TutteLayout::doCall(GraphAttributes &AG,
			 const List<node> &fixedNodes, 
			 List<DPoint> &fixedPositions)
  // does the actual computation. fixedNodes and fixedPositions
  // contain the nodes with fixed positions.
{
  node v, w;
  edge e;

  const Graph &G = AG.constGraph();
  GraphCopy GC(G);
  GraphCopyAttributes AGC(GC, AG);

  // mark fixed nodes and set their positions in a
  NodeArray<bool> fixed(GC,false);
  forall_listiterators(node, it, fixedNodes) {
    fixed[*it] = true;
    DPoint p = fixedPositions.popFrontRet();   // slightly dirty...
    fixedPositions.pushBack(p);          // ...

    AGC.x(*it) = p.m_x;
    AGC.y(*it) = p.m_y;
  }
  
  if(fixedNodes.size() == G.numberOfNodes()) {
    forall_nodes(v,GC) {
      AG.x(GC.original(v)) = AGC.x(v);
      AG.y(GC.original(v)) = AGC.y(v);
    }
    return true;
  }
  // all nodes have fixed positions - nothing left to do

  // collect other nodes
  List<node> otherNodes;
  forall_nodes(v,GC) if(!fixed[v]) otherNodes.pushBack(v);

  NodeArray<int> ind(GC);       // position of v in otherNodes and A

  int i = 0;

  forall_listiterators(node, it, otherNodes) ind[*it] = i++;

  int n = otherNodes.size();           // #other nodes
  Array<double> coord(n);              // coordinates (first x then y)
  Array<double> rhs(n);                // right hand side
  double oneOverD = 0.0;

  CoinPackedMatrix A(false,0,0);       // equations
  A.setDimensions(n,n);
  
  // initialize non-zero entries in matrix A
  forall_listiterators(node, it, otherNodes) {
    oneOverD = (double)(1.0/((*it)->degree()));
    forall_adj_edges(e,*it) {
      // get second node of e
      w = (*it == e->source()) ? e->target() : e->source();
      if(!fixed[w]) {
	A.modifyCoefficient(ind[*it],ind[w],oneOverD);
      }
    }
    A.modifyCoefficient(ind[*it],ind[*it],-1);
  }

  // compute right hand side for x coordinates
  forall_listiterators(node, it, otherNodes) {
    rhs[ind[*it]] = 0;
    oneOverD = (double)(1.0/((*it)->degree()));
    forall_adj_edges(e,*it) {
      // get second node of e
      w = (*it == e->source()) ? e->target() : e->source();
      if(fixed[w]) rhs[ind[*it]] -= (oneOverD*AGC.x(w));
    }
  }

  // compute x coordinates
  if(!(solveLP(n, n, A, rhs, coord))) return false;
  forall_listiterators(node, it, otherNodes) AGC.x(*it) = coord[ind[*it]];
  
  // compute right hand side for y coordinates
  forall_listiterators(node, it, otherNodes) {
    rhs[ind[*it]] = 0;
    oneOverD = (double)(1.0/((*it)->degree()));
    forall_adj_edges(e,*it) {
      // get second node of e
      w = (*it == e->source()) ? e->target() : e->source();
      if(fixed[w]) rhs[ind[*it]] -= (oneOverD*AGC.y(w));
    }
  }

  // compute y coordinates
  if(!(solveLP(n, n, A, rhs, coord))) return false;
  forall_listiterators(node, it, otherNodes) AGC.y(*it) = coord[ind[*it]];
  
  // translate coordinates, such that the center lies in
  // the center of the bounding box
  DPoint center(0.5 * m_bbox.width(),0.5 * m_bbox.height());
  
  forall_nodes (v, GC) {
    AGC.x(v) += center.m_x;
    AGC.y(v) += center.m_y; 
  }

  forall_nodes(v,GC) {
    AG.x(GC.original(v)) = AGC.x(v);
    AG.y(GC.original(v)) = AGC.y(v);
  }

  return true;
}
} // end namespace ogdf

#endif
