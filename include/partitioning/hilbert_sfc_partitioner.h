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



#ifndef __hilbert_sfc_partitioner_h__
#define __hilbert_sfc_partitioner_h__

// Local Includes -----------------------------------
#include "sfc_partitioner.h"

// C++ Includes   -----------------------------------

namespace libMesh
{



/**
 * The \p HilbertSFCPartitioner uses a Hilbert space
 * filling curve to partition the elements.
 */

// ------------------------------------------------------------
// HilbertSFCLinearPartitioner class definition
class HilbertSFCPartitioner : public SFCPartitioner
{
 public:

  /**
   * Constructor.
   */
  HilbertSFCPartitioner ()
  {
    this->set_sfc_type ("Hilbert");
  }

  /**
   * Creates a new partitioner of this type and returns it in
   * an \p AutoPtr.
   */
  virtual AutoPtr<Partitioner> clone () const {
    AutoPtr<Partitioner> cloned_partitioner
      (new HilbertSFCPartitioner());
    return cloned_partitioner;
  }

protected:
  /**
   * Partition the \p MeshBase into \p n subdomains.
   */
  virtual void _do_partition (MeshBase& mesh,
			      const unsigned int n)
  { SFCPartitioner::_do_partition (mesh, n); }



private:

};

} // namespace libMesh


#endif // #define __hilbert_sfc_partitioner_h__
