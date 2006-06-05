
// $Id: fem_system.h,v 1.1 2006-06-05 00:32:23 roystgnr Exp $

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



#ifndef __fem_system_h__
#define __fem_system_h__

// C++ includes

// Local Includes
#include "diff_system.h"

// Forward Declarations

template <typename T> class NumericVector;

/**
 * This class provides a specific system class.  It aims
 * at nonlinear implicit systems, requiring only a
 * cell residual calculation from the user.  Note
 * that still additional vectors/matrices may be added,
 * as offered in the class \p ExplicitSystem.
 */

// ------------------------------------------------------------
// FEMSystem class definition

class FEMSystem : public DifferentiableSystem
{
public:

  /**
   * Constructor.  Optionally initializes required
   * data structures.
   */
  FEMSystem (EquationSystems& es,
	         const std::string& name,
	         const unsigned int number);

  /**
   * Destructor.
   */
  virtual ~FEMSystem ();

  /**
   * The type of system.
   */
  typedef FEMSystem sys_type;

  /**
   * The type of the parent.
   */
  typedef DifferentiableSystem Parent;
  
  /**
   * Clear all the data structures associated with
   * the system. 
   */
  virtual void clear ();

  /**
   * Prepares \p matrix or \p rhs for matrix assembly.
   * Users should not need to reimplement this
   */
  void assembly (bool get_residual, bool get_jacobian);

  /**
   * @returns \p "General".  Helps in identifying
   * the system type in an equation system file.
   */
  virtual std::string system_type () const { return "PDE"; }

protected:

  /**
   * Initializes the member data fields associated with
   * the system, so that, e.g., \p assemble() may be used.
   */
  virtual void init_data ();

  /**
   * Uses the results of multiple element_residual() calls
   * to 
   */
  void compute_numerical_jacobian ();

  /**
   * Finite element objects for each variable's interior and sides.
   */
  std::map<FEType, FEBase *> element_fe;
  std::map<FEType, FEBase *> side_fe;

  /**
   * Quadrature rules for element interior and sides.
   * The FEM system will try to find a quadrature rule that
   * correctly integrates all variables
   */
  QBase *element_qrule;
  QBase *side_qrule;

  /**
   * Current element for element_* to examine
   */
  Elem *elem;

  /**
   * Current element for side_* to examine
   */
  unsigned int side;

  /**
   * Local components of nonlinear_solution
   */
  DenseVector<Number> elem_solution;
  std::vector<DenseSubVector<Number> *> elem_subsolutions;

  /**
   * Element residual vector and Jacobian matrix
   */
  DenseVector<Number> elem_residual;
  DenseMatrix<Number> elem_jacobian;

  /**
   * Element residual subvectors and Jacobian submatrices
   */
  std::vector<DenseSubVector<Number> *> elem_subresiduals;
  std::vector<std::vector<DenseSubMatrix<Number> *> > elem_subjacobians;

  /** 
   * Global Degree of freedom index lists
   */
  std::vector<unsigned int> dof_indices;
  std::vector<std::vector<unsigned int> > dof_indices_var;
};


#endif
