// $Id$

// The libMesh Finite Element Library.
// Copyright (C) 2002-2008 Benjamin S. Kirk, John W. Peterson, Roy H. Stogner
  
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




#ifndef __petsc_vector_h__
#define __petsc_vector_h__


#include "libmesh_config.h"


#ifdef LIBMESH_HAVE_PETSC

// C++ includes
#include <vector>

// Local includes
#include "numeric_vector.h"
#include "petsc_macro.h"

/**
 * Petsc include files.
 */
EXTERN_C_FOR_PETSC_BEGIN
# include <petscvec.h>
EXTERN_C_FOR_PETSC_END



// forward declarations
template <typename T> class SparseMatrix;

/**
 * Petsc vector. Provides a nice interface to the
 * Petsc C-based data structures for parallel vectors.
 *
 * @author Benjamin S. Kirk, 2002
 */

template <typename T>
class PetscVector : public NumericVector<T>
{
public:

  /**
   *  Dummy-Constructor. Dimension=0
   */
  PetscVector (const ParallelType type = AUTOMATIC);
  
  /**
   * Constructor. Set dimension to \p n and initialize all elements with zero.
   */
  PetscVector (const unsigned int n,
               const ParallelType type = AUTOMATIC);
    
  /**
   * Constructor. Set local dimension to \p n_local, the global dimension
   * to \p n, and initialize all elements with zero.
   */
  PetscVector (const unsigned int n,
	       const unsigned int n_local,
               const ParallelType type = AUTOMATIC);

  /**
   * Constructor. Set local dimension to \p n_local, the global
   * dimension to \p n, but additionally reserve memory for the
   * indices specified by the \p ghost argument.
   */
  PetscVector (const unsigned int N,
	       const unsigned int n_local,
	       const std::vector<unsigned int>& ghost,
               const ParallelType type = AUTOMATIC);
  
  /**
   * Constructor.  Creates a PetscVector assuming you already have a
   * valid PETSc Vec object.  In this case, v is NOT destroyed by the
   * PetscVector constructor when this object goes out of scope.
   * This allows ownership of v to remain with the original creator,
   * and to simply provide additional functionality with the PetscVector.
   */
  PetscVector(Vec v);
  
  /**
   * Destructor, deallocates memory. Made virtual to allow
   * for derived classes to behave properly.
   */
  ~PetscVector ();

  /**
   * Call the assemble functions
   */
  void close (); 

  /**
   * @returns the \p PetscVector<T> to a pristine state.
   */
  void clear ();
  
  /**
   * Set all entries to zero. Equivalent to \p v = 0, but more obvious and
   * faster. 
   */
  void zero ();    

  /**
   * Creates a copy of this vector and returns it in an \p AutoPtr.
   */
  AutoPtr<NumericVector<T> > clone () const;
  
  /**
   * Change the dimension of the vector to \p N. The reserved memory for
   * this vector remains unchanged if possible, to make things faster, but
   * this may waste some memory, so take this in the back of your head.
   * However, if \p N==0 all memory is freed, i.e. if you want to resize
   * the vector and release the memory not needed, you have to first call
   * \p init(0) and then \p init(N). This cited behaviour is analogous
   * to that of the STL containers.
   *
   * On \p fast==false, the vector is filled by
   * zeros.
   */
    
  void init (const unsigned int N,
	     const unsigned int n_local,
	     const bool         fast=false,
	     const ParallelType type=AUTOMATIC);
    
  /**
   * call init with n_local = N,
   */
  void init (const unsigned int N,
	     const bool         fast=false,
	     const ParallelType type=AUTOMATIC);
    
  /**
   * Create a vector that holds tha local indices plus those specified
   * in the \p ghost argument.
   */
  virtual void init (const unsigned int /*N*/,
		     const unsigned int /*n_local*/,
		     const std::vector<unsigned int>& /*ghost*/,
		     const bool /*fast*/ = false,
		     const ParallelType = AUTOMATIC);
    
  //   /**
  //    * Change the dimension to that of the
  //    * vector \p V. The same applies as for
  //    * the other \p init function.
  //    *
  //    * The elements of \p V are not copied, i.e.
  //    * this function is the same as calling
  //    * \p init(V.size(),fast).
  //    */
  //   void init (const NumericVector<T>& V,
  // 	     const bool fast=false);

  /**
   * \f$U(0-N) = s\f$: fill all components.
   */
  NumericVector<T> & operator= (const T s);
    
  /**
   *  \f$U = V\f$: copy all components.
   */
  NumericVector<T> & operator= (const NumericVector<T> &V);

  /**
   *  \f$U = V\f$: copy all components.
   */
  PetscVector<T> & operator= (const PetscVector<T> &V);

  /**
   *  \f$U = V\f$: copy all components.
   */
  NumericVector<T> & operator= (const std::vector<T> &v);

  /**
   * @returns the minimum element in the vector.
   * In case of complex numbers, this returns the minimum
   * Real part.
   */
  Real min () const;
  
  /**
   * @returns the maximum element in the vector.
   * In case of complex numbers, this returns the maximum
   * Real part.
   */
  Real max () const;

  /**
   * @returns the sum of values in a vector
   */
  T sum () const;

  /**
   * @returns the \f$l_1\f$-norm of the vector, i.e.
   * the sum of the absolute values.
   */
  Real l1_norm () const;

  /**
   * @returns the \f$l_2\f$-norm of the vector, i.e.
   * the square root of the sum of the
   * squares of the elements.
   */
  Real l2_norm () const;

  /**
   * @returns the maximum absolute value of the
   * elements of this vector, which is the
   * \f$l_\infty\f$-norm of a vector.
   */
  Real linfty_norm () const;

  /**
   * @returns dimension of the vector. This
   * function was formerly called \p n(), but
   * was renamed to get the \p PetscVector<T> class
   * closer to the C++ standard library's
   * \p std::vector container.
   */
  unsigned int size () const;

  /**
   * @returns the local size of the vector
   * (index_stop-index_start)
   */
  unsigned int local_size() const;

  /**
   * @returns the index of the first vector element
   * actually stored on this processor
   */
  unsigned int first_local_index() const;

  /**
   * @returns the index of the last vector element
   * actually stored on this processor
   */
  unsigned int last_local_index() const;

  /**
   * Maps the global index \p i to the corresponding global index. If
   * the index is not a ghost cell, this is done by subtraction the
   * number of the first local index.  If it is a ghost cell, it has
   * to be looked up in the map.
   */
  unsigned int map_global_to_local_index(const unsigned int i) const;
    
  /**
   * Access components, returns \p U(i).
   */
  T operator() (const unsigned int i) const;
    
  /**
   * Addition operator.
   * Fast equivalent to \p U.add(1, V).
   */
  NumericVector<T> & operator += (const NumericVector<T> &V);

  /**
   * Subtraction operator.
   * Fast equivalent to \p U.add(-1, V).
   */
  NumericVector<T> & operator -= (const NumericVector<T> &V);
    
  /**
   * v(i) = value
   */
  void set (const unsigned int i, const T value);
    
  /**
   * v(i) += value
   */
  void add (const unsigned int i, const T value);
    
  /**
   * \f$U(0-LIBMESH_DIM)+=s\f$.
   * Addition of \p s to all components. Note
   * that \p s is a scalar and not a vector.
   */
  void add (const T s);
    
  /**
   * \f$ U+=V \f$ .
   * Simple vector addition, equal to the
   * \p operator +=.
   */
  void add (const NumericVector<T>& V);

  /**
   * \f$ U+=a*V \f$ .
   * Simple vector addition, equal to the
   * \p operator +=.
   */
  void add (const T a, const NumericVector<T>& v);
  
  /**
   * \f$ U+=v \f$ where \p v is a std::vector<T> 
   * and you
   * want to specify WHERE to add it
   */
  void add_vector (const std::vector<T>& v,
		   const std::vector<unsigned int>& dof_indices);

  /**
   * \f$ U+=V \f$ where U and V are type 
   * \p NumericVector<T> and you
   * want to specify WHERE to add
   * the \p NumericVector<T> V 
   */
  void add_vector (const NumericVector<T>& V,
		   const std::vector<unsigned int>& dof_indices);


  /**
   * \f$U+=A*V\f$, add the product of a \p SparseMatrix \p A
   * and a \p NumericVector \p V to \p this, where \p this=U.
   */
  void add_vector (const NumericVector<T> &V,
		   const SparseMatrix<T> &A);
     
  /**
   * \f$U+=V \f$ where U and V are type 
   * DenseVector<T> and you
   * want to specify WHERE to add
   * the DenseVector<T> V 
   */
  void add_vector (const DenseVector<T>& V,
		   const std::vector<unsigned int>& dof_indices);
  
  /**
   * \f$ U=v \f$ where v is a std::vector<T> 
   * and you want to specify WHERE to insert it
   */
  virtual void insert (const std::vector<T>& v,
		       const std::vector<unsigned int>& dof_indices);

  /**
   * \f$U=V\f$, where U and V are type 
   * NumericVector<T> and you
   * want to specify WHERE to insert
   * the NumericVector<T> V 
   */
  virtual void insert (const NumericVector<T>& V,
		       const std::vector<unsigned int>& dof_indices);
      
  /**
   * \f$ U=V \f$ where V is type 
   * DenseVector<T> and you
   * want to specify WHERE to insert it
   */
  virtual void insert (const DenseVector<T>& V,
		       const std::vector<unsigned int>& dof_indices);
    
  /**
   * \f$ U=V \f$ where V is type 
   * DenseSubVector<T> and you
   * want to specify WHERE to insert it
   */
  virtual void insert (const DenseSubVector<T>& V,
		       const std::vector<unsigned int>& dof_indices);
    
  
  /**
   * Scale each element of the
   * vector by the given factor.
   */
  void scale (const T factor);

  /**
   * Computes the dot product, p = U.V
   */
  virtual T dot(const NumericVector<T>& V) const;
  
  /**
   * Creates a copy of the global vector in the
   * local vector \p v_local.
   */
  void localize (std::vector<T>& v_local) const;

  /**
   * Same, but fills a \p NumericVector<T> instead of
   * a \p std::vector.
   */
  void localize (NumericVector<T>& v_local) const;

  /**
   * Creates a local vector \p v_local containing
   * only information relevant to this processor, as
   * defined by the \p send_list.
   */
  void localize (NumericVector<T>& v_local,
		 const std::vector<unsigned int>& send_list) const;
  
  /**
   * Updates a local vector with selected values from neighboring
   * processors, as defined by \p send_list.
   */
  void localize (const unsigned int first_local_idx,
		 const unsigned int last_local_idx,
		 const std::vector<unsigned int>& send_list);
  
  /**
   * Creates a local copy of the global vector in
   * \p v_local only on processor \p proc_id.  By
   * default the data is sent to processor 0.  This method
   * is useful for outputting data from one processor.
   */
  void localize_to_one (std::vector<T>& v_local,
			const unsigned int proc_id=0) const;
  
  /**
   * Computes the pointwise (i.e. component-wise) product of \p vec1
   * and \p vec2 and stores the result in \p *this.
   */
  virtual void pointwise_mult (const NumericVector<T>& vec1,
			       const NumericVector<T>& vec2);

  /**
   * Print the contents of the vector in Matlab
   * format. Optionally prints the
   * matrix to the file named \p name.  If \p name
   * is not specified it is dumped to the screen.
   */
  void print_matlab(const std::string name="NULL") const;

  /**
   * Creates a "subvector" from this vector using the rows indices
   * of the "rows" array.
   */
  virtual void create_subvector(NumericVector<T>& subvector,
				const std::vector<unsigned int>& rows) const;

  /**
   * Swaps the raw PETSc vector context pointers.
   */
  void swap (PetscVector<T> &v);

  /**
   * Returns the raw PETSc vector context pointer.  Note this is generally
   * not required in user-level code. Just don't do anything crazy like
   * calling VecDestroy()!
   */
  Vec vec () { libmesh_assert (_vec != NULL); return _vec; }


  
private:

  /**
   * Actual Petsc vector datatype
   * to hold vector entries
   */
  Vec _vec;


  /**
   * Type for map that maps global to local ghost cells.
   */
  typedef std::map<unsigned int,unsigned int> GlobalToLocalMap;

  /**
   * Map that maps global to local ghost cells (will be empty if not
   * in ghost cell mode).
   */
  GlobalToLocalMap _global_to_local_map;

  /**
   * This boolean value should only be set to false
   * for the constructor which takes a PETSc Vec object. 
   */
  bool _destroy_vec_on_exit;
};


/*----------------------- Inline functions ----------------------------------*/



template <typename T>
inline
PetscVector<T>::PetscVector (const ParallelType type)
  : _global_to_local_map(),
    _destroy_vec_on_exit(true)
{
  this->_type = type;
}



template <typename T>
inline
PetscVector<T>::PetscVector (const unsigned int n,
                             const ParallelType type)
  : _global_to_local_map(),
    _destroy_vec_on_exit(true)
{
  this->init(n, n, false, type);
}



template <typename T>
inline
PetscVector<T>::PetscVector (const unsigned int n,
			     const unsigned int n_local,
                             const ParallelType type)
  : _global_to_local_map(),
    _destroy_vec_on_exit(true)
{
  this->init(n, n_local, false, type);
}



template <typename T>
inline
PetscVector<T>::PetscVector (const unsigned int n,
			     const unsigned int n_local,
			     const std::vector<unsigned int>& ghost,
                             const ParallelType type)
  : _global_to_local_map(),
    _destroy_vec_on_exit(true)
{
  this->init(n, n_local, ghost, false, type);
}





template <typename T>
inline
PetscVector<T>::PetscVector (Vec v)
  : _global_to_local_map(),
    _destroy_vec_on_exit(false)
{
  this->_vec = v;
  this->_is_initialized = true;

  /* We need to ask PETSc about the (local to global) ghost value
     mapping and create the inverse mapping out of it.  */
  int ierr=0, petsc_size=0, petsc_local_size=0;
  ierr = VecGetSize(_vec, &petsc_size);
  CHKERRABORT(libMesh::COMM_WORLD,ierr);
  ierr = VecGetLocalSize(_vec, &petsc_local_size);
  CHKERRABORT(libMesh::COMM_WORLD,ierr);

  /* \p petsc_local_size is the number of non-ghost values.  If it
     equals the global size, then we are a serial vector, and there
     are no ghost values.  */
  if(petsc_size!=petsc_local_size)
    {
      ISLocalToGlobalMapping mapping = _vec->mapping;

      // If is a sparsely stored vector, set up our new mapping
      if (mapping)
        {
          const unsigned int local_size = static_cast<unsigned int>(petsc_local_size);
          const unsigned int ghost_begin = static_cast<unsigned int>(petsc_local_size);
          const unsigned int ghost_end = static_cast<unsigned int>(mapping->n);
          for(unsigned int i=ghost_begin; i<ghost_end; i++)
	    _global_to_local_map[mapping->indices[i]] = i-local_size;
	}
    }
}




template <typename T>
inline
PetscVector<T>::~PetscVector ()
{
  this->clear ();
}



template <typename T>
inline
void PetscVector<T>::init (const unsigned int n,
			   const unsigned int n_local,
			   const bool fast,
                           const ParallelType type)
{
  int ierr=0;
  int petsc_n=static_cast<int>(n);
  int petsc_n_local=static_cast<int>(n_local);


  // Clear initialized vectors 
  if (this->initialized())
    this->clear();

  if (type == AUTOMATIC)
    {
      if (n == n_local)
        this->_type = SERIAL;
      else
        this->_type = PARALLEL;
    }
  else
    this->_type = type;

  libmesh_assert ((this->_type==SERIAL && n==n_local) ||
                  this->_type==PARALLEL);
  
  // create a sequential vector if on only 1 processor 
  if (this->_type == SERIAL)
    {
      ierr = VecCreateSeq (PETSC_COMM_SELF, petsc_n, &_vec);
             CHKERRABORT(PETSC_COMM_SELF,ierr);
      
      ierr = VecSetFromOptions (_vec);
             CHKERRABORT(PETSC_COMM_SELF,ierr);
    }
  // otherwise create an MPI-enabled vector
  else if (this->_type == PARALLEL)
    {
      libmesh_assert (n_local < n);
      
      ierr = VecCreateMPI (libMesh::COMM_WORLD, petsc_n_local, petsc_n,
			   &_vec);
             CHKERRABORT(libMesh::COMM_WORLD,ierr);
      
      ierr = VecSetFromOptions (_vec);
             CHKERRABORT(libMesh::COMM_WORLD,ierr);
    }  
  else
    libmesh_error();
  
  this->_is_initialized = true;
  
  
  if (fast == false)
    this->zero ();
}



template <typename T>
inline
void PetscVector<T>::init (const unsigned int n,
			   const bool fast,
                           const ParallelType type)
{
  this->init(n,n,fast,type);
}



template <typename T>
inline
void PetscVector<T>::init (const unsigned int n,
			   const unsigned int n_local,
			   const std::vector<unsigned int>& ghost,
			   const bool fast,
                           const ParallelType type)
{
  int ierr=0;
  int petsc_n=static_cast<int>(n);
  int petsc_n_local=static_cast<int>(n_local);
  int petsc_n_ghost=static_cast<int>(ghost.size());
  int* petsc_ghost = ghost.empty() ? PETSC_NULL :
    const_cast<int*>(reinterpret_cast<const int*>(&ghost[0]));

  // Clear initialized vectors 
  if (this->initialized())
    this->clear();

  libmesh_assert(type == AUTOMATIC || type == GHOSTED);
  this->_type = GHOSTED;

  /* Make the global-to-local ghost cell map.  */
  for(unsigned int i=0; i<ghost.size(); i++)
    {
      _global_to_local_map[ghost[i]] = i;
    }

  /* Create vector.  */
  ierr = VecCreateGhost (libMesh::COMM_WORLD, petsc_n_local, petsc_n,
			 petsc_n_ghost, petsc_ghost,
			 &_vec);
  CHKERRABORT(libMesh::COMM_WORLD,ierr);
  
  ierr = VecSetFromOptions (_vec);
  CHKERRABORT(libMesh::COMM_WORLD,ierr);
  
  this->_is_initialized = true;
  
  if (fast == false)
    this->zero ();
}



template <typename T>
inline
void PetscVector<T>::close ()
{
  libmesh_assert (this->initialized());
  
  int ierr=0;
  
  ierr = VecAssemblyBegin(_vec);
  CHKERRABORT(libMesh::COMM_WORLD,ierr);
  ierr = VecAssemblyEnd(_vec);
  CHKERRABORT(libMesh::COMM_WORLD,ierr);

  if(_global_to_local_map.size()!=0)
    {
      ierr = VecGhostUpdateBegin(_vec,ADD_VALUES,SCATTER_REVERSE);
      CHKERRABORT(libMesh::COMM_WORLD,ierr);
      ierr = VecGhostUpdateEnd(_vec,ADD_VALUES,SCATTER_REVERSE);
      CHKERRABORT(libMesh::COMM_WORLD,ierr);
    }

  this->_is_closed = true;
}



template <typename T>
inline
void PetscVector<T>::clear ()
{
  if ((this->initialized()) && (this->_destroy_vec_on_exit))
    {
      int ierr=0;

      ierr = VecDestroy(_vec);
             CHKERRABORT(libMesh::COMM_WORLD,ierr);
    }

  this->_is_closed = this->_is_initialized = false;

  _global_to_local_map.clear();
}



template <typename T>
inline
void PetscVector<T>::zero ()
{
  libmesh_assert (this->initialized());
  
  int ierr=0;

  PetscScalar z=0.;

#if PETSC_VERSION_LESS_THAN(2,3,0)  
  
  // 2.2.x & earlier style
  ierr = VecSet (&z, _vec);
         CHKERRABORT(libMesh::COMM_WORLD,ierr);

#else
  
  // 2.3.x & newer
  ierr = VecSet (_vec, z);
         CHKERRABORT(libMesh::COMM_WORLD,ierr);

#endif
}



template <typename T>
inline
AutoPtr<NumericVector<T> > PetscVector<T>::clone () const
{
  AutoPtr<NumericVector<T> > cloned_vector (new PetscVector<T>);

  *cloned_vector = *this;

  return cloned_vector;
}



template <typename T>
inline
unsigned int PetscVector<T>::size () const
{
  libmesh_assert (this->initialized());
  
  int ierr=0, petsc_size=0;
  
  if (!this->initialized())
    return 0;
  
  ierr = VecGetSize(_vec, &petsc_size);
         CHKERRABORT(libMesh::COMM_WORLD,ierr);

  return static_cast<unsigned int>(petsc_size);
}



template <typename T>
inline
unsigned int PetscVector<T>::local_size () const
{
  libmesh_assert (this->initialized());
  
  int ierr=0, petsc_size=0;
  
  ierr = VecGetLocalSize(_vec, &petsc_size);
         CHKERRABORT(libMesh::COMM_WORLD,ierr);
  
  return static_cast<unsigned int>(petsc_size);
}



template <typename T>
inline
unsigned int PetscVector<T>::first_local_index () const
{
  libmesh_assert (this->initialized());
  
  int ierr=0, petsc_first=0, petsc_last=0;
  
  ierr = VecGetOwnershipRange (_vec, &petsc_first, &petsc_last);
         CHKERRABORT(libMesh::COMM_WORLD,ierr);
  
  return static_cast<unsigned int>(petsc_first);
}



template <typename T>
inline
unsigned int PetscVector<T>::last_local_index () const
{
  libmesh_assert (this->initialized());
  
  int ierr=0, petsc_first=0, petsc_last=0;
  
  ierr = VecGetOwnershipRange (_vec, &petsc_first, &petsc_last);
         CHKERRABORT(libMesh::COMM_WORLD,ierr);
  
  return static_cast<unsigned int>(petsc_last);
}



template <typename T>
inline
unsigned int PetscVector<T>::map_global_to_local_index (const unsigned int i) const
{
  libmesh_assert (this->initialized());

  const unsigned int first = this->first_local_index();
  const unsigned int last = this->last_local_index();

  if((i>=first) && (i<last))
    {
      return i-first;
    }

  GlobalToLocalMap::const_iterator it = _global_to_local_map.find(i);
  libmesh_assert (it!=_global_to_local_map.end());
  return it->second+last-first;
}



template <typename T>
inline
T PetscVector<T>::operator() (const unsigned int i) const
{
  const unsigned int local_index = this->map_global_to_local_index(i);
  libmesh_assert (this->initialized());

  int ierr=0;
  PetscScalar value=0.;

  if(_global_to_local_map.empty())
    {
      PetscScalar *values;
      ierr = VecGetArray(_vec, &values);
      CHKERRABORT(libMesh::COMM_WORLD,ierr);
      value = values[local_index];
      ierr = VecRestoreArray (_vec, &values);
      CHKERRABORT(libMesh::COMM_WORLD,ierr);
    }
  else
    {
      /* Vectors that include ghost values require a special
	 handling.  */
      Vec loc_vec;
      PetscScalar *values;
      ierr = VecGhostGetLocalForm (_vec,&loc_vec);
      CHKERRABORT(libMesh::COMM_WORLD,ierr);
      ierr = VecGetArray(loc_vec, &values);
      CHKERRABORT(libMesh::COMM_WORLD,ierr);
      value = values[local_index];
      ierr = VecRestoreArray (loc_vec, &values);
      CHKERRABORT(libMesh::COMM_WORLD,ierr);
      ierr = VecGhostRestoreLocalForm (_vec,&loc_vec);
      CHKERRABORT(libMesh::COMM_WORLD,ierr);
    }
  
  return static_cast<T>(value);
}



template <typename T>
inline
Real PetscVector<T>::min () const
{
  libmesh_assert (this->initialized());

  int index=0, ierr=0;
  PetscReal min=0.;

  ierr = VecMin (_vec, &index, &min);
         CHKERRABORT(libMesh::COMM_WORLD,ierr);

  // this return value is correct: VecMin returns a PetscReal
  return static_cast<Real>(min);
}



template <typename T>
inline
Real PetscVector<T>::max() const
{
  libmesh_assert (this->initialized());

  int index=0, ierr=0;
  PetscReal max=0.;

  ierr = VecMax (_vec, &index, &max);
         CHKERRABORT(libMesh::COMM_WORLD,ierr);

  // this return value is correct: VecMax returns a PetscReal
  return static_cast<Real>(max);
}



template <typename T>
inline
void PetscVector<T>::swap (PetscVector<T> &v)
{
  std::swap(_vec, v._vec);
  std::swap(_destroy_vec_on_exit, v._destroy_vec_on_exit);
  std::swap(_global_to_local_map, v._global_to_local_map);
}


#endif // #ifdef LIBMESH_HAVE_PETSC
#endif // #ifdef __petsc_vector_h__








