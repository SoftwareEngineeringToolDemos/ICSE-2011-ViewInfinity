

#include "readmodel.h"
#include <QtGui>
#include <iostream>
#include <QFileDialog>
#include <QWidget>
#include <QApplication>
#include <QList>
#include "ogdf/basic/Graph.h"
#include "ogdf/basic/GraphAttributes.h"

using namespace ogdf;
namespace smv
{

	ReadModel::ReadModel()
	{
	}
	bool ReadModel::init()
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
						   GraphAttributes::nodeColor |
						   GraphAttributes::edgeType );

		return false;
	}

	bool ReadModel::loadFile(QString input)
	{
		mInputFile.setFileName(input);

		if (!mInputFile.open(QIODevice::ReadOnly)) 
		{
			QMessageBox::warning(0, "Error Opening File",
									"Could not open file "+input);
			return false;
		}
		return true;
	}

	bool ReadModel::readFileLine()
	{	
		if(!mInputFile.size())
			return false;
		
		// read one line of text from the .m file
		QString strLine = mInputFile.readLine(0);
		//mstrList = strLine.split(QRegExp("\\W+"), QString::SkipEmptyParts);

		// split the input text line after ":" "::" "space" ";" and save the splitted 
		// text in a QStringList
		mstrList = strLine.split(QRegExp("\\:+|\\::+|\\s+|\\;+"), QString::SkipEmptyParts);
		
		if(mstrList.isEmpty())
			return false;
		else 
			return true;	
		
		return false;
	}

	bool ReadModel::isObligatory(QStringList strInputList)
	{
		if(strInputList.isEmpty())
			return false;

		// first and last entries represent the parent nodes
		strInputList.removeFirst();
		strInputList.removeLast();

		// search for entries without []
		mstrObligatoryList = strInputList.filter(QRegExp("^\\w"));
		
		if(!mstrObligatoryList.isEmpty())
			return true;

		return false;

	}
	int ReadModel::isAlternative(QStringList strInputList)
	{
		if(strInputList.isEmpty())
			return -1;

		// search for entries with a _
		mstrNoAlternativeList = strInputList.filter(QRegExp("^\\_"));
		
		if(mstrNoAlternativeList.isEmpty())
		{
			mstrParentNodes.insert(miIndex,strInputList.last());
			if(strInputList.contains("|",Qt::CaseSensitive))
				return 2;
			else
				return 1;
		}

		return 0;
	}

	bool ReadModel::isOptional(QStringList strInputList)
	{
		if(strInputList.isEmpty())
			return false;

		// search for entries with a []
		mstrOptionalList = strInputList.filter(QRegExp("\\[+"));
		
		if(!mstrOptionalList.isEmpty())
			return true;	

		return false;
	}

	bool ReadModel::isOr(QStringList strInputList)
	{
		if(strInputList.isEmpty())
			return false;

		// kein Beispiel in den gegebenen Codebasen

		return false;
	}

	String ReadModel::setNodeLabel(QStringList strInputList, int iIndex)
	{
		// get QString from the list
		QString strNodeName = (QString)strInputList.at(iIndex);
		if(strNodeName.contains("[",Qt::CaseSensitive))
			strNodeName.remove(QRegExp("\\[+|\\]+"));
		// convert from QString to String
		QByteArray name = strNodeName.toLocal8Bit();
		String strOutput(name.constData());

		return strOutput;
	}

	bool ReadModel::createNodeList(QStringList strInputList)
	{
		// first and last entries were already added
		strInputList.removeFirst();
		strInputList.removeLast();

		// add potential parents
		mstrParentNodes.append(strInputList);
		
		if(!mstrParentNodes.isEmpty())
			return true;

		return false;
	}
	node ReadModel::addNodeToGraph(int iIndex, String strLabelText, int iType)
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

	edge ReadModel::addEdgeToGraph(node v, node w,int iIndex)
	{
		edge e = mGraph->newEdge(v,w,iIndex);
		//mGA->type(e) = Graph::EdgeType(iType);
		
		return e;
	}

	node ReadModel::getParentNode(int iIndex)
	{
		// find a node in the node array and return it
		return (node)mParentArray[iIndex];
	}

	Graph ReadModel::getGraph()
	{
		//mGA->writeGML("featureGraph.gml");
		//printf("finshed");
		return *mGraph;
	}

	String ReadModel::getLabel(node v)
	{
		String label;
		label = mGA->labelNode(v);
		return label;
	}
	String ReadModel::getColor(node v)
	{
		String color;
		color = mGA->colorNode(v);
		return color;
	}
	Graph::NodeType ReadModel::getType(node v)
	{
		return mGA->type(v);
	}
	/*
	int ReadModel::getType(node v)
	{
		return (int)mGA->type(v);
	}
	*/
	node ReadModel::setRoot(int alternative)
	{
		node parent;
		
		if(!alternative)
		{
			mstrParentNodes.insert(0,mstrList.first());
			parent = addNodeToGraph(0,setNodeLabel(mstrList,0),0);
			miIndex = 1;
			createSubGraph(parent,0,mstrList);
			return parent;		
		}
		else
		{
			mstrParentNodes.insert(0,mstrList.first());
			node root = addNodeToGraph(0,setNodeLabel(mstrList,0),2);
			parent = addNodeToGraph(1,setNodeLabel(mstrList,mstrList.length()-1),0);
			addEdgeToGraph(root,parent,0);
			miIndex = 2;		 
			createSubGraph(parent,1,mstrList);
			return parent;
		}

		return parent;			
	}
	int ReadModel::createSubGraph(node parent, int iEdgeIndex, QStringList strInputList)
	{
		int i;

		if(strInputList.isEmpty())
			return -1;
		
		strInputList.removeFirst();
		strInputList.removeLast();
		
		//isOptional(strInputList);
		//isObligatory(strInputList);

		for(i = 0; i < strInputList.size(); i++)
		{	
			if(mstrOptionalList.contains(strInputList[i]))
			{
				// create a new node with a global ID and set the label
				// if the child is optional it is in the optional stringlist 
				// and will be added with the optional attribute
				node child = addNodeToGraph(miIndex,setNodeLabel(strInputList,i),1);
				addEdgeToGraph(parent,child,iEdgeIndex);
				iEdgeIndex++;
				miIndex++;
				continue;
			}
			else if(getType(parent) == 2)
			{
				node child = addNodeToGraph(miIndex,setNodeLabel(strInputList,i),0);
				addEdgeToGraph(parent,child,iEdgeIndex);
				iEdgeIndex++;
				miIndex++;
				continue;
			}
			else
			{
				node child = addNodeToGraph(miIndex,setNodeLabel(strInputList,i),3);
				addEdgeToGraph(parent,child,iEdgeIndex);
				iEdgeIndex++;
				miIndex++;
				continue;
			}		
		}

		miEdgeIndex = iEdgeIndex;
		
		return miIndex;		
	}

	Graph ReadModel::createGraph()
	{
		int iCounter = 1;
		int iParentID;
		int alternative;
		miIndex = 0;
		node parent;
		node root;
		
		while(!mInputFile.atEnd())
		{
			// read .m-file line by line
			if(!readFileLine())
				continue;
			if(mstrList.contains("%%",Qt::CaseSensitive))
			{
				return getGraph();
			}			
			else
			{
				createNodeList(mstrList);
				alternative = isAlternative(mstrList);
				isOptional(mstrList);

				if(iCounter == 1)
				{
					//createNodeList(mstrList);
					setRoot(alternative);
					iCounter++;
					continue;
				}
				else
				{
					if(!alternative)
					{
						//isOptional(mstrList);
						// find the index of the parent 
						iParentID = mstrParentNodes.indexOf(QRegExp("\\["+mstrList[0]+ "\\]"));
						if(iParentID == -1)
							iParentID = mstrParentNodes.indexOf(mstrList[0]);
						// create a parent node from the NodeArray
						parent = getParentNode(iParentID);
						// create children and edges
						createSubGraph(parent,miEdgeIndex,mstrList);
						continue;
					}				
					else if(alternative == 1) // multiple alternatives
					{
						// ToDo: parent ID noch falsch
						// find the index of the parent
						iParentID = mstrParentNodes.indexOf(QRegExp("\\["+mstrList[0]+ "\\]"));
						if(iParentID == -1)
							iParentID = mstrParentNodes.indexOf(mstrList[0]);
						// create a parent node from the NodeArray
						root = getParentNode(iParentID);
						mGA->type(root) = Graph::NodeType(2);
						parent = addNodeToGraph(miIndex,setNodeLabel(mstrList,mstrList.length()-1),0);
						addEdgeToGraph(root,parent,miEdgeIndex);
						miIndex++;
						miEdgeIndex++;
						// create children and edges
						createSubGraph(parent,miEdgeIndex,mstrList);
						continue;
					}	
					else // one alternative
					{
						parent = addNodeToGraph(miIndex,setNodeLabel(mstrList,mstrList.length()-1),0);
						addEdgeToGraph(root,parent,miEdgeIndex);
						miIndex++;
						miEdgeIndex++;
						// create children and edges
						createSubGraph(parent,miEdgeIndex,mstrList);
					}				
				}
			}
		}
		// return the created graph and create the gml file
		return getGraph();
		//mFeatureGraph.createLayout();

	}
} // end of namespace smv
