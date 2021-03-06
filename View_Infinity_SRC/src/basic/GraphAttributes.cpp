/*
 * $Revision: 1.6 $
 * 
 * last checkin:
 *   $Author: klein $ 
 *   $Date: 2007-11-15 11:48:55 +0100 (Do, 15 Nov 2007) $ 
 ***************************************************************/
 
/** \file
 * \brief Implementation of class GraphAttributes. 
 * 
 * Class GraphAttributes extends a graph by graphical attributes like
 * node position, color, etc.
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


#include <ogdf/basic/GraphAttributes.h>
#include <ogdf/fileformats/GmlParser.h>
#include <ogdf/fileformats/XmlParser.h>


namespace ogdf {

//---------------------------------------------------------
// GraphAttributes
// graph topology + graphical attributes
//---------------------------------------------------------

GraphAttributes::GraphAttributes() : m_pGraph(0) { }



GraphAttributes::GraphAttributes(const Graph &G, long initAttr) :
	m_pGraph(&G), m_attributes(0)
{
	initAttributes(m_attributes = initAttr);
}



void GraphAttributes::initAttributes(long attr)
{
	m_attributes |= attr;

	 //no color without graphics
	OGDF_ASSERT( (m_attributes & nodeGraphics) != 0 || (m_attributes & nodeColor) == 0);
	//no fill and linewithout graphics
	OGDF_ASSERT( (m_attributes & nodeGraphics) != 0 || (attr & nodeStyle) == 0);
	//no color without graphics
	OGDF_ASSERT( (m_attributes & edgeGraphics) != 0 || (attr & edgeColor) == 0);

	if (attr & nodeGraphics) {
		m_x     .init(*m_pGraph,0);
		m_y     .init(*m_pGraph,0);
		m_width .init(*m_pGraph,0);
		m_height.init(*m_pGraph,0);
		m_nodeShape.init(*m_pGraph,rectangle);
	}

	if (attr & nodeColor) 
	{
		m_nodeColor.init(*m_pGraph, "");
		m_nodeLine.init(*m_pGraph, "");
	}

	if (attr & nodeStyle)
	{
		m_nodePattern.init(*m_pGraph, bpNone);
		m_nodeStyle.init(*m_pGraph, esSolid);
		m_nodeLineWidth.init(*m_pGraph, 1);
	}

	if (attr & edgeGraphics) {
		m_bends.init(*m_pGraph,DPolyline());
	}

	if (attr & edgeColor) 
		m_edgeColor.init(*m_pGraph);

	if (attr & edgeStyle)
	{
		m_edgeStyle.init(*m_pGraph, esSolid);
		m_edgeWidth.init(*m_pGraph, 1.0);
	}

	if (attr & nodeLevel) {
		m_level.init(*m_pGraph,0);
	}
	if (attr & edgeIntWeight) {
		m_intWeight.init(*m_pGraph,1);
	}
	if (attr & edgeDoubleWeight) {
		m_doubleWeight.init(*m_pGraph,1.0);
	}
	if (attr & nodeLabel) {
		m_nodeLabel.init(*m_pGraph);
	}
	if (attr & edgeLabel) {
		m_edgeLabel.init(*m_pGraph);
	}
	if (attr & edgeType) {
        m_eType.init(*m_pGraph,Graph::association);//should be Graph::standard end explicitly set
	}
	if (attr & nodeType) {
		m_vType.init(*m_pGraph,Graph::vertex);
	}
    if (attr & nodeId) {
        m_nodeId.init(*m_pGraph, -1);
    }
    if (attr & edgeArrow) {
        m_edgeArrow.init(*m_pGraph, undefined);
    }
	if (attr & nodeTemplate) {
		m_nodeTemplate.init(*m_pGraph);
	}
	if (attr & edgeSubGraph) {
		m_subGraph.init(*m_pGraph,0);
	}
}

void GraphAttributes::destroyAttributes(long attr)
{
	m_attributes &= ~attr;

	if (attr & nodeGraphics) {
		m_x     .init();
		m_y     .init();
		m_width .init();
		m_height.init();
		m_nodeShape.init();
		if (attr & nodeColor) 
			m_nodeColor.init();
		if (attr & nodeStyle)
		{
			m_nodePattern.init();
			m_nodeLine.init();
			m_nodeLineWidth.init();
		}
	}

	if (attr & edgeGraphics) {
		m_bends.init();
		if (attr & edgeColor)
			m_edgeColor.init();
		if (attr & edgeStyle)
		{
			m_edgeStyle.init();
			m_edgeWidth.init();
		}
	}

	if (attr & nodeLevel) {
		m_level.init();
	}
	if (attr & edgeIntWeight) {
		m_intWeight.init();
	}
	if (attr & edgeDoubleWeight) {
		m_doubleWeight.init();
	}
	if (attr & nodeLabel) {
		m_nodeLabel.init();
	}
	if (attr & edgeLabel) {
		m_edgeLabel.init();
	}
    if (attr & nodeId) {
        m_nodeId.init();
    }
    if (attr & edgeArrow) {
        m_edgeArrow.init();
    }
    if (attr & nodeTemplate) {
        m_nodeTemplate.init();
    }
	if (attr & edgeSubGraph) {
		m_subGraph.init();
	}
}


void GraphAttributes::init(const Graph &G, long initAttr)
{
	m_pGraph = &G;
	destroyAttributes(m_attributes);
	m_attributes = 0;
	initAttributes(m_attributes = initAttr);
}

void GraphAttributes::setAllWidth(double w)
{
	node v;
	forall_nodes(v,*m_pGraph)
		m_width[v] = w;
}

void GraphAttributes::setAllHeight(double h)
{
	node v;
	forall_nodes(v,*m_pGraph)
		m_height[v] = h;
}


void GraphAttributes::clearAllBends()
{
	edge e;
	forall_edges(e,*m_pGraph)
		m_bends[e].clear();
}


bool GraphAttributes::readGML(Graph &G, const String &fileName)
{
	ifstream is(fileName.cstr());
	if (!is)
		return false; // couldn't open file

	return readGML(G,is);
}


bool GraphAttributes::readGML(Graph &G, istream &is)
{
	GmlParser gml(is);
	if (gml.error())
		return false;

	return gml.read(G,*this);
}


bool GraphAttributes::readRudy(Graph &G, const String &fileName)
{
  ifstream is(fileName.cstr());
  if (!is)
    return false;
  return readRudy(G,is);
}


bool GraphAttributes::readRudy(Graph &G, istream &is)
{
  if (!is) 
    return false;
  int i;
  int n, m;
  int src, tgt;
  double weight;
  edge e;

  is >> n >> m;

  G.clear();
  Array<node> mapToNode(0,n-1,0);

  if (attributes() & edgeDoubleWeight){
    for(i=0; i<m; i++) {
      is >> src >> tgt >> weight;
      src--;
      tgt--;
      if(mapToNode[src] == 0) mapToNode[src] = G.newNode(src);
      if(mapToNode[tgt] == 0) mapToNode[tgt] = G.newNode(tgt);
      e = G.newEdge(mapToNode[src],mapToNode[tgt]);
      this->doubleWeight(e)=weight;
    }  
  }
  return true;
}


void GraphAttributes::writeRudy(const String &fileName) const
{
  ofstream os(fileName.cstr());
  writeRudy(os);
}


void GraphAttributes::writeRudy(ostream &os) const
{
  const Graph &G = this->constGraph();
  os << G.numberOfNodes() << " " << G.numberOfEdges() << endl;

  edge e;
  if (attributes() & edgeDoubleWeight){
    forall_edges(e,G) {
      os << (e->source()->index())+1 << " " << (e->target()->index())+1;
      os << " " << this->doubleWeight(e) << endl;
    }
  }
  else forall_edges(e,G) os << (e->source()->index())+1 << " " << (e->target()->index())+1 << endl;
}


const int c_maxLengthPerLine = 200;

void GraphAttributes::writeLongString(ostream &os, const String &str) const
{
	os << "\"";

	int num = 1;
	const char *p = str.cstr();
	while(*p != 0)
	{
		switch(*p) {
		case '\\':
			os << "\\\\";
			num += 2;
			break;
		case '\"':
			os << "\\\"";
			num += 2;
			break;

		// ignored white space
		case '\r':
		case '\n':
		case '\t':
			break;

		default:
			os << *p;
			++num;
		}

		if(num >= c_maxLengthPerLine) {
			os << "\\\n";
			num = 0;
		}

		++p;
	}

	os << "\"";
}


void GraphAttributes::writeGML(const String &fileName) const
{
	ofstream os(fileName.cstr());
	writeGML(os);
}


void GraphAttributes::writeGML(ostream &os) const
{
	NodeArray<int> id(*m_pGraph);
	int nextId = 0;

	os.setf(ios::showpoint);
	os.precision(10);

	os << "Creator \"ogdf::GraphAttributes::writeGML\"\n";
	os << "directed 1\n";

	os << "graph [\n";

	node v;
	forall_nodes(v,*m_pGraph) {
		os << "node [\n";

		os << "id " << (id[v] = nextId++) << "\n";

		if (attributes() & nodeTemplate) {
			os << "template ";
			writeLongString(os, templateNode(v));
			os << "\n";
		}

		if (attributes() & nodeLabel) {
			//os << "label \"" << labelNode(v) << "\"\n";
			os << "label ";
			writeLongString(os, labelNode(v));
			os << "\n";
		}

		if (m_attributes & nodeGraphics) {
			os << "graphics [\n";
			os << "x " << m_x[v] << "\n";
			os << "y " << m_y[v] << "\n";
			os << "w " << m_width[v] << "\n";
			os << "h " << m_height[v] << "\n";
			if (m_attributes & nodeColor)
			{
				os << "fill \"" << m_nodeColor[v] << "\"\n";
				os << "line \"" << m_nodeLine[v] << "\"\n";
			}//color
			if (m_attributes & nodeStyle)
			{
				os << "pattern \"" << m_nodePattern[v] << "\"\n";
				os << "stipple " << styleNode(v) << "\n";
				os << "lineWidth " << lineWidthNode(v) << "\n";
			}
			switch (m_nodeShape[v])
			{
				case rectangle: os << "type \"rectangle\"\n"; break;
				case oval: os << "type \"oval\"\n"; break;
			}
			os << "width 1.0\n";
			os << "]\n"; // graphics
		}

		os << "]\n"; // node
	}

	edge e;
	forall_edges(e,*m_pGraph) {
		os << "edge [\n";

		os << "source " << id[e->source()] << "\n";
		os << "target " << id[e->target()] << "\n";
		
		if (attributes() & edgeLabel){
			os << "label ";
			writeLongString(os, labelEdge(e));
			os << "\n";
		}
		if (attributes() & edgeType)
			os << "generalization " << type(e) << "\n";

		if (attributes() & edgeSubGraph)
		        os << "subgraph " << subGraphBits(e) << "\n";

		if (m_attributes & edgeGraphics) {
			os << "graphics [\n";

			os << "type \"line\"\n";

			if (attributes() & GraphAttributes::edgeType) {
				if (type(e) == Graph::generalization)
					os << "arrow \"last\"\n";
				else
					os << "arrow \"none\"\n";

				//os << "generalization " << type(e) << "\n";

			} else {
				os << "arrow \"last\"\n";
			}

			if (attributes() & GraphAttributes::edgeStyle)
			{
				os << "stipple " << styleEdge(e) << "\n";
				os << "lineWidth " << edgeWidth(e) << "\n";
			}//edgestyle is gml graphlet extension!!!

			//hier noch Unterscheidung Knotentypen, damit die Berechnung
			//fuer Ellipsen immer bis zum echten Rand rechnet
			const DPolyline &dpl = m_bends[e];
			if (!dpl.empty()) {
				os << "Line [\n";

				node v = e->source();
				if(dpl.front().m_x < m_x[v] - m_width[v]/2 ||
					dpl.front().m_x > m_x[v] + m_width[v]/2 ||
					dpl.front().m_y < m_y[v] - m_height[v]/2 ||
					dpl.front().m_y > m_y[v] + m_height[v]/2)
				{
					os << "point [ x " << m_x[e->source()] << " y " <<
						m_y[e->source()] << " ]\n";
				}

				ListConstIterator<DPoint> it;
				for(it = dpl.begin(); it.valid(); ++it)
					os << "point [ x " << (*it).m_x << " y " << (*it).m_y << " ]\n";

				v = e->target();
				if(dpl.back().m_x < m_x[v] - m_width[v]/2 ||
					dpl.back().m_x > m_x[v] + m_width[v]/2 ||
					dpl.back().m_y < m_y[v] - m_height[v]/2 ||
					dpl.back().m_y > m_y[v] + m_height[v]/2)
				{
					os << "point [ x " << m_x[e->target()] << " y " <<
						m_y[e->target()] << " ]\n";
				}

				os << "]\n"; // Line
			}//bends

			//output width and color
			if ((m_attributes & edgeColor) &&
				(m_edgeColor[e].length() != 0))
				os << "fill \"" << m_edgeColor[e] << "\"\n";

			os << "]\n"; // graphics
		}

		os << "]\n"; // edge
	}

	os << "]\n"; // graph
}


bool GraphAttributes::readXML(Graph &G, const String &fileName)
{
	ifstream is(fileName.cstr());
	return readXML(G,is);
}


bool GraphAttributes::readXML(Graph &G, istream &is)
{
	// need at least these attributes
	initAttributes(~m_attributes &
		(nodeGraphics | edgeGraphics | nodeLabel | edgeLabel));

	XmlParser xml(is);
	if (xml.error()) return false;

	return xml.read(G,*this);
}



//
// calculates the bounding box of the graph
const DRect GraphAttributes::boundingBox() const
{
    double minx, maxx, miny, maxy;
    const Graph           &G  = constGraph();
    const GraphAttributes &AG = *this;
    node v = G.firstNode();

    if (v == 0) {
        minx = maxx = miny = maxy = 0.0;
    }
    else {
        minx = AG.x(v) - AG.width(v)/2;
        maxx = AG.x(v) + AG.width(v)/2;
        miny = AG.y(v) - AG.height(v)/2;
        maxy = AG.y(v) + AG.height(v)/2;

        forall_nodes(v, G) {
            double x1 = AG.x(v) - AG.width(v)/2;
            double x2 = AG.x(v) + AG.width(v)/2;
            double y1 = AG.y(v) - AG.height(v)/2;
            double y2 = AG.y(v) + AG.height(v)/2;

            if (x1 < minx) minx = x1;
            if (x2 > maxx) maxx = x2;
            if (y1 < miny) miny = y1;
            if (y2 > maxy) maxy = y2;
        }
    }

    edge e;
    forall_edges(e, G) {
        const DPolyline &dpl = AG.bends(e);
        ListConstIterator<DPoint> iter;
        for (iter = dpl.begin(); iter.valid(); ++iter) {
            if ((*iter).m_x < minx) minx = (*iter).m_x;
            if ((*iter).m_x > maxx) maxx = (*iter).m_x;
            if ((*iter).m_y < miny) miny = (*iter).m_y;
            if ((*iter).m_y > maxy) maxy = (*iter).m_y;
        }
    }

    return DRect(minx, miny, maxx, maxy);
}


//
// returns a list of all hierachies in the graph (a hierachy consists of a set of nodes)
// at least one list is returned, which is the list of all nodes not belonging to any hierachy
// this is always the first list
// the return-value of this function is the number of hierachies
int GraphAttributes::hierarchyList(List<List<node>* > &list) const
{
    // list must be empty during startup
    OGDF_ASSERT(list.empty());

    const Graph &G = constGraph();
    Array<bool> processed(0, G.maxNodeIndex(), false);
    node v;
    edge e;

    // initialize the first list of all single nodes
    List<node> *firstList = new List<node>;
    list.pushBack(firstList);

    forall_nodes(v, G) { // scan all nodes

        // skip, if already processed
        if (processed[v->index()])
            continue;

        List<node> nodeSet;                    // set of nodes in this hierachy,
                                               // whose neighbours have to be processed
        List<node> *hierachy = new List<node>; // holds all nodes in this hierachy

        nodeSet.pushBack(v);           // push the unprocessed node to the list
        processed[v->index()] = true;  // and mark it as processed

        do { // scan all neighbours of nodes in 'nodeSet'
            node v = nodeSet.popFrontRet();
            hierachy->pushBack(v); // push v to the list of nodes in this hierachy

            // process all the neighbours of v, e.g. push them into 'nodeSet'
            forall_adj_edges(e, v) {
                if (type(e) == Graph::generalization) {
                    node w = e->source() == v ? e->target() : e->source();
                    if (!processed[w->index()]) {
                        nodeSet.pushBack(w);
                        processed[w->index()] = true;
                    }
                }
            }
        } while (!nodeSet.empty());

        // skip adding 'hierachy', if it contains only one node
        if (hierachy->size() == 1) {
            firstList->conc(*hierachy);
            delete hierachy;
        }
        else
            list.pushBack(hierachy);
    }

    return list.size() - 1 + (*list.begin())->size();
}

//
// returns a list of all hierarchies in the graph (in this case, a hierarchy consists of a set of edges)
// list may be empty, if no generalizations are used
// the return-value of this function is the number of hierarchies with generalizations
int GraphAttributes::hierarchyList(List<List<edge>* > &list) const
{
    // list must be empty during startup
    OGDF_ASSERT(list.empty());

    const Graph &G = constGraph();
    Array<bool> processed(0, G.maxNodeIndex(), false);
    node v;
    edge e;

    forall_nodes(v, G) { // scan all nodes

        // skip, if already processed
        if (processed[v->index()])
            continue;

        List<node> nodeSet;                    // set of nodes in this hierarchy,
                                               // whose neighbours have to be processed
        List<edge> *hierarchy = new List<edge>; // holds all edges in this hierarchy

        nodeSet.pushBack(v);           // push the unprocessed node to the list
        processed[v->index()] = true;  // and mark it as processed

        do { // scan all neighbours of nodes in 'nodeSet'
            node v = nodeSet.popFrontRet();

            // process all the neighbours of v, e.g. push them into 'nodeSet'
            forall_adj_edges(e, v) {
                if (type(e) == Graph::generalization) {
                    node w = e->source() == v ? e->target() : e->source();
                    if (!processed[w->index()]) {
                        nodeSet.pushBack(w);
                        processed[w->index()] = true;
                        hierarchy->pushBack(e); // push e to the list of edges in this hierarchy
                    }
                }
            }
        } while (!nodeSet.empty());

        // skip adding 'hierarchy', if it contains only one node
        if (hierarchy->empty())
            delete hierarchy;
        else
            list.pushBack(hierarchy);
    }

    return list.size();
}



void GraphAttributes::removeUnnecessaryBendsHV()
{
	edge e;
	forall_edges(e,*m_pGraph)
	{
		DPolyline &dpl = m_bends[e];

		if(dpl.size() < 3)
			continue;

		ListIterator<DPoint> it1, it2, it3;

		it1 = dpl.begin();
		it2 = it1.succ();
		it3 = it2.succ();

		do {
			if(((*it1).m_x == (*it2).m_x && (*it2).m_x == (*it3).m_x) ||
				((*it1).m_y == (*it2).m_y && (*it2).m_y == (*it3).m_y))
			{
				dpl.del(it2);
				it2 = it3;
			} else {
				it1 = it2;
				it2 = it3;
			}

			it3 = it2.succ();
		} while(it3.valid());
	}
}


void GraphAttributes::writeXML(
	const String &fileName,
	const char* delimiter,
	const char* offset) const
{
	ofstream os(fileName.cstr());
	writeXML(os,delimiter,offset);
}


void GraphAttributes::writeXML(
	ostream &os,
	const char* delimiter,
	const char* offset) const
{
	NodeArray<int> id(*m_pGraph);

	int nextId = 0;

	os.setf(ios::showpoint);
	os.precision(10);

	os << "<GRAPH TYPE=\"SSJ\">" << delimiter;

	node v;
	forall_nodes(v,*m_pGraph) {
		if (m_attributes & nodeLabel)
		{
			os << "<NODE NAME=\"" << m_nodeLabel[v] << "\">" << delimiter;
		}
		id[v] = nextId++;

		if (m_attributes & nodeGraphics) {
			os << offset << "<POSITION X=\"" << m_x[v] << "\" ";
			os << "Y=\"" << m_y[v] << "\" /> " << delimiter;
			os << offset << "<SIZE WIDTH=\"" << m_width[v] << "\" ";
			os << "HEIGHT=\"" << m_height[v] << "\" />"  << delimiter;

		}
		os << "</NODE>" << delimiter;
	}

	edge e;
	forall_edges(e,*m_pGraph) {
		if (m_attributes & edgeLabel)
		{
			os << "<EDGE NAME=\"" << m_edgeLabel[e] << "\" ";
		}
		if (m_attributes & nodeLabel)
		{
			os << "SOURCE=\"" << m_nodeLabel[e->source()] << "\" ";
			os << "TARGET=\"" << m_nodeLabel[e->target()] << "\" ";
			os << "GENERALIZATION=\"" << (m_eType[e]==Graph::generalization?1:0) << "\">" << delimiter;
		}

		if (m_attributes & edgeGraphics) {

			const DPolyline &dpl = m_bends[e];
			if (!dpl.empty()) {
				os << offset << "<PATH TYPE=\"polyline\">" << delimiter;
/*				if (m_backward[e])
				{
					os << (*dpl.rbegin()).m_x  << " " <<  (*dpl.rbegin()).m_y << " ";
					if (dpl.size() > 1)
						os << (*dpl.begin()).m_x << " " << (*dpl.begin()).m_y << " ";
				}
				else
				{
*/
				ListConstIterator<DPoint> iter;
				for (iter = dpl.begin(); iter.valid(); ++iter)
					os << offset << offset << "<POSITION X=\"" << (*iter).m_x << "\" " 
					   << "Y=\"" << (*iter).m_y << "\" />" << delimiter;
//					if (dpl.size() > 1)
//						os << "<POSITION X=\"" << (*dpl.rbegin()).m_x << "\" " 
//						   << "Y=\"" << (*dpl.rbegin()).m_y << "\" />";
//				}
				os << offset << "</PATH>" << delimiter;
			}

		}

		os << "</EDGE>" << delimiter; // edge
	}

	os << "</GRAPH>";
}

void GraphAttributes::addNodeCenter2Bends(int mode)
{
    Graph            &m_G  =  (Graph&) *this;
    GraphAttributes  &m_AG =           *this;

    edge e;
    forall_edges(e, m_G) {
        node v = e->source();
        node w = e->target();
        DPolyline &bends = m_AG.bends(e);
        switch (mode) {
        case 0 : // push center to the bends and return
            bends.pushFront(DPoint(m_AG.x(v), m_AG.y(v)));
            bends.pushBack (DPoint(m_AG.x(w), m_AG.y(w)));
            break;
        case 1 : // determine intersection with node and [center, last-bend-point]
            bends.pushFront(DPoint(m_AG.x(v), m_AG.y(v)));
            bends.pushBack (DPoint(m_AG.x(w), m_AG.y(w)));
        case 2 : // determine intersection between node and last bend-segment
            {
                DPoint sp1(m_AG.x(v) - m_AG.width(v)/2, m_AG.y(v) - m_AG.height(v)/2);
                DPoint sp2(m_AG.x(v) - m_AG.width(v)/2, m_AG.y(v) + m_AG.height(v)/2);
                DPoint sp3(m_AG.x(v) + m_AG.width(v)/2, m_AG.y(v) + m_AG.height(v)/2);
                DPoint sp4(m_AG.x(v) + m_AG.width(v)/2, m_AG.y(v) - m_AG.height(v)/2);
                DLine sourceRect[4] = { DLine(sp1, sp2),
                                        DLine(sp2, sp3),
                                        DLine(sp3, sp4),
                                        DLine(sp4, sp1) };

                DPoint tp1(m_AG.x(w) - m_AG.width(w)/2, m_AG.y(w) - m_AG.height(w)/2);
                DPoint tp2(m_AG.x(w) - m_AG.width(w)/2, m_AG.y(w) + m_AG.height(w)/2);
                DPoint tp3(m_AG.x(w) + m_AG.width(w)/2, m_AG.y(w) + m_AG.height(w)/2);
                DPoint tp4(m_AG.x(w) + m_AG.width(w)/2, m_AG.y(w) - m_AG.height(w)/2);
                DLine targetRect[4] = { DLine(tp1, tp2),
                                        DLine(tp2, tp3),
                                        DLine(tp3, tp4),
                                        DLine(tp4, tp1) };

                DRect source(sp1, sp3);
                DRect target(tp1, tp3);

                DPoint c1 = bends.popFrontRet();
                DPoint c2 = bends.popBackRet();

                while (!bends.empty() && source.contains(bends.front()))
                    c1 = bends.popFrontRet();
                while (!bends.empty() && target.contains(bends.back()))
                    c2 = bends.popBackRet();

                DPoint a1, a2;
                int i;
                if (bends.size() == 0) {
                    DLine cross(c1, c2);
                    for (i = 0; i < 4; i++)
                        if (cross.intersection(sourceRect[i], a1)) break;
                    for (i = 0; i < 4; i++)
                        if (cross.intersection(targetRect[i], a2)) break;
                }
                else {
                    DLine cross1(c1, bends.front());
                    for (i = 0; i < 4; i++)
                        if (cross1.intersection(sourceRect[i], a1)) break;
                    DLine cross2(bends.back(), c2);
                    for (i = 0; i < 4; i++)
                        if (cross2.intersection(targetRect[i], a2)) break;
                }
                bends.pushFront(a1);
                bends.pushBack(a2);
                break;
            }
        default:
            OGDF_ASSERT(0); // must never occur
            break;
        }
        bends.normalize();
    }
}



} // end namespace ogdf
