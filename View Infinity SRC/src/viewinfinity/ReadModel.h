

#pragma once

#ifndef READMODEL_H
#define READMODEL_H

#include "ogdf/basic/Graph.h"
#include <QApplication>
#include <QtGui>
#include <iostream>
#include <QFileDialog>
#include <QWidget>
#include <QApplication>
#include <QList>
#include "ogdf/basic/GraphAttributes.h"
#include "ogdf/basic/NodeArray.h"
#include "ogdf/basic/AdjEntryArray.h"
#include "ogdf/basic/Array.h"
#include "ogdf/layered/SugiyamaLayout.h"
#include "ogdf/layered/OptimalRanking.h"
#include "ogdf/layered/MedianHeuristic.h"
#include "ogdf/layered/OptimalHierarchyLayout.h"


class SMVMain;

using namespace ogdf;
namespace smv
{
	class ReadModel
	{

	public:

		/// Constructor
		ReadModel();
		bool init();


		/// Creates a feature graph from a model.m file by reading the file line per line
		Graph createGraph();
		
		bool loadFile(QString input);
		
				/// Methode to add a new node to an existing graph with a global ID,
				/// a lable and the type can be alternative (2) or oder (3), return the created node
				node addNodeToGraph(int iIndex,String strLabelText, int iType);

				/// Methode to add a new edge to the existing graph
				/// needs two nodes as input and a global ID, the type can be optional(0)
				/// or obligatory (2)
				edge addEdgeToGraph(node v, node w,int iIndex);

				/// Function to find a node in the graph by its global ID and returns it
				node getParentNode(int iIndex);
				//bool setNodeAtt(node v, int index, String labelText, int type);
				
				/// Function to get the created graph
				Graph getGraph();

				String getLabel(node v);
				String getColor(node v);
				Graph::NodeType getType(node v);
			
	protected:

		/// Input model.m file
		QFile mInputFile;

		/// List containing the actual line of text from the model.m
		QStringList mstrList;

		/// List containing the optional features of the actual text line
		QStringList mstrOptionalList;

		/// List filled with the actual obligatory features
		QStringList mstrObligatoryList;

		/// List containing the actual features with were not alternative
		/// only parent nodes/features can be alternative and
		/// non-alternative features were marked with a "_", so if a parent feature is not marked, it is alternative
		QStringList mstrNoAlternativeList;

		/// List of all parent nodes, parent nodes are the first nodes 
		/// of each text line in the model.m file
		/// used to find the ID of a node
		QStringList mstrParentNodes;

		/// Actual edge ID
		int miEdgeIndex;

		/// Actual node ID
		int miIndex;

		/// ID of actual parent node
		int miParentID;
		
		/// Methode for reading a text line of the model.m file, returns true for success
		bool readFileLine();

		/// Methode which creates the mstrObligatoryList from a 
		/// actual text line and checks if the features were obligatory or not
		bool isObligatory(QStringList strInputList);

		/// Methode which creates the mstrNoAlternativeList from a 
		/// actual text line and checks if the features were alternatvie or not
		int isAlternative(QStringList strInputList);

		/// Methode which creates the mstrOptionalList from a 
		/// actual text line and checks if the features were optional or not
		bool isOptional(QStringList strInputList);

		/// Not yet implemented 
		bool isOr(QStringList strInputList);

		/// Methode to create the mstrParentNodes list, containing all parent nodes, returns true for success
		bool createNodeList(QStringList strInputList);

		/// Function to set the root node in the feature graph
		node setRoot(int alternative);

		/// Function to create a parent node and all children, by reading out one text line
		/// and set the first node a parent node and all following as children except the last one
		int createSubGraph(node parent, int iEdgeIndex,QStringList strInputList);

		/// Methode to convert a QString in a String and returns the String
		String setNodeLabel(QStringList strInputList, int iIndex);

	private:

		/// Create a new empty graph
			Graph *mGraph;

		/// Create a new instance of graph attributes
			GraphAttributes *mGA;

			AdjEntryArray<node> mParentArray;

		/// Create a new empty node array to save the created nodes with an ID
			NodeArray<node> mFeatureArray;
	};
}; // end of namespace smv
#endif // READMODEL_H
