/*
 * $Revision: 1.5 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2007-11-17 23:06:08 +0100 (Sa, 17 Nov 2007) $ 
 ***************************************************************/
 
/** \file
 * \brief Declaration of class XmlParser.
 * 
 * \author Sebastian Leipert and Carsten Gutwenger
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

#ifndef OGDF_XML_PARSER_H
#define OGDF_XML_PARSER_H


#include <ogdf/basic/Hashing.h>
#include <ogdf/basic/String.h>
#include <ogdf/basic/GraphAttributes.h>
#include <ogdf/fileformats/XmlObject.h>
#include <ogdf/basic/SList.h>


namespace ogdf {



//---------------------------------------------------------
// XmlParser
// reads XML file and constructs XML parse tree
//---------------------------------------------------------
class XmlParser {
	Hashing<String,int> m_hashTable; // hash table for tags
	int m_num;

	istream *m_is;
	bool m_error;
	String m_errorString;

	char *m_rLineBuffer, *m_lineBuffer, *m_pCurrent, *m_pStore, m_cStore;

	int         m_intSymbol;    // integer attribute
	double      m_doubleSymbol; // double attribute
	const char *m_stringSymbol; // string attribute
	char	   *m_keyName;		// Tag name
	XmlKey		m_keySymbol;	// Tag name and Attribute Name in Hash Table
	String		m_longString;
	bool		m_eoTag;		// end of Tag recognized

	XmlObject *m_objectTree; // root node of XML parse tree

	bool m_doCheck;

	SList<char*> m_objectBody;

public:
	// predefined id constants for all used keys
	enum PredefinedKey { idPredefKey = 0, labelPredefKey, CreatorPredefKey,
		namePredefKey, graphPredefKey, versionPredefKey, directedPredefKey,
		nodePredefKey, edgePredefKey, graphicsPredefKey, xPredefKey,
		yPredefKey, wPredefKey, hPredefKey, nodetypePredefKey, edgetypePredefKey,
		typePredefKey, widthPredefKey, heightPredefKey, sizePredefKey,
		positionPredefKey, pathPredefKey,
		sourcePredefKey, targetPredefKey, sensePredefKey, arrowPredefKey, LinePredefKey,
		pointPredefKey, NEXTPREDEFKEY };

	// construction: creates object tree
	// sets m_error flag if an error occured
	XmlParser(const char *fileName, bool doCheck = false);
	XmlParser(istream &is, bool doCheck = false);

	// destruction: destroys object tree
	~XmlParser();

	// returns root object
	XmlObject *root() { return m_objectTree; }

	// id of a string in hash table; -1 if not contained
	int getId(const String &tag) const {
		HashElement<String,int> *it = m_hashTable.lookup(tag);
		return (it != 0) ? it->info() : -1;
	}

	// returns id of object
	int id(XmlObject *object) const { return object->m_key->info(); }

	// true <=> an error in XML files has been detected
	bool error() const { return m_error; }
	// returns error message
	const String &errorString() const { return m_errorString; }

	// creates graph from XML parse tree
	bool read(Graph &G);
	// creates attributed graph from XML parse tree
	bool read(Graph &G, GraphAttributes &AG);

private:
	void createObjectTree(istream &is, bool doCheck);
	void initPredefinedKeys();
	void setError(const char *errorString);

	XmlObject *parseList(XmlObjectType closingKey, XmlObjectType errorKey, const char *objectBodyName);
	XmlObjectType getNextSymbol();
	bool getLine();

	XmlKey hashString(const String &str);

	XmlObject *getNodeIdRange(int &minId,int &maxId, 
							  int &nodetypeCount,
							  XmlObject *graphObject);

	bool makeIdMap(int maxId,
				   Array<char*> &idMap, 
				   int nodetypeCount,
				   Array<char*> & typeName,
				   Array<double> &typeWidth,
				   Array<double> &typeHeight, 
				   XmlObject *graphObject);

	void closeLabels(Array<char*> idMap, Array<char*>  typeName);

	void readLineAttribute(XmlObject *object, DPolyline &dpl);

	void destroyObjectList(XmlObject *object);

	void indent(ostream &os, int d);
	void output(ostream &os, XmlObject *object, int d);

};


} // end namespace ogdf

#endif
