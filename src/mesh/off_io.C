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

// C++ includes
#include <fstream>

// Local includes
#include "off_io.h"
#include "mesh_base.h"
#include "face_tri3.h"

namespace libMesh
{



// ------------------------------------------------------------
// OFFIO class members

void OFFIO::read(const std::string& name)
{
  std::ifstream in (name.c_str());

  read_stream(in);
}



void OFFIO::read_stream(std::istream& in)
{
  // This is a serial-only process for now;
  // the Mesh should be read on processor 0 and
  // broadcast later
  libmesh_assert(libMesh::processor_id() == 0);

  // Get a reference to the mesh
  MeshBase& mesh = MeshInput<MeshBase>::mesh();

  // Clear any existing mesh data
  mesh.clear();

  // STL only works in 2D
  mesh.set_mesh_dimension(2);

#if LIBMESH_DIM < 2
  libMesh::err << "Cannot open dimension 2 mesh file when configured without 2D support." <<
                  std::endl;
  libmesh_error();
#endif

  // Check the input buffer
  libmesh_assert (in.good());

  unsigned int nn, ne, nf;

  std::string label;

  // Read the first string.  It should say "OFF"
  in >> label;

  libmesh_assert (label == "OFF");

  // read the number of nodes, faces, and edges
  in >> nn >> nf >> ne;

  // resize local vectors.
  //  _nodes.resize(nn);
  //_elements.resize(nf);


  Real x=0., y=0., z=0.;

  // Read the nodes
  for (unsigned int n=0; n<nn; n++)
    {
      libmesh_assert (in.good());

      in >> x
	 >> y
	 >> z;

      mesh.add_point ( Point(x,y,z), n );
    }

  unsigned int dummy, n0, n1, n2;

  // Read the triangles
  for (unsigned int e=0; e<nf; e++)
    {
      libmesh_assert (in.good());

      // _elements[e] = new Tri3;
      // _elements[e]->set_id (e);
      Elem* elem = new Tri3;
      elem->set_id(e);
      mesh.add_elem (elem);

      // The number of nodes in the object
      in >> dummy;

      libmesh_assert (dummy == 3);

      in >> n0
	 >> n1
	 >> n2;

      elem->set_node(0) = mesh.node_ptr(n0);
      elem->set_node(1) = mesh.node_ptr(n1);
      elem->set_node(2) = mesh.node_ptr(n2);
    }
}

} // namespace libMesh
