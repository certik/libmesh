// $Id: libmesh_base.h 2672 2008-02-17 00:31:13Z benkirk $

// The libMesh Finite Element Library.
// Copyright (C) 2002-2007  Benjamin S. Kirk, John W. Peterson
  
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



#ifndef __libmesh_exceptions_h__
#define __libmesh_exceptions_h__

#include "libmesh_config.h"

#ifdef ENABLE_EXCEPTIONS
#include <stdexcept>


namespace libMesh {

// A class to represent the internal "this should never happen"
// errors to be thrown by "libmesh_error();"
class LogicError : public std::logic_error
{
public:
    LogicError() : std::logic_error( "Error in libMesh internal logic" ) {}
};
  
}

#define LIBMESH_THROW(e) { throw e; }

#else

#define LIBMESH_THROW(e) { std::abort(); }

#endif // ENABLE_EXCEPTIONS

#endif // #define __libmesh_exceptions_h__