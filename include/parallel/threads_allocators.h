// $Id: parallel.h 2631 2008-02-04 16:46:11Z roystgnr $

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


#ifndef __threads_allocators_h__
#define __threads_allocators_h__

// System includes

// Local includes
#include "libmesh_config.h"
#include "threads.h"

// Threading building blocks includes
#ifdef HAVE_TBB_API
#  include "tbb/scalable_allocator.h"
#endif


/**
 * The Threads namespace is for wrapper functions
 * for common general multithreading algorithms and tasks.
 */
namespace Threads
{
#ifdef HAVE_TBB_API
 
 //-------------------------------------------------------------------
  /**
   * Scalable allocator to be used in multithreaded code chunks which 
   * allocate a lot of dynamic memory.  This allocator can be faster
   * than the std::allocator when there are multiple threads.
   */ 
  template <typename T>
  class scalable_allocator : public tbb::scalable_allocator<T>
  {
  public:
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    typedef T value_type;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

    template<typename U> 
    struct rebind 
    {
      typedef scalable_allocator<U> other;
    };

    scalable_allocator () :
      tbb::scalable_allocator<T>() {}

    scalable_allocator (const scalable_allocator &a) :
      tbb::scalable_allocator<T>(a) {}

    template<typename U> 
    scalable_allocator(const scalable_allocator<U> &a) :
      tbb::scalable_allocator<T>(a) {}
  };

  

#else //HAVE_TBB_API


 
 //-------------------------------------------------------------------
  /**
   * Just use std::allocator when the Threading Building Blocks is absent.
   */ 
  template <typename T>
  class scalable_allocator : public std::allocator<T>
  {
  public:
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    typedef T value_type;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

    template<typename U> 
    struct rebind 
    {
      typedef scalable_allocator<U> other;
    };

    scalable_allocator () :
      std::allocator<T>() {}

    scalable_allocator (const scalable_allocator &a) :
      std::allocator<T>(a) {}

    template<typename U> 
    scalable_allocator(const scalable_allocator<U> &a) :
      std::allocator<T>(a) {}
  };
  

#endif // #ifdef HAVE_TBB_API

} // namespace Threads

#endif // #define __threads_allocators_h__
