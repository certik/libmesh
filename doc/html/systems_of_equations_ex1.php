<?php $root=""; ?>
<?php require($root."navigation.php"); ?>
<html>
<head>
  <?php load_style($root); ?>
</head>
 
<body>
 
<?php make_navigation("systems_of_equations_ex1",$root)?>
 
<div class="content">
<a name="comments"></a> 
<div class = "comment">
<h1>Systems Example 1 - Stokes Equations</h1>

<br><br>This example shows how a simple, linear system of equations
can be solved in parallel.  The system of equations are the familiar
Stokes equations for low-speed incompressible fluid flow.


<br><br>C++ include files that we need
</div>

<div class ="fragment">
<pre>
        #include &lt;iostream&gt;
        #include &lt;algorithm&gt;
        #include &lt;math.h&gt;
        
</pre>
</div>
<div class = "comment">
Basic include file needed for the mesh functionality.
</div>

<div class ="fragment">
<pre>
        #include "libmesh.h"
        #include "mesh.h"
        #include "mesh_generation.h"
        #include "exodusII_io.h"
        #include "equation_systems.h"
        #include "fe.h"
        #include "quadrature_gauss.h"
        #include "dof_map.h"
        #include "sparse_matrix.h"
        #include "numeric_vector.h"
        #include "dense_matrix.h"
        #include "dense_vector.h"
        #include "linear_implicit_system.h"
        
</pre>
</div>
<div class = "comment">
For systems of equations the \p DenseSubMatrix
and \p DenseSubVector provide convenient ways for
assembling the element matrix and vector on a
component-by-component basis.
</div>

<div class ="fragment">
<pre>
        #include "dense_submatrix.h"
        #include "dense_subvector.h"
        
</pre>
</div>
<div class = "comment">
The definition of a geometric element
</div>

<div class ="fragment">
<pre>
        #include "elem.h"
        
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
Function prototype.  This function will assemble the system
matrix and right-hand-side.
</div>

<div class ="fragment">
<pre>
        void assemble_stokes (EquationSystems& es,
                              const std::string& system_name);
        
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
Skip this 2D example if libMesh was compiled as 1D-only.
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
Use the MeshTools::Generation mesh generator to create a uniform
2D grid on the square [-1,1]^2.  We instruct the mesh generator
to build a mesh of 8x8 \p Quad9 elements.  Building these
higher-order elements allows us to use higher-order
approximation, as in example 3.
</div>

<div class ="fragment">
<pre>
          MeshTools::Generation::build_square (mesh,
                                               15, 15,
                                               0., 1.,
                                               0., 1.,
                                               QUAD9);
          
</pre>
</div>
<div class = "comment">
Print information about the mesh to the screen.
</div>

<div class ="fragment">
<pre>
          mesh.print_info();
          
</pre>
</div>
<div class = "comment">
Create an equation systems object.
</div>

<div class ="fragment">
<pre>
          EquationSystems equation_systems (mesh);
          
</pre>
</div>
<div class = "comment">
Declare the system and its variables.
Create a transient system named "Convection-Diffusion"
</div>

<div class ="fragment">
<pre>
          LinearImplicitSystem & system = 
            equation_systems.add_system&lt;LinearImplicitSystem&gt; ("Stokes");
          
</pre>
</div>
<div class = "comment">
Add the variables "u" & "v" to "Stokes".  They
will be approximated using second-order approximation.
</div>

<div class ="fragment">
<pre>
          system.add_variable ("u", SECOND);
          system.add_variable ("v", SECOND);
        
</pre>
</div>
<div class = "comment">
Add the variable "p" to "Stokes". This will
be approximated with a first-order basis,
providing an LBB-stable pressure-velocity pair.
</div>

<div class ="fragment">
<pre>
          system.add_variable ("p", FIRST);
        
</pre>
</div>
<div class = "comment">
Give the system a pointer to the matrix assembly
function.
</div>

<div class ="fragment">
<pre>
          system.attach_assemble_function (assemble_stokes);
          
</pre>
</div>
<div class = "comment">
Initialize the data structures for the equation system.
</div>

<div class ="fragment">
<pre>
          equation_systems.init ();
        
          equation_systems.parameters.set&lt;unsigned int&gt;("linear solver maximum iterations") = 250;
          equation_systems.parameters.set&lt;Real&gt;        ("linear solver tolerance") = TOLERANCE;
              
</pre>
</div>
<div class = "comment">
Prints information about the system to the screen.
</div>

<div class ="fragment">
<pre>
          equation_systems.print_info();
            
</pre>
</div>
<div class = "comment">
Assemble & solve the linear system,
then write the solution.
</div>

<div class ="fragment">
<pre>
          equation_systems.get_system("Stokes").solve();
        
        #ifdef LIBMESH_HAVE_EXODUS_API
          ExodusII_IO(mesh).write_equation_systems ("out.e",
                                              equation_systems);
        #endif // #ifdef LIBMESH_HAVE_EXODUS_API
        
</pre>
</div>
<div class = "comment">
All done.  
</div>

<div class ="fragment">
<pre>
          return 0;
        }
        
        void assemble_stokes (EquationSystems& es,
                              const std::string& system_name)
        {
</pre>
</div>
<div class = "comment">
It is a good idea to make sure we are assembling
the proper system.
</div>

<div class ="fragment">
<pre>
          libmesh_assert (system_name == "Stokes");
          
</pre>
</div>
<div class = "comment">
Get a constant reference to the mesh object.
</div>

<div class ="fragment">
<pre>
          const MeshBase& mesh = es.get_mesh();
          
</pre>
</div>
<div class = "comment">
The dimension that we are running
</div>

<div class ="fragment">
<pre>
          const unsigned int dim = mesh.mesh_dimension();
          
</pre>
</div>
<div class = "comment">
Get a reference to the Convection-Diffusion system object.
</div>

<div class ="fragment">
<pre>
          LinearImplicitSystem & system =
            es.get_system&lt;LinearImplicitSystem&gt; ("Stokes");
        
</pre>
</div>
<div class = "comment">
Numeric ids corresponding to each variable in the system
</div>

<div class ="fragment">
<pre>
          const unsigned int u_var = system.variable_number ("u");
          const unsigned int v_var = system.variable_number ("v");
          const unsigned int p_var = system.variable_number ("p");
          
</pre>
</div>
<div class = "comment">
Get the Finite Element type for "u".  Note this will be
the same as the type for "v".
</div>

<div class ="fragment">
<pre>
          FEType fe_vel_type = system.variable_type(u_var);
          
</pre>
</div>
<div class = "comment">
Get the Finite Element type for "p".
</div>

<div class ="fragment">
<pre>
          FEType fe_pres_type = system.variable_type(p_var);
        
</pre>
</div>
<div class = "comment">
Build a Finite Element object of the specified type for
the velocity variables.
</div>

<div class ="fragment">
<pre>
          AutoPtr&lt;FEBase&gt; fe_vel  (FEBase::build(dim, fe_vel_type));
            
</pre>
</div>
<div class = "comment">
Build a Finite Element object of the specified type for
the pressure variables.
</div>

<div class ="fragment">
<pre>
          AutoPtr&lt;FEBase&gt; fe_pres (FEBase::build(dim, fe_pres_type));
          
</pre>
</div>
<div class = "comment">
A Gauss quadrature rule for numerical integration.
Let the \p FEType object decide what order rule is appropriate.
</div>

<div class ="fragment">
<pre>
          QGauss qrule (dim, fe_vel_type.default_quadrature_order());
        
</pre>
</div>
<div class = "comment">
Tell the finite element objects to use our quadrature rule.
</div>

<div class ="fragment">
<pre>
          fe_vel-&gt;attach_quadrature_rule (&qrule);
          fe_pres-&gt;attach_quadrature_rule (&qrule);
          
</pre>
</div>
<div class = "comment">
Here we define some references to cell-specific data that
will be used to assemble the linear system.

<br><br>The element Jacobian * quadrature weight at each integration point.   
</div>

<div class ="fragment">
<pre>
          const std::vector&lt;Real&gt;& JxW = fe_vel-&gt;get_JxW();
          
</pre>
</div>
<div class = "comment">
The element shape function gradients for the velocity
variables evaluated at the quadrature points.
</div>

<div class ="fragment">
<pre>
          const std::vector&lt;std::vector&lt;RealGradient&gt; &gt;& dphi = fe_vel-&gt;get_dphi();
        
</pre>
</div>
<div class = "comment">
The element shape functions for the pressure variable
evaluated at the quadrature points.
</div>

<div class ="fragment">
<pre>
          const std::vector&lt;std::vector&lt;Real&gt; &gt;& psi = fe_pres-&gt;get_phi();
          
</pre>
</div>
<div class = "comment">
A reference to the \p DofMap object for this system.  The \p DofMap
object handles the index translation from node and element numbers
to degree of freedom numbers.  We will talk more about the \p DofMap
in future examples.
</div>

<div class ="fragment">
<pre>
          const DofMap & dof_map = system.get_dof_map();
        
</pre>
</div>
<div class = "comment">
Define data structures to contain the element matrix
and right-hand-side vector contribution.  Following
basic finite element terminology we will denote these
"Ke" and "Fe".
</div>

<div class ="fragment">
<pre>
          DenseMatrix&lt;Number&gt; Ke;
          DenseVector&lt;Number&gt; Fe;
        
          DenseSubMatrix&lt;Number&gt;
            Kuu(Ke), Kuv(Ke), Kup(Ke),
            Kvu(Ke), Kvv(Ke), Kvp(Ke),
            Kpu(Ke), Kpv(Ke), Kpp(Ke);
        
          DenseSubVector&lt;Number&gt;
            Fu(Fe),
            Fv(Fe),
            Fp(Fe);
        
</pre>
</div>
<div class = "comment">
This vector will hold the degree of freedom indices for
the element.  These define where in the global system
the element degrees of freedom get mapped.
</div>

<div class ="fragment">
<pre>
          std::vector&lt;unsigned int&gt; dof_indices;
          std::vector&lt;unsigned int&gt; dof_indices_u;
          std::vector&lt;unsigned int&gt; dof_indices_v;
          std::vector&lt;unsigned int&gt; dof_indices_p;
          
</pre>
</div>
<div class = "comment">
Now we will loop over all the elements in the mesh that
live on the local processor. We will compute the element
matrix and right-hand-side contribution.  In case users later
modify this program to include refinement, we will be safe and
will only consider the active elements; hence we use a variant of
the \p active_elem_iterator.


<br><br></div>

<div class ="fragment">
<pre>
          MeshBase::const_element_iterator       el     = mesh.active_local_elements_begin();
          const MeshBase::const_element_iterator end_el = mesh.active_local_elements_end(); 
          
          for ( ; el != end_el; ++el)
            {    
</pre>
</div>
<div class = "comment">
Store a pointer to the element we are currently
working on.  This allows for nicer syntax later.
</div>

<div class ="fragment">
<pre>
              const Elem* elem = *el;
              
</pre>
</div>
<div class = "comment">
Get the degree of freedom indices for the
current element.  These define where in the global
matrix and right-hand-side this element will
contribute to.
</div>

<div class ="fragment">
<pre>
              dof_map.dof_indices (elem, dof_indices);
              dof_map.dof_indices (elem, dof_indices_u, u_var);
              dof_map.dof_indices (elem, dof_indices_v, v_var);
              dof_map.dof_indices (elem, dof_indices_p, p_var);
        
              const unsigned int n_dofs   = dof_indices.size();
              const unsigned int n_u_dofs = dof_indices_u.size(); 
              const unsigned int n_v_dofs = dof_indices_v.size(); 
              const unsigned int n_p_dofs = dof_indices_p.size();
              
</pre>
</div>
<div class = "comment">
Compute the element-specific data for the current
element.  This involves computing the location of the
quadrature points (q_point) and the shape functions
(phi, dphi) for the current element.
</div>

<div class ="fragment">
<pre>
              fe_vel-&gt;reinit  (elem);
              fe_pres-&gt;reinit (elem);
        
</pre>
</div>
<div class = "comment">
Zero the element matrix and right-hand side before
summing them.  We use the resize member here because
the number of degrees of freedom might have changed from
the last element.  Note that this will be the case if the
element type is different (i.e. the last element was a
triangle, now we are on a quadrilateral).
</div>

<div class ="fragment">
<pre>
              Ke.resize (n_dofs, n_dofs);
              Fe.resize (n_dofs);
        
</pre>
</div>
<div class = "comment">
Reposition the submatrices...  The idea is this:

<br><br>-           -          -  -
| Kuu Kuv Kup |        | Fu |
Ke = | Kvu Kvv Kvp |;  Fe = | Fv |
| Kpu Kpv Kpp |        | Fp |
-           -          -  -

<br><br>The \p DenseSubMatrix.repostition () member takes the
(row_offset, column_offset, row_size, column_size).

<br><br>Similarly, the \p DenseSubVector.reposition () member
takes the (row_offset, row_size)
</div>

<div class ="fragment">
<pre>
              Kuu.reposition (u_var*n_u_dofs, u_var*n_u_dofs, n_u_dofs, n_u_dofs);
              Kuv.reposition (u_var*n_u_dofs, v_var*n_u_dofs, n_u_dofs, n_v_dofs);
              Kup.reposition (u_var*n_u_dofs, p_var*n_u_dofs, n_u_dofs, n_p_dofs);
              
              Kvu.reposition (v_var*n_v_dofs, u_var*n_v_dofs, n_v_dofs, n_u_dofs);
              Kvv.reposition (v_var*n_v_dofs, v_var*n_v_dofs, n_v_dofs, n_v_dofs);
              Kvp.reposition (v_var*n_v_dofs, p_var*n_v_dofs, n_v_dofs, n_p_dofs);
              
              Kpu.reposition (p_var*n_u_dofs, u_var*n_u_dofs, n_p_dofs, n_u_dofs);
              Kpv.reposition (p_var*n_u_dofs, v_var*n_u_dofs, n_p_dofs, n_v_dofs);
              Kpp.reposition (p_var*n_u_dofs, p_var*n_u_dofs, n_p_dofs, n_p_dofs);
        
              Fu.reposition (u_var*n_u_dofs, n_u_dofs);
              Fv.reposition (v_var*n_u_dofs, n_v_dofs);
              Fp.reposition (p_var*n_u_dofs, n_p_dofs);
              
</pre>
</div>
<div class = "comment">
Now we will build the element matrix.
</div>

<div class ="fragment">
<pre>
              for (unsigned int qp=0; qp&lt;qrule.n_points(); qp++)
                {
</pre>
</div>
<div class = "comment">
Assemble the u-velocity row
uu coupling
</div>

<div class ="fragment">
<pre>
                  for (unsigned int i=0; i&lt;n_u_dofs; i++)
                    for (unsigned int j=0; j&lt;n_u_dofs; j++)
                      Kuu(i,j) += JxW[qp]*(dphi[i][qp]*dphi[j][qp]);
        
</pre>
</div>
<div class = "comment">
up coupling
</div>

<div class ="fragment">
<pre>
                  for (unsigned int i=0; i&lt;n_u_dofs; i++)
                    for (unsigned int j=0; j&lt;n_p_dofs; j++)
                      Kup(i,j) += -JxW[qp]*psi[j][qp]*dphi[i][qp](0);
        
        
</pre>
</div>
<div class = "comment">
Assemble the v-velocity row
vv coupling
</div>

<div class ="fragment">
<pre>
                  for (unsigned int i=0; i&lt;n_v_dofs; i++)
                    for (unsigned int j=0; j&lt;n_v_dofs; j++)
                      Kvv(i,j) += JxW[qp]*(dphi[i][qp]*dphi[j][qp]);
        
</pre>
</div>
<div class = "comment">
vp coupling
</div>

<div class ="fragment">
<pre>
                  for (unsigned int i=0; i&lt;n_v_dofs; i++)
                    for (unsigned int j=0; j&lt;n_p_dofs; j++)
                      Kvp(i,j) += -JxW[qp]*psi[j][qp]*dphi[i][qp](1);
        
                  
</pre>
</div>
<div class = "comment">
Assemble the pressure row
pu coupling
</div>

<div class ="fragment">
<pre>
                  for (unsigned int i=0; i&lt;n_p_dofs; i++)
                    for (unsigned int j=0; j&lt;n_u_dofs; j++)
                      Kpu(i,j) += -JxW[qp]*psi[i][qp]*dphi[j][qp](0);
        
</pre>
</div>
<div class = "comment">
pv coupling
</div>

<div class ="fragment">
<pre>
                  for (unsigned int i=0; i&lt;n_p_dofs; i++)
                    for (unsigned int j=0; j&lt;n_v_dofs; j++)
                      Kpv(i,j) += -JxW[qp]*psi[i][qp]*dphi[j][qp](1);
                  
                } // end of the quadrature point qp-loop
        
</pre>
</div>
<div class = "comment">
At this point the interior element integration has
been completed.  However, we have not yet addressed
boundary conditions.  For this example we will only
consider simple Dirichlet boundary conditions imposed
via the penalty method. The penalty method used here
is equivalent (for Lagrange basis functions) to lumping
the matrix resulting from the L2 projection penalty
approach introduced in example 3.
</div>

<div class ="fragment">
<pre>
              {
</pre>
</div>
<div class = "comment">
The following loops over the sides of the element.
If the element has no neighbor on a side then that
side MUST live on a boundary of the domain.
</div>

<div class ="fragment">
<pre>
                for (unsigned int s=0; s&lt;elem-&gt;n_sides(); s++)
                  if (elem-&gt;neighbor(s) == NULL)
                    {
                      AutoPtr&lt;Elem&gt; side (elem-&gt;build_side(s));
                                    
</pre>
</div>
<div class = "comment">
Loop over the nodes on the side.
</div>

<div class ="fragment">
<pre>
                      for (unsigned int ns=0; ns&lt;side-&gt;n_nodes(); ns++)
                        {
</pre>
</div>
<div class = "comment">
The location on the boundary of the current
node.
                   

<br><br>const Real xf = side->point(ns)(0);
</div>

<div class ="fragment">
<pre>
                          const Real yf = side-&gt;point(ns)(1);
                          
</pre>
</div>
<div class = "comment">
The penalty value.  \f$ \frac{1}{\epsilon \f$
</div>

<div class ="fragment">
<pre>
                          const Real penalty = 1.e10;
                          
</pre>
</div>
<div class = "comment">
The boundary values.
                   

<br><br>Set u = 1 on the top boundary, 0 everywhere else
</div>

<div class ="fragment">
<pre>
                          const Real u_value = (yf &gt; .99) ? 1. : 0.;
                          
</pre>
</div>
<div class = "comment">
Set v = 0 everywhere
</div>

<div class ="fragment">
<pre>
                          const Real v_value = 0.;
                          
</pre>
</div>
<div class = "comment">
Find the node on the element matching this node on
the side.  That defined where in the element matrix
the boundary condition will be applied.
</div>

<div class ="fragment">
<pre>
                          for (unsigned int n=0; n&lt;elem-&gt;n_nodes(); n++)
                            if (elem-&gt;node(n) == side-&gt;node(ns))
                              {
</pre>
</div>
<div class = "comment">
Matrix contribution.
</div>

<div class ="fragment">
<pre>
                                Kuu(n,n) += penalty;
                                Kvv(n,n) += penalty;
                                            
</pre>
</div>
<div class = "comment">
Right-hand-side contribution.
</div>

<div class ="fragment">
<pre>
                                Fu(n) += penalty*u_value;
                                Fv(n) += penalty*v_value;
                              }
                        } // end face node loop          
                    } // end if (elem-&gt;neighbor(side) == NULL)
              } // end boundary condition section          
              
</pre>
</div>
<div class = "comment">
If this assembly program were to be used on an adaptive mesh,
we would have to apply any hanging node constraint equations.
</div>

<div class ="fragment">
<pre>
              dof_map.constrain_element_matrix_and_vector (Ke, Fe, dof_indices);
        
</pre>
</div>
<div class = "comment">
The element matrix and right-hand-side are now built
for this element.  Add them to the global matrix and
right-hand-side vector.  The \p NumericMatrix::add_matrix()
and \p NumericVector::add_vector() members do this for us.
</div>

<div class ="fragment">
<pre>
              system.matrix-&gt;add_matrix (Ke, dof_indices);
              system.rhs-&gt;add_vector    (Fe, dof_indices);
            } // end of element loop
          
</pre>
</div>
<div class = "comment">
That's it.
</div>

<div class ="fragment">
<pre>
          return;
        }
</pre>
</div>

<a name="nocomments"></a> 
<br><br><br> <h1> The program without comments: </h1> 
<pre> 
  
  #include &lt;iostream&gt;
  #include &lt;algorithm&gt;
  #include &lt;math.h&gt;
  
  #include <B><FONT COLOR="#BC8F8F">&quot;libmesh.h&quot;</FONT></B>
  #include <B><FONT COLOR="#BC8F8F">&quot;mesh.h&quot;</FONT></B>
  #include <B><FONT COLOR="#BC8F8F">&quot;mesh_generation.h&quot;</FONT></B>
  #include <B><FONT COLOR="#BC8F8F">&quot;exodusII_io.h&quot;</FONT></B>
  #include <B><FONT COLOR="#BC8F8F">&quot;equation_systems.h&quot;</FONT></B>
  #include <B><FONT COLOR="#BC8F8F">&quot;fe.h&quot;</FONT></B>
  #include <B><FONT COLOR="#BC8F8F">&quot;quadrature_gauss.h&quot;</FONT></B>
  #include <B><FONT COLOR="#BC8F8F">&quot;dof_map.h&quot;</FONT></B>
  #include <B><FONT COLOR="#BC8F8F">&quot;sparse_matrix.h&quot;</FONT></B>
  #include <B><FONT COLOR="#BC8F8F">&quot;numeric_vector.h&quot;</FONT></B>
  #include <B><FONT COLOR="#BC8F8F">&quot;dense_matrix.h&quot;</FONT></B>
  #include <B><FONT COLOR="#BC8F8F">&quot;dense_vector.h&quot;</FONT></B>
  #include <B><FONT COLOR="#BC8F8F">&quot;linear_implicit_system.h&quot;</FONT></B>
  
  #include <B><FONT COLOR="#BC8F8F">&quot;dense_submatrix.h&quot;</FONT></B>
  #include <B><FONT COLOR="#BC8F8F">&quot;dense_subvector.h&quot;</FONT></B>
  
  #include <B><FONT COLOR="#BC8F8F">&quot;elem.h&quot;</FONT></B>
  
  using namespace libMesh;
  
  <B><FONT COLOR="#228B22">void</FONT></B> assemble_stokes (EquationSystems&amp; es,
                        <B><FONT COLOR="#228B22">const</FONT></B> std::string&amp; system_name);
  
  <B><FONT COLOR="#228B22">int</FONT></B> main (<B><FONT COLOR="#228B22">int</FONT></B> argc, <B><FONT COLOR="#228B22">char</FONT></B>** argv)
  {
    LibMeshInit init (argc, argv);
  
    libmesh_example_assert(2 &lt;= LIBMESH_DIM, <B><FONT COLOR="#BC8F8F">&quot;2D support&quot;</FONT></B>);
      
    Mesh mesh;
      
    <B><FONT COLOR="#5F9EA0">MeshTools</FONT></B>::Generation::build_square (mesh,
                                         15, 15,
                                         0., 1.,
                                         0., 1.,
                                         QUAD9);
    
    mesh.print_info();
    
    EquationSystems equation_systems (mesh);
    
    LinearImplicitSystem &amp; system = 
      equation_systems.add_system&lt;LinearImplicitSystem&gt; (<B><FONT COLOR="#BC8F8F">&quot;Stokes&quot;</FONT></B>);
    
    system.add_variable (<B><FONT COLOR="#BC8F8F">&quot;u&quot;</FONT></B>, SECOND);
    system.add_variable (<B><FONT COLOR="#BC8F8F">&quot;v&quot;</FONT></B>, SECOND);
  
    system.add_variable (<B><FONT COLOR="#BC8F8F">&quot;p&quot;</FONT></B>, FIRST);
  
    system.attach_assemble_function (assemble_stokes);
    
    equation_systems.init ();
  
    equation_systems.parameters.set&lt;<B><FONT COLOR="#228B22">unsigned</FONT></B> <B><FONT COLOR="#228B22">int</FONT></B>&gt;(<B><FONT COLOR="#BC8F8F">&quot;linear solver maximum iterations&quot;</FONT></B>) = 250;
    equation_systems.parameters.set&lt;Real&gt;        (<B><FONT COLOR="#BC8F8F">&quot;linear solver tolerance&quot;</FONT></B>) = TOLERANCE;
        
    equation_systems.print_info();
      
    equation_systems.get_system(<B><FONT COLOR="#BC8F8F">&quot;Stokes&quot;</FONT></B>).solve();
  
  #ifdef LIBMESH_HAVE_EXODUS_API
    ExodusII_IO(mesh).write_equation_systems (<B><FONT COLOR="#BC8F8F">&quot;out.e&quot;</FONT></B>,
                                        equation_systems);
  #endif <I><FONT COLOR="#B22222">// #ifdef LIBMESH_HAVE_EXODUS_API
</FONT></I>  
    <B><FONT COLOR="#A020F0">return</FONT></B> 0;
  }
  
  <B><FONT COLOR="#228B22">void</FONT></B> assemble_stokes (EquationSystems&amp; es,
                        <B><FONT COLOR="#228B22">const</FONT></B> std::string&amp; system_name)
  {
    libmesh_assert (system_name == <B><FONT COLOR="#BC8F8F">&quot;Stokes&quot;</FONT></B>);
    
    <B><FONT COLOR="#228B22">const</FONT></B> MeshBase&amp; mesh = es.get_mesh();
    
    <B><FONT COLOR="#228B22">const</FONT></B> <B><FONT COLOR="#228B22">unsigned</FONT></B> <B><FONT COLOR="#228B22">int</FONT></B> dim = mesh.mesh_dimension();
    
    LinearImplicitSystem &amp; system =
      es.get_system&lt;LinearImplicitSystem&gt; (<B><FONT COLOR="#BC8F8F">&quot;Stokes&quot;</FONT></B>);
  
    <B><FONT COLOR="#228B22">const</FONT></B> <B><FONT COLOR="#228B22">unsigned</FONT></B> <B><FONT COLOR="#228B22">int</FONT></B> u_var = system.variable_number (<B><FONT COLOR="#BC8F8F">&quot;u&quot;</FONT></B>);
    <B><FONT COLOR="#228B22">const</FONT></B> <B><FONT COLOR="#228B22">unsigned</FONT></B> <B><FONT COLOR="#228B22">int</FONT></B> v_var = system.variable_number (<B><FONT COLOR="#BC8F8F">&quot;v&quot;</FONT></B>);
    <B><FONT COLOR="#228B22">const</FONT></B> <B><FONT COLOR="#228B22">unsigned</FONT></B> <B><FONT COLOR="#228B22">int</FONT></B> p_var = system.variable_number (<B><FONT COLOR="#BC8F8F">&quot;p&quot;</FONT></B>);
    
    FEType fe_vel_type = system.variable_type(u_var);
    
    FEType fe_pres_type = system.variable_type(p_var);
  
    AutoPtr&lt;FEBase&gt; fe_vel  (FEBase::build(dim, fe_vel_type));
      
    AutoPtr&lt;FEBase&gt; fe_pres (FEBase::build(dim, fe_pres_type));
    
    QGauss qrule (dim, fe_vel_type.default_quadrature_order());
  
    fe_vel-&gt;attach_quadrature_rule (&amp;qrule);
    fe_pres-&gt;attach_quadrature_rule (&amp;qrule);
    
    <B><FONT COLOR="#228B22">const</FONT></B> std::vector&lt;Real&gt;&amp; JxW = fe_vel-&gt;get_JxW();
    
    <B><FONT COLOR="#228B22">const</FONT></B> std::vector&lt;std::vector&lt;RealGradient&gt; &gt;&amp; dphi = fe_vel-&gt;get_dphi();
  
    <B><FONT COLOR="#228B22">const</FONT></B> std::vector&lt;std::vector&lt;Real&gt; &gt;&amp; psi = fe_pres-&gt;get_phi();
    
    <B><FONT COLOR="#228B22">const</FONT></B> DofMap &amp; dof_map = system.get_dof_map();
  
    DenseMatrix&lt;Number&gt; Ke;
    DenseVector&lt;Number&gt; Fe;
  
    DenseSubMatrix&lt;Number&gt;
      Kuu(Ke), Kuv(Ke), Kup(Ke),
      Kvu(Ke), Kvv(Ke), Kvp(Ke),
      Kpu(Ke), Kpv(Ke), Kpp(Ke);
  
    DenseSubVector&lt;Number&gt;
      Fu(Fe),
      Fv(Fe),
      Fp(Fe);
  
    <B><FONT COLOR="#5F9EA0">std</FONT></B>::vector&lt;<B><FONT COLOR="#228B22">unsigned</FONT></B> <B><FONT COLOR="#228B22">int</FONT></B>&gt; dof_indices;
    <B><FONT COLOR="#5F9EA0">std</FONT></B>::vector&lt;<B><FONT COLOR="#228B22">unsigned</FONT></B> <B><FONT COLOR="#228B22">int</FONT></B>&gt; dof_indices_u;
    <B><FONT COLOR="#5F9EA0">std</FONT></B>::vector&lt;<B><FONT COLOR="#228B22">unsigned</FONT></B> <B><FONT COLOR="#228B22">int</FONT></B>&gt; dof_indices_v;
    <B><FONT COLOR="#5F9EA0">std</FONT></B>::vector&lt;<B><FONT COLOR="#228B22">unsigned</FONT></B> <B><FONT COLOR="#228B22">int</FONT></B>&gt; dof_indices_p;
    
  
    <B><FONT COLOR="#5F9EA0">MeshBase</FONT></B>::const_element_iterator       el     = mesh.active_local_elements_begin();
    <B><FONT COLOR="#228B22">const</FONT></B> MeshBase::const_element_iterator end_el = mesh.active_local_elements_end(); 
    
    <B><FONT COLOR="#A020F0">for</FONT></B> ( ; el != end_el; ++el)
      {    
        <B><FONT COLOR="#228B22">const</FONT></B> Elem* elem = *el;
        
        dof_map.dof_indices (elem, dof_indices);
        dof_map.dof_indices (elem, dof_indices_u, u_var);
        dof_map.dof_indices (elem, dof_indices_v, v_var);
        dof_map.dof_indices (elem, dof_indices_p, p_var);
  
        <B><FONT COLOR="#228B22">const</FONT></B> <B><FONT COLOR="#228B22">unsigned</FONT></B> <B><FONT COLOR="#228B22">int</FONT></B> n_dofs   = dof_indices.size();
        <B><FONT COLOR="#228B22">const</FONT></B> <B><FONT COLOR="#228B22">unsigned</FONT></B> <B><FONT COLOR="#228B22">int</FONT></B> n_u_dofs = dof_indices_u.size(); 
        <B><FONT COLOR="#228B22">const</FONT></B> <B><FONT COLOR="#228B22">unsigned</FONT></B> <B><FONT COLOR="#228B22">int</FONT></B> n_v_dofs = dof_indices_v.size(); 
        <B><FONT COLOR="#228B22">const</FONT></B> <B><FONT COLOR="#228B22">unsigned</FONT></B> <B><FONT COLOR="#228B22">int</FONT></B> n_p_dofs = dof_indices_p.size();
        
        fe_vel-&gt;reinit  (elem);
        fe_pres-&gt;reinit (elem);
  
        Ke.resize (n_dofs, n_dofs);
        Fe.resize (n_dofs);
  
        Kuu.reposition (u_var*n_u_dofs, u_var*n_u_dofs, n_u_dofs, n_u_dofs);
        Kuv.reposition (u_var*n_u_dofs, v_var*n_u_dofs, n_u_dofs, n_v_dofs);
        Kup.reposition (u_var*n_u_dofs, p_var*n_u_dofs, n_u_dofs, n_p_dofs);
        
        Kvu.reposition (v_var*n_v_dofs, u_var*n_v_dofs, n_v_dofs, n_u_dofs);
        Kvv.reposition (v_var*n_v_dofs, v_var*n_v_dofs, n_v_dofs, n_v_dofs);
        Kvp.reposition (v_var*n_v_dofs, p_var*n_v_dofs, n_v_dofs, n_p_dofs);
        
        Kpu.reposition (p_var*n_u_dofs, u_var*n_u_dofs, n_p_dofs, n_u_dofs);
        Kpv.reposition (p_var*n_u_dofs, v_var*n_u_dofs, n_p_dofs, n_v_dofs);
        Kpp.reposition (p_var*n_u_dofs, p_var*n_u_dofs, n_p_dofs, n_p_dofs);
  
        Fu.reposition (u_var*n_u_dofs, n_u_dofs);
        Fv.reposition (v_var*n_u_dofs, n_v_dofs);
        Fp.reposition (p_var*n_u_dofs, n_p_dofs);
        
        <B><FONT COLOR="#A020F0">for</FONT></B> (<B><FONT COLOR="#228B22">unsigned</FONT></B> <B><FONT COLOR="#228B22">int</FONT></B> qp=0; qp&lt;qrule.n_points(); qp++)
          {
            <B><FONT COLOR="#A020F0">for</FONT></B> (<B><FONT COLOR="#228B22">unsigned</FONT></B> <B><FONT COLOR="#228B22">int</FONT></B> i=0; i&lt;n_u_dofs; i++)
              <B><FONT COLOR="#A020F0">for</FONT></B> (<B><FONT COLOR="#228B22">unsigned</FONT></B> <B><FONT COLOR="#228B22">int</FONT></B> j=0; j&lt;n_u_dofs; j++)
                Kuu(i,j) += JxW[qp]*(dphi[i][qp]*dphi[j][qp]);
  
            <B><FONT COLOR="#A020F0">for</FONT></B> (<B><FONT COLOR="#228B22">unsigned</FONT></B> <B><FONT COLOR="#228B22">int</FONT></B> i=0; i&lt;n_u_dofs; i++)
              <B><FONT COLOR="#A020F0">for</FONT></B> (<B><FONT COLOR="#228B22">unsigned</FONT></B> <B><FONT COLOR="#228B22">int</FONT></B> j=0; j&lt;n_p_dofs; j++)
                Kup(i,j) += -JxW[qp]*psi[j][qp]*dphi[i][qp](0);
  
  
            <B><FONT COLOR="#A020F0">for</FONT></B> (<B><FONT COLOR="#228B22">unsigned</FONT></B> <B><FONT COLOR="#228B22">int</FONT></B> i=0; i&lt;n_v_dofs; i++)
              <B><FONT COLOR="#A020F0">for</FONT></B> (<B><FONT COLOR="#228B22">unsigned</FONT></B> <B><FONT COLOR="#228B22">int</FONT></B> j=0; j&lt;n_v_dofs; j++)
                Kvv(i,j) += JxW[qp]*(dphi[i][qp]*dphi[j][qp]);
  
            <B><FONT COLOR="#A020F0">for</FONT></B> (<B><FONT COLOR="#228B22">unsigned</FONT></B> <B><FONT COLOR="#228B22">int</FONT></B> i=0; i&lt;n_v_dofs; i++)
              <B><FONT COLOR="#A020F0">for</FONT></B> (<B><FONT COLOR="#228B22">unsigned</FONT></B> <B><FONT COLOR="#228B22">int</FONT></B> j=0; j&lt;n_p_dofs; j++)
                Kvp(i,j) += -JxW[qp]*psi[j][qp]*dphi[i][qp](1);
  
            
            <B><FONT COLOR="#A020F0">for</FONT></B> (<B><FONT COLOR="#228B22">unsigned</FONT></B> <B><FONT COLOR="#228B22">int</FONT></B> i=0; i&lt;n_p_dofs; i++)
              <B><FONT COLOR="#A020F0">for</FONT></B> (<B><FONT COLOR="#228B22">unsigned</FONT></B> <B><FONT COLOR="#228B22">int</FONT></B> j=0; j&lt;n_u_dofs; j++)
                Kpu(i,j) += -JxW[qp]*psi[i][qp]*dphi[j][qp](0);
  
            <B><FONT COLOR="#A020F0">for</FONT></B> (<B><FONT COLOR="#228B22">unsigned</FONT></B> <B><FONT COLOR="#228B22">int</FONT></B> i=0; i&lt;n_p_dofs; i++)
              <B><FONT COLOR="#A020F0">for</FONT></B> (<B><FONT COLOR="#228B22">unsigned</FONT></B> <B><FONT COLOR="#228B22">int</FONT></B> j=0; j&lt;n_v_dofs; j++)
                Kpv(i,j) += -JxW[qp]*psi[i][qp]*dphi[j][qp](1);
            
          } <I><FONT COLOR="#B22222">// end of the quadrature point qp-loop
</FONT></I>  
        {
          <B><FONT COLOR="#A020F0">for</FONT></B> (<B><FONT COLOR="#228B22">unsigned</FONT></B> <B><FONT COLOR="#228B22">int</FONT></B> s=0; s&lt;elem-&gt;n_sides(); s++)
            <B><FONT COLOR="#A020F0">if</FONT></B> (elem-&gt;neighbor(s) == NULL)
              {
                AutoPtr&lt;Elem&gt; side (elem-&gt;build_side(s));
                              
                <B><FONT COLOR="#A020F0">for</FONT></B> (<B><FONT COLOR="#228B22">unsigned</FONT></B> <B><FONT COLOR="#228B22">int</FONT></B> ns=0; ns&lt;side-&gt;n_nodes(); ns++)
                  {
                     
                    <B><FONT COLOR="#228B22">const</FONT></B> Real yf = side-&gt;point(ns)(1);
                    
                    <B><FONT COLOR="#228B22">const</FONT></B> Real penalty = 1.e10;
                    
                     
                    <B><FONT COLOR="#228B22">const</FONT></B> Real u_value = (yf &gt; .99) ? 1. : 0.;
                    
                    <B><FONT COLOR="#228B22">const</FONT></B> Real v_value = 0.;
                    
                    <B><FONT COLOR="#A020F0">for</FONT></B> (<B><FONT COLOR="#228B22">unsigned</FONT></B> <B><FONT COLOR="#228B22">int</FONT></B> n=0; n&lt;elem-&gt;n_nodes(); n++)
                      <B><FONT COLOR="#A020F0">if</FONT></B> (elem-&gt;node(n) == side-&gt;node(ns))
                        {
                          Kuu(n,n) += penalty;
                          Kvv(n,n) += penalty;
                                      
                          Fu(n) += penalty*u_value;
                          Fv(n) += penalty*v_value;
                        }
                  } <I><FONT COLOR="#B22222">// end face node loop          
</FONT></I>              } <I><FONT COLOR="#B22222">// end if (elem-&gt;neighbor(side) == NULL)
</FONT></I>        } <I><FONT COLOR="#B22222">// end boundary condition section          
</FONT></I>        
        dof_map.constrain_element_matrix_and_vector (Ke, Fe, dof_indices);
  
        system.matrix-&gt;add_matrix (Ke, dof_indices);
        system.rhs-&gt;add_vector    (Fe, dof_indices);
      } <I><FONT COLOR="#B22222">// end of element loop
</FONT></I>    
    <B><FONT COLOR="#A020F0">return</FONT></B>;
  }
</pre> 
<a name="output"></a> 
<br><br><br> <h1> The console output of the program: </h1> 
<pre>
Compiling C++ (in optimized mode) systems_of_equations_ex1.C...
Linking systems_of_equations_ex1-opt...
***************************************************************
* Running Example  ./systems_of_equations_ex1-opt
***************************************************************
 
 Mesh Information:
  mesh_dimension()=2
  spatial_dimension()=3
  n_nodes()=961
    n_local_nodes()=961
  n_elem()=225
    n_local_elem()=225
    n_active_elem()=225
  n_subdomains()=1
  n_partitions()=1
  n_processors()=1
  n_threads()=1
  processor_id()=0

 EquationSystems
  n_systems()=1
   System #0, "Stokes"
    Type "LinearImplicit"
    Variables="u" "v" "p" 
    Finite Element Types="LAGRANGE", "JACOBI_20_00" "LAGRANGE", "JACOBI_20_00" "LAGRANGE", "JACOBI_20_00" 
    Infinite Element Mapping="CARTESIAN" "CARTESIAN" "CARTESIAN" 
    Approximation Orders="SECOND", "THIRD" "SECOND", "THIRD" "FIRST", "THIRD" 
    n_dofs()=2178
    n_local_dofs()=2178
    n_constrained_dofs()=0
    n_local_constrained_dofs()=0
    n_vectors()=1
    n_matrices()=1
    DofMap Sparsity
      Average  On-Processor Bandwidth <= 38.4683
      Average Off-Processor Bandwidth <= 0
      Maximum  On-Processor Bandwidth <= 59
      Maximum Off-Processor Bandwidth <= 0
    DofMap Constraints
      Number of DoF Constraints = 0
      Number of Node Constraints = 0


-------------------------------------------------------------------
| Time:           Sat Apr  7 16:03:29 2012                         |
| OS:             Linux                                            |
| HostName:       lkirk-home                                       |
| OS Release:     3.0.0-17-generic                                 |
| OS Version:     #30-Ubuntu SMP Thu Mar 8 20:45:39 UTC 2012       |
| Machine:        x86_64                                           |
| Username:       benkirk                                          |
| Configuration:  ./configure run on Sat Apr  7 15:49:27 CDT 2012  |
-------------------------------------------------------------------
 ------------------------------------------------------------------------------------------------------------
| libMesh Performance: Alive time=0.150438, Active time=0.078755                                             |
 ------------------------------------------------------------------------------------------------------------
| Event                          nCalls    Total Time  Avg Time    Total Time  Avg Time    % of Active Time  |
|                                          w/o Sub     w/o Sub     With Sub    With Sub    w/o S    With S   |
|------------------------------------------------------------------------------------------------------------|
|                                                                                                            |
|                                                                                                            |
| DofMap                                                                                                     |
|   add_neighbors_to_send_list() 1         0.0004      0.000381    0.0004      0.000381    0.48     0.48     |
|   compute_sparsity()           1         0.0057      0.005687    0.0065      0.006499    7.22     8.25     |
|   create_dof_constraints()     1         0.0002      0.000235    0.0002      0.000235    0.30     0.30     |
|   distribute_dofs()            1         0.0011      0.001149    0.0032      0.003187    1.46     4.05     |
|   dof_indices()                1800      0.0036      0.000002    0.0036      0.000002    4.53     4.53     |
|   prepare_send_list()          1         0.0000      0.000001    0.0000      0.000001    0.00     0.00     |
|   reinit()                     1         0.0020      0.002036    0.0020      0.002036    2.59     2.59     |
|                                                                                                            |
| EquationSystems                                                                                            |
|   build_solution_vector()      1         0.0009      0.000931    0.0015      0.001464    1.18     1.86     |
|                                                                                                            |
| ExodusII_IO                                                                                                |
|   write_nodal_data()           1         0.0017      0.001714    0.0017      0.001714    2.18     2.18     |
|                                                                                                            |
| FE                                                                                                         |
|   compute_affine_map()         450       0.0008      0.000002    0.0008      0.000002    1.00     1.00     |
|   compute_shape_functions()    450       0.0003      0.000001    0.0003      0.000001    0.39     0.39     |
|   init_shape_functions()       2         0.0001      0.000067    0.0001      0.000067    0.17     0.17     |
|                                                                                                            |
| Mesh                                                                                                       |
|   find_neighbors()             1         0.0009      0.000919    0.0009      0.000919    1.17     1.17     |
|   renumber_nodes_and_elem()    2         0.0001      0.000066    0.0001      0.000066    0.17     0.17     |
|                                                                                                            |
| MeshOutput                                                                                                 |
|   write_equation_systems()     1         0.0000      0.000037    0.0032      0.003217    0.05     4.08     |
|                                                                                                            |
| MeshTools::Generation                                                                                      |
|   build_cube()                 1         0.0011      0.001089    0.0011      0.001089    1.38     1.38     |
|                                                                                                            |
| Parallel                                                                                                   |
|   allgather()                  1         0.0000      0.000001    0.0000      0.000001    0.00     0.00     |
|                                                                                                            |
| Partitioner                                                                                                |
|   single_partition()           1         0.0001      0.000109    0.0001      0.000109    0.14     0.14     |
|                                                                                                            |
| PetscLinearSolver                                                                                          |
|   solve()                      1         0.0482      0.048209    0.0482      0.048209    61.21    61.21    |
|                                                                                                            |
| System                                                                                                     |
|   assemble()                   1         0.0113      0.011324    0.0152      0.015221    14.38    19.33    |
 ------------------------------------------------------------------------------------------------------------
| Totals:                        2719      0.0788                                          100.00            |
 ------------------------------------------------------------------------------------------------------------

 
***************************************************************
* Done Running Example  ./systems_of_equations_ex1-opt
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
