// The libMesh Finite Element Library.
// Copyright (C) 2002-2012 Benjamin S. Kirk, John W. Peterson, Roy H. Stogner

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

#if 0

#ifdef LIBMESH_ENABLE_AMR

namespace libMesh
{

// C++ includes
#include <algorithm> // for std::fill
#include <cmath>    // for sqrt


// Local Includes
#include "dof_map.h"
#include "elem.h"
#include "equation_systems.h"
#include "error_vector.h"
#include "fe.h"
#include "fe_interface.h"
#include "libmesh_common.h"
#include "libmesh_logging.h"
#include "mesh_refinement.h"
#include "numeric_vector.h"
#include "quadrature.h"
#include "system.h"
#include "adjoint_refinement_estimator.h"

//-----------------------------------------------------------------
// ErrorEstimator implementations
void AdjointRefinementEstimator::estimate_error (const System& _system,
					         ErrorVector& error_per_cell,
					         const NumericVector<Number>* solution_vector,
					         bool estimate_parent_error)
{
  // Make sure the user doesn't misunderstand what we're returning:
  libmesh_assert(error_norm == INVALID_NORM);

  // We have to break the rules here, because we can't refine a const System
  System *system = const_cast<System *>(_system);

  // An EquationSystems reference will be convenient.
  EquationSystems& es = system->get_equation_systems());

  // The current mesh
  MeshBase& mesh = es.get_mesh();

  // The dimensionality of the mesh
  const unsigned int dim = mesh.mesh_dimension();

  // Resize the error_per_cell vector to be
  // the number of elements, initialized to 0.
  error_per_cell.clear();
  error_per_cell->resize (mesh.max_elem_id(), 0.);

  // We'll want to back up all coarse grid vectors
  std::map<std::string, NumericVector<Number> *>
    coarse_vectors();
  for (System::vectors_iterator vec = system.vectors_begin(); vec !=
       system.vectors_end(); ++vec)
    {
      // The (string) name of this vector
      const std::string& var_name = vec->first;

      coarse_vectors[var_name] = vec->second->clone().release();
    }
  NumericVector<Number> * coarse_solution =
    system.solution->clone().release();
  NumericVector<Number> * coarse_local_solution =
    system.current_local_solution->clone().release();
  // And make copies of projected solutions
  NumericVector<Number> * projected_solution;

  // And we'll need to temporarily change solution projection settings
  bool old_projection_setting;

  // And it'll be best to avoid any repartitioning
  AutoPtr<Partitioner> old_partitioner = mesh.partitioner();
  mesh.partitioner().reset(NULL);

  // Use a non-standard solution vector if necessary
  if (solution_vector && solution_vector != system.solution.get())
    {
      NumericVector<Number>* newsol =
        const_cast<NumericVector<Number>*> (solution_vector);
      newsol->swap(*system.solution);
      system.update();
    }

  // Make sure the solution is projected when we refine the mesh
  old_projection_setting = system.project_solution_on_reinit();
  system.project_solution_on_reinit() = true;

  // Find the number of coarse mesh elements, to make it possible
  // to find correct coarse elem ids later
  const unsigned int max_coarse_elem_id = mesh.max_elem_id();
#ifndef NDEBUG
  // n_coarse_elem is only used in an assertion later so
  // avoid declaring it unless asserts are active.
  const unsigned int n_coarse_elem = mesh.n_elem();
#endif

  // Uniformly refine the mesh
  MeshRefinement mesh_refinement(mesh);

  libmesh_assert (number_h_refinements > 0 || number_p_refinements > 0);

  // FIXME: this may break if there is more than one System
  // on this mesh but estimate_error was still called instead of
  // estimate_errors
  for (unsigned int i = 0; i != number_h_refinements; ++i)
    {
      mesh_refinement.uniformly_refine(1);
      es.reinit();
    }

  for (unsigned int i = 0; i != number_p_refinements; ++i)
    {
      mesh_refinement.uniformly_p_refine(1);
      es.reinit();
    }

  for (unsigned int i=0; i != system_list.size(); ++i)
    {
      System &system = *system_list[i];

      // Copy the projected coarse grid solutions, which will be
      // overwritten by solve()
//      projected_solutions[i] = system.solution->clone().release();
      projected_solutions[i] = NumericVector<Number>::build().release();
      projected_solutions[i]->init(system.solution->size(), true, SERIAL);
      system.solution->localize(*projected_solutions[i],
                                system.get_dof_map().get_send_list());
    }

  // Get the uniformly refined solution.

  if (_es)
    {
      // No specified vectors == forward solve
      if (!solution_vectors)
        es.solve();
      else
        {
          libmesh_assert(solution_vectors->size() == es.n_systems());
	  libmesh_assert(solution_vectors->find(system_list[0]) !=
			 solution_vectors->end());
	  const bool solve_adjoint =
            (system_list[0]->have_vector("adjoint_solution0") &&
             (solution_vectors->find(system_list[0])->second ==
	      &system_list[0]->get_adjoint_solution()));
	  libmesh_assert(solve_adjoint ||
	    (solution_vectors->find(system_list[0])->second ==
	     system_list[0]->solution.get()) ||
	     !solution_vectors->find(system_list[0])->second);

#ifdef DEBUG
	  for (unsigned int i=0; i != system_list.size(); ++i)
	    {
	      libmesh_assert(solution_vectors->find(system_list[i]) !=
			     solution_vectors->end());
	      libmesh_assert(!solve_adjoint ||
			     solution_vectors->find(system_list[i])->second ==
			     &system_list[i]->get_adjoint_solution());
	      libmesh_assert(solve_adjoint ||
			     solution_vectors->find(system_list[i])->second ==
			     system_list[i]->solution.get() ||
			     !solution_vectors->find(system_list[i])->second);
	    }
#endif

          if (solve_adjoint)
            {
              // Set up proper initial guesses
	      for (unsigned int i=0; i != system_list.size(); ++i)
	        system_list[i]->get_adjoint_solution() = *system_list[i]->solution;
              es.adjoint_solve();
              // Put the adjoint_solution into solution for
              // comparisons
	      for (unsigned int i=0; i != system_list.size(); ++i)
                {
	          system_list[i]->get_adjoint_solution().swap(*system_list[i]->solution);
	          system_list[i]->update();
                }
            }
	  else
            es.solve();
	}
    }
  else
    {
      // No specified vectors == forward solve
      if (!solution_vectors)
        system_list[0]->solve();
      else
        {
	  libmesh_assert(solution_vectors->find(system_list[0]) !=
			 solution_vectors->end());

	  const bool solve_adjoint =
            (system_list[0]->have_vector("adjoint_solution0") &&
             (solution_vectors->find(system_list[0])->second ==
	      &system_list[0]->get_adjoint_solution()));
	  libmesh_assert(solve_adjoint ||
	    (solution_vectors->find(system_list[0])->second ==
	     system_list[0]->solution.get()) ||
	    !solution_vectors->find(system_list[0])->second);

          if (solve_adjoint)
            {
              // Set up proper initial guesses
	      for (unsigned int i=0; i != system_list.size(); ++i)
	        system_list[0]->get_adjoint_solution() = *system_list[0]->solution;
              system_list[0]->adjoint_solve();
              // Put the adjoint_solution into solution for
              // comparisons
	      system_list[0]->get_adjoint_solution().swap(*system_list[0]->solution);
	      system_list[0]->update();
            }
	  else
            system_list[0]->solve();
        }
    }

  // Get the error in the uniformly refined solution(s).

  for (unsigned int i=0; i != system_list.size(); ++i)
    {
      System &system = *system_list[i];

      unsigned int n_vars = system.n_vars();

      DofMap &dof_map = system.get_dof_map();

      const SystemNorm &system_i_norm =
        _error_norms->find(&system)->second;

      NumericVector<Number> *projected_solution = projected_solutions[i];

      // Loop over all the variables in the system
      for (unsigned int var=0; var<n_vars; var++)
        {
          // Get the error vector to fill for this system and variable
          ErrorVector *err_vec = error_per_cell;
          if (!err_vec)
            {
              libmesh_assert(errors_per_cell);
	      err_vec =
		(*errors_per_cell)[std::make_pair(&system,var)];
            }

          // The type of finite element to use for this variable
          const FEType& fe_type = dof_map.variable_type (var);

          // Finite element object for each fine element
          AutoPtr<FEBase> fe (FEBase::build (dim, fe_type));

          // Build and attach an appropriate quadrature rule
          AutoPtr<QBase> qrule = fe_type.default_quadrature_rule(dim);
          fe->attach_quadrature_rule (qrule.get());

          const std::vector<Real>&  JxW = fe->get_JxW();
          const std::vector<std::vector<Real> >& phi = fe->get_phi();
          const std::vector<std::vector<RealGradient> >& dphi =
            fe->get_dphi();
#ifdef LIBMESH_ENABLE_SECOND_DERIVATIVES
          const std::vector<std::vector<RealTensor> >& d2phi =
            fe->get_d2phi();
#endif

          // The global DOF indices for the fine element
          std::vector<unsigned int> dof_indices;

          // Iterate over all the active elements in the fine mesh
          // that live on this processor.
          MeshBase::const_element_iterator       elem_it  = mesh.active_local_elements_begin();
          const MeshBase::const_element_iterator elem_end = mesh.active_local_elements_end();

          for (; elem_it != elem_end; ++elem_it)
	    {
	      // e is necessarily an active element on the local processor
	      const Elem* elem = *elem_it;

              // Find the element id for the corresponding coarse grid element
              const Elem* coarse = elem;
              unsigned int e_id = coarse->id();
              while (e_id >= max_coarse_elem_id)
                {
                  libmesh_assert (coarse->parent());
                  coarse = coarse->parent();
                  e_id = coarse->id();
                }

              double L2normsq = 0., H1seminormsq = 0., H2seminormsq = 0.;

              // reinitialize the element-specific data
              // for the current element
              fe->reinit (elem);

              // Get the local to global degree of freedom maps
              dof_map.dof_indices (elem, dof_indices, var);

              // The number of quadrature points
              const unsigned int n_qp = qrule->n_points();

              // The number of shape functions
              const unsigned int n_sf = dof_indices.size();

              //
              // Begin the loop over the Quadrature points.
              //
              for (unsigned int qp=0; qp<n_qp; qp++)
                {
                  Number u_fine = 0., u_coarse = 0.;

                  Gradient grad_u_fine, grad_u_coarse;
#ifdef LIBMESH_ENABLE_SECOND_DERIVATIVES
                  Tensor grad2_u_fine, grad2_u_coarse;
#endif

                  // Compute solution values at the current
                  // quadrature point.  This reqiures a sum
                  // over all the shape functions evaluated
                  // at the quadrature point.
                  for (unsigned int i=0; i<n_sf; i++)
                    {
                      u_fine            += phi[i][qp]*system.current_solution (dof_indices[i]);
                      u_coarse          += phi[i][qp]*(*projected_solution) (dof_indices[i]);
                      grad_u_fine       += dphi[i][qp]*system.current_solution (dof_indices[i]);
                      grad_u_coarse     += dphi[i][qp]*(*projected_solution) (dof_indices[i]);
#ifdef LIBMESH_ENABLE_SECOND_DERIVATIVES
                      grad2_u_fine      += d2phi[i][qp]*system.current_solution (dof_indices[i]);
                      grad2_u_coarse    += d2phi[i][qp]*(*projected_solution) (dof_indices[i]);
#endif
                    }

                  // Compute the value of the error at this quadrature point
                  const Number val_error = u_fine - u_coarse;

                  // Add the squares of the error to each contribution
                  if (system_i_norm.type(var) == L2 ||
                      system_i_norm.type(var) == H1 ||
                      system_i_norm.type(var) == H2)
                    {
		      L2normsq += JxW[qp] * system_i_norm.weight_sq(var) *
                                  libmesh_norm(val_error);
                      libmesh_assert (L2normsq     >= 0.);
                    }


                  // Compute the value of the error in the gradient at this
                  // quadrature point
                  if (system_i_norm.type(var) == H1 ||
                      system_i_norm.type(var) == H2 ||
                      system_i_norm.type(var) == H1_SEMINORM)
                    {
                      Gradient grad_error = grad_u_fine - grad_u_coarse;

                      H1seminormsq += JxW[qp] * system_i_norm.weight_sq(var) *
                        grad_error.size_sq();
                      libmesh_assert (H1seminormsq >= 0.);
                    }

#ifdef LIBMESH_ENABLE_SECOND_DERIVATIVES
                  // Compute the value of the error in the hessian at this
                  // quadrature point
                  if (system_i_norm.type(var) == H2 ||
                      system_i_norm.type(var) == H2_SEMINORM)
                    {
                      Tensor grad2_error = grad2_u_fine - grad2_u_coarse;

		      H2seminormsq += JxW[qp] * system_i_norm.weight_sq(var) *
                        grad2_error.size_sq();
                      libmesh_assert (H2seminormsq >= 0.);
                    }
#endif
                } // end qp loop

              if (system_i_norm.type(var) == L2 ||
                  system_i_norm.type(var) == H1 ||
                  system_i_norm.type(var) == H2)
                (*err_vec)[e_id] += L2normsq;
              if (system_i_norm.type(var) == H1 ||
                  system_i_norm.type(var) == H2 ||
                  system_i_norm.type(var) == H1_SEMINORM)
                (*err_vec)[e_id] += H1seminormsq;

              if (system_i_norm.type(var) == H2 ||
                  system_i_norm.type(var) == H2_SEMINORM)
                (*err_vec)[e_id] += H2seminormsq;
            } // End loop over active local elements
        } // End loop over variables

      // Don't bother projecting the solution; we'll restore from backup
      // after coarsening
      system.project_solution_on_reinit() = false;
    }


  // Uniformly coarsen the mesh, without projecting the solution
  libmesh_assert (number_h_refinements > 0 || number_p_refinements > 0);

  for (unsigned int i = 0; i != number_h_refinements; ++i)
    {
      mesh_refinement.uniformly_coarsen(1);
      // FIXME - should the reinits here be necessary? - RHS
      es.reinit();
    }

  for (unsigned int i = 0; i != number_p_refinements; ++i)
    {
      mesh_refinement.uniformly_p_coarsen(1);
      es.reinit();
    }

  // We should be back where we started
  libmesh_assert(n_coarse_elem == mesh.n_elem());

  // Each processor has now computed the error contribuions
  // for its local elements.  We need to sum the vector
  // and then take the square-root of each component.  Note
  // that we only need to sum if we are running on multiple
  // processors, and we only need to take the square-root
  // if the value is nonzero.  There will in general be many
  // zeros for the inactive elements.

  if (error_per_cell)
    {
      // First sum the vector of estimated error values
      this->reduce_error(*error_per_cell);

      // Compute the square-root of each component.
      START_LOG("std::sqrt()", "AdjointRefinementEstimator");
      for (unsigned int i=0; i<error_per_cell->size(); i++)
        if ((*error_per_cell)[i] != 0.)
          (*error_per_cell)[i] = std::sqrt((*error_per_cell)[i]);
      STOP_LOG("std::sqrt()", "AdjointRefinementEstimator");
    }
  else
    {
      for (ErrorMap::iterator i = errors_per_cell->begin();
           i != errors_per_cell->end(); ++i)
        {
          ErrorVector *e = i->second;
          // First sum the vector of estimated error values
          this->reduce_error(*e);

          // Compute the square-root of each component.
          START_LOG("std::sqrt()", "AdjointRefinementEstimator");
          for (unsigned int i=0; i<e->size(); i++)
            if ((*e)[i] != 0.)
              (*e)[i] = std::sqrt((*e)[i]);
          STOP_LOG("std::sqrt()", "AdjointRefinementEstimator");
        }
    }

  // Restore old solutions and clean up the heap
  for (unsigned int i=0; i != system_list.size(); ++i)
    {
      System &system = *system_list[i];

      system.project_solution_on_reinit() = old_projection_settings[i];

      // Restore the coarse solution vectors and delete their copies
      *system.solution = *coarse_solutions[i];
      delete coarse_solutions[i];
      *system.current_local_solution = *coarse_local_solutions[i];
      delete coarse_local_solutions[i];
      delete projected_solutions[i];

      for (System::vectors_iterator vec = system.vectors_begin(); vec !=
           system.vectors_end(); ++vec)
        {
          // The (string) name of this vector
          const std::string& var_name = vec->first;

          system.get_vector(var_name) = *coarse_vectors[i][var_name];

          coarse_vectors[i][var_name]->clear();
          delete coarse_vectors[i][var_name];
        }
    }

  // Restore old partitioner settings
  mesh.partitioner() = old_partitioner;
}

} // namespace libMesh

#endif // #ifdef LIBMESH_ENABLE_AMR

#endif
