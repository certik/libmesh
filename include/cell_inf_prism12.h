// $Id: cell_inf_prism12.h,v 1.5 2003-01-24 17:24:37 jwpeterson Exp $

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



#ifndef __cell_inf_prism12_h__
#define __cell_inf_prism12_h__

// C++ includes

// Local includes
#include "mesh_config.h"
#include "cell_prism.h"




#ifdef ENABLE_INFINITE_ELEMENTS

/**
 * The \p InfPrism12 is an infinite element in 3D composed of 12 nodes.
 * It is numbered like this:
   \verbatim
   INFPRISM12:
            5      
            o      
           /|\     
          / | \    
         /  |  \
     11 o   |   o 10
       /   2|    \  
      /     o     \ 
     /     / \     \
   3o-------o-------o4
    |    /  9  \    |
    |   o       o   |
    |  / 8     7 \  |
    | /           \ |
    |/             \|
    o-------o-------o
    0       6       1
   \endverbatim
 */

// ------------------------------------------------------------
// InfPrism12 class definition
class InfPrism12 : public Prism
{
public:

  /**
   * Constructor.  By default this element has no parent.
   */
  InfPrism12  (Cell* p=NULL);
  
  /**
   * @returns \p INFPRISM12
   */
  ElemType     type () const   { return INFPRISM12; };

  /**
   * @returns 12
   */
  unsigned int n_nodes() const { return 12; };
  
  /**
   * @returns 4
   */
  unsigned int n_sub_elem() const { return 4; };
  
  /**
   * @returns SECOND
   */
  Order default_order() const { return SECOND; };
  
  /**
   * Returns a TRI6 built coincident with face 0, an INFQUAD6 
   * built coincident with faces 1 to 3.  Face 4 not supported. 
   * This method allocates memory, so be sure to delete
   * the returned pointer when it is no longer needed.
   */
  AutoPtr<Elem> build_side (const unsigned int i) const;

  const std::vector<unsigned int> tecplot_connectivity(const unsigned int sc=0) const;
  
  void vtk_connectivity(const unsigned int,
			std::vector<unsigned int>*) const
  { error(); };
  
  unsigned int vtk_element_type (const unsigned int) const
  { return 13; };
  
  void write_tecplot_connectivity(std::ostream &out) const;
  
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

  
#ifdef ENABLE_AMR
  
  /**
   * Matrix that computes new nodal locations/solution values
   * from current nodes/solution.
   */
  static const real embedding_matrix[4][12][12];
  
  /**
   * Matrix that tells which children share which of
   * my sides. Note that infinite elements use different
   * storage scheme than conventional elements.
   */
  static const unsigned int side_children_matrix[5][5];
  
#endif
  
};



// ------------------------------------------------------------
// InfPrism12 class member functions
inline
InfPrism12::InfPrism12(Cell* p) :
  Prism(InfPrism12::n_nodes(), p) 
{
};



#endif

#endif
