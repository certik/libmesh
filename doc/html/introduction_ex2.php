<?php $root=""; ?>
<?php require($root."navigation.php"); ?>
<html>
<head>
  <?php load_style($root); ?>
</head>
 
<body>
 
<?php make_navigation("introduction_ex2",$root)?>
 
<div class="content">
<a name="comments"></a> 
<div class = "comment">
<h1>Introduction Example 2 - Defining a Simple System</h1>

<br><br>This is the second example program.  It demonstrates how to
create an equation system for a simple scalar system.  This
example will also introduce some of the issues involved with using PETSc
in your application.

<br><br>This is the first example program that indirectly
uses the PETSc library.  By default equation data is stored
in PETSc vectors, which may span multiple processors.  Before
PETSc is used it must be initialized via libMesh::init().  Note that
by passing argc and argv to PETSc you may specify
command line arguments to PETSc.  For example, you might
try running this example as:

<br><br>./ex2 -log_info

<br><br>to see what PETSc is doing behind the scenes or

<br><br>./ex2 -log_summary

<br><br>to get a summary of what PETSc did.
Among other things, libMesh::init() initializes the MPI
communications library and PETSc numeric library on your system if
you haven't already done so.


<br><br>C++ include files that we need
</div>

<div class ="fragment">
<pre>
        #include &lt;iostream&gt;
</pre>
</div>
<div class = "comment">
Basic include file needed for the mesh functionality.
</div>

<div class ="fragment">
<pre>
        #include "libmesh.h"
        #include "mesh.h"
</pre>
</div>
<div class = "comment">
Include file that defines various mesh generation utilities
</div>

<div class ="fragment">
<pre>
        #include "mesh_generation.h"
</pre>
</div>
<div class = "comment">
Include file that defines (possibly multiple) systems of equations.
</div>

<div class ="fragment">
<pre>
        #include "equation_systems.h"
</pre>
</div>
<div class = "comment">
Include files that define a simple steady system
</div>

<div class ="fragment">
<pre>
        #include "linear_implicit_system.h"
        #include "transient_system.h"
        #include "explicit_system.h"
        
</pre>
</div>
<div class = "comment">
Bring in everything from the libMesh namespace
</div>

<div class ="fragment">
<pre>
        using namespace libMesh;
        
        
        
        int main (int argc, char** argv)
        {
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
A brief message to the user to inform her of the
exact name of the program being run, and its command line.
</div>

<div class ="fragment">
<pre>
          std::cout &lt;&lt; "Running " &lt;&lt; argv[0];
          for (int i=1; i&lt;argc; i++)
            std::cout &lt;&lt; " " &lt;&lt; argv[i];
          std::cout &lt;&lt; std::endl &lt;&lt; std::endl;
          
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
2D grid on the unit square.  By default a mesh of QUAD4
elements will be created.  We instruct the mesh generator
to build a mesh of 5x5 elements.
</div>

<div class ="fragment">
<pre>
          MeshTools::Generation::build_square (mesh, 5, 5);
        
</pre>
</div>
<div class = "comment">
Create an equation systems object. This object can
contain multiple systems of different 
flavors for solving loosely coupled physics.  Each system can 
contain multiple variables of different approximation orders.  
Here we will simply create a single system with one variable.  
Later on, other flavors of systems will be introduced.  For the 
moment, we use the general system.
The EquationSystems object needs a reference to the mesh
object, so the order of construction here is important.
</div>

<div class ="fragment">
<pre>
          EquationSystems equation_systems (mesh);
          
</pre>
</div>
<div class = "comment">
Add a flag "test" that is visible for all systems.  This
helps in inter-system communication.
</div>

<div class ="fragment">
<pre>
          equation_systems.parameters.set&lt;bool&gt; ("test") = true;
            
</pre>
</div>
<div class = "comment">
Set a simulation-specific parameter visible for all systems.
This helps in inter-system-communication.
</div>

<div class ="fragment">
<pre>
          equation_systems.parameters.set&lt;Real&gt; ("dummy") = 42.;
            
</pre>
</div>
<div class = "comment">
Set another simulation-specific parameter 
</div>

<div class ="fragment">
<pre>
          equation_systems.parameters.set&lt;Real&gt; ("nobody") = 0.;
          
</pre>
</div>
<div class = "comment">
Now we declare the system and its variables.
We begin by adding a "TransientLinearImplicitSystem" to the
EquationSystems object, and we give it the name
"Simple System".
</div>

<div class ="fragment">
<pre>
          equation_systems.add_system&lt;TransientLinearImplicitSystem&gt; ("Simple System");
            
</pre>
</div>
<div class = "comment">
Adds the variable "u" to "Simple System".  "u"
will be approximated using first-order approximation.
</div>

<div class ="fragment">
<pre>
          equation_systems.get_system("Simple System").add_variable("u", FIRST);
        
</pre>
</div>
<div class = "comment">
Next we'll by add an "ExplicitSystem" to the
EquationSystems object, and we give it the name
"Complex System".
</div>

<div class ="fragment">
<pre>
          equation_systems.add_system&lt;ExplicitSystem&gt; ("Complex System");
        
</pre>
</div>
<div class = "comment">
Give "Complex System" three variables -- each with a different approximation
order.  Variables "c" and "T" will use first-order Lagrange approximation, 
while variable "dv" will use a second-order discontinuous
approximation space.
</div>

<div class ="fragment">
<pre>
          equation_systems.get_system("Complex System").add_variable("c", FIRST);
          equation_systems.get_system("Complex System").add_variable("T", FIRST);
          equation_systems.get_system("Complex System").add_variable("dv", SECOND, MONOMIAL);
            
</pre>
</div>
<div class = "comment">
Initialize the data structures for the equation system.
</div>

<div class ="fragment">
<pre>
          equation_systems.init();
                
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
Prints information about the system to the screen.
</div>

<div class ="fragment">
<pre>
          equation_systems.print_info();
        
</pre>
</div>
<div class = "comment">
Write the equation system if the user specified an
output file name.  Note that there are two possible
formats to write to.  Specifying libMeshEnums::WRITE will
create a formatted ASCII file.  Optionally, you can specify
libMeshEnums::ENCODE and get an XDR-encoded binary file.

<br><br>We will write the data, clear the object, and read the file
we just wrote.  This is simply to demonstrate capability.
Note that you might use this in an application to periodically
dump the state of your simulation.  You can then restart from
this data later.
</div>

<div class ="fragment">
<pre>
          if (argc &gt; 1)
            if (argv[1][0] != '-')
              {
                std::cout &lt;&lt; "&lt;&lt;&lt; Writing system to file " &lt;&lt; argv[1]
                          &lt;&lt; std::endl;
                
</pre>
</div>
<div class = "comment">
Write the system.
</div>

<div class ="fragment">
<pre>
                equation_systems.write (argv[1], libMeshEnums::WRITE);
                
</pre>
</div>
<div class = "comment">
Clear the equation systems data structure.
</div>

<div class ="fragment">
<pre>
                equation_systems.clear ();
        
                std::cout &lt;&lt; "&gt;&gt;&gt; Reading system from file " &lt;&lt; argv[1]
                          &lt;&lt; std::endl &lt;&lt; std::endl;
                
</pre>
</div>
<div class = "comment">
Read the file we just wrote.  This better
work!
</div>

<div class ="fragment">
<pre>
                equation_systems.read (argv[1], libMeshEnums::READ);
        
</pre>
</div>
<div class = "comment">
Print the information again.
</div>

<div class ="fragment">
<pre>
                equation_systems.print_info();
              }
          
</pre>
</div>
<div class = "comment">
All done.  libMesh objects are destroyed here.  Because the
LibMeshInit object was created first, its destruction occurs
last, and it's destructor finalizes any external libraries and
checks for leaked memory.
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
  #include <B><FONT COLOR="#BC8F8F">&quot;libmesh.h&quot;</FONT></B>
  #include <B><FONT COLOR="#BC8F8F">&quot;mesh.h&quot;</FONT></B>
  #include <B><FONT COLOR="#BC8F8F">&quot;mesh_generation.h&quot;</FONT></B>
  #include <B><FONT COLOR="#BC8F8F">&quot;equation_systems.h&quot;</FONT></B>
  #include <B><FONT COLOR="#BC8F8F">&quot;linear_implicit_system.h&quot;</FONT></B>
  #include <B><FONT COLOR="#BC8F8F">&quot;transient_system.h&quot;</FONT></B>
  #include <B><FONT COLOR="#BC8F8F">&quot;explicit_system.h&quot;</FONT></B>
  
  using namespace libMesh;
  
  
  
  <B><FONT COLOR="#228B22">int</FONT></B> main (<B><FONT COLOR="#228B22">int</FONT></B> argc, <B><FONT COLOR="#228B22">char</FONT></B>** argv)
  {
    LibMeshInit init (argc, argv);
  
    libmesh_example_assert(2 &lt;= LIBMESH_DIM, <B><FONT COLOR="#BC8F8F">&quot;2D support&quot;</FONT></B>);
    
    <B><FONT COLOR="#5F9EA0">std</FONT></B>::cout &lt;&lt; <B><FONT COLOR="#BC8F8F">&quot;Running &quot;</FONT></B> &lt;&lt; argv[0];
    <B><FONT COLOR="#A020F0">for</FONT></B> (<B><FONT COLOR="#228B22">int</FONT></B> i=1; i&lt;argc; i++)
      <B><FONT COLOR="#5F9EA0">std</FONT></B>::cout &lt;&lt; <B><FONT COLOR="#BC8F8F">&quot; &quot;</FONT></B> &lt;&lt; argv[i];
    <B><FONT COLOR="#5F9EA0">std</FONT></B>::cout &lt;&lt; std::endl &lt;&lt; std::endl;
    
    Mesh mesh;
    
    <B><FONT COLOR="#5F9EA0">MeshTools</FONT></B>::Generation::build_square (mesh, 5, 5);
  
    EquationSystems equation_systems (mesh);
    
    equation_systems.parameters.set&lt;<B><FONT COLOR="#228B22">bool</FONT></B>&gt; (<B><FONT COLOR="#BC8F8F">&quot;test&quot;</FONT></B>) = true;
      
    equation_systems.parameters.set&lt;Real&gt; (<B><FONT COLOR="#BC8F8F">&quot;dummy&quot;</FONT></B>) = 42.;
      
    equation_systems.parameters.set&lt;Real&gt; (<B><FONT COLOR="#BC8F8F">&quot;nobody&quot;</FONT></B>) = 0.;
    
    equation_systems.add_system&lt;TransientLinearImplicitSystem&gt; (<B><FONT COLOR="#BC8F8F">&quot;Simple System&quot;</FONT></B>);
      
    equation_systems.get_system(<B><FONT COLOR="#BC8F8F">&quot;Simple System&quot;</FONT></B>).add_variable(<B><FONT COLOR="#BC8F8F">&quot;u&quot;</FONT></B>, FIRST);
  
    equation_systems.add_system&lt;ExplicitSystem&gt; (<B><FONT COLOR="#BC8F8F">&quot;Complex System&quot;</FONT></B>);
  
    equation_systems.get_system(<B><FONT COLOR="#BC8F8F">&quot;Complex System&quot;</FONT></B>).add_variable(<B><FONT COLOR="#BC8F8F">&quot;c&quot;</FONT></B>, FIRST);
    equation_systems.get_system(<B><FONT COLOR="#BC8F8F">&quot;Complex System&quot;</FONT></B>).add_variable(<B><FONT COLOR="#BC8F8F">&quot;T&quot;</FONT></B>, FIRST);
    equation_systems.get_system(<B><FONT COLOR="#BC8F8F">&quot;Complex System&quot;</FONT></B>).add_variable(<B><FONT COLOR="#BC8F8F">&quot;dv&quot;</FONT></B>, SECOND, MONOMIAL);
      
    equation_systems.init();
          
    mesh.print_info();
    equation_systems.print_info();
  
    <B><FONT COLOR="#A020F0">if</FONT></B> (argc &gt; 1)
      <B><FONT COLOR="#A020F0">if</FONT></B> (argv[1][0] != <B><FONT COLOR="#BC8F8F">'-'</FONT></B>)
        {
          <B><FONT COLOR="#5F9EA0">std</FONT></B>::cout &lt;&lt; <B><FONT COLOR="#BC8F8F">&quot;&lt;&lt;&lt; Writing system to file &quot;</FONT></B> &lt;&lt; argv[1]
                    &lt;&lt; std::endl;
          
          equation_systems.write (argv[1], libMeshEnums::WRITE);
          
          equation_systems.clear ();
  
          <B><FONT COLOR="#5F9EA0">std</FONT></B>::cout &lt;&lt; <B><FONT COLOR="#BC8F8F">&quot;&gt;&gt;&gt; Reading system from file &quot;</FONT></B> &lt;&lt; argv[1]
                    &lt;&lt; std::endl &lt;&lt; std::endl;
          
          equation_systems.read (argv[1], libMeshEnums::READ);
  
          equation_systems.print_info();
        }
    
    <B><FONT COLOR="#A020F0">return</FONT></B> 0;
  }
</pre> 
<a name="output"></a> 
<br><br><br> <h1> The console output of the program: </h1> 
<pre>
Compiling C++ (in optimized mode) introduction_ex2.C...
Linking introduction_ex2-opt...
***************************************************************
* Running Example  mpirun -np 6 ./introduction_ex2-opt -pc_type bjacobi -sub_pc_type ilu -sub_pc_factor_levels 4 -sub_pc_factor_zeropivot 0 -ksp_right_pc -log_summary
***************************************************************
 
./introduction_ex2-opt: error while loading shared libraries: librythmos.so: cannot open shared object file: No such file or directory
./introduction_ex2-opt: error while loading shared libraries: librythmos.so: cannot open shared object file: No such file or directory
./introduction_ex2-opt: error while loading shared libraries: librythmos.so: cannot open shared object file: No such file or directory
./introduction_ex2-opt: error while loading shared libraries: librythmos.so: cannot open shared object file: No such file or directory
./introduction_ex2-opt: error while loading shared libraries: librythmos.so: cannot open shared object file: No such file or directory
./introduction_ex2-opt: error while loading shared libraries: librythmos.so: cannot open shared object file: No such file or directory
make[1]: *** [run] Error 127
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
