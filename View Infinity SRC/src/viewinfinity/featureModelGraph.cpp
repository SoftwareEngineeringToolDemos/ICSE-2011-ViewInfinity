#include "featureModelGraph.h"
#include <iostream>

using namespace ogdf;
using namespace smv;
/*
	obligatorisch = edgeType dependency (2)
	optional = edgeType association (0)
	alternative = nodeType generalisationMerger (2)
	oder = nodeType generalisationExpander (3)
*/

FeatureModelGraph::FeatureModelGraph()
{
}

bool FeatureModelGraph::initGraph()
{
	mGraph = new Graph();
	mFeatureArray.init(*mGraph);
	mParentArray.init(*mGraph);
	mGA = new GraphAttributes();
	mGA->init(*mGraph, GraphAttributes::nodeGraphics |	
					   GraphAttributes::edgeGraphics |
					   GraphAttributes::nodeId |
					   GraphAttributes::nodeLabel |
					   GraphAttributes::nodeType |	
					   GraphAttributes::edgeType );

	return true;
}

node FeatureModelGraph::addNodeToGraph(int iIndex, String strLabelText, int iType)
{
	// creates a new node with its global ID
	node child = mGraph->newNode(iIndex);
	// set ID as attribute
	mGA->idNode(child) = iIndex;
	mGA->labelNode(child) = strLabelText;
	mGA->type(child) = Graph::NodeType(iType);
	// add node to node array
	mParentArray[iIndex] = child;

	return child;
}

edge FeatureModelGraph::addEdgeToGraph(node v, node w,int iIndex, int iType)
{
	edge e = mGraph->newEdge(v,w,iIndex);
	mGA->type(e) = Graph::EdgeType(iType);
	
	return e;
}

node FeatureModelGraph::getParentNode(int iIndex)
{
	// find a node in the node array and return it
	return (node)mParentArray[iIndex];
}


// funktionier aus unbekannten gründen nicht, weil mGA plötzlich nicht mehr initialisiert ist
/*bool FeatureModelGraph::setNodeAtt(node v, int index, String labelText, int type )
{
	mGA->idNode(v) = index;
	mGA->labelNode(v) = labelText;
	mGA->type(v) = Graph::NodeType(type);
	return true;
}*/

Graph FeatureModelGraph::getGraph()
{
	mGA->writeGML("featureGraph.gml");
	printf("finshed");
	return *mGraph;
}
 
int FeatureModelGraph::createLayout()
{
    /*SugiyamaLayout SL;
    SL.setRanking(new OptimalRanking);
    SL.setCrossMin(new MedianHeuristic);
 
    OptimalHierarchyLayout *ohl = new OptimalHierarchyLayout;
	ohl->layerDistance(30.0);
    ohl->nodeDistance(25.0);
    ohl->weightBalancing(0.8);
    SL.setLayout(ohl);
 
    SL.call(*mGA);
    mGA->writeGML("unix-history-layout.gml");
 */
    return 0;
}