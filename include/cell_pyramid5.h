// $Id: cell_pyramid5.h,v 1.1.1.1 2003-01-10 16:17:48 libmesh Exp $

// The Next Great Finite Element Library.
// Copyright (C) 2002  Benjamin S. Kirk, John W. Peterson
  
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
  
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
  
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA



#ifndef __cell_pyramid5_h__
#define __cell_pyramid5_h__

// C++ includes

// Local includes
#include "cell_pyramid.h"




/**
 * The \p Pyramid5 is an element in 3D composed of 5 nodes.
 * It is numbered like this:
   \verbatim
   PYRAMID5:
           4 
           o
          / \
         /   \
        /     \
     3 o-------o 2 
       |       |
       |       |
       o-------o
       0       1
   \endverbatim
 */

// ------------------------------------------------------------
// Pyramid class definition
class Pyramid5 : public Pyramid
{
public:

  /**
   * Constructor.  By default this element has no parent.
   */
  Pyramid5  (Cell* p=NULL);
  
  /**
   * @returns \p PRYAMID
   */
  ElemType     type () const   { return PYRAMID5; };

  /**
   * @returns 1
   */
  unsigned int n_sub_elem() const { return 1; };
  
  /**
   * @returns FIRST
   */
  Order default_order() const { return FIRST; };
  
  /**
   * Builds a QUAD4 or TRI3 built coincident with face i.  This
   * method allocates memory, so be sure to delete
   * the returned pointer when it is no longer needed.
   */
  std::auto_ptr<Elem> build_side (const unsigned int i) const;

  const std::vector<unsigned int> tecplot_connectivity(const unsigned int sc=0) const;
  
  
  void vtk_connectivity(const unsigned int sc,
			std::vector<unsigned int> *conn = NULL) const;
  
  unsigned int vtk_element_type (const unsigned int sc) const;
  
#ifdef ENABLE_AMR

  /**
   * Refine the element.
   */
  void refine(Mesh& mesh);
  
  /**
   * Coarsen the element.
   */
  void coarsen();

#endif
  
private:
  
};



// ------------------------------------------------------------
// Pyramid5 class member functions
inline
Pyramid5::Pyramid5(Cell* p) :
  Pyramid(Pyramid5::n_nodes(), p) 
{
};



#endif
