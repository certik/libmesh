// $Id: mesh_unv_support.h,v 1.3 2004-03-18 15:10:32 jwpeterson Exp $

// The libMesh Finite Element Library.
// Copyright (C) 2002-2004  Benjamin S. Kirk, John W. Peterson
  
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


#ifndef __mesh_unv_support_h__
#define __mesh_unv_support_h__

// C++ includes
#include <fstream>
#include <vector>



// Local includes
#include "mesh_base.h"



// Forward Declarations
class MeshData;


/**
 * Class \p UnvMeshInterface provides an interface
 * for reading a mesh (datasets 2411 and 2412)
 * from a file in I-deas Universal file format,
 * \p .unv.   Thanks to ,John: when \p zlib.h was 
 * found (see \p configure), then this interface 
 * may also read and write packed files of format 
 * \p .unv.gz @e directly.  Cool, hm?
 *
 * @author: Tammo Kaschner, Daniel Dreyer
 */
class UnvMeshInterface
{
public:

  /**
   * Constructor.  Takes the data relevant 
   * for reading/writing mesh or reading data.
   * Note that for simplicity, the node and element
   * vectors have to be writable even when only
   * read access is needed.  Optionally produces
   * some babble on \p std::cout with \p be_verbose.
   */
  UnvMeshInterface (std::vector<Node*>& nodes,
		    std::vector<Elem*>& elements,
		    MeshData& md,
		    const bool be_verbose=false);

  /**
   * Reads a mesh (nodes & elements) from the file
   * named \p file_name.
   */
  void read (const std::string& file_name);

  /**
   * Writes a mesh (nodes & elements) to the file
   * named \p file_name.
   */
  void write (const std::string& file_name);
  
  /**
   * Destructor.
   */
  ~UnvMeshInterface();

protected:

  /**
   * The actual implementation of the read function.
   * The public read interface simply decides which
   * type of stream to pass the implementation.
   */
  void read_implementation (std::istream& in_stream);

  /**
   * The actual implementation of the write function.
   * The public write interface simply decides which
   * type of stream to pass the implementation.
   */
  void write_implementation (std::ostream& out_stream);
  
  /**
   * Clears the data structures to a pristine
   * state.
   */
  void clear();


  //-------------------------------------------------------------
  // read support methods
  /**
   * When reading, counting the nodes first
   * helps pre-allocation.  Also determine
   * whether we need to convert from "D" to "e".
   */
  void count_nodes (std::istream& in_file);
  
  /**
   * Method reads nodes from \p in_file and stores them in
   * vector<Node*> \p nodes in the order they come in.
   * The original node labels are being stored in
   * \p _assign_nodes in order to assign the elements to
   * the correct nodes later.  In addition, provided it is 
   * active, the \p MeshData gets to know the node id from
   * the Universal file, too.
   */
  void node_in(std::istream& in_file);

  /**
   * When reading, counting the elements first
   * helps pre-allocation.
   */
  void count_elements (std::istream& in_file);

  /**
   * Method reads elements and stores them in
   * \p std::vector<Elem*> \p _elements in the same order as they
   * come in. Within \p UnvMeshInterface, element labels are
   * ignored, but \p MeshData takes care of such things
   * (if active).
   */
  void element_in(std::istream& in_file);

  /**
   * @returns \p false when error occured, \p true otherwise.
   * Adjusts the \p in_stream to the beginning of the
   * dataset \p ds_name.
   */
  bool beginning_of_dataset(std::istream& in_file, 
			    const std::string& ds_name) const;

  /**
   * Method for converting exponential notation
   * from "D" to "e", for example
   * \p 3.141592654D+00 \p --> \p 3.141592654e+00
   * in order to make it readable for C++.
   */
  Real D_to_e(std::string& number) const;


  //-------------------------------------------------------------
  // write support methods
  /**
   * Outputs nodes to the file \p out_file.
   * For this to work, the \p MeshData of the current
   * \p Mesh has to be active.  Do not use this directly,
   * but through the proper write method.
   */
  void node_out(std::ostream& out_file);

  /**
   * Outputs the element data to the file \p out_file.
   * For this to work, the \p MeshData of the current
   * \p Mesh has to be active. Do not use this directly,
   * but through the proper write method.
   */
  void element_out(std::ostream& out_file);


  //-------------------------------------------------------------
  // local data

  /**
   * should be be verbose?
   */
  const bool _verbose;

  /**
   * Reference to the mesh's vector holding the nodes
   */
  std::vector<Node*>& _nodes;    

  /**
   * Reference to the mesh's vector holding the elements
   */
  std::vector<Elem*>& _elements; 

  /**
   * maps node id's from UNV to internal.  Used when reading.
   */
  std::vector<unsigned int> _assign_nodes;

  /**
   * stores positions of relevant datasets in the file, should
   * help to re-read the data faster.  Used when reading.
   */
  std::map<std::string,std::streampos> _ds_position;

  /**
   * total number of nodes, determined through \p count_nodes().
   * Primarily used when reading.
   */
  unsigned int _n_nodes;

  /**
   * total number of elements, determined through 
   * \p count_elements().  Primarily used when reading.
   */
  unsigned int _n_elements;

  /**
   * writable reference to the class that
   * handles foreign node/element ids
   */
  MeshData& _mesh_data;

  /**
   * label for the node dataset
   */
  static const std::string _label_dataset_nodes;

  /**
   * label for the element dataset
   */
  static const std::string _label_dataset_elements;

  /**
   * whether we need to convert notation of exponentials.
   * Used when reading.
   */
  bool _need_D_to_e;

};


// ------------------------------------------------------------
// UnvMeshInterface inline functions
inline
bool UnvMeshInterface::beginning_of_dataset(std::istream& in_file, 
					    const std::string& ds_name) const
{
  assert (in_file.good());
  assert (!ds_name.empty());

  std::string olds, news;

  while (true)
    {
      in_file >> olds >> news;

      /*
       * a "-1" followed by a number means the beginning of a dataset
       * stop combing at the end of the file
       */
      while( ((olds != "-1") || (news == "-1") ) && !in_file.eof() )
	{	  
	  olds = news;
	  in_file >> news;
	}

      if(in_file.eof())
	return false;
      
      if (news == ds_name)
	return true;
    }

  // should never end up here
  error();
  return false;
}




inline
Real UnvMeshInterface::D_to_e (std::string& number) const
{
  /* find "D" in string, start looking at 
   * 6th element, to improve speed.
   * We dont expect a "D" earlier
   */

#ifdef __HP_aCC
  // Use an int instead of an unsigned int,
  // otherwise HP aCC may crash!
  const int position = number.find("D",6);
#else
  const unsigned int position = number.find("D",6);
#endif

  assert (position!=std::string::npos);
  number.replace(position, 1, "e"); 

  return atof (number.c_str());
}




#endif





