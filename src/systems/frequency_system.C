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



// Local includes
#include "libmesh_config.h"

/*
 * Require complex arithmetic
 */
#if defined(LIBMESH_USE_COMPLEX_NUMBERS)


// C++ includes
#include <cstdio>          // for sprintf

// Local includes
#include "frequency_system.h"
#include "equation_systems.h"
#include "libmesh_logging.h"
#include "linear_solver.h"
#include "numeric_vector.h"

namespace libMesh
{



// ------------------------------------------------------------
// FrequencySystem implementation
FrequencySystem::FrequencySystem (EquationSystems& es,
				  const std::string& name,
				  const unsigned int number) :
  LinearImplicitSystem      (es, name, number),
  solve_system              (NULL),
  _finished_set_frequencies (false),
  _keep_solution_duplicates (true),
  _finished_init            (false),
  _finished_assemble        (false)
{
  // default value for wave speed & fluid density
  //_equation_systems.parameters.set<Real>("wave speed") = 340.;
  //_equation_systems.parameters.set<Real>("rho")        = 1.225;
}



FrequencySystem::~FrequencySystem ()
{
  this->clear ();

  // the additional matrices and vectors are cleared and zero'ed in System
}




void FrequencySystem::clear ()
{
  LinearImplicitSystem::clear();

  _finished_set_frequencies = false;
  _keep_solution_duplicates = true;
  _finished_init            = false;
  _finished_assemble        = false;

  /*
   * We have to distinguish between the
   * simple straightforward "clear()"
   * and the clear that also touches the
   * EquationSystems parameters "current frequency" etc.
   * Namely, when reading from file (through equation_systems_io.C
   * methods), the param's are read in, then the systems.
   * Prior to reading a system, this system gets cleared...
   * And there, all the previously loaded frequency parameters
   * would get lost...
   */
}



void FrequencySystem::clear_all ()
{
  this->clear ();

  EquationSystems& es =
    this->get_equation_systems();

  // clear frequencies in the parameters section of the
  // EquationSystems object
  if (es.parameters.have_parameter<unsigned int> ("n_frequencies"))
    {
      unsigned int n_frequencies = es.parameters.get<unsigned int>("n_frequencies");
      for (unsigned int n=0; n < n_frequencies; n++)
	es.parameters.remove(this->form_freq_param_name(n));
      es.parameters.remove("current frequency");
    }
}




void FrequencySystem::init_data ()
{
  // initialize parent data and additional solution vectors
  LinearImplicitSystem::init_data();

  // Log how long initializing the system takes
  START_LOG("init()", "FrequencySystem");

  EquationSystems& es =
    this->get_equation_systems();

  // make sure we have frequencies to solve for
  if (!_finished_set_frequencies)
    {
      /*
       * when this system was read from file, check
       * if this has a "n_frequencies" parameter,
       * and initialize us with these.
       */
      if (es.parameters.have_parameter<unsigned int> ("n_frequencies"))
        {
	  const unsigned int n_freq =
	    es.parameters.get<unsigned int>("n_frequencies");

	  libmesh_assert(n_freq > 0);

	  _finished_set_frequencies = true;

	  this->set_current_frequency(0);
        }
      else
        {
	  libMesh::err << "ERROR: Need to set frequencies before calling init(). " << std::endl;
	  libmesh_error();
	}
    }

  _finished_init = true;

  // Stop logging init()
  STOP_LOG("init()", "FrequencySystem");
}



void FrequencySystem::assemble ()
{
  libmesh_assert (_finished_init);

  if (_finished_assemble)
    {
      libMesh::err << "ERROR: Matrices already assembled." << std::endl;
      libmesh_error ();
    }

  // Log how long assemble() takes
  START_LOG("assemble()", "FrequencySystem");

  // prepare matrix with the help of the _dof_map,
  // fill with sparsity pattern, initialize the
  // additional matrices
  LinearImplicitSystem::assemble();

  //matrix.print ();
  //rhs.print    ();

  _finished_assemble = true;

  // Log how long assemble() takes
  STOP_LOG("assemble()", "FrequencySystem");
}



void FrequencySystem::set_frequencies_by_steps (const Real base_freq,
						const Real freq_step,
						const unsigned int n_freq,
						const bool allocate_solution_duplicates)
{
  this->_keep_solution_duplicates = allocate_solution_duplicates;

  // sanity check
  if (_finished_set_frequencies)
    {
      libMesh::err << "ERROR: frequencies already initialized."
		    << std::endl;
      libmesh_error();
    }

  EquationSystems& es =
    this->get_equation_systems();

  // store number of frequencies as parameter
  es.parameters.set<unsigned int>("n_frequencies") = n_freq;

  for (unsigned int n=0; n<n_freq; n++)
    {
      // remember frequencies as parameters
      es.parameters.set<Real>(this->form_freq_param_name(n)) =
	  base_freq + n * freq_step;

      // build storage for solution vector, if wanted
      if (this->_keep_solution_duplicates)
	  this->add_vector(this->form_solu_vec_name(n));
    }

  _finished_set_frequencies = true;

  // set the current frequency
  this->set_current_frequency(0);
}



void FrequencySystem::set_frequencies_by_range (const Real min_freq,
						const Real max_freq,
						const unsigned int n_freq,
						const bool allocate_solution_duplicates)
{
  this->_keep_solution_duplicates = allocate_solution_duplicates;

  // sanity checks
  libmesh_assert(max_freq > min_freq);
  libmesh_assert(n_freq > 0);

  if (_finished_set_frequencies)
    {
      libMesh::err << "ERROR: frequencies already initialized. " << std::endl;
      libmesh_error();
    }

  EquationSystems& es =
    this->get_equation_systems();

  // store number of frequencies as parameter
  es.parameters.set<unsigned int>("n_frequencies") = n_freq;

  // set frequencies, build solution storage
  for (unsigned int n=0; n<n_freq; n++)
    {
      // remember frequencies as parameters
      es.parameters.set<Real>(this->form_freq_param_name(n)) =
	  min_freq + n*(max_freq-min_freq)/(n_freq-1);

      // build storage for solution vector, if wanted
      if (this->_keep_solution_duplicates)
	  System::add_vector(this->form_solu_vec_name(n));
    }

  _finished_set_frequencies = true;

  // set the current frequency
  this->set_current_frequency(0);
}



void FrequencySystem::set_frequencies (const std::vector<Real>& frequencies,
				       const bool allocate_solution_duplicates)
{
  this->_keep_solution_duplicates = allocate_solution_duplicates;

  // sanity checks
  libmesh_assert(!frequencies.empty());

  if (_finished_set_frequencies)
    {
      libMesh::err << "ERROR: frequencies already initialized. " << std::endl;
      libmesh_error();
    }

  EquationSystems& es =
    this->get_equation_systems();

  // store number of frequencies as parameter
  es.parameters.set<unsigned int>("n_frequencies") = frequencies.size();

  // set frequencies, build solution storage
  for (unsigned int n=0; n<frequencies.size(); n++)
    {
      // remember frequencies as parameters
      es.parameters.set<Real>(this->form_freq_param_name(n)) = frequencies[n];

      // build storage for solution vector, if wanted
      if (this->_keep_solution_duplicates)
	  System::add_vector(this->form_solu_vec_name(n));
    }

  _finished_set_frequencies = true;

  // set the current frequency
  this->set_current_frequency(0);
}




unsigned int FrequencySystem::n_frequencies () const
{
  libmesh_assert(_finished_set_frequencies);
  return this->get_equation_systems().parameters.get<unsigned int>("n_frequencies");
}



void FrequencySystem::solve ()
{
  libmesh_assert (this->n_frequencies() > 0);

  // Solve for all the specified frequencies
  this->solve (0, this->n_frequencies()-1);
}



void FrequencySystem::solve (const unsigned int n_start,
			     const unsigned int n_stop)
{
  // Assemble the linear system, if not already done
  if (!_finished_assemble)
    this->assemble ();

  // the user-supplied solve method _has_ to be provided by the user
  libmesh_assert (solve_system != NULL);

  // existence & range checks
  libmesh_assert(this->n_frequencies() > 0);
  libmesh_assert(n_stop < this->n_frequencies());

  EquationSystems& es =
    this->get_equation_systems();

  // Get the user-specified linear solver tolerance,
  //     the user-specified maximum # of linear solver iterations,
  //     the user-specified wave speed
  const Real tol            =
    es.parameters.get<Real>("linear solver tolerance");
  const unsigned int maxits =
    es.parameters.get<unsigned int>("linear solver maximum iterations");


//   // return values
//   std::vector< std::pair<unsigned int, Real> > vec_rval;

  // start solver loop
  for (unsigned int n=n_start; n<= n_stop; n++)
    {
      // set the current frequency
      this->set_current_frequency(n);

      // Call the user-supplied pre-solve method
      START_LOG("user_pre_solve()", "FrequencySystem");

      this->solve_system (es, this->name());

      STOP_LOG("user_pre_solve()", "FrequencySystem");


      // Solve the linear system for this specific frequency
      const std::pair<unsigned int, Real> rval =
	linear_solver->solve (*matrix, *solution, *rhs, tol, maxits);

      _n_linear_iterations   = rval.first;
      _final_linear_residual = rval.second;

       vec_rval.push_back(rval);

      /**
       * store the current solution in the additional vector
       */
      if (this->_keep_solution_duplicates)
	this->get_vector(this->form_solu_vec_name(n)) = *solution;
    }

  // sanity check
  //libmesh_assert (vec_rval.size() == (n_stop-n_start+1));

  //return vec_rval;
}



void FrequencySystem::attach_solve_function(void fptr(EquationSystems& es,
						      const std::string& name))
{
  libmesh_assert (fptr != NULL);

  solve_system = fptr;
}



void FrequencySystem::set_current_frequency(unsigned int n)
{
  libmesh_assert(n < n_frequencies());

  EquationSystems& es =
    this->get_equation_systems();

  es.parameters.set<Real>("current frequency") =
    es.parameters.get<Real>(this->form_freq_param_name(n));
}



std::string FrequencySystem::form_freq_param_name(const unsigned int n) const
{
  libmesh_assert (n < 9999);
  char buf[15];
  sprintf(buf, "frequency %04d", n);
  return (buf);
}



std::string FrequencySystem::form_solu_vec_name(const unsigned int n) const
{
  libmesh_assert (n < 9999);
  char buf[15];
  sprintf(buf, "solution %04d", n);
  return (buf);
}

} // namespace libMesh


#endif // if defined(LIBMESH_USE_COMPLEX_NUMBERS)
