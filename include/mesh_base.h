// $Id: mesh_base.h,v 1.35 2003-07-15 12:40:12 benkirk Exp $

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



#ifndef __mesh_base_h__
#define __mesh_base_h__



// C++ Includes   -----------------------------------
#include <vector>
#include <set>
#include <string>



// forward declarations
class Elem;
class EquationSystems;
template <typename T> class PetscMatrix;


// Local Includes -----------------------------------
#include "mesh_common.h"
#include "mesh_refinement.h"
#include "mesh_communication.h"
#include "boundary_info.h"
#include "mesh_data.h"
#include "node.h"
#include "enum_elem_type.h"
#include "sphere.h"
#include "enum_order.h"
#include "elem_iterators.h"
#include "node_iterators.h"


/**
 * This is the \p MeshBase class. This class provides all the data necessary
 * to describe a geometric entity.  It allows for the description of a
 * \p dim dimensional object that lives in \p DIM-dimensional space.
 * \par
 * A mesh is made of nodes and elements, and this class provides data
 * structures to store and access both.  A mesh may be partitioned into a
 * number of subdomains, and this class provides that functionality.
 * Furthermore, this class provides functions for reading and writing a
 * mesh to disk in various formats.
 *
 * \author Benjamin S. Kirk
 * \date 2002-2003
 * \version $Revision: 1.35 $
 */


// ------------------------------------------------------------
// MeshBase class definition
class MeshBase
{
public:

  /**
   * Constructor.
   */
  MeshBase (unsigned int d);
  
  /**
   * Copy-constructor.
   */
  MeshBase (const MeshBase& other_mesh);

  /**
   * Destructor. Deletes all the elements that are currently stored.
   */
  virtual ~MeshBase ();

  /**
   * Deletes all the data that are currently stored.
   */
  virtual void clear ();
  
#ifdef ENABLE_AMR

  /**
   * Class that handles adaptive mesh refinement implementation.
   */  
  MeshRefinement mesh_refinement;
  
#endif
  
  /**
   * This class holds the boundary information.  It can store nodes, edges,
   * and faces with a corresponding id that facilitates setting boundary
   * conditions.
   */
  BoundaryInfo boundary_info;
  
  /**
   * This class holds arbitrary (floating point) data associated with 
   * nodes or elements, commonly imported from files.  When this class
   * should be used, it has to be activated @e prior to using any
   * of \p MeshBase or \p Mesh \p read() methods.
   */
  MeshData data;

  /**
   * This class enables parallelization of the mesh.  All
   * required inter-processor communication is done via this class.
   */
  MeshCommunication mesh_communication;
  
  /**
   * @returns \p true if the mesh has been prepared via a call
   * to \p prepare_for_use, \p false otherwise.
   */
  bool is_prepared () const
  { return _is_prepared; }
  
  /**
   * Returns the logical dimension of the mesh.
   */
  unsigned int mesh_dimension () const
  { return static_cast<unsigned int>(_dim); }
  
  /**
   * Returns the spatial dimension of the mesh.
   */
  unsigned int spatial_dimension () const
  { return static_cast<unsigned int>(DIM); }
  
  /**
   * Returns the number of nodes in the mesh.
   */
  unsigned int n_nodes () const { return _nodes.size(); }

  /**
   * Returns the number of elements in the mesh.
   */
  unsigned int n_elem ()  const { return _elements.size(); }

  /**
   * Returns the number of active elements in the mesh.
   */
  unsigned int n_active_elem () const;

  /**
   * Return a vector of all
   * element types for the mesh.
   */
  std::vector<ElemType> elem_types () const;
  
  /**
   * Return the number of elements of type \p type.
   */
  unsigned int n_elem_of_type (const ElemType type) const;

  /**
   * Return the number of active elements of type \p type.
   */
  unsigned int n_active_elem_of_type (const ElemType type) const;

  /**
   * Returns the number of elements on processor \p proc.
   */
  unsigned int n_elem_on_proc (const unsigned int proc) const;

  /**
   * Returns the number of elements on the local processor.
   */
  unsigned int n_local_elem () const
  { return this->n_elem_on_proc (libMeshBase::processor_id()); }

  /**
   * Returns the number of active elements on processor \p proc.
   */
  unsigned int n_active_elem_on_proc (const unsigned int proc) const;

  /**
   * Returns the number of active elements on the local processor.
   */
  unsigned int n_active_local_elem () const
  { return this->n_active_elem_on_proc (libMeshBase::processor_id()); }
  
  /**
   * This function returns the number of elements that will be written
   * out in the Tecplot format.  For example, a 9-noded quadrilateral will
   * be broken into 4 linear sub-elements for plotting purposes.  Thus, for
   * a mesh of 2 \p QUAD9 elements  \p n_tecplot_elem() will return 8.
   */
  unsigned int n_sub_elem () const;

  /**
   * Same, but only counts active elements.
   */
  unsigned int n_active_sub_elem () const;

  /**
   * This function returns the sum over all the elemenents of the number
   * of nodes per element.  This can be useful for partitioning hybrid meshes.
   * A feasible load balancing scheme is to keep the weight per processor as
   * uniform as possible.
   */
  unsigned int total_weight () const;
  
  /**
   * Return a constant reference (for reading only) to the
   * \f$ i^{th} \f$ point.
   */  
  const Point& point (const unsigned int i) const;
  
  /**
   * Return a reference to the \f$ i^{th} \f$ node.
   */  
  Node& node (const unsigned int i);

  /**
   * Return a constant reference (for reading only) to the
   * \f$ i^{th} \f$ node.
   */  
  const Node& node (const unsigned int i) const;

  /**
   * Return a constant reference to the \p nodes vector holding the nodes.
   */
  const std::vector<Node*> & get_nodes () const { return _nodes; }

  /**
   * Add \p Node \p n to the vertex array, optionally at the specified position \p nn.
   */
  Node* add_point (const Point& n,
		   const unsigned int nn=static_cast<unsigned int>(-1));

  /**
   * Return a pointer to the \f$ i^{th} \f$ element.
   */
  Elem* elem (const unsigned int i) const;

  /**
   * Return a reference to the \p cells vector holding the elements.
   */
  const std::vector<Elem*> & get_elem () const { return _elements; }

  /**
   * Add elem \p e to the elem array.
   */
  void add_elem (Elem* e,
		 const unsigned int n=static_cast<unsigned int>(-1));

#ifdef ENABLE_INFINITE_ELEMENTS

  /**
   * Build infinite elements atop a volume-based mesh,
   * determine origin automatically.  Also returns the
   * origin as a \p const \p Point to make it more obvious that
   * the origin should not change after the infinite elements 
   * have been built.  When symmetry planes are present, use 
   * the version with optional symmetry switches.  
   * The flag \p be_verbose enables some diagnostic output.
   */
  const Point build_inf_elem (const bool be_verbose = false);

  /**
   * Build infinite elements atop a volume-based mesh.
   * Find all faces on the outer boundary and build infinite elements
   * on them, based on the \p origin.  During the search for faces
   * on which infinite elements are built, @e interior faces that 
   * are not on symmetry planes are found, too.  When an (optional)
   * pointer to \p inner_faces is provided, a set of pairs (first:
   * element number, second: side number) of inner faces is returned
   * that may be used for identifying interior boundaries.
   *
   * Faces which lie in at least one symmetry plane are skipped.
   * The source of the infinite elements must be given to \p origin,
   * the three optional booleans \p x_sym, \p y_sym,
   * \p z_sym indicate symmetry planes (through the origin, obviously)
   * perpendicular to the \p x, \p y and \p z direction, 
   * respectively.  
   * The flag \p be_verbose enables some diagnostic output.
   */
  void build_inf_elem (const Point& origin,
		       const bool x_sym = false,
		       const bool y_sym = false,
		       const bool z_sym = false,
		       const bool be_verbose = false,
		       std::set<std::pair<unsigned int,
		                          unsigned int> >* inner_faces = NULL);
		      
#endif
		      
  /**
   * Locate element face (edge in 2D) neighbors.  This is done with the help
   * of a \p std::map that functions like a hash table.  When this function is
   * called only elements with \p NULL neighbor pointers are considered, so
   * the first call should take the longest.  Subsequent calls will only
   * consider new elements and the elements that lie on the boundary.

   * After this routine is called all the elements with a \p NULL neighbor
   * pointer are guaranteed to be on the boundary.  Thus this routine is
   * useful for automatically determining the boundaries of the domain.
   */
  void find_neighbors ();
  
  /**
   * After calling this function the input vector \p nodes_to_elem_map
   * will contain the node to element connectivity.  That is to say
   * \p nodes_to_elem_map[i][j] is the global number of \f$ j^{th} \f$
   * element connected to node \p i.
   */
  void build_nodes_to_elem_map (std::vector<std::vector<unsigned int> >&
				nodes_to_elem_map) const;


  /**
   * Calling this function on a 2D mesh will convert all the elements
   * to triangles.  \p QUAD4s will be converted to \p TRI3s, \p QUAD8s
   * and \p QUAD9s will be converted to \p TRI6s. 
   */
  void all_tri ();
  
//   /**
//    * Partition the mesh into \p n_sbdmns subdomians. Currently this routine
//    * uses Bill Barth's space-filling curve library to do the partitioning,
//    * and the default is a \p hilbert curve, but \p morton is also supported.
//    */
//   virtual void sfc_partition (const unsigned int n_sbdmns=libMeshBase::n_processors(),
// 			      const std::string& type="hilbert");

//   /**
//    * Partition the mesh using the Metis library. Only works if \p ./configure
//    * detected the library.
//    */
//   virtual void metis_partition (const unsigned int n_sbdmns=libMeshBase::n_processors(),
// 				const std::string& type="kway");

  /**
   * Call the default partitioner (currently \p metis_partition()).
   */
  virtual void partition (const unsigned int n_sbdmns=libMeshBase::n_processors());

  /**
   * After partitoning a mesh it is useful to renumber the nodes and elements
   * so that they lie in contiguous blocks on the processors.  This method
   * does just that.
   */
  void renumber_nodes_and_elements ();    

  /**
   * Randomly perturb the nodal locations.  This function will
   * move each node \p factor fraction of its minimum neighboring
   * node separation distance.  Nodes on the boundary are not moved
   * by default, however they may be by setting the flag
   * \p perturb_boundary true.
   */
  void distort (const Real factor, const bool perturb_boundary=false);
  
  /**
   * Translates the mesh.  The grid points are translated in the
   * \p x direction by \p xt, in the \p y direction by \p yt,
   * etc...
   */
  void translate (const Real xt=0., const Real yt=0., const Real zt=0.); 

  /**
   * Rotates the mesh.  The grid points are rotated about the 
   * \p x axis by \p xr , about the \p y axis by \p yr,
   * etc...  
   */
  void rotate (const Real xr, const Real yr=0., const Real zr=0.); 

  /**
   * Scales the mesh.  The grid points are scaled in the
   * \p x direction by \p xs, in the \p y direction by \p ys,
   * etc...  If only \p xs is specified then the scaling is
   * assumed uniform in all directions.
   */
  void scale (const Real xs, const Real ys=0., const Real zs=0.);
    
  /**
   * @returns two points defining a cartesian box that bounds the
   * mesh.  The first entry in the pair is the mininum, the second 
   * is the maximim.
   */
  std::pair<Point, Point> bounding_box () const;

  /**
   * Same, but returns a sphere instead of a box.
   */
  Sphere bounding_sphere () const;
  
  /**
   * @returns two points defining a cartesian box that bounds the
   * elements belonging to processor pid.  If no processor id is specified
   * the bounding box for the whole mesh is returned.
   */
  std::pair<Point, Point> 
  processor_bounding_box (const unsigned int pid = static_cast<unsigned int>(-1)) const;

  /**
   * Same, but returns a sphere instead of a box.
   */
  Sphere 
  processor_bounding_sphere (const unsigned int pid = static_cast<unsigned int>(-1)) const;

  /**
   * @returns two points defining a Cartesian box that bounds the
   * elements belonging to subdomain sid.  If no subdomain id is specified
   * the bounding box for the whole mesh is returned.
   */
  std::pair<Point, Point> 
  subdomain_bounding_box (const unsigned int sid = static_cast<unsigned int>(-1)) const;

  /**
   * Same, but returns a sphere instead of a box.
   */
  Sphere 
  subdomain_bounding_sphere (const unsigned int pid = static_cast<unsigned int>(-1)) const;


  /**
   * Builds the connectivity graph. The matrix \p conn is such that the
   * valence of each node is on the diagonal and there is a -1 for each
   * node connected to the node.
   */
  void build_L_graph (PetscMatrix<Number>& conn) const;
  
  /**
   * Builds the connectivity graph. The matrix \p conn is such that the
   * valence of each node is on the diagonal and there is a -1 for each
   * node connected to the node.
   */
  void build_script_L_graph (PetscMatrix<Number>& conn) const;
  
  /**
   * Returns the number of subdomains in the global mesh. Note that it is
   * convenient to have one subdomain on each processor on parallel machines,
   * however this is not required. Multiple subdomains can exist on the same
   * processor.
   *
   */
  unsigned int n_subdomains () const { return _n_sbd; }

  /**
   * @returns the number of processors used in the
   * current simulation.
   */
  unsigned int n_processors () const { return libMeshBase::n_processors(); }


  /**
   * @returns the subdomain id for this processor.
   */
  unsigned int processor_id () const { return libMeshBase::processor_id(); }
  
  /**
   * Reads the file specified by \p name.  Attempts to figure out the
   * proper method by the file extension.
   */
  virtual void read (const std::string& name);
  
  /**
   * Write to the file specified by \p name.  Attempts to figure out the
   * proper method by the file extension.
   */
  virtual void write (const std::string& name);
  
  /**
   * Write to the file specified by \p name.  Attempts to figure out the
   * proper method by the file extension. Also writes data.
   */
  virtual void write (const std::string& name,
		      std::vector<Number>& values,
		      std::vector<std::string>& variable_names);
  
  /**
   * Write a Tecplot-formatted ASCII text file to the file specified by 
   * \p name.  Writes both the mesh and the solution from \p es. 
   */
  void write_tecplot (const std::string& name,
		      EquationSystems& es);
  
  /**
   * Write a Tecplot-formatted ASCII text file to the file specified by 
   * \p name.  The optional parameters can be used to write nodal data 
   * in addition to the mesh.
   */
  void write_tecplot (const std::string& name,
		      const std::vector<Number>* v=NULL,
		      const std::vector<std::string>* solution_names=NULL);
  
  /**
   * Write a Tecplot-formatted binary file to the file specified by \p name.
   * Also writes the solution from \p es.
   * For this to work properly the Tecplot API must be available.  If the API 
   * is not present this function will simply call the ASCII output version.
   */
  void write_tecplot_binary (const std::string& name,
			     EquationSystems& es);
  
  /**
   * Write a Tecplot-formatted binary file to the file specified by \p name.
   * The optional parameters can be used to write nodal data in addition to
   * the mesh.
   * For this to work properly the Tecplot API must be available.  If the API 
   * is not present this function will simply call the ASCII output version.
   */
  void write_tecplot_binary (const std::string& name,
			     const std::vector<Number>* v=NULL,
			     const std::vector<std::string>* solution_names=NULL);

  /**
   * Write the mesh in AVS's UCD format to  the file specified by \p name.
   * May be expanded in the future to handle data as well.
   */
  void write_ucd (const std::string& name);  
  
  /**
   * Write the mesh in the GMV ASCII format to a file specified by \p name.
   * GMV is the General Mesh Viewer from
   * LANL.  This function writes the solution from \p es as well.  Also,
   * since GMV understands cell-based data, this function can optionally
   * write the partitioning information.
   */
  void write_gmv (const std::string& name,
		  EquationSystems& es,
		  const bool write_partitioning=true);

  /**
   * Write the mesh in the GMV ASCII format to a file specified by \p name.
   * GMV is the General Mesh Viewer from
   * LANL.  This function optionally writes nodal data as well.  Also,
   * since GMV understands cell-based data, this function can optionally
   * write the partitioning information.
   */
  void write_gmv (const std::string& name,
		  const std::vector<Number>* v=NULL,
		  const std::vector<std::string>* solution_names=NULL,
		  const bool write_partitioning=true);
  
  /**
   * Write the mesh in the GMV binary format to  a file specified by \p name.
   * GMV is the General Mesh Viewer from
   * LANL.  This function writes the solution from \p es as well.  Also,
   * since GMV understands cell-based data, this function can optionally
   * write the partitioning information.
   */
  void write_gmv_binary (const std::string& name,
			 EquationSystems& es,
			 const bool write_partitioning=true);

  /**
   * Write the mesh in the GMV binary format to  a file specified by \p name.
   * GMV is the General Mesh Viewer from
   * LANL.  This function optionally writes nodal data as well.  Also,
   * since GMV understands cell-based data, this function can optionally
   * write the partitioning information.
   */
  void write_gmv_binary (const std::string& name,
			 const std::vector<Number>* v=NULL,
			 const std::vector<std::string>* solution_names=NULL,
			 const bool write_partitioning=true);

  /** 
   * Write mesh to file specified by \p name in Universal (unv) format.
   */
  void write_unv (const std::string& name);

  /** 
   * Read mesh from the file specified by \p name in Universal (unv) format.  
   */
  void read_unv (const std::string& name);

  /**
   * @returns a string containing relevant information
   * about the mesh.
   */
  std::string get_info () const;

  /**
   * Prints relevant information about the mesh.
   */
  void print_info () const;
  

#ifdef USE_COMPLEX_NUMBERS

  /**
   * @returns for \p r_o_c = 0 the filename for output of the real part
   * of complex data, and for  \p r_o_c = 1 the filename for the imaginary 
   * part.
   */
  const char* complex_filename (const std::string& _n,
				unsigned int r_o_c=0) const;

  /**
   * Prepare complex data for writing.
   */
  void prepare_complex_data (const std::vector<Number>* source,
			     std::vector<Real>* real_part,
			     std::vector<Real>* imag_part) const;

#endif


  /**
   * Returns a pair of std::vector<Elem*>::iterators which point
   * to the beginning and end of the _elements vector.
   */
  std::pair<std::vector<Elem*>::iterator,
	    std::vector<Elem*>::iterator>
  elements_begin ();

  /**
   * Returns a pair of std::vector<Elem*>::const_iterators which point
   * to the beginning and end of the _elements vector.
   */
  std::pair<std::vector<Elem*>::const_iterator,
	    std::vector<Elem*>::const_iterator>
  elements_begin () const;
  
  /**
   * Returns a pair of std::vector<Elem*>::iterators which point
   * to the end of the _elements vector.  This simulates a normal
   * end() iterator.
   */
  std::pair<std::vector<Elem*>::iterator,
	    std::vector<Elem*>::iterator>
  elements_end ();
  
  /**
   * Returns a pair of std::vector<Elem*>::const_iterators which point
   * to the end of the _elements vector.  This simulates a normal
   * end() const_iterator.
   */
  std::pair<std::vector<Elem*>::const_iterator,
	    std::vector<Elem*>::const_iterator>
  elements_end () const;
  
  /**
   * Returns a pair of std::vector<Node*>::iterators which point
   * to the beginning and end of the _nodeents vector.
   */
  std::pair<std::vector<Node*>::iterator,
	    std::vector<Node*>::iterator>
  nodes_begin (); 
  
  /**
   * Returns a pair of std::vector<Node*>::const_iterators which point
   * to the beginning and end of the _nodes vector.
   */
  std::pair<std::vector<Node*>::const_iterator,
	    std::vector<Node*>::const_iterator>
  nodes_begin () const;

  /**
   * Returns a pair of std::vector<Node*>::iterators which point
   * to the end of the _nodes vector.  This simulates a normal
   * end() iterator.
   */
  std::pair<std::vector<Node*>::iterator,
	    std::vector<Node*>::iterator>
  nodes_end ();
  
  /**
   * Returns a pair of std::vector<Node*>::const_iterators which point
   * to the end of the _nodes vector.  This simulates a normal
   * end() const_iterator.
   */
  std::pair<std::vector<Node*>::const_iterator,
	    std::vector<Node*>::const_iterator>
  nodes_end () const;
  
  /**
   * Fills the vector "on_boundary" with flags that tell whether each node
   * is on the domain boundary (true)) or not (false).
   */
  void find_boundary_nodes(std::vector<bool>& on_boundary);
  
  
protected:


  
  /**
   * Prepare a newly created (or read) mesh for use.
   * This involves 3 steps:
   *  1.) call \p find_neighbors()
   *  2.) call \p partition()
   *  3.) call \p renumber_nodes_and_elements() 
   */
  virtual void prepare_for_use ();
  
  /**
   * Return a pointer to the \f$ i^{th} \f$ node.
   */  
  const Node* node_ptr (const unsigned int i) const;

  /**
   * Return a pointer to the \f$ i^{th} \f$ node.
   */  
  Node* & node_ptr (const unsigned int i);
    
  /**
   * Reads an unstructured 2D triangular mesh from the file
   * specified by \p name.  The format is described in the 
   * implementation.  This function facilitates reading meshes 
   * created by Matlab's PDE Toolkit.
   */
  void read_matlab (const std::string& name);
  
  /**
   * Reads a matlab-format mesh from a stream.  
   * Implements the read process initiated by the associated public method.
   */
  void read_matlab (std::istream& in);

  /**
   * Reads an unstructured, triangulated surface in the
   * standard OFF OOGL format from the file specified by \p name.
   */
  void read_off (const std::string& name);
  
  /**
   * Reads an unstructured, triangulated surface in the
   * standard OFF OOGL format from a stream.
   * Implements the read process initiated by the associated public method.
   */
  void read_off (std::istream& in);
  
  /**
   * Read meshes in AVS's UCD format from the file specified by \p name.
   * For the format description see the AVS Developer's guide.
   * This is the format of choice for reading meshes
   * created by Gridgen, since that tool can output UCD files.
   */
  void read_ucd (const std::string& name);
  
  /**
   * Read meshes in AVS's UCD format from a stream.
   * Implements the read process initiated by the associated public method.
   */
  void read_ucd (std::istream& in);
  
  /**
   * Read a 2D mesh in the shanee format from the file specified
   * by \p name.  This is for compatibility with
   * Ben Kirk's old code, and may be removed at any time in the future.
   *
   * @sect2{Write Methods}
   *
   */
  void read_shanee (const std::string& name);

  /**
   * Read a 2D mesh in the shanee format from a stream.
   * Implements the read process initiated by the associated public method.
   */
  void read_shanee (std::istream& in);

  /**
   * Write a Tecplot-formatted ASCII text file to a stream.  Fetches
   * nodal data from \p es.
   */
  void write_tecplot (std::ostream& out,
		      EquationSystems& es);
  
  /**
   * Actual Implementation of writing a Tecplot-formatted 
   * ASCII text file to a stream.
   */
  void write_tecplot (std::ostream& out,
		      const std::vector<Number>* v=NULL,
		      const std::vector<std::string>* solution_names=NULL);

  /**
   * Actual implementation of writing a mesh in AVS's UCD format.
   */
  void write_ucd (std::ostream& out);
  
  /**
   * Write the mesh and solution from \p es in the GMV ASCII 
   * format to a stream.
   */
  void write_gmv (std::ostream& out,
		  EquationSystems& es,
		  const bool write_partitioning=false);

  /**
   * Actual implementation of writing a mesh in the GMV ASCII format.
   */
  void write_gmv (std::ostream& out,
		  const std::vector<Number>* v=NULL,
		  const std::vector<std::string>* solution_names=NULL,
		  const bool write_partitioning=false);


  /**
   * Write the mesh and solution from \p es in the GMV binary
   * format to a stream.
   */
  void write_gmv_binary (std::ostream& out,
			 EquationSystems& es,
			 const bool write_partitioning=false);
  
  /**
   * Actual implementation of writing a mesh in the GMV binary format.
   */
  void write_gmv_binary (std::ostream& out,
			 const std::vector<Number>* v=NULL,
			 const std::vector<std::string>* solution_names=NULL,
			 const bool write_partitioning=false);

  
  
#ifdef ENABLE_AMR

  /**
   * After coarsening a mesh it is possible that
   * there are some voids in the \p nodes and
   * \p elements vectors that need to be cleaned
   * up.  This functions does that.  The indices
   * of the entities to be removed are contained
   * in the two input sets.
   */
  void trim_unused_elements(std::set<unsigned int>& unused_elements);

#endif

  /**
   * Returns a writeable reference to the number of subdomains.
   */
  unsigned int& set_n_subdomains ()
  { return _n_sbd; }

  /**
   * Reads input from \p in, skipping all the lines
   * that start with the character \p comment_start.
   */
  void skip_comment_lines (std::istream& in,
			   const char comment_start);

  /**
   * The verices (spatial coordinates) of the mesh.
   */
  std::vector<Node*> _nodes;

  /**
   * The elements in the mesh.
   */
  std::vector<Elem*> _elements;

  /**
   * The number of subdomains the mesh has been partitioned into.
   */
  unsigned int _n_sbd;

  /**
   * The logical dimension of the mesh.
   */     
  const unsigned int _dim;

  /**
   * Flag indicating if the mesh has been prepared for use.
   */
  bool _is_prepared;
  
  /**
   * Make the \p BoundaryInfo class a friend so that
   * they can create a \p BoundaryMesh.
   */
  friend class BoundaryInfo;
  
  /**
   * The \p MeshCommunication class needs to be a friend.
   */
  friend class MeshCommunication;
  
#ifdef ENABLE_AMR
  
  /**
   * The \p MeshRefinement functions need to be able to
   * call trim_unused_elements()
   */
  friend class MeshRefinement;
  
#endif
};



// ------------------------------------------------------------
// MeshBase inline methods
inline
Elem* MeshBase::elem (const unsigned int i) const
{
  assert (i < this->n_elem());
  assert (_elements[i] != NULL);
  
  return _elements[i];
}



inline
const Point& MeshBase::point (const unsigned int i) const
{
  assert (i < this->n_nodes());
  assert (_nodes[i] != NULL);
  assert (_nodes[i]->id() != Node::invalid_id);  

  return (*_nodes[i]);
}



inline
const Node& MeshBase::node (const unsigned int i) const
{
  assert (i < this->n_nodes());
  assert (_nodes[i] != NULL);
  assert (_nodes[i]->id() != Node::invalid_id);  
  
  return (*_nodes[i]);
}



inline
Node& MeshBase::node (const unsigned int i)
{
  if (i >= this->n_nodes())
    {
      std::cout << " i=" << i
		<< ", n_nodes()=" << this->n_nodes()
		<< std::endl;
      error();
    }
  
  assert (i < this->n_nodes());
  assert (_nodes[i] != NULL);

  return (*_nodes[i]);
}



inline
const Node* MeshBase::node_ptr (const unsigned int i) const
{
  assert (i < this->n_nodes());
  assert (_nodes[i] != NULL);
  assert (_nodes[i]->id() != Node::invalid_id);  
  
  return _nodes[i];
}



inline
Node* & MeshBase::node_ptr (const unsigned int i)
{
  assert (i < this->n_nodes());

  return _nodes[i];
}



inline
std::pair<std::vector<Elem*>::iterator,
	  std::vector<Elem*>::iterator>
MeshBase::elements_begin ()
{
  return std::make_pair(_elements.begin(), _elements.end());
}
  


inline
std::pair<std::vector<Elem*>::const_iterator,
	  std::vector<Elem*>::const_iterator>
MeshBase::elements_begin () const
{
  return std::make_pair(_elements.begin(), _elements.end());
}



inline
std::pair<std::vector<Elem*>::iterator,
	  std::vector<Elem*>::iterator>
MeshBase::elements_end ()
{
  return std::make_pair(_elements.end(), _elements.end());
}



inline
std::pair<std::vector<Elem*>::const_iterator,
	  std::vector<Elem*>::const_iterator>
MeshBase::elements_end () const
{
  return std::make_pair(_elements.end(), _elements.end());
}
  


inline
std::pair<std::vector<Node*>::iterator,
	  std::vector<Node*>::iterator>
MeshBase::nodes_begin ()
{
  return std::make_pair(_nodes.begin(), _nodes.end());
}



inline
std::pair<std::vector<Node*>::const_iterator,
	  std::vector<Node*>::const_iterator>
MeshBase::nodes_begin () const
  {
    return std::make_pair(_nodes.begin(), _nodes.end());
  }



inline
std::pair<std::vector<Node*>::iterator,
	  std::vector<Node*>::iterator>
MeshBase::nodes_end ()
{
  return std::make_pair(_nodes.end(), _nodes.end());
}



inline
std::pair<std::vector<Node*>::const_iterator,
	  std::vector<Node*>::const_iterator>
MeshBase::nodes_end () const
{
  return std::make_pair(_nodes.end(), _nodes.end());
}


#endif
