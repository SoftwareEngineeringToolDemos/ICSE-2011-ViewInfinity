/*
 * $Revision: 1.22 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2007-11-09 12:12:41 +0100 (Fr, 09 Nov 2007) $ 
 ***************************************************************/
 
/** \file
 * \brief Offers variety of possible algorithm calls for simultaneous
 * drawing.
 * 
 * \author Michael Schulz
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

#ifndef OGDF_SIMDRAW_CALLER_H
#define OGDF_SIMDRAW_CALLER_H

#include <ogdf/simultaneous/SimDrawManipulatorModule.h>

namespace ogdf 
{

  //! Calls modified algorithms for simdraw instances
  /**
   *  Runs special algorithms suitable for simultaneous drawing
   *  on current SimDraw instance. The algorithms take
   *  care of all necessary GraphAttributes activations and
   *  take over calculated coordinates and dummy nodes.
   *
   *  A typical use of SimDrawCaller involves a predefined SimDraw
   *  instance on which SimDrawCaller works.
   *  \code 
   *  SimDraw SD;
   *  ...
   *  SimDrawCaller SDC(SD);
   *  SDC.callSubgraphPlanarizer();
   *  \endcode
   */
  class SimDrawCaller : public SimDrawManipulatorModule
    {

    private:
      EdgeArray<unsigned int> *m_esg;     //!< saves edgeSubGraph data

      //! updates m_esg
      /**
       *  Should be called whenever graph changed and current 
       *  basic graph membership is needed.
       */
      void updateESG();

    public:
      //! constructor
      SimDrawCaller(SimDraw &SD);

      //! runs SugiyamaLayout with modified SplitHeuristic
      /**
       *  Runs special call of SugiyamaLayout using 
       *  SugiyamaLayout::setSubgraphs(). 
       *  Saves node coordinates and dummy node bends in current 
       *  simdraw instance.
       *
       *  Uses TwoLayerCrossMinSimDraw object to perform crossing 
       *  minimization. The default is SplitHeuristic.
       *
       *  Automatically activates GraphAttributes::nodeGraphics.\n
       *  Automatically activates GraphAttributes::edgeGraphics.
       */
      void callSugiyamaLayout();

      //! runs UMLPlanarizationLayout with modified inserter
      /**
       *  Runs UMLPlanarizationLayout with callSimDraw and retransfers
       *  node coordinates and dummy node bend to current simdraw 
       *  instance.
       *
       *  Automatically activates GraphAttributes::nodeGraphics.\n
       *  Automatically activates GraphAttributes::edgeGraphics.
       */
      void callUMLPlanarizationLayout();

      //! runs SubgraphPlanarizer with modified inserter
      /**
       *  Runs SubgraphPlanarizer on connected component \a cc with simdraw 
       *  call. Integer edge costs of GraphAttributes are used 
       *  (1 for each edge if not available).
       *  
       *  Modifies graph by inserting dummy nodes for each crossing.
       *  All dummy nodes are marked as dummy. 
       *  (Method SimDrawColorizer::addColorNodeVersion is recommended
       *  for visualizing dummy nodes.)
       *
       *  No layout is calculated. The result is a planar graph.
       */
      int callSubgraphPlanarizer(int cc = 0, int numberOfPermutations = 1);

    };

} // end namespace ogdf

#endif
