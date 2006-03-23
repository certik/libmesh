// $Id: cell_inf_hex16.h,v 1.9 2006-03-23 20:24:36 roystgnr Exp $

// The libMesh Finite Element Library.
// Copyright (C) 2002-2005  Benjamin S. Kirk, John W. Peterson
  
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



#ifndef __cell_inf_hex16_h__
#define __cell_inf_hex16_h__

// C++ includes

// Local includes
#include "libmesh_config.h"
#ifdef ENABLE_INFINITE_ELEMENTS

#include "cell_inf_hex.h"




/**
 * The \p InfHex16 is an infinite element in 3D composed of 16 nodes.
 * It is numbered like this:
   \verbatim
   INFHEX16:   7              14             6     		     
               o              o              o      closer to infinity
               :              :              |     		    
               :              :              |     		     
               :              :              |     		     
         15    :              :        13    |     		      
          o    :              :         o    |     		      
          :    :              :         |    |     		      
          :    :              :         |    |
          :    :              :         |    |     
     4    :    :    12        :    5    |    |     
     o    :    :    o         :    o    |    |     		      
     |    :    :    |         :    |    |    |     
     |    :    :    |         :    |    |    |     
     |    :    :    |         :10  |    |    |                       
     |    :   3o....|.........o....|....|....o     
     |    :   .     |              |    |   / 2      
     |    :  .      |              |    |  /        
     |    : .       |              |    | /         
     |    :.        |              |    |/          
     |  11o         |              |    o           base face
     |   .          |              |   / 9          
     |  .           |              |  /             
     | .            |              | /              
     |.             |              |/               
     o--------------o--------------o                
     0              8              1                
   \endverbatim
 */

// ------------------------------------------------------------
// InfHex16 class definition
class InfHex16 : public InfHex
{
public:

  /**
   * Constructor.  By default this element has no parent.
   */
  InfHex16  (Elem* p=NULL);
    
  /**
   * @returns 16.  The \p InfHex16 has 16 nodes.
   */
  unsigned int n_nodes() const { return 16; }
  
  /**
   * @returns \p INFHEX16
   */
  ElemType     type ()   const { return INFHEX16; }
  
  /**
   * @returns 4
   */
  unsigned int n_sub_elem() const { return 1; }
  
  /**
   * @returns true iff the specified (local) node number is a vertex.
   */
  virtual bool is_vertex(const unsigned int i) const;

  /**
   * @returns true iff the specified (local) node number is an edge.
   */
  virtual bool is_edge(const unsigned int i) const;

  /**
   * @returns true iff the specified (local) node number is a face.
   */
  virtual bool is_face(const unsigned int i) const;
  
  /*
   * @returns true iff the specified (local) node number is on the
   * specified side
   */
  virtual bool is_node_on_side(const unsigned int n,
			       const unsigned int s) const;
  
  /*
   * @returns true iff the specified (local) node number is on the
   * specified edge
   */
  virtual bool is_node_on_edge(const unsigned int n,
			       const unsigned int e) const;
  
  /**
   * @returns SECOND
   */
  Order default_order() const { return SECOND; }
  
  /**
   * Returns a \p QUAD8 built coincident with face 0, an \p INFQUAD6 
   * built coincident with faces 1 to 4. Note that the \p AutoPtr<Elem>
   * takes care of freeing memory.
   */
  AutoPtr<Elem> build_side (const unsigned int i) const;

  /**
   * Returns a \p EDGE3 built coincident with edges 0 to 3, or \p INFEDGE2
   * built coincident with edges 4 to 11. Note that the \p AutoPtr<Elem>
   * takes care of freeing memory.
   */
  AutoPtr<Elem> build_edge (const unsigned int i) const;

  virtual void connectivity(const unsigned int sc,
			    const IOPackage iop,
			    std::vector<unsigned int>& conn) const;

//   void tecplot_connectivity(const unsigned int sc,
// 			    std::vector<unsigned int>& conn) const;
  
//   void vtk_connectivity(const unsigned int,
// 			std::vector<unsigned int>*) const
//   { error(); }
  
  unsigned int vtk_element_type (const unsigned int) const
  { return 12; }

  /**
   * @returns 2 for all \p n
   */
  unsigned int n_second_order_adjacent_vertices (const unsigned int) const
      { return 2; }

  /**
   * @returns the element-local number of the  \f$ v^{th} \f$ vertex
   * that defines the \f$ n^{th} \f$ second-order node.
   * Note that \p n is counted as depicted above, \f$ 8 \le n < 16 \f$.
   */
  unsigned short int second_order_adjacent_vertex (const unsigned int n,
						   const unsigned int v) const;

  /**
   * This maps the \f$ j^{th} \f$ node of the \f$ i^{th} \f$ side to
   * element node numbers.
   */
  static const unsigned int side_nodes_map[5][8];

  /**
   * This maps the \f$ j^{th} \f$ node of the \f$ i^{th} \f$ edge to
   * element node numbers.
   */
  static const unsigned int edge_nodes_map[8][3];
  
  

protected:  

  
#ifdef ENABLE_AMR

  /**
   * Matrix used to create the elements children.
   */
  float embedding_matrix (const unsigned int i,
			  const unsigned int j,
			  const unsigned int k) const
  { return _embedding_matrix[i][j][k]; }

  /**
   * Matrix that computes new nodal locations/solution values
   * from current nodes/solution.
   */
  static const float _embedding_matrix[4][16][16];
  
#endif

};



// ------------------------------------------------------------
// InfHex16 class member functions
inline
InfHex16::InfHex16(Elem* p) :
  InfHex(InfHex16::n_nodes(), p) 
{
}


#endif  // ifdef ENABLE_INFINITE_ELEMENTS


#endif
