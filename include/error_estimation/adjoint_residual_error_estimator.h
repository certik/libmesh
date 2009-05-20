// $Id$

// The libMesh Finite Element Library.
// Copyright (C) 2002-2008 Benjamin S. Kirk, John W. Peterson, Roy H. Stogner
  
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



#ifndef __adjoint_residual_error_estimator_h__
#define __adjoint_residual_error_estimator_h__

// C++ includes
#include <vector>
#include <string>

// Local Includes
#include "auto_ptr.h"
#include "error_estimator.h"

// Forward Declarations





/**
 * This class implements a goal oriented error indicator, by weighting
 * residual-based estimates from the primal problem against estimates
 * from the adjoint problem.
 *
 * This is based on a trick suggested by Brian Carnes, but if it
 * doesn't actually work then the misunderstanding or
 * misimplementation will be the fault of Roy Stogner.  It's also
 * Roy's fault there's no literature reference here yet.
 * 
 * @author Roy H. Stogner, 2009.
 */
class AdjointResidualErrorEstimator : public ErrorEstimator
{
public:

  /**
   * Constructor.  Responsible for picking default subestimators.
   */
  AdjointResidualErrorEstimator();
  
  /**
   * Destructor.  
   */
  ~AdjointResidualErrorEstimator() {}

  /**
   * Access to the "subestimator" (default: Kelly) to use on the primal/forward solution
   */
  AutoPtr<ErrorEstimator> &primal_error_estimator() { return _primal_error_estimator; }
  
  /**
   * Access to the "subestimator" (default: Kelly) to use on the dual/adjoint solution
   */
  AutoPtr<ErrorEstimator> &dual_error_estimator() { return _dual_error_estimator; }
  
  /**
   * Has the adjoint problem already been solved?  If the user sets
   * \p adjoint_already_solved to \p true, we won't waste time solving
   * it again.
   */
  bool adjoint_already_solved;

  /**
   * Compute the adjoint-weighted error on each element and place it
   * in the \p error_per_cell vector
   */
  void estimate_error (const System& system,
                       ErrorVector& error_per_cell,
                       bool estimate_parent_error = false);
  
protected:

  /**
   * An error estimator for the forward problem
   */
  AutoPtr<ErrorEstimator> _primal_error_estimator;

  /**
   * An error estimator for the adjoint problem
   */
  AutoPtr<ErrorEstimator> _dual_error_estimator;
};


#endif //__adjoint_residual_error_estimator_h__
