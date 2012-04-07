<?php $root=""; ?>
<?php require($root."navigation.php"); ?>
<html>
<head>
  <?php load_style($root); ?>
</head>
 
<body>
 
<?php make_navigation("adjoints_ex1",$root)?>
 
<div class="content">
<a name="comments"></a> 
<div class = "comment">
<h1>Adjoints Example 1 - Laplace Equation in the L-Shaped Domain with Adjoint based mesh refinement</h1>

<br><br>This example solves the Laplace equation on the classic "L-shaped"
domain with adaptive mesh refinement. The exact
solution is u(r,\theta) = r^{2/3} * \sin ( (2/3) * \theta). The kelly and 
adjoint residual error estimators are used to develop error indicators and 
guide mesh adaptation. Since we use the adjoint capabilities of libMesh in 
this example, we use the DiffSystem framework. This file (main.C) contains 
the declaration of mesh and equation system objects, L-shaped.C contains the 
assembly of the system, element_qoi_derivative.C and side_qoi_derivative.C
contain the RHS for the adjoint systems. Postprocessing to compute the 
the QoIs is done in element_postprocess.C and side_postprocess.C


<br><br>The initial mesh contains three QUAD9 elements which represent the
standard quadrants I, II, and III of the domain [-1,1]x[-1,1],
i.e.
Element 0: [-1,0]x[ 0,1]
Element 1: [ 0,1]x[ 0,1]
Element 2: [-1,0]x[-1,0]
The mesh is provided in the standard libMesh ASCII format file
named "lshaped.xda".  In addition, an input file named "ex26.in"
is provided which allows the user to set several parameters for
the solution so that the problem can be re-run without a
re-compile.  The solution technique employed is to have a
refinement loop with a linear (forward and adjoint) solve inside followed by a
refinement of the grid and projection of the solution to the new grid
In the final loop iteration, there is no additional
refinement after the solve.  In the input file "general.in", the variable
"max_adaptivesteps" controls the number of refinement steps, and
"refine_fraction" / "coarsen_fraction" determine the number of
elements which will be refined / coarsened at each step.


<br><br>C++ includes
</div>

<div class ="fragment">
<pre>
        #include &lt;iostream&gt;
        
</pre>
</div>
<div class = "comment">
General libMesh includes
</div>

<div class ="fragment">
<pre>
        #include "equation_systems.h"
        #include "linear_solver.h"
        #include "error_vector.h"
        #include "mesh.h"
        #include "mesh_refinement.h"
        #include "newton_solver.h"
        #include "numeric_vector.h"
        #include "steady_solver.h"
        #include "system_norm.h"
        
</pre>
</div>
<div class = "comment">
Error Estimator includes
</div>

<div class ="fragment">
<pre>
        #include "kelly_error_estimator.h"
        #include "patch_recovery_error_estimator.h"
        
</pre>
</div>
<div class = "comment">
Adjoint Related includes
</div>

<div class ="fragment">
<pre>
        #include "adjoint_residual_error_estimator.h"
        #include "qoi_set.h"
        
</pre>
</div>
<div class = "comment">
libMesh I/O includes
</div>

<div class ="fragment">
<pre>
        #include "getpot.h"
        #include "gmv_io.h"
        
</pre>
</div>
<div class = "comment">
Some (older) compilers do not offer full stream
functionality, OStringStream works around this.
</div>

<div class ="fragment">
<pre>
        #include "o_string_stream.h"
        
</pre>
</div>
<div class = "comment">
Local includes
</div>

<div class ="fragment">
<pre>
        #include "femparameters.h"
        #include "L-shaped.h"
        
</pre>
</div>
<div class = "comment">
Bring in everything from the libMesh namespace
</div>

<div class ="fragment">
<pre>
        using namespace libMesh;
        
</pre>
</div>
<div class = "comment">
Local function declarations


<br><br>Number output files, the files are give a prefix of primal or adjoint_i depending on
whether the output is the primal solution or the dual solution for the ith QoI


<br><br>Write gmv output


<br><br></div>

<div class ="fragment">
<pre>
        void write_output(EquationSystems &es,                                   
        		  unsigned int a_step,       // The adaptive step count
        		  std::string solution_type) // primal or adjoint solve
        {
          MeshBase &mesh = es.get_mesh();
        
        #ifdef LIBMESH_HAVE_GMV
          OStringStream file_name_gmv;
          file_name_gmv &lt;&lt; solution_type &lt;&lt; ".out.gmv.";      
          OSSRealzeroright(file_name_gmv,2,0,a_step);
          
          GMVIO(mesh).write_equation_systems
            (file_name_gmv.str(), es);
        #endif    
        }
        
</pre>
</div>
<div class = "comment">
Set the parameters for the nonlinear and linear solvers to be used during the simulation


<br><br></div>

<div class ="fragment">
<pre>
        void set_system_parameters(LaplaceSystem &system, FEMParameters &param)
        {
</pre>
</div>
<div class = "comment">
Use analytical jacobians?
</div>

<div class ="fragment">
<pre>
          system.analytic_jacobians() = param.analytic_jacobians;
        
</pre>
</div>
<div class = "comment">
Verify analytic jacobians against numerical ones?
</div>

<div class ="fragment">
<pre>
          system.verify_analytic_jacobians = param.verify_analytic_jacobians;
        
</pre>
</div>
<div class = "comment">
Use the prescribed FE type
</div>

<div class ="fragment">
<pre>
          system.fe_family() = param.fe_family[0];
          system.fe_order() = param.fe_order[0];
        
</pre>
</div>
<div class = "comment">
More desperate debugging options
</div>

<div class ="fragment">
<pre>
          system.print_solution_norms = param.print_solution_norms;
          system.print_solutions      = param.print_solutions;
          system.print_residual_norms = param.print_residual_norms;
          system.print_residuals      = param.print_residuals;
          system.print_jacobian_norms = param.print_jacobian_norms;
          system.print_jacobians      = param.print_jacobians;
        
</pre>
</div>
<div class = "comment">
No transient time solver
</div>

<div class ="fragment">
<pre>
          system.time_solver =
              AutoPtr&lt;TimeSolver&gt;(new SteadySolver(system));
        
</pre>
</div>
<div class = "comment">
Nonlinear solver options
</div>

<div class ="fragment">
<pre>
          {
            NewtonSolver *solver = new NewtonSolver(system);
            system.time_solver-&gt;diff_solver() = AutoPtr&lt;DiffSolver&gt;(solver);
            
            solver-&gt;quiet                       = param.solver_quiet;
            solver-&gt;max_nonlinear_iterations    = param.max_nonlinear_iterations;
            solver-&gt;minsteplength               = param.min_step_length;
            solver-&gt;relative_step_tolerance     = param.relative_step_tolerance;
            solver-&gt;relative_residual_tolerance = param.relative_residual_tolerance;
            solver-&gt;require_residual_reduction  = param.require_residual_reduction;
            solver-&gt;linear_tolerance_multiplier = param.linear_tolerance_multiplier;
            if (system.time_solver-&gt;reduce_deltat_on_diffsolver_failure)
              {
        	solver-&gt;continue_after_max_iterations = true;
        	solver-&gt;continue_after_backtrack_failure = true;
              }
            
</pre>
</div>
<div class = "comment">
And the linear solver options
</div>

<div class ="fragment">
<pre>
            solver-&gt;max_linear_iterations       = param.max_linear_iterations;
            solver-&gt;initial_linear_tolerance    = param.initial_linear_tolerance;
            solver-&gt;minimum_linear_tolerance    = param.minimum_linear_tolerance;      
          }
        }
        
</pre>
</div>
<div class = "comment">
Build the mesh refinement object and set parameters for refining/coarsening etc


<br><br></div>

<div class ="fragment">
<pre>
        #ifdef LIBMESH_ENABLE_AMR
        
        AutoPtr&lt;MeshRefinement&gt; build_mesh_refinement(MeshBase &mesh,
                                                      FEMParameters &param)
        {
          AutoPtr&lt;MeshRefinement&gt; mesh_refinement(new MeshRefinement(mesh));
          mesh_refinement-&gt;coarsen_by_parents() = true;
          mesh_refinement-&gt;absolute_global_tolerance() = param.global_tolerance;
          mesh_refinement-&gt;nelem_target()      = param.nelem_target;  
          mesh_refinement-&gt;refine_fraction()   = param.refine_fraction;
          mesh_refinement-&gt;coarsen_fraction()  = param.coarsen_fraction;  
          mesh_refinement-&gt;coarsen_threshold() = param.coarsen_threshold;
        
          return mesh_refinement;
        }
        
        #endif // LIBMESH_ENABLE_AMR
        
</pre>
</div>
<div class = "comment">
This is where we declare the error estimators to be built and used for
mesh refinement. The adjoint residual estimator needs two estimators.
One for the forward component of the estimate and one for the adjoint
weighting factor. Here we use the Patch Recovery indicator to estimate both the
forward and adjoint weights. The H1 seminorm component of the error is used
as dictated by the weak form the Laplace equation.


<br><br></div>

<div class ="fragment">
<pre>
        AutoPtr&lt;ErrorEstimator&gt; build_error_estimator(FEMParameters &param, QoISet &qois)
        {
          AutoPtr&lt;ErrorEstimator&gt; error_estimator;
        
          if (param.indicator_type == "kelly")
            {
              std::cout&lt;&lt;"Using Kelly Error Estimator"&lt;&lt;std::endl;
        
              error_estimator.reset(new KellyErrorEstimator);
            }
          else if (param.indicator_type == "adjoint_residual")
            {
              std::cout&lt;&lt;"Using Adjoint Residual Error Estimator with Patch Recovery Weights"&lt;&lt;std::endl;
        
              AdjointResidualErrorEstimator *adjoint_residual_estimator = new AdjointResidualErrorEstimator;
              
              error_estimator.reset (adjoint_residual_estimator);
              
              adjoint_residual_estimator-&gt;qoi_set() = qois;
              
              adjoint_residual_estimator-&gt;adjoint_already_solved = true;
        
              adjoint_residual_estimator-&gt;error_plot_suffix = "error.gmv";
              
              PatchRecoveryErrorEstimator *p1 =
        	new PatchRecoveryErrorEstimator;
              adjoint_residual_estimator-&gt;primal_error_estimator().reset(p1);
        	   	   	   
              PatchRecoveryErrorEstimator *p2 =
        	new PatchRecoveryErrorEstimator;
              adjoint_residual_estimator-&gt;dual_error_estimator().reset(p2);   
        
              adjoint_residual_estimator-&gt;primal_error_estimator()-&gt;error_norm.set_type(0, H1_SEMINORM);
              p1-&gt;set_patch_reuse(param.patch_reuse);   
        
              adjoint_residual_estimator-&gt;dual_error_estimator()-&gt;error_norm.set_type(0, H1_SEMINORM);	 
              p2-&gt;set_patch_reuse(param.patch_reuse);
            }
          else
            {
              std::cerr &lt;&lt; "Unknown indicator_type" &lt;&lt; std::endl;
              libmesh_error();
            }
          return error_estimator;
        }
        
</pre>
</div>
<div class = "comment">
The main program.
</div>

<div class ="fragment">
<pre>
        int main (int argc, char** argv)
        {
</pre>
</div>
<div class = "comment">
Initialize libMesh.
</div>

<div class ="fragment">
<pre>
          LibMeshInit init (argc, argv);
        
</pre>
</div>
<div class = "comment">
Skip adaptive examples on a non-adaptive libMesh build
</div>

<div class ="fragment">
<pre>
        #ifndef LIBMESH_ENABLE_AMR
          libmesh_example_assert(false, "--enable-amr");
        #else
</pre>
</div>
<div class = "comment">
Only our PETSc interface currently supports adjoint solves
</div>

<div class ="fragment">
<pre>
          libmesh_example_assert(libMesh::default_solver_package() == PETSC_SOLVERS, "--enable-petsc");
        
          std::cout &lt;&lt; "Started " &lt;&lt; argv[0] &lt;&lt; std::endl;
        
</pre>
</div>
<div class = "comment">
Make sure the general input file exists, and parse it
</div>

<div class ="fragment">
<pre>
          {
            std::ifstream i("general.in");
            if (!i)
              {
                std::cerr &lt;&lt; '[' &lt;&lt; libMesh::processor_id() 
                          &lt;&lt; "] Can't find general.in; exiting early." 
                          &lt;&lt; std::endl; 
                libmesh_error();
              }
          }
          GetPot infile("general.in");
        
</pre>
</div>
<div class = "comment">
Read in parameters from the input file
</div>

<div class ="fragment">
<pre>
          FEMParameters param;
          param.read(infile);
        
</pre>
</div>
<div class = "comment">
Skip this default-2D example if libMesh was compiled as 1D-only.
</div>

<div class ="fragment">
<pre>
          libmesh_example_assert(2 &lt;= LIBMESH_DIM, "2D support");
          
</pre>
</div>
<div class = "comment">
Create a mesh.
</div>

<div class ="fragment">
<pre>
          Mesh mesh;
        
</pre>
</div>
<div class = "comment">
And an object to refine it
</div>

<div class ="fragment">
<pre>
          AutoPtr&lt;MeshRefinement&gt; mesh_refinement =
            build_mesh_refinement(mesh, param);
        
</pre>
</div>
<div class = "comment">
And an EquationSystems to run on it
</div>

<div class ="fragment">
<pre>
          EquationSystems equation_systems (mesh);
        
          std::cout &lt;&lt; "Reading in and building the mesh" &lt;&lt; std::endl;
        
</pre>
</div>
<div class = "comment">
Read in the mesh
</div>

<div class ="fragment">
<pre>
          mesh.read(param.domainfile.c_str());
</pre>
</div>
<div class = "comment">
Make all the elements of the mesh second order so we can compute
with a higher order basis
</div>

<div class ="fragment">
<pre>
          mesh.all_second_order();
        
</pre>
</div>
<div class = "comment">
Create a mesh refinement object to do the initial uniform refinements
on the coarse grid read in from lshaped.xda
</div>

<div class ="fragment">
<pre>
          MeshRefinement initial_uniform_refinements(mesh);
          initial_uniform_refinements.uniformly_refine(param.coarserefinements);
            
          std::cout &lt;&lt; "Building system" &lt;&lt; std::endl;
        
</pre>
</div>
<div class = "comment">
Build the FEMSystem
</div>

<div class ="fragment">
<pre>
          LaplaceSystem &system = equation_systems.add_system&lt;LaplaceSystem&gt; ("LaplaceSystem");
        
</pre>
</div>
<div class = "comment">
Set its parameters
</div>

<div class ="fragment">
<pre>
          set_system_parameters(system, param);
          
          std::cout &lt;&lt; "Initializing systems" &lt;&lt; std::endl;
        
          equation_systems.init ();
        
</pre>
</div>
<div class = "comment">
Print information about the mesh and system to the screen.
</div>

<div class ="fragment">
<pre>
          mesh.print_info();
          equation_systems.print_info();
          LinearSolver&lt;Number&gt; *linear_solver = system.get_linear_solver();
        
          {	
</pre>
</div>
<div class = "comment">
Adaptively solve the timestep
</div>

<div class ="fragment">
<pre>
            unsigned int a_step = 0;
            for (; a_step != param.max_adaptivesteps; ++a_step)
              {	    	    
</pre>
</div>
<div class = "comment">
We can't adapt to both a tolerance and a
target mesh size
</div>

<div class ="fragment">
<pre>
                if (param.global_tolerance != 0.)
        	  libmesh_assert (param.nelem_target == 0);
</pre>
</div>
<div class = "comment">
If we aren't adapting to a tolerance we need a
target mesh size
</div>

<div class ="fragment">
<pre>
                    else
                      libmesh_assert (param.nelem_target &gt; 0);
            	
        	linear_solver-&gt;reuse_preconditioner(false);
        
</pre>
</div>
<div class = "comment">
Solve the forward problem
</div>

<div class ="fragment">
<pre>
                system.solve();
        
</pre>
</div>
<div class = "comment">
Write out the computed primal solution	
</div>

<div class ="fragment">
<pre>
                write_output(equation_systems, a_step, "primal");
        	
</pre>
</div>
<div class = "comment">
Get a pointer to the primal solution vector
</div>

<div class ="fragment">
<pre>
                NumericVector&lt;Number&gt; &primal_solution = *system.solution;
        
</pre>
</div>
<div class = "comment">
Declare a QoISet object, we need this object to set weights for our QoI error contributions
</div>

<div class ="fragment">
<pre>
                QoISet qois;
        
</pre>
</div>
<div class = "comment">
Declare a qoi_indices vector, each index will correspond to a QoI
</div>

<div class ="fragment">
<pre>
                std::vector&lt;unsigned int&gt; qoi_indices;
        	qoi_indices.push_back(0);
        	qoi_indices.push_back(1);
        	qois.add_indices(qoi_indices);
        
</pre>
</div>
<div class = "comment">
Set weights for each index, these will weight the contribution of each QoI in the final error
estimate to be used for flagging elements for refinement
</div>

<div class ="fragment">
<pre>
                qois.set_weight(0, 0.5);        
        	qois.set_weight(1, 0.5);	
        
</pre>
</div>
<div class = "comment">
Make sure we get the contributions to the adjoint RHS from the sides
</div>

<div class ="fragment">
<pre>
                system.assemble_qoi_sides = true;
        
</pre>
</div>
<div class = "comment">
We are about to solve the adjoint system, but before we do this we see the same preconditioner
flag to reuse the preconditioner from the forward solver		
</div>

<div class ="fragment">
<pre>
                linear_solver-&gt;reuse_preconditioner(param.reuse_preconditioner);
        	
</pre>
</div>
<div class = "comment">
Solve the adjoint system. This takes the transpose of the stiffness matrix and then
solves the resulting system
</div>

<div class ="fragment">
<pre>
                system.adjoint_solve();
        
</pre>
</div>
<div class = "comment">
Get a pointer to the solution vector of the adjoint problem for QoI 0
</div>

<div class ="fragment">
<pre>
                NumericVector&lt;Number&gt; &dual_solution_0 = system.get_adjoint_solution(0);
        
</pre>
</div>
<div class = "comment">
Swap the primal and dual solutions so we can write out the adjoint solution
</div>

<div class ="fragment">
<pre>
                primal_solution.swap(dual_solution_0);            
        	write_output(equation_systems, a_step, "adjoint_0");
        
</pre>
</div>
<div class = "comment">
Swap back
</div>

<div class ="fragment">
<pre>
                primal_solution.swap(dual_solution_0);
        	
</pre>
</div>
<div class = "comment">
Get a pointer to the solution vector of the adjoint problem for QoI 0
</div>

<div class ="fragment">
<pre>
                NumericVector&lt;Number&gt; &dual_solution_1 = system.get_adjoint_solution(1);
        	
</pre>
</div>
<div class = "comment">
Swap again
</div>

<div class ="fragment">
<pre>
                primal_solution.swap(dual_solution_1);            
        	write_output(equation_systems, a_step, "adjoint_1");
        
</pre>
</div>
<div class = "comment">
Swap back again
</div>

<div class ="fragment">
<pre>
                primal_solution.swap(dual_solution_1);
        			    
        	std::cout &lt;&lt; "Adaptive step " &lt;&lt; a_step &lt;&lt; ", we have " &lt;&lt; mesh.n_active_elem()
                              &lt;&lt; " active elements and "
                              &lt;&lt; equation_systems.n_active_dofs()
        		  &lt;&lt; " active dofs." &lt;&lt; std::endl ;
        
</pre>
</div>
<div class = "comment">
Postprocess, compute the approximate QoIs and write them out to the console
</div>

<div class ="fragment">
<pre>
                std::cout &lt;&lt; "Postprocessing: " &lt;&lt; std::endl; 
        	system.postprocess_sides = true;
        	system.postprocess();
        	Number QoI_0_computed = system.get_QoI_value("computed", 0);
        	Number QoI_0_exact = system.get_QoI_value("exact", 0);
        	Number QoI_1_computed = system.get_QoI_value("computed", 1);
        	Number QoI_1_exact = system.get_QoI_value("exact", 1);
        		
        	std::cout&lt;&lt; "The relative error in QoI 0 is " &lt;&lt; std::setprecision(17)
                         &lt;&lt; std::abs(QoI_0_computed - QoI_0_exact) /
                            std::abs(QoI_0_exact) &lt;&lt; std::endl;
        		
        	std::cout&lt;&lt; "The relative error in QoI 1 is " &lt;&lt; std::setprecision(17) 
                         &lt;&lt; std::abs(QoI_1_computed - QoI_1_exact) /
                            std::abs(QoI_1_exact) &lt;&lt; std::endl &lt;&lt; std::endl;
        	
</pre>
</div>
<div class = "comment">
Now we construct the data structures for the mesh refinement process	
</div>

<div class ="fragment">
<pre>
                ErrorVector error;
        	
</pre>
</div>
<div class = "comment">
Build an error estimator object
</div>

<div class ="fragment">
<pre>
                AutoPtr&lt;ErrorEstimator&gt; error_estimator =
        	  build_error_estimator(param, qois);
        	
</pre>
</div>
<div class = "comment">
Estimate the error in each element using the Adjoint Residual or Kelly error estimator
</div>

<div class ="fragment">
<pre>
                error_estimator-&gt;estimate_error(system, error);
        			    	    
</pre>
</div>
<div class = "comment">
We have to refine either based on reaching an error tolerance or 
a number of elements target, which should be verified above
Otherwise we flag elements by error tolerance or nelem target	  


<br><br>Uniform refinement
</div>

<div class ="fragment">
<pre>
                if(param.refine_uniformly)
        	  {	  			    
        	    mesh_refinement-&gt;uniformly_refine(1);
        	  }
</pre>
</div>
<div class = "comment">
Adaptively refine based on reaching an error tolerance
</div>

<div class ="fragment">
<pre>
                else if(param.global_tolerance &gt;= 0. && param.nelem_target == 0.)
        	  {	      	    
        	    mesh_refinement-&gt;flag_elements_by_error_tolerance (error);              
        	    
        	    mesh_refinement-&gt;refine_and_coarsen_elements();
        	  }
</pre>
</div>
<div class = "comment">
Adaptively refine based on reaching a target number of elements
</div>

<div class ="fragment">
<pre>
                else 
        	  {	      	    	    
        	    if (mesh.n_active_elem() &gt;= param.nelem_target)
        	      {
        		std::cout&lt;&lt;"We reached the target number of elements."&lt;&lt;std::endl &lt;&lt;std::endl;
        		break;
        	      }				
        	    
        	    mesh_refinement-&gt;flag_elements_by_nelem_target (error);
        	    	    
        	    mesh_refinement-&gt;refine_and_coarsen_elements();
        	  }
                    
</pre>
</div>
<div class = "comment">
Dont forget to reinit the system after each adaptive refinement !
</div>

<div class ="fragment">
<pre>
                equation_systems.reinit();
        
                std::cout &lt;&lt; "Refined mesh to "
                          &lt;&lt; mesh.n_active_elem()
                          &lt;&lt; " active elements and "
                          &lt;&lt; equation_systems.n_active_dofs()
                          &lt;&lt; " active dofs." &lt;&lt; std::endl;
              }
        
</pre>
</div>
<div class = "comment">
Do one last solve if necessary
</div>

<div class ="fragment">
<pre>
            if (a_step == param.max_adaptivesteps)
              {	 
        	linear_solver-&gt;reuse_preconditioner(false);	
        	system.solve();
        	
        	write_output(equation_systems, a_step, "primal");
        
        	NumericVector&lt;Number&gt; &primal_solution = *system.solution;
        	
        	QoISet qois;
        	std::vector&lt;unsigned int&gt; qoi_indices;
        
        	qoi_indices.push_back(0);
        	qoi_indices.push_back(1);
        	qois.add_indices(qoi_indices);
        	
        	qois.set_weight(0, 0.5);	
        	qois.set_weight(1, 0.5);	
        
        	system.assemble_qoi_sides = true;	
        	linear_solver-&gt;reuse_preconditioner(param.reuse_preconditioner);
        	system.adjoint_solve();
        		
        	NumericVector&lt;Number&gt; &dual_solution_0 = system.get_adjoint_solution(0);
        	
        	primal_solution.swap(dual_solution_0);	    
        	write_output(equation_systems, a_step, "adjoint_0");
        
        	primal_solution.swap(dual_solution_0);
        	
        	NumericVector&lt;Number&gt; &dual_solution_1 = system.get_adjoint_solution(1);
        	
        	primal_solution.swap(dual_solution_1);	    
        	write_output(equation_systems, a_step, "adjoint_1");
        
        	primal_solution.swap(dual_solution_1);
        				
        	std::cout &lt;&lt; "Adaptive step " &lt;&lt; a_step &lt;&lt; ", we have " &lt;&lt; mesh.n_active_elem()
        		  &lt;&lt; " active elements and "
        		  &lt;&lt; equation_systems.n_active_dofs()
        		  &lt;&lt; " active dofs." &lt;&lt; std::endl ;	
        	
        	std::cout &lt;&lt; "Postprocessing: " &lt;&lt; std::endl; 
        	system.postprocess_sides = true;
        	system.postprocess();
        	
        	Number QoI_0_computed = system.get_QoI_value("computed", 0);
        	Number QoI_0_exact = system.get_QoI_value("exact", 0);
        	Number QoI_1_computed = system.get_QoI_value("computed", 1);
        	Number QoI_1_exact = system.get_QoI_value("exact", 1);
        	
        	std::cout&lt;&lt; "The relative error in QoI 0 is " &lt;&lt; std::setprecision(17)
                         &lt;&lt; std::abs(QoI_0_computed - QoI_0_exact) /
                            std::abs(QoI_0_exact) &lt;&lt; std::endl;
        	
        	std::cout&lt;&lt; "The relative error in QoI 1 is " &lt;&lt; std::setprecision(17)
                         &lt;&lt; std::abs(QoI_1_computed - QoI_1_exact) /
                            std::abs(QoI_1_exact) &lt;&lt; std::endl &lt;&lt; std::endl;
              }
          }
        
          std::cerr &lt;&lt; '[' &lt;&lt; libMesh::processor_id() 
                    &lt;&lt; "] Completing output." &lt;&lt; std::endl; 
        
        #endif // #ifndef LIBMESH_ENABLE_AMR
            
</pre>
</div>
<div class = "comment">
All done.  
</div>

<div class ="fragment">
<pre>
          return 0;
        }
</pre>
</div>

<a name="nocomments"></a> 
<br><br><br> <h1> The program without comments: </h1> 
<pre> 
  
  
  #include &lt;iostream&gt;
  
  #include <B><FONT COLOR="#BC8F8F">&quot;equation_systems.h&quot;</FONT></B>
  #include <B><FONT COLOR="#BC8F8F">&quot;linear_solver.h&quot;</FONT></B>
  #include <B><FONT COLOR="#BC8F8F">&quot;error_vector.h&quot;</FONT></B>
  #include <B><FONT COLOR="#BC8F8F">&quot;mesh.h&quot;</FONT></B>
  #include <B><FONT COLOR="#BC8F8F">&quot;mesh_refinement.h&quot;</FONT></B>
  #include <B><FONT COLOR="#BC8F8F">&quot;newton_solver.h&quot;</FONT></B>
  #include <B><FONT COLOR="#BC8F8F">&quot;numeric_vector.h&quot;</FONT></B>
  #include <B><FONT COLOR="#BC8F8F">&quot;steady_solver.h&quot;</FONT></B>
  #include <B><FONT COLOR="#BC8F8F">&quot;system_norm.h&quot;</FONT></B>
  
  #include <B><FONT COLOR="#BC8F8F">&quot;kelly_error_estimator.h&quot;</FONT></B>
  #include <B><FONT COLOR="#BC8F8F">&quot;patch_recovery_error_estimator.h&quot;</FONT></B>
  
  #include <B><FONT COLOR="#BC8F8F">&quot;adjoint_residual_error_estimator.h&quot;</FONT></B>
  #include <B><FONT COLOR="#BC8F8F">&quot;qoi_set.h&quot;</FONT></B>
  
  #include <B><FONT COLOR="#BC8F8F">&quot;getpot.h&quot;</FONT></B>
  #include <B><FONT COLOR="#BC8F8F">&quot;gmv_io.h&quot;</FONT></B>
  
  #include <B><FONT COLOR="#BC8F8F">&quot;o_string_stream.h&quot;</FONT></B>
  
  #include <B><FONT COLOR="#BC8F8F">&quot;femparameters.h&quot;</FONT></B>
  #include <B><FONT COLOR="#BC8F8F">&quot;L-shaped.h&quot;</FONT></B>
  
  using namespace libMesh;
  
  
  
  
  <B><FONT COLOR="#228B22">void</FONT></B> write_output(EquationSystems &amp;es,		                   
  		  <B><FONT COLOR="#228B22">unsigned</FONT></B> <B><FONT COLOR="#228B22">int</FONT></B> a_step,       <I><FONT COLOR="#B22222">// The adaptive step count
</FONT></I>  		  <B><FONT COLOR="#5F9EA0">std</FONT></B>::string solution_type) <I><FONT COLOR="#B22222">// primal or adjoint solve
</FONT></I>  {
    MeshBase &amp;mesh = es.get_mesh();
  
  #ifdef LIBMESH_HAVE_GMV
    OStringStream file_name_gmv;
    file_name_gmv &lt;&lt; solution_type &lt;&lt; <B><FONT COLOR="#BC8F8F">&quot;.out.gmv.&quot;</FONT></B>;      
    OSSRealzeroright(file_name_gmv,2,0,a_step);
    
    GMVIO(mesh).write_equation_systems
      (file_name_gmv.str(), es);
  #endif    
  }
  
  
  <B><FONT COLOR="#228B22">void</FONT></B> set_system_parameters(LaplaceSystem &amp;system, FEMParameters &amp;param)
  {
    system.analytic_jacobians() = param.analytic_jacobians;
  
    system.verify_analytic_jacobians = param.verify_analytic_jacobians;
  
    system.fe_family() = param.fe_family[0];
    system.fe_order() = param.fe_order[0];
  
    system.print_solution_norms = param.print_solution_norms;
    system.print_solutions      = param.print_solutions;
    system.print_residual_norms = param.print_residual_norms;
    system.print_residuals      = param.print_residuals;
    system.print_jacobian_norms = param.print_jacobian_norms;
    system.print_jacobians      = param.print_jacobians;
  
    system.time_solver =
        AutoPtr&lt;TimeSolver&gt;(<B><FONT COLOR="#A020F0">new</FONT></B> SteadySolver(system));
  
    {
      NewtonSolver *solver = <B><FONT COLOR="#A020F0">new</FONT></B> NewtonSolver(system);
      system.time_solver-&gt;diff_solver() = AutoPtr&lt;DiffSolver&gt;(solver);
      
      solver-&gt;quiet                       = param.solver_quiet;
      solver-&gt;max_nonlinear_iterations    = param.max_nonlinear_iterations;
      solver-&gt;minsteplength               = param.min_step_length;
      solver-&gt;relative_step_tolerance     = param.relative_step_tolerance;
      solver-&gt;relative_residual_tolerance = param.relative_residual_tolerance;
      solver-&gt;require_residual_reduction  = param.require_residual_reduction;
      solver-&gt;linear_tolerance_multiplier = param.linear_tolerance_multiplier;
      <B><FONT COLOR="#A020F0">if</FONT></B> (system.time_solver-&gt;reduce_deltat_on_diffsolver_failure)
        {
  	solver-&gt;continue_after_max_iterations = true;
  	solver-&gt;continue_after_backtrack_failure = true;
        }
      
      solver-&gt;max_linear_iterations       = param.max_linear_iterations;
      solver-&gt;initial_linear_tolerance    = param.initial_linear_tolerance;
      solver-&gt;minimum_linear_tolerance    = param.minimum_linear_tolerance;      
    }
  }
  
  
  #ifdef LIBMESH_ENABLE_AMR
  
  AutoPtr&lt;MeshRefinement&gt; build_mesh_refinement(MeshBase &amp;mesh,
                                                FEMParameters &amp;param)
  {
    AutoPtr&lt;MeshRefinement&gt; mesh_refinement(<B><FONT COLOR="#A020F0">new</FONT></B> MeshRefinement(mesh));
    mesh_refinement-&gt;coarsen_by_parents() = true;
    mesh_refinement-&gt;absolute_global_tolerance() = param.global_tolerance;
    mesh_refinement-&gt;nelem_target()      = param.nelem_target;  
    mesh_refinement-&gt;refine_fraction()   = param.refine_fraction;
    mesh_refinement-&gt;coarsen_fraction()  = param.coarsen_fraction;  
    mesh_refinement-&gt;coarsen_threshold() = param.coarsen_threshold;
  
    <B><FONT COLOR="#A020F0">return</FONT></B> mesh_refinement;
  }
  
  #endif <I><FONT COLOR="#B22222">// LIBMESH_ENABLE_AMR
</FONT></I>  
  
  AutoPtr&lt;ErrorEstimator&gt; build_error_estimator(FEMParameters &amp;param, QoISet &amp;qois)
  {
    AutoPtr&lt;ErrorEstimator&gt; error_estimator;
  
    <B><FONT COLOR="#A020F0">if</FONT></B> (param.indicator_type == <B><FONT COLOR="#BC8F8F">&quot;kelly&quot;</FONT></B>)
      {
        <B><FONT COLOR="#5F9EA0">std</FONT></B>::cout&lt;&lt;<B><FONT COLOR="#BC8F8F">&quot;Using Kelly Error Estimator&quot;</FONT></B>&lt;&lt;std::endl;
  
        error_estimator.reset(<B><FONT COLOR="#A020F0">new</FONT></B> KellyErrorEstimator);
      }
    <B><FONT COLOR="#A020F0">else</FONT></B> <B><FONT COLOR="#A020F0">if</FONT></B> (param.indicator_type == <B><FONT COLOR="#BC8F8F">&quot;adjoint_residual&quot;</FONT></B>)
      {
        <B><FONT COLOR="#5F9EA0">std</FONT></B>::cout&lt;&lt;<B><FONT COLOR="#BC8F8F">&quot;Using Adjoint Residual Error Estimator with Patch Recovery Weights&quot;</FONT></B>&lt;&lt;std::endl;
  
        AdjointResidualErrorEstimator *adjoint_residual_estimator = <B><FONT COLOR="#A020F0">new</FONT></B> AdjointResidualErrorEstimator;
        
        error_estimator.reset (adjoint_residual_estimator);
        
        adjoint_residual_estimator-&gt;qoi_set() = qois;
        
        adjoint_residual_estimator-&gt;adjoint_already_solved = true;
  
        adjoint_residual_estimator-&gt;error_plot_suffix = <B><FONT COLOR="#BC8F8F">&quot;error.gmv&quot;</FONT></B>;
        
        PatchRecoveryErrorEstimator *p1 =
  	<B><FONT COLOR="#A020F0">new</FONT></B> PatchRecoveryErrorEstimator;
        adjoint_residual_estimator-&gt;primal_error_estimator().reset(p1);
  	   	   	   
        PatchRecoveryErrorEstimator *p2 =
  	<B><FONT COLOR="#A020F0">new</FONT></B> PatchRecoveryErrorEstimator;
        adjoint_residual_estimator-&gt;dual_error_estimator().reset(p2);   
  
        adjoint_residual_estimator-&gt;primal_error_estimator()-&gt;error_norm.set_type(0, H1_SEMINORM);
        p1-&gt;set_patch_reuse(param.patch_reuse);   
  
        adjoint_residual_estimator-&gt;dual_error_estimator()-&gt;error_norm.set_type(0, H1_SEMINORM);	 
        p2-&gt;set_patch_reuse(param.patch_reuse);
      }
    <B><FONT COLOR="#A020F0">else</FONT></B>
      {
        <B><FONT COLOR="#5F9EA0">std</FONT></B>::cerr &lt;&lt; <B><FONT COLOR="#BC8F8F">&quot;Unknown indicator_type&quot;</FONT></B> &lt;&lt; std::endl;
        libmesh_error();
      }
    <B><FONT COLOR="#A020F0">return</FONT></B> error_estimator;
  }
  
  <B><FONT COLOR="#228B22">int</FONT></B> main (<B><FONT COLOR="#228B22">int</FONT></B> argc, <B><FONT COLOR="#228B22">char</FONT></B>** argv)
  {
    LibMeshInit init (argc, argv);
  
  #ifndef LIBMESH_ENABLE_AMR
    libmesh_example_assert(false, <B><FONT COLOR="#BC8F8F">&quot;--enable-amr&quot;</FONT></B>);
  #<B><FONT COLOR="#A020F0">else</FONT></B>
    libmesh_example_assert(libMesh::default_solver_package() == PETSC_SOLVERS, <B><FONT COLOR="#BC8F8F">&quot;--enable-petsc&quot;</FONT></B>);
  
    <B><FONT COLOR="#5F9EA0">std</FONT></B>::cout &lt;&lt; <B><FONT COLOR="#BC8F8F">&quot;Started &quot;</FONT></B> &lt;&lt; argv[0] &lt;&lt; std::endl;
  
    {
      <B><FONT COLOR="#5F9EA0">std</FONT></B>::ifstream i(<B><FONT COLOR="#BC8F8F">&quot;general.in&quot;</FONT></B>);
      <B><FONT COLOR="#A020F0">if</FONT></B> (!i)
        {
          <B><FONT COLOR="#5F9EA0">std</FONT></B>::cerr &lt;&lt; <B><FONT COLOR="#BC8F8F">'['</FONT></B> &lt;&lt; libMesh::processor_id() 
                    &lt;&lt; <B><FONT COLOR="#BC8F8F">&quot;] Can't find general.in; exiting early.&quot;</FONT></B> 
                    &lt;&lt; std::endl; 
          libmesh_error();
        }
    }
    GetPot infile(<B><FONT COLOR="#BC8F8F">&quot;general.in&quot;</FONT></B>);
  
    FEMParameters param;
    param.read(infile);
  
    libmesh_example_assert(2 &lt;= LIBMESH_DIM, <B><FONT COLOR="#BC8F8F">&quot;2D support&quot;</FONT></B>);
    
    Mesh mesh;
  
    AutoPtr&lt;MeshRefinement&gt; mesh_refinement =
      build_mesh_refinement(mesh, param);
  
    EquationSystems equation_systems (mesh);
  
    <B><FONT COLOR="#5F9EA0">std</FONT></B>::cout &lt;&lt; <B><FONT COLOR="#BC8F8F">&quot;Reading in and building the mesh&quot;</FONT></B> &lt;&lt; std::endl;
  
    mesh.read(param.domainfile.c_str());
    mesh.all_second_order();
  
    MeshRefinement initial_uniform_refinements(mesh);
    initial_uniform_refinements.uniformly_refine(param.coarserefinements);
      
    <B><FONT COLOR="#5F9EA0">std</FONT></B>::cout &lt;&lt; <B><FONT COLOR="#BC8F8F">&quot;Building system&quot;</FONT></B> &lt;&lt; std::endl;
  
    LaplaceSystem &amp;system = equation_systems.add_system&lt;LaplaceSystem&gt; (<B><FONT COLOR="#BC8F8F">&quot;LaplaceSystem&quot;</FONT></B>);
  
    set_system_parameters(system, param);
    
    <B><FONT COLOR="#5F9EA0">std</FONT></B>::cout &lt;&lt; <B><FONT COLOR="#BC8F8F">&quot;Initializing systems&quot;</FONT></B> &lt;&lt; std::endl;
  
    equation_systems.init ();
  
    mesh.print_info();
    equation_systems.print_info();
    LinearSolver&lt;Number&gt; *linear_solver = system.get_linear_solver();
  
    {	
      <B><FONT COLOR="#228B22">unsigned</FONT></B> <B><FONT COLOR="#228B22">int</FONT></B> a_step = 0;
      <B><FONT COLOR="#A020F0">for</FONT></B> (; a_step != param.max_adaptivesteps; ++a_step)
        {	    	    
  	<B><FONT COLOR="#A020F0">if</FONT></B> (param.global_tolerance != 0.)
  	  libmesh_assert (param.nelem_target == 0);
              <B><FONT COLOR="#A020F0">else</FONT></B>
                libmesh_assert (param.nelem_target &gt; 0);
      	
  	linear_solver-&gt;reuse_preconditioner(false);
  
  	system.solve();
  
  	write_output(equation_systems, a_step, <B><FONT COLOR="#BC8F8F">&quot;primal&quot;</FONT></B>);
  	
  	NumericVector&lt;Number&gt; &amp;primal_solution = *system.solution;
  
  	QoISet qois;
  
  	<B><FONT COLOR="#5F9EA0">std</FONT></B>::vector&lt;<B><FONT COLOR="#228B22">unsigned</FONT></B> <B><FONT COLOR="#228B22">int</FONT></B>&gt; qoi_indices;
  	qoi_indices.push_back(0);
  	qoi_indices.push_back(1);
  	qois.add_indices(qoi_indices);
  
  	qois.set_weight(0, 0.5);	
  	qois.set_weight(1, 0.5);	
  
  	system.assemble_qoi_sides = true;
  
  	linear_solver-&gt;reuse_preconditioner(param.reuse_preconditioner);
  	
  	system.adjoint_solve();
  
  	NumericVector&lt;Number&gt; &amp;dual_solution_0 = system.get_adjoint_solution(0);
  
  	primal_solution.swap(dual_solution_0);	    
  	write_output(equation_systems, a_step, <B><FONT COLOR="#BC8F8F">&quot;adjoint_0&quot;</FONT></B>);
  
  	primal_solution.swap(dual_solution_0);
  	
  	NumericVector&lt;Number&gt; &amp;dual_solution_1 = system.get_adjoint_solution(1);
  	
  	primal_solution.swap(dual_solution_1);	    
  	write_output(equation_systems, a_step, <B><FONT COLOR="#BC8F8F">&quot;adjoint_1&quot;</FONT></B>);
  
  	primal_solution.swap(dual_solution_1);
  			    
  	<B><FONT COLOR="#5F9EA0">std</FONT></B>::cout &lt;&lt; <B><FONT COLOR="#BC8F8F">&quot;Adaptive step &quot;</FONT></B> &lt;&lt; a_step &lt;&lt; <B><FONT COLOR="#BC8F8F">&quot;, we have &quot;</FONT></B> &lt;&lt; mesh.n_active_elem()
                        &lt;&lt; <B><FONT COLOR="#BC8F8F">&quot; active elements and &quot;</FONT></B>
                        &lt;&lt; equation_systems.n_active_dofs()
  		  &lt;&lt; <B><FONT COLOR="#BC8F8F">&quot; active dofs.&quot;</FONT></B> &lt;&lt; std::endl ;
  
  	<B><FONT COLOR="#5F9EA0">std</FONT></B>::cout &lt;&lt; <B><FONT COLOR="#BC8F8F">&quot;Postprocessing: &quot;</FONT></B> &lt;&lt; std::endl; 
  	system.postprocess_sides = true;
  	system.postprocess();
  	Number QoI_0_computed = system.get_QoI_value(<B><FONT COLOR="#BC8F8F">&quot;computed&quot;</FONT></B>, 0);
  	Number QoI_0_exact = system.get_QoI_value(<B><FONT COLOR="#BC8F8F">&quot;exact&quot;</FONT></B>, 0);
  	Number QoI_1_computed = system.get_QoI_value(<B><FONT COLOR="#BC8F8F">&quot;computed&quot;</FONT></B>, 1);
  	Number QoI_1_exact = system.get_QoI_value(<B><FONT COLOR="#BC8F8F">&quot;exact&quot;</FONT></B>, 1);
  		
  	<B><FONT COLOR="#5F9EA0">std</FONT></B>::cout&lt;&lt; <B><FONT COLOR="#BC8F8F">&quot;The relative error in QoI 0 is &quot;</FONT></B> &lt;&lt; std::setprecision(17)
                   &lt;&lt; std::abs(QoI_0_computed - QoI_0_exact) /
                      <B><FONT COLOR="#5F9EA0">std</FONT></B>::abs(QoI_0_exact) &lt;&lt; std::endl;
  		
  	<B><FONT COLOR="#5F9EA0">std</FONT></B>::cout&lt;&lt; <B><FONT COLOR="#BC8F8F">&quot;The relative error in QoI 1 is &quot;</FONT></B> &lt;&lt; std::setprecision(17) 
                   &lt;&lt; std::abs(QoI_1_computed - QoI_1_exact) /
                      <B><FONT COLOR="#5F9EA0">std</FONT></B>::abs(QoI_1_exact) &lt;&lt; std::endl &lt;&lt; std::endl;
  	
  	ErrorVector error;
  	
  	AutoPtr&lt;ErrorEstimator&gt; error_estimator =
  	  build_error_estimator(param, qois);
  	
  	error_estimator-&gt;estimate_error(system, error);
  			    	    
  
  	<B><FONT COLOR="#A020F0">if</FONT></B>(param.refine_uniformly)
  	  {	  			    
  	    mesh_refinement-&gt;uniformly_refine(1);
  	  }
  	<B><FONT COLOR="#A020F0">else</FONT></B> <B><FONT COLOR="#A020F0">if</FONT></B>(param.global_tolerance &gt;= 0. &amp;&amp; param.nelem_target == 0.)
  	  {	      	    
  	    mesh_refinement-&gt;flag_elements_by_error_tolerance (error);              
  	    
  	    mesh_refinement-&gt;refine_and_coarsen_elements();
  	  }
  	<B><FONT COLOR="#A020F0">else</FONT></B> 
  	  {	      	    	    
  	    <B><FONT COLOR="#A020F0">if</FONT></B> (mesh.n_active_elem() &gt;= param.nelem_target)
  	      {
  		<B><FONT COLOR="#5F9EA0">std</FONT></B>::cout&lt;&lt;<B><FONT COLOR="#BC8F8F">&quot;We reached the target number of elements.&quot;</FONT></B>&lt;&lt;std::endl &lt;&lt;std::endl;
  		<B><FONT COLOR="#A020F0">break</FONT></B>;
  	      }				
  	    
  	    mesh_refinement-&gt;flag_elements_by_nelem_target (error);
  	    	    
  	    mesh_refinement-&gt;refine_and_coarsen_elements();
  	  }
              
  	equation_systems.reinit();
  
          <B><FONT COLOR="#5F9EA0">std</FONT></B>::cout &lt;&lt; <B><FONT COLOR="#BC8F8F">&quot;Refined mesh to &quot;</FONT></B>
                    &lt;&lt; mesh.n_active_elem()
                    &lt;&lt; <B><FONT COLOR="#BC8F8F">&quot; active elements and &quot;</FONT></B>
                    &lt;&lt; equation_systems.n_active_dofs()
                    &lt;&lt; <B><FONT COLOR="#BC8F8F">&quot; active dofs.&quot;</FONT></B> &lt;&lt; std::endl;
        }
  
      <B><FONT COLOR="#A020F0">if</FONT></B> (a_step == param.max_adaptivesteps)
        {	 
  	linear_solver-&gt;reuse_preconditioner(false);	
  	system.solve();
  	
  	write_output(equation_systems, a_step, <B><FONT COLOR="#BC8F8F">&quot;primal&quot;</FONT></B>);
  
  	NumericVector&lt;Number&gt; &amp;primal_solution = *system.solution;
  	
  	QoISet qois;
  	<B><FONT COLOR="#5F9EA0">std</FONT></B>::vector&lt;<B><FONT COLOR="#228B22">unsigned</FONT></B> <B><FONT COLOR="#228B22">int</FONT></B>&gt; qoi_indices;
  
  	qoi_indices.push_back(0);
  	qoi_indices.push_back(1);
  	qois.add_indices(qoi_indices);
  	
  	qois.set_weight(0, 0.5);	
  	qois.set_weight(1, 0.5);	
  
  	system.assemble_qoi_sides = true;	
  	linear_solver-&gt;reuse_preconditioner(param.reuse_preconditioner);
  	system.adjoint_solve();
  		
  	NumericVector&lt;Number&gt; &amp;dual_solution_0 = system.get_adjoint_solution(0);
  	
  	primal_solution.swap(dual_solution_0);	    
  	write_output(equation_systems, a_step, <B><FONT COLOR="#BC8F8F">&quot;adjoint_0&quot;</FONT></B>);
  
  	primal_solution.swap(dual_solution_0);
  	
  	NumericVector&lt;Number&gt; &amp;dual_solution_1 = system.get_adjoint_solution(1);
  	
  	primal_solution.swap(dual_solution_1);	    
  	write_output(equation_systems, a_step, <B><FONT COLOR="#BC8F8F">&quot;adjoint_1&quot;</FONT></B>);
  
  	primal_solution.swap(dual_solution_1);
  				
  	<B><FONT COLOR="#5F9EA0">std</FONT></B>::cout &lt;&lt; <B><FONT COLOR="#BC8F8F">&quot;Adaptive step &quot;</FONT></B> &lt;&lt; a_step &lt;&lt; <B><FONT COLOR="#BC8F8F">&quot;, we have &quot;</FONT></B> &lt;&lt; mesh.n_active_elem()
  		  &lt;&lt; <B><FONT COLOR="#BC8F8F">&quot; active elements and &quot;</FONT></B>
  		  &lt;&lt; equation_systems.n_active_dofs()
  		  &lt;&lt; <B><FONT COLOR="#BC8F8F">&quot; active dofs.&quot;</FONT></B> &lt;&lt; std::endl ;	
  	
  	<B><FONT COLOR="#5F9EA0">std</FONT></B>::cout &lt;&lt; <B><FONT COLOR="#BC8F8F">&quot;Postprocessing: &quot;</FONT></B> &lt;&lt; std::endl; 
  	system.postprocess_sides = true;
  	system.postprocess();
  	
  	Number QoI_0_computed = system.get_QoI_value(<B><FONT COLOR="#BC8F8F">&quot;computed&quot;</FONT></B>, 0);
  	Number QoI_0_exact = system.get_QoI_value(<B><FONT COLOR="#BC8F8F">&quot;exact&quot;</FONT></B>, 0);
  	Number QoI_1_computed = system.get_QoI_value(<B><FONT COLOR="#BC8F8F">&quot;computed&quot;</FONT></B>, 1);
  	Number QoI_1_exact = system.get_QoI_value(<B><FONT COLOR="#BC8F8F">&quot;exact&quot;</FONT></B>, 1);
  	
  	<B><FONT COLOR="#5F9EA0">std</FONT></B>::cout&lt;&lt; <B><FONT COLOR="#BC8F8F">&quot;The relative error in QoI 0 is &quot;</FONT></B> &lt;&lt; std::setprecision(17)
                   &lt;&lt; std::abs(QoI_0_computed - QoI_0_exact) /
                      <B><FONT COLOR="#5F9EA0">std</FONT></B>::abs(QoI_0_exact) &lt;&lt; std::endl;
  	
  	<B><FONT COLOR="#5F9EA0">std</FONT></B>::cout&lt;&lt; <B><FONT COLOR="#BC8F8F">&quot;The relative error in QoI 1 is &quot;</FONT></B> &lt;&lt; std::setprecision(17)
                   &lt;&lt; std::abs(QoI_1_computed - QoI_1_exact) /
                      <B><FONT COLOR="#5F9EA0">std</FONT></B>::abs(QoI_1_exact) &lt;&lt; std::endl &lt;&lt; std::endl;
        }
    }
  
    <B><FONT COLOR="#5F9EA0">std</FONT></B>::cerr &lt;&lt; <B><FONT COLOR="#BC8F8F">'['</FONT></B> &lt;&lt; libMesh::processor_id() 
              &lt;&lt; <B><FONT COLOR="#BC8F8F">&quot;] Completing output.&quot;</FONT></B> &lt;&lt; std::endl; 
  
  #endif <I><FONT COLOR="#B22222">// #ifndef LIBMESH_ENABLE_AMR
</FONT></I>      
    <B><FONT COLOR="#A020F0">return</FONT></B> 0;
  }
</pre> 
<a name="output"></a> 
<br><br><br> <h1> The console output of the program: </h1> 
<pre>
Compiling C++ (in optimized mode) adjoints_ex1.C...
Compiling C++ (in optimized mode) element_postprocess.C...
Compiling C++ (in optimized mode) element_qoi_derivative.C...
Compiling C++ (in optimized mode) femparameters.C...
Compiling C++ (in optimized mode) L-shaped.C...
Compiling C++ (in optimized mode) side_postprocess.C...
Compiling C++ (in optimized mode) side_qoi_derivative.C...
Linking ./adjoints_ex1-opt...
***************************************************************
* Running Example  ./adjoints_ex1-opt
***************************************************************
 
Started ./adjoints_ex1-opt
Reading in and building the mesh
Building system
Initializing systems
 Mesh Information:
  mesh_dimension()=2
  spatial_dimension()=3
  n_nodes()=833
    n_local_nodes()=833
  n_elem()=255
    n_local_elem()=255
    n_active_elem()=192
  n_subdomains()=1
  n_partitions()=1
  n_processors()=1
  n_threads()=1
  processor_id()=0

 EquationSystems
  n_systems()=1
   System #0, "LaplaceSystem"
    Type "Implicit"
    Variables="T" 
    Finite Element Types="LAGRANGE", "JACOBI_20_00" 
    Infinite Element Mapping="CARTESIAN" 
    Approximation Orders="SECOND", "THIRD" 
    n_dofs()=833
    n_local_dofs()=833
    n_constrained_dofs()=0
    n_local_constrained_dofs()=0
    n_vectors()=1
    n_matrices()=1
    DofMap Sparsity
      Average  On-Processor Bandwidth <= 15.06
      Average Off-Processor Bandwidth <= 0
      Maximum  On-Processor Bandwidth <= 25
      Maximum Off-Processor Bandwidth <= 0
    DofMap Constraints
      Number of DoF Constraints = 0
      Number of Node Constraints = 0

  Nonlinear solver converged, step 0, residual reduction 1.18018e-13 < 1e-09
Adaptive step 0, we have 192 active elements and 833 active dofs.
Postprocessing: 
The relative error in QoI 0 is 0.00012462746034657153
The relative error in QoI 1 is 0.00014255871210532757

Using Adjoint Residual Error Estimator with Patch Recovery Weights
Refined mesh to 222 active elements and 945 active dofs.
  Nonlinear solver converged, step 0, residual reduction 1.4866235111362822e-10 < 1.0000000000000001e-09
Adaptive step 1, we have 222 active elements and 945 active dofs.
Postprocessing: 
The relative error in QoI 0 is 4.9657720064985281e-05
The relative error in QoI 1 is 6.2108600831051721e-06

Using Adjoint Residual Error Estimator with Patch Recovery Weights
Refined mesh to 261 active elements and 1093 active dofs.
  Nonlinear solver converged, step 0, residual reduction 1.8274703051752381e-10 < 1.0000000000000001e-09
Adaptive step 2, we have 261 active elements and 1093 active dofs.
Postprocessing: 
The relative error in QoI 0 is 3.2489141757316423e-05
The relative error in QoI 1 is 9.1543505429770195e-05

Using Adjoint Residual Error Estimator with Patch Recovery Weights
Refined mesh to 309 active elements and 1279 active dofs.
  Nonlinear solver reached maximum step 1, latest evaluated residual 3.0789956885790906e-05
  Continuing...
Adaptive step 3, we have 309 active elements and 1279 active dofs.
Postprocessing: 
The relative error in QoI 0 is 1.3068055374124031e-05
The relative error in QoI 1 is 2.7384569859752794e-05

Using Adjoint Residual Error Estimator with Patch Recovery Weights
Refined mesh to 375 active elements and 1511 active dofs.
  Nonlinear solver reached maximum step 1, latest evaluated residual 3.0464562981439094e-05
  Continuing...
Adaptive step 4, we have 375 active elements and 1511 active dofs.
Postprocessing: 
The relative error in QoI 0 is 5.1850614729802354e-06
The relative error in QoI 1 is 5.8552194126613999e-07

Using Adjoint Residual Error Estimator with Patch Recovery Weights
Refined mesh to 438 active elements and 1737 active dofs.
  Nonlinear solver reached maximum step 1, latest evaluated residual 3.0412835297461065e-05
  Continuing...
Adaptive step 5, we have 438 active elements and 1737 active dofs.
Postprocessing: 
The relative error in QoI 0 is 1.5909681422576661e-06
The relative error in QoI 1 is 1.0292566220613908e-05

Using Adjoint Residual Error Estimator with Patch Recovery Weights
Refined mesh to 516 active elements and 2045 active dofs.
  Nonlinear solver reached maximum step 1, latest evaluated residual 2.6206318508493051e-05
  Continuing...
Adaptive step 6, we have 516 active elements and 2045 active dofs.
Postprocessing: 
The relative error in QoI 0 is 3.7224084878748516e-07
The relative error in QoI 1 is 7.9879129741668715e-06

[0] Completing output.

-------------------------------------------------------------------
| Time:           Sat Apr  7 15:56:41 2012                         |
| OS:             Linux                                            |
| HostName:       lkirk-home                                       |
| OS Release:     3.0.0-17-generic                                 |
| OS Version:     #30-Ubuntu SMP Thu Mar 8 20:45:39 UTC 2012       |
| Machine:        x86_64                                           |
| Username:       benkirk                                          |
| Configuration:  ./configure run on Sat Apr  7 15:49:27 CDT 2012  |
-------------------------------------------------------------------
 ---------------------------------------------------------------------------------------------------------------------
| libMesh Performance: Alive time=1.8108, Active time=1.67587                                                         |
 ---------------------------------------------------------------------------------------------------------------------
| Event                                   nCalls    Total Time  Avg Time    Total Time  Avg Time    % of Active Time  |
|                                                   w/o Sub     w/o Sub     With Sub    With Sub    w/o S    With S   |
|---------------------------------------------------------------------------------------------------------------------|
|                                                                                                                     |
|                                                                                                                     |
| AdjointResidualErrorEstimator                                                                                       |
|   estimate_error()                      6         0.0862      0.014366    0.3720      0.062002    5.14     22.20    |
|                                                                                                                     |
| DofMap                                                                                                              |
|   add_neighbors_to_send_list()          19        0.0029      0.000153    0.0029      0.000153    0.17     0.17     |
|   build_constraint_matrix()             10605     0.0278      0.000003    0.0278      0.000003    1.66     1.66     |
|   cnstrn_elem_mat_vec()                 2121      0.0128      0.000006    0.0128      0.000006    0.76     0.76     |
|   compute_sparsity()                    7         0.0179      0.002559    0.0204      0.002909    1.07     1.21     |
|   constrain_elem_matrix()               2121      0.0051      0.000002    0.0051      0.000002    0.31     0.31     |
|   constrain_elem_vector()               6363      0.0236      0.000004    0.0236      0.000004    1.41     1.41     |
|   create_dof_constraints()              19        0.0267      0.001403    0.0348      0.001832    1.59     2.08     |
|   distribute_dofs()                     19        0.0051      0.000268    0.0185      0.000973    0.30     1.10     |
|   dof_indices()                         65096     0.0703      0.000001    0.0703      0.000001    4.20     4.20     |
|   enforce_constraints_exactly()         48        0.0054      0.000112    0.0054      0.000112    0.32     0.32     |
|   old_dof_indices()                     12726     0.0110      0.000001    0.0110      0.000001    0.65     0.65     |
|   prepare_send_list()                   19        0.0000      0.000001    0.0000      0.000001    0.00     0.00     |
|   reinit()                              19        0.0134      0.000704    0.0134      0.000704    0.80     0.80     |
|                                                                                                                     |
| EquationSystems                                                                                                     |
|   build_discontinuous_solution_vector() 12        0.0038      0.000317    0.0059      0.000495    0.23     0.35     |
|   build_solution_vector()               21        0.0119      0.000566    0.0188      0.000895    0.71     1.12     |
|                                                                                                                     |
| FE                                                                                                                  |
|   compute_affine_map()                  32136     0.0541      0.000002    0.0541      0.000002    3.23     3.23     |
|   compute_face_map()                    3360      0.0208      0.000006    0.0482      0.000014    1.24     2.88     |
|   compute_shape_functions()             32136     0.0329      0.000001    0.0329      0.000001    1.96     1.96     |
|   init_face_shape_functions()           35        0.0001      0.000003    0.0001      0.000003    0.01     0.01     |
|   init_shape_functions()                3857      0.0453      0.000012    0.0453      0.000012    2.70     2.70     |
|   inverse_map()                         24030     0.0430      0.000002    0.0430      0.000002    2.57     2.57     |
|                                                                                                                     |
| FEMSystem                                                                                                           |
|   assemble_qoi_derivative()             7         0.0351      0.005012    0.0904      0.012917    2.09     5.40     |
|   assembly()                            7         0.0581      0.008304    0.1770      0.025279    3.47     10.56    |
|   assembly(get_jacobian)                7         0.0645      0.009209    0.1627      0.023239    3.85     9.71     |
|   assembly(get_residual)                7         0.0282      0.004035    0.0818      0.011682    1.69     4.88     |
|   numerical_elem_jacobian()             4626      0.0767      0.000017    0.0767      0.000017    4.58     4.58     |
|   numerical_side_jacobian()             1344      0.0343      0.000025    0.0343      0.000025    2.04     2.04     |
|   postprocess()                         7         0.0144      0.002053    0.0426      0.006092    0.86     2.54     |
|                                                                                                                     |
| GMVIO                                                                                                               |
|   write_nodal_data()                    21        0.1749      0.008329    0.1749      0.008329    10.44    10.44    |
|                                                                                                                     |
| ImplicitSystem                                                                                                      |
|   adjoint_solve()                       7         0.0006      0.000089    0.4786      0.068372    0.04     28.56    |
|                                                                                                                     |
| LocationMap                                                                                                         |
|   find()                                3420      0.0051      0.000001    0.0051      0.000001    0.30     0.30     |
|   init()                                15        0.0045      0.000302    0.0045      0.000302    0.27     0.27     |
|                                                                                                                     |
| Mesh                                                                                                                |
|   all_first_order()                     12        0.0037      0.000307    0.0037      0.000307    0.22     0.22     |
|   all_second_order()                    1         0.0001      0.000103    0.0001      0.000103    0.01     0.01     |
|   contract()                            6         0.0002      0.000033    0.0006      0.000101    0.01     0.04     |
|   find_neighbors()                      33        0.0301      0.000911    0.0301      0.000911    1.79     1.79     |
|   renumber_nodes_and_elem()             70        0.0031      0.000044    0.0031      0.000044    0.18     0.18     |
|                                                                                                                     |
| MeshOutput                                                                                                          |
|   write_equation_systems()              21        0.0002      0.000011    0.1939      0.009235    0.01     11.57    |
|                                                                                                                     |
| MeshRefinement                                                                                                      |
|   _coarsen_elements()                   12        0.0002      0.000018    0.0002      0.000018    0.01     0.01     |
|   _refine_elements()                    15        0.0076      0.000505    0.0203      0.001352    0.45     1.21     |
|   add_point()                           3420      0.0061      0.000002    0.0119      0.000003    0.36     0.71     |
|   make_coarsening_compatible()          17        0.0060      0.000352    0.0060      0.000352    0.36     0.36     |
|   make_refinement_compatible()          17        0.0004      0.000024    0.0004      0.000024    0.02     0.02     |
|                                                                                                                     |
| NewtonSolver                                                                                                        |
|   solve()                               7         0.0019      0.000279    0.4559      0.065135    0.12     27.21    |
|                                                                                                                     |
| Parallel                                                                                                            |
|   allgather()                           19        0.0000      0.000001    0.0000      0.000001    0.00     0.00     |
|                                                                                                                     |
| Partitioner                                                                                                         |
|   set_node_processor_ids()              12        0.0017      0.000144    0.0017      0.000144    0.10     0.10     |
|   single_partition()                    33        0.0016      0.000049    0.0016      0.000049    0.10     0.10     |
|                                                                                                                     |
| PatchRecoveryErrorEstimator                                                                                         |
|   estimate_error()                      18        0.1469      0.008159    0.2291      0.012728    8.76     13.67    |
|                                                                                                                     |
| PetscLinearSolver                                                                                                   |
|   solve()                               21        0.4166      0.019839    0.4166      0.019839    24.86    24.86    |
|                                                                                                                     |
| ProjectVector                                                                                                       |
|   operator()                            18        0.0222      0.001231    0.0481      0.002671    1.32     2.87     |
|                                                                                                                     |
| System                                                                                                              |
|   project_vector()                      18        0.0108      0.000599    0.0681      0.003782    0.64     4.06     |
 ---------------------------------------------------------------------------------------------------------------------
| Totals:                                 208013    1.6759                                          100.00            |
 ---------------------------------------------------------------------------------------------------------------------

 
***************************************************************
* Done Running Example  ./adjoints_ex1-opt
***************************************************************
</pre>
</div>
<?php make_footer() ?>
</body>
</html>
<?php if (0) { ?>
\#Local Variables:
\#mode: html
\#End:
<?php } ?>
