// $Id: transient_system.h,v 1.5 2005-02-22 22:17:35 jwpeterson Exp $

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



#ifndef __transient_system_h__
#define __transient_system_h__

// C++ includes

// Local Includes
#include "system.h"
#include "linear_implicit_system.h"
#include "nonlinear_implicit_system.h"
#include "explicit_system.h"


/**
 * This class provides a specific system class.  It aims
 * at transient systems, offering nothing more than just
 * the essentials needed to solve a system.  Note
 * that still additional vectors/matrices may be added,
 * as offered in the parent classes.
 */

// ------------------------------------------------------------
// TransientSystem class definition
template <class Base>
class TransientSystem : public Base
{
public:

  /**
   * Constructor.  Initializes required
   * data structures.
   */
  TransientSystem (EquationSystems& es,
		   const std::string& name,
		   const unsigned int number);

  /**
   * Destructor.
   */
  ~TransientSystem ();
  
  /**
   * The type of system.
   */
  typedef TransientSystem<Base> sys_type;

  /**
   * @returns a clever pointer to the system.
   */
  sys_type & system () { return *this; }

  /**
   * Clear all the data structures associated with
   * the system. 
   */
  virtual void clear ();

  /**
   * Reinitializes the member data fields associated with
   * the system, so that, e.g., \p assemble() may be used.
   */
  virtual void reinit ();
  
  /**
   * @returns \p "Transient" prepended to T::system_type().
   * Helps in identifying the system type in an equation
   * system file.
   */
  virtual std::string system_type () const;

  
  //-----------------------------------------------------------------
  // access to the solution data fields
  
  /**
   * @returns the old solution (at the previous timestep)
   * for the specified global DOF.
   */
  Number old_solution (const unsigned int global_dof_number) const;

  /**
   * @returns the older solution (two timesteps ago)
   * for the specified global DOF.
   */
  Number older_solution (const unsigned int global_dof_number) const;

  /**
   * All the values I need to compute my contribution
   * to the simulation at hand.  Think of this as the
   * current solution with any ghost values needed from
   * other processors.
   */
  AutoPtr<NumericVector<Number> > old_local_solution;

  /**
   * All the values I need to compute my contribution
   * to the simulation at hand.  Think of this as the
   * current solution with any ghost values needed from
   * other processors.
   */
  AutoPtr<NumericVector<Number> > older_local_solution;


protected:
  

  /**
   * Initializes the member data fields associated with
   * the system, so that, e.g., \p assemble() may be used.
   */
  virtual void init_data ();

  /**
   * Re-update the local values when the mesh has changed.
   * This method takes the data updated by \p update() and
   * makes it up-to-date on the current mesh.
   */
  virtual void re_update ();
};



// -----------------------------------------------------------
// Useful typedefs
typedef TransientSystem<LinearImplicitSystem> TransientImplicitSystem;
typedef TransientSystem<LinearImplicitSystem> TransientLinearImplicitSystem;
typedef TransientSystem<NonlinearImplicitSystem> TransientNonlinearImplicitSystem;
typedef TransientSystem<ExplicitSystem> TransientExplicitSystem;



// ------------------------------------------------------------
// TransientSystem inline methods
template <class Base>
inline
std::string TransientSystem<Base>::system_type () const
{
  std::string type = "Transient";
  type += Base::system_type ();
  
  return type;
}



template <class Base>
inline
Number TransientSystem<Base>::old_solution (const unsigned int global_dof_number) const
{
  // Check the sizes
  assert (global_dof_number < this->_dof_map.n_dofs());
  assert (global_dof_number < old_local_solution->size());
   
  return (*old_local_solution)(global_dof_number);
}



template <class Base>
inline
Number TransientSystem<Base>::older_solution (const unsigned int global_dof_number) const
{
  // Check the sizes
  assert (global_dof_number < this->_dof_map.n_dofs());
  assert (global_dof_number < older_local_solution->size());
   
  return (*older_local_solution)(global_dof_number);
}



#endif
