// $Id: point_locator_list.h,v 1.1 2003-07-05 14:58:56 ddreyer Exp $

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



#ifndef __point_locator_list_h__
#define __point_locator_list_h__

// C++ includes
#include <vector>


// Local Includes
#include "point_locator_base.h"



// Forward Declarations
class Mesh;
class Point;
class Elem;


/**
 * This is a point locator.  It locates points in space
 * using a list of element centroids: given a mesh this 
 * locator returns the element that is @e closest to the 
 * given point in global coordinates.
 * Note that this may yield @e severe difficulties in
 * case of extremely distorted elements, e.g.\ infinite
 * elements.
 *
 * This list version is not efficient, but especially
 * @e reliable for the case of finding the @e closest 
 * dim-1 element (nearest-surface-element, e.g. used
 * for projecting boundary conditions from a surface
 * mesh onto a volumtric mesh).
 * It should be noted that this class only works when
 * the element list in the associated mesh object is
 * @e not modified (like refinement etc).  Otherwise,
 * the point locator has to be cleared and re-initialized.
 * Use \p PointLocatorBase::build() to create objects 
 * of this type at run time.
 *
 * @author Daniel Dreyer, 2003
 */

// ------------------------------------------------------------
// PointLocatorList class definition
class PointLocatorList : public PointLocatorBase
{
public:

  /**
   * Constructor.  Needs the \p mesh which holds the 
   * elements that should be identified as being close.
   * Optionally takes a master interpolator.  This master 
   * helps in saving memory by simply only setting up @e 
   * one list for all point locators.  Only the master 
   * locator holds a list, the others simply  use the 
   * master's list.
   */
  PointLocatorList (const Mesh& mesh,
		    const PointLocatorBase* master = NULL);


public:

  /**
   * Destructor.
   */
  ~PointLocatorList ();

  /**
   * Clears the locator.
   */
  void clear();

  /**
   * Initializes the locator, so that the \p operator() methods can
   * be used.
   */
  void init();

  /**
   * Locates the element in which the point with global coordinates
   * \p p is located.  This cannot be a const method, because the
   * resulting element is remembered for the next request.
   */
  const Elem* operator() (const Point& p);


protected:

  /**
   * Pointer to the list of element centroids.  Only the
   * master @e has such a list.  For servants, this
   * pointer points to the list of the master.
   */
  std::vector<Point>* _list;

};


// ------------------------------------------------------------
// PointLocatorList inline methods


#endif


