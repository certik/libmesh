// $Id: point.h,v 1.4 2005-02-22 22:17:33 jwpeterson Exp $

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



#ifndef __point_h__
#define __point_h__

// C++ includes
#include <cmath>

// Local includes
#include "type_vector.h"




/**
 * A \p Point defines a location in DIM dimensional Real space.  Points
 * are always real-valued, even if the library is configured with
 * \p --enable-complex.
 *
 * \author Benjamin S. Kirk, 2003. 
 */

class Point : public TypeVector<Real>
{
 public:

  /**
   * Constructor.  By default sets all entries to 0.  Gives the point 0 in
   * \p DIM dimensions.
   */
  Point  (const Real x=0.,
	  const Real y=0.,
	  const Real z=0.);

  /**
   * Copy-constructor.
   */
  Point (const Point& p);

  /**
   * Copy-constructor.
   */
  Point (const TypeVector<Real>& p);

//   /**
//    * @returns a key associated with this point.  Useful for sorting.
//    */
//   unsigned int key() const;

  
 protected:

  
  /**
   * Make the derived class a friend
   */
  friend class Node;
};



//------------------------------------------------------
// Inline functions
inline
Point::Point (const Real x,
	      const Real y,
	      const Real z) :
  TypeVector<Real> (x,y,z)
{
}



inline
Point::Point (const Point& p) :
  TypeVector<Real> (p)
{
}



inline
Point::Point (const TypeVector<Real>& p) :
  TypeVector<Real> (p)
{
}


#endif // #define __point_h__
