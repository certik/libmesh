// rbOOmit: An implementation of the Certified Reduced Basis method.
// Copyright (C) 2009, 2010 David J. Knezevic

// This file is part of rbOOmit.

// rbOOmit is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
  
// rbOOmit is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
  
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#ifndef __rb_scm_system_h__
#define __rb_scm_system_h__

// Configuration data
#include "libmesh_config.h"

// Currently, the RBSCMConstruction is only usable
// if SLEPc is enabled.
#if defined(LIBMESH_HAVE_SLEPC) && (LIBMESH_HAVE_GLPK)

// rbOOmit includes
#include "rb_construction_base.h"
#include "rb_scm_evaluation.h"

// libMesh includes
#include "condensed_eigen_system.h"

namespace libMesh
{

/**
 * This class is part of the rbOOmit framework.
 *
 * RBSCMConstruction implements the the Successive Constraint Method (SCM)
 * for computing rigorous lower bounds for stability constants.
 *
 * @author David J. Knezevic 2009
 */

// ------------------------------------------------------------
// RBSCMConstruction class definition

class RBSCMConstruction : public RBConstructionBase<CondensedEigenSystem>
{
public:

  /**
   * Constructor.  Optionally initializes required
   * data structures.
   */
  RBSCMConstruction (EquationSystems& es,
               const std::string& name,
               const unsigned int number);

  /**
   * Destructor.
   */
  virtual ~RBSCMConstruction ();

  /**
   * The type of system.
   */
  typedef RBSCMConstruction sys_type;

  /**
   * The type of the parent.
   */
  typedef RBConstructionBase<CondensedEigenSystem> Parent;

  /**
   * Clear all the data structures associated with
   * the system.
   */
  virtual void clear ();

  /**
   * Clear and resize the SCM data vectors. Overload
   * in subclass as necessary.
   */
  virtual void resize_SCM_vectors ();

  /**
   * Read in the parameters from file specified by \p parameters_filename
   * and set the this system's member variables accordingly.
   */
  virtual void process_parameters_file(const std::string& parameters_filename);

  /**
   * Print out info that describes the current setup of this RBSCMConstruction.
   */
  virtual void print_info();

  /**
   * This function is called before truth eigensolves in
   * compute_SCM_bounding_box and evaluate_stability_constant.
   * Overload it to set specific properties to optimize
   * eigensolver performance. The argument refers to
   * the operator index in compute_SCM_bounding_box;
   * a negative value of the argument indicates we are
   * not performing a bounding box solve.
   */
  virtual void set_eigensolver_properties(int ) { }

  /**
   * Set the name of the associated RB system --- we need
   * this to load the (symmetrized) affine operators.
   */
  void set_RB_system_name(const std::string& name) { RB_system_name = name; }

  /**
   * Get/set SCM_eps: tolerance for SCM greedy.
   */
  Real get_SCM_eps() const          { return SCM_eps; }
  void set_SCM_eps(Real SCM_eps_in) { this->SCM_eps = SCM_eps_in; }

  /**
   * Perform the SCM greedy algorithm to develop a lower bound
   * over the training set.
   */
  virtual void perform_SCM_greedy();

  /**
   * Attach the deflation space defined by the specified vector, can
   * be useful in solving constrained eigenvalue problems.
   *
   * This function is called at the start of perform_SCM_greedy and by
   * default is does nothing. Overload in subclass to attach a specific
   * vector.
   */
  virtual void attach_deflation_space() {}

  //----------- PUBLIC DATA MEMBERS -----------//

  /**
   * The current RBSCMEvaluation object we are using to
   * perform the Evaluation stage of the SCM.
   */
  RBSCMEvaluation* rb_scm_eval;

protected:

  /**
   * Add the scaled symmetrized affine matrix from the
   * associated RBSystem to matrix_A.
   */
  virtual void add_scaled_symm_Aq(unsigned int q_a, Number scalar);

  /**
   * Copy over the matrix to store in matrix_B,
   * usually this is the mass or inner-product
   * matrix, but needs to be implemented in subclass.
   */
  virtual void load_matrix_B() ;

  /**
   * Compute the SCM bounding box.
   */
  virtual void compute_SCM_bounding_box();

  /**
   * Compute the stability constant for current_parameters
   * by solving a generalized eigenvalue problem over the
   * truth space.
   */
  virtual void evaluate_stability_constant();

  /**
   * Enrich C_J by adding the element of SCM_training_samples
   * that has the largest gap between alpha_LB and alpha_LB.
   */
  virtual void enrich_C_J(unsigned int new_C_J_index);

  /**
   * Compute upper and lower bounds for each SCM training point.
   * Return a pair containing the maximum SCM error, and the
   * index of the parameter in the training set at which the max
   * error is achieved.
   */
  virtual std::pair<unsigned int,Real> compute_SCM_bounds_on_training_set();

  /**
   * Compute the inner product between two vectors using the system's
   * matrix_B.
   */
  Number B_inner_product(const NumericVector<Number>& v, const NumericVector<Number>& w) const;

  /**
   * Compute the inner product between two vectors using
   * matrix Aq.
   */
  Number Aq_inner_product(unsigned int q,
                        const NumericVector<Number>& v,
                        const NumericVector<Number>& w);

  /**
   * Helper function which provides an error
   * indicator to be used in the SCM greedy.
   * Overload in subclasses to specialize behavior.
   */
  virtual Real SCM_greedy_error_indicator(Real LB, Real UB) { return fabs(UB-LB)/fabs(UB); }

  //----------- PROTECTED DATA MEMBERS -----------//

  /**
   * SCM tolerance, where SCM_eps \in (0,1).
   */
  Real SCM_eps;

  /**
   * The name of the associated RB system.
   */
  std::string RB_system_name;

private:

};

} // namespace libMesh

#endif // LIBMESH_HAVE_SLEPC && LIBMESH_HAVE_GLPK

#endif
