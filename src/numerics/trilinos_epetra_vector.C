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
#include <limits>

// Local Includes
#include "trilinos_epetra_vector.h"

#ifdef LIBMESH_HAVE_TRILINOS

#include "dense_subvector.h"
#include "dense_vector.h"
#include "parallel.h"
#include "utility.h"

// Trilinos Includes
#include <Epetra_LocalMap.h>
#include <Epetra_Comm.h>
#include <Epetra_Map.h>
#include <Epetra_BlockMap.h>
#include <Epetra_Import.h>
#include <Epetra_Export.h>
#include <Epetra_Util.h>
#include <Epetra_IntSerialDenseVector.h>
#include <Epetra_SerialDenseVector.h>
#include <Epetra_Vector.h>

namespace libMesh
{

template <typename T>
T EpetraVector<T>::sum () const
{
  libmesh_assert(this->closed());

  const unsigned int nl = _vec->MyLength();

  T sum=0.0;

  T * values = _vec->Values();

  for (unsigned int i=0; i<nl; i++)
    sum += values[i];

  Parallel::sum<T>(sum);

  return sum;
}

template <typename T>
Real EpetraVector<T>::l1_norm () const
{
  libmesh_assert(this->closed());

  Real value;

  _vec->Norm1(&value);

  return value;
}

template <typename T>
Real EpetraVector<T>::l2_norm () const
{
  libmesh_assert(this->closed());

  Real value;

  _vec->Norm2(&value);

  return value;
}

template <typename T>
Real EpetraVector<T>::linfty_norm () const
{
  libmesh_assert(this->closed());

  Real value;

  _vec->NormInf(&value);

  return value;
}

template <typename T>
NumericVector<T>&
EpetraVector<T>::operator += (const NumericVector<T>& v)
{
  libmesh_assert(this->closed());

  this->add(1., v);

  return *this;
}



template <typename T>
NumericVector<T>&
EpetraVector<T>::operator -= (const NumericVector<T>& v)
{
  libmesh_assert(this->closed());

  this->add(-1., v);

  return *this;
}



template <typename T>
void EpetraVector<T>::set (const unsigned int i_in, const T value_in)
{
  int i = static_cast<int> (i_in);
  T value = value_in;

  libmesh_assert(i_in<this->size());

  ReplaceGlobalValues(1, &i, &value);

  this->_is_closed = false;
}



template <typename T>
void EpetraVector<T>::reciprocal()
{
  // The Epetra::reciprocal() function takes a constant reference to *another* vector,
  // and fills _vec with its reciprocal.  Does that mean we can't pass *_vec as the
  // argument?
  // _vec->reciprocal( *_vec );

  // Alternatively, compute the reciprocal by hand... see also the add(T) member that does this...
  const unsigned int nl = _vec->MyLength();

  T* values = _vec->Values();

  for (unsigned int i=0; i<nl; i++)
    {
      // Don't divide by zero (maybe only check this in debug mode?)
      if (std::abs(values[i]) < std::numeric_limits<T>::min())
        {
          libMesh::err << "Error, divide by zero in DistributedVector<T>::reciprocal()!" << std::endl;
          libmesh_error();
        }

      values[i] = 1. / values[i];
    }

  // Leave the vector in a closed state...
  this->close();
}



template <typename T>
void EpetraVector<T>::add (const unsigned int i_in, const T value_in)
{
  int i = static_cast<int> (i_in);
  T value = value_in;

  libmesh_assert(i_in<this->size());

  SumIntoGlobalValues(1, &i, &value);

  this->_is_closed = false;
}



template <typename T>
void EpetraVector<T>::add_vector (const std::vector<T>& v,
				  const std::vector<unsigned int>& dof_indices)
{
  libmesh_assert (v.size() == dof_indices.size());

  SumIntoGlobalValues (v.size(),
                       (int*) &dof_indices[0],
                       const_cast<T*>(&v[0]));
}



template <typename T>
void EpetraVector<T>::add_vector (const NumericVector<T>& V,
				 const std::vector<unsigned int>& dof_indices)
{
  libmesh_assert (V.size() == dof_indices.size());

  for (unsigned int i=0; i<V.size(); i++)
    this->add (dof_indices[i], V(i));
}



// TODO: fill this in after creating an EpetraMatrix
template <typename T>
void EpetraVector<T>::add_vector (const NumericVector<T>& /* V_in */,
				  const SparseMatrix<T>& /* A_in */)
{
  libmesh_not_implemented();

//   const EpetraVector<T>* V = libmesh_cast_ptr<const EpetraVector<T>*>(&V_in);
//   const EpetraMatrix<T>* A = libmesh_cast_ptr<const EpetraMatrix<T>*>(&A_in);

//   int ierr=0;

//   A->close();

//   // The const_cast<> is not elegant, but it is required since Epetra
//   // is not const-correct.
//   ierr = MatMultAdd(const_cast<EpetraMatrix<T>*>(A)->mat(), V->_vec, _vec, _vec);
//          CHKERRABORT(libMesh::COMM_WORLD,ierr);
}



template <typename T>
void EpetraVector<T>::add_vector (const DenseVector<T>& V_in,
				  const std::vector<unsigned int>& dof_indices)
{
  libmesh_assert (V_in.size() == dof_indices.size());

  SumIntoGlobalValues(dof_indices.size(),
                      (int *)&dof_indices[0],
                      &const_cast<DenseVector<T> *>(&V_in)->get_values()[0]);
}


// TODO: fill this in after creating an EpetraMatrix
template <typename T>
void EpetraVector<T>::add_vector_transpose (const NumericVector<T>& /* V_in */,
				            const SparseMatrix<T>& /* A_in */)
{
  libmesh_not_implemented();
}



template <typename T>
void EpetraVector<T>::add (const T v_in)
{
  const unsigned int nl = _vec->MyLength();

  T * values = _vec->Values();

  for (unsigned int i=0; i<nl; i++)
    values[i]+=v_in;

  this->_is_closed = false;
}


template <typename T>
void EpetraVector<T>::add (const NumericVector<T>& v)
{
  this->add (1., v);
}


template <typename T>
void EpetraVector<T>::add (const T a_in, const NumericVector<T>& v_in)
{
  const EpetraVector<T>* v = libmesh_cast_ptr<const EpetraVector<T>*>(&v_in);

  libmesh_assert(this->size() == v->size());

  _vec->Update(a_in,*v->_vec, 1.);
}



template <typename T>
void EpetraVector<T>::insert (const std::vector<T>& v,
			      const std::vector<unsigned int>& dof_indices)
{
  libmesh_assert (v.size() == dof_indices.size());

  ReplaceGlobalValues (v.size(),
                       (int*) &dof_indices[0],
                       const_cast<T*>(&v[0]));
}



template <typename T>
void EpetraVector<T>::insert (const NumericVector<T>& V,
			      const std::vector<unsigned int>& dof_indices)
{
  libmesh_assert (V.size() == dof_indices.size());

  // TODO: If V is an EpetraVector this can be optimized
  for (unsigned int i=0; i<V.size(); i++)
    this->set (dof_indices[i], V(i));
}



template <typename T>
void EpetraVector<T>::insert (const DenseVector<T>& v,
			      const std::vector<unsigned int>& dof_indices)
{
  libmesh_assert (v.size() == dof_indices.size());

  std::vector<T> &vals = const_cast<DenseVector<T>&>(v).get_values();

  ReplaceGlobalValues (v.size(),
                       (int*) &dof_indices[0],
                       &vals[0]);
}



template <typename T>
void EpetraVector<T>::insert (const DenseSubVector<T>& v,
			      const std::vector<unsigned int>& dof_indices)
{
  libmesh_assert (v.size() == dof_indices.size());

  for (unsigned int i=0; i < v.size(); ++i)
    this->set (dof_indices[i], v(i));
}



template <typename T>
void EpetraVector<T>::scale (const T factor_in)
{
  _vec->Scale(factor_in);
}

template <typename T>
void EpetraVector<T>::abs()
{
  _vec->Abs(*_vec);
}


template <typename T>
T EpetraVector<T>::dot (const NumericVector<T>& V_in) const
{
  const EpetraVector<T>* V = libmesh_cast_ptr<const EpetraVector<T>*>(&V_in);

  T result=0.0;

  _vec->Dot(*V->_vec, &result);

  return result;
}


template <typename T>
void EpetraVector<T>::pointwise_mult (const NumericVector<T>& vec1,
                                      const NumericVector<T>& vec2)
{
  const EpetraVector<T>* V1 = libmesh_cast_ptr<const EpetraVector<T>*>(&vec1);
  const EpetraVector<T>* V2 = libmesh_cast_ptr<const EpetraVector<T>*>(&vec2);

  _vec->Multiply(1.0, *V1->_vec, *V2->_vec, 0.0);
}


template <typename T>
NumericVector<T>&
EpetraVector<T>::operator = (const T s_in)
{
  _vec->PutScalar(s_in);

  return *this;
}



template <typename T>
NumericVector<T>&
EpetraVector<T>::operator = (const NumericVector<T>& v_in)
{
  const EpetraVector<T>* v = libmesh_cast_ptr<const EpetraVector<T>*>(&v_in);

  *this = *v;

  return *this;
}



template <typename T>
EpetraVector<T>&
EpetraVector<T>::operator = (const EpetraVector<T>& v)
{
  (*_vec) = *v._vec;

  return *this;
}



template <typename T>
NumericVector<T>&
EpetraVector<T>::operator = (const std::vector<T>& v)
{
  T * values = _vec->Values();

  /**
   * Case 1:  The vector is the same size of
   * The global vector.  Only add the local components.
   */
  if(this->size() == v.size())
  {
    const unsigned int nl=this->local_size();
    const unsigned int fli=this->first_local_index();

    for(unsigned int i=0;i<nl;i++)
      values[i]=v[fli+i];
  }

  /**
   * Case 2: The vector is the same size as our local
   * piece.  Insert directly to the local piece.
   */
  else
  {
    libmesh_assert(v.size()==this->local_size());

    const unsigned int nl=this->local_size();

    for(unsigned int i=0;i<nl;i++)
      values[i]=v[i];
  }

  return *this;
}



template <typename T>
void EpetraVector<T>::localize (NumericVector<T>& v_local_in) const
{
  EpetraVector<T>* v_local = libmesh_cast_ptr<EpetraVector<T>*>(&v_local_in);

  Epetra_Map rootMap = Epetra_Util::Create_Root_Map( *_map, -1);
  v_local->_vec->ReplaceMap(rootMap);

  Epetra_Import importer(v_local->_vec->Map(), *_map);
  v_local->_vec->Import(*_vec, importer, Insert);
}



template <typename T>
void EpetraVector<T>::localize (NumericVector<T>& v_local_in,
				const std::vector<unsigned int>& /* send_list */) const
{
  // TODO: optimize to sync only the send list values
  this->localize(v_local_in);

//   EpetraVector<T>* v_local =
//   libmesh_cast_ptr<EpetraVector<T>*>(&v_local_in);

//   libmesh_assert (this->_map.get() != NULL);
//   libmesh_assert (v_local->_map.get() != NULL);
//   libmesh_assert (v_local->local_size() == this->size());
//   libmesh_assert (send_list.size() <= v_local->size());

//   Epetra_Import importer (*v_local->_map, *this->_map);

//   v_local->_vec->Import (*this->_vec, importer, Insert);
}


template <typename T>
void EpetraVector<T>::localize (const unsigned int /* first_local_idx */,
				const unsigned int /* last_local_idx */,
				const std::vector<unsigned int>& /* send_list */)
{
  libmesh_not_implemented();

//   // Only good for serial vectors.
//   libmesh_assert (this->size() == this->local_size());
//   libmesh_assert (last_local_idx > first_local_idx);
//   libmesh_assert (send_list.size() <= this->size());
//   libmesh_assert (last_local_idx < this->size());

//   const unsigned int size       = this->size();
//   const unsigned int local_size = (last_local_idx - first_local_idx + 1);
//   int ierr=0;

//   // Don't bother for serial cases
//   if ((first_local_idx == 0) &&
//       (local_size == size))
//     return;


//   // Build a parallel vector, initialize it with the local
//   // parts of (*this)
//   EpetraVector<T> parallel_vec;

//   parallel_vec.init (size, local_size, true, PARALLEL);


//   // Copy part of *this into the parallel_vec
//   {
//     IS is;
//     VecScatter scatter;

//     // Create idx, idx[i] = i+first_local_idx;
//     std::vector<int> idx(local_size);
//     Utility::iota (idx.begin(), idx.end(), first_local_idx);

//     // Create the index set & scatter object
//     ierr = ISCreateGeneral(libMesh::COMM_WORLD, local_size, &idx[0], &is);
//            CHKERRABORT(libMesh::COMM_WORLD,ierr);

//     ierr = VecScatterCreate(_vec,              is,
// 			    parallel_vec._vec, is,
// 			    &scatter);
//            CHKERRABORT(libMesh::COMM_WORLD,ierr);

//     // Perform the scatter
// #if EPETRA_VERSION_LESS_THAN(2,3,3)

//     ierr = VecScatterBegin(_vec, parallel_vec._vec, INSERT_VALUES,
// 			   SCATTER_FORWARD, scatter);
//            CHKERRABORT(libMesh::COMM_WORLD,ierr);

//     ierr = VecScatterEnd  (_vec, parallel_vec._vec, INSERT_VALUES,
// 			   SCATTER_FORWARD, scatter);
//            CHKERRABORT(libMesh::COMM_WORLD,ierr);

// #else

//       // API argument order change in Epetra 2.3.3
//     ierr = VecScatterBegin(scatter, _vec, parallel_vec._vec,
// 			   INSERT_VALUES, SCATTER_FORWARD);
//            CHKERRABORT(libMesh::COMM_WORLD,ierr);

//     ierr = VecScatterEnd  (scatter, _vec, parallel_vec._vec,
// 			   INSERT_VALUES, SCATTER_FORWARD);
//            CHKERRABORT(libMesh::COMM_WORLD,ierr);

// #endif

//     // Clean up
//     ierr = LibMeshISDestroy (is);
//            CHKERRABORT(libMesh::COMM_WORLD,ierr);

//     ierr = LibMeshVecScatterDestroy(scatter);
//            CHKERRABORT(libMesh::COMM_WORLD,ierr);
//   }

//   // localize like normal
//   parallel_vec.close();
//   parallel_vec.localize (*this, send_list);
//   this->close();
}



template <typename T>
void EpetraVector<T>::localize (std::vector<T>& v_local) const
{
  // This function must be run on all processors at once
  parallel_only();

  const unsigned int n  = this->size();
  const unsigned int nl = this->local_size();

  libmesh_assert (this->_vec != NULL);

  v_local.clear();
  v_local.reserve(n);

  // build up my local part
  for (unsigned int i=0; i<nl; i++)
    v_local.push_back((*this->_vec)[i]);

  Parallel::allgather (v_local);
}



template <typename T>
void EpetraVector<T>::localize_to_one (std::vector<T>&  v_local,
				       const unsigned int  pid) const
{
  // This function must be run on all processors at once
  parallel_only();

  const unsigned int n  = this->size();
  const unsigned int nl = this->local_size();

  libmesh_assert (pid < libMesh::n_processors());
  libmesh_assert (this->_vec != NULL);

  v_local.clear();
  v_local.reserve(n);


  // build up my local part
  for (unsigned int i=0; i<nl; i++)
    v_local.push_back((*this->_vec)[i]);

  Parallel::gather (pid, v_local);
}



template <typename T>
void EpetraVector<T>::print_matlab (const std::string /* name */) const
{
  libmesh_not_implemented();

//   libmesh_assert (this->initialized());
//   libmesh_assert (this->closed());

//   int ierr=0;
//   EpetraViewer epetra_viewer;


//   ierr = EpetraViewerCreate (libMesh::COMM_WORLD,
// 			    &epetra_viewer);
//          CHKERRABORT(libMesh::COMM_WORLD,ierr);

//   /**
//    * Create an ASCII file containing the matrix
//    * if a filename was provided.
//    */
//   if (name != "NULL")
//     {
//       ierr = EpetraViewerASCIIOpen( libMesh::COMM_WORLD,
// 				   name.c_str(),
// 				   &epetra_viewer);
//              CHKERRABORT(libMesh::COMM_WORLD,ierr);

//       ierr = EpetraViewerSetFormat (epetra_viewer,
// 				   EPETRA_VIEWER_ASCII_MATLAB);
//              CHKERRABORT(libMesh::COMM_WORLD,ierr);

//       ierr = VecView (_vec, epetra_viewer);
//              CHKERRABORT(libMesh::COMM_WORLD,ierr);
//     }

//   /**
//    * Otherwise the matrix will be dumped to the screen.
//    */
//   else
//     {
//       ierr = EpetraViewerSetFormat (EPETRA_VIEWER_STDOUT_WORLD,
// 				   EPETRA_VIEWER_ASCII_MATLAB);
//              CHKERRABORT(libMesh::COMM_WORLD,ierr);

//       ierr = VecView (_vec, EPETRA_VIEWER_STDOUT_WORLD);
//              CHKERRABORT(libMesh::COMM_WORLD,ierr);
//     }


//   /**
//    * Destroy the viewer.
//    */
//   ierr = EpetraViewerDestroy (epetra_viewer);
//          CHKERRABORT(libMesh::COMM_WORLD,ierr);
}





template <typename T>
void EpetraVector<T>::create_subvector(NumericVector<T>& /* subvector */,
				       const std::vector<unsigned int>& /* rows */) const
{
  libmesh_not_implemented();

//   // Epetra data structures
//   IS parent_is, subvector_is;
//   VecScatter scatter;
//   int ierr = 0;

//   // Make sure the passed int subvector is really a EpetraVector
//   EpetraVector<T>* epetra_subvector = libmesh_cast_ptr<EpetraVector<T>*>(&subvector);
//   libmesh_assert(epetra_subvector != NULL);

//   // If the epetra_subvector is already initialized, we assume that the
//   // user has already allocated the *correct* amount of space for it.
//   // If not, we use the appropriate Epetra routines to initialize it.
//   if (!epetra_subvector->initialized())
//     {
//       // Initialize the epetra_subvector to have enough space to hold
//       // the entries which will be scattered into it.  Note: such an
//       // init() function (where we let Epetra decide the number of local
//       // entries) is not currently offered by the EpetraVector
//       // class.  Should we differentiate here between sequential and
//       // parallel vector creation based on libMesh::n_processors() ?
//       ierr = VecCreateMPI(libMesh::COMM_WORLD,
// 			  EPETRA_DECIDE,          // n_local
// 			  rows.size(),           // n_global
// 			  &(epetra_subvector->_vec)); CHKERRABORT(libMesh::COMM_WORLD,ierr);

//       ierr = VecSetFromOptions (epetra_subvector->_vec); CHKERRABORT(libMesh::COMM_WORLD,ierr);

//       // Mark the subvector as initialized
//       epetra_subvector->_is_initialized = true;
//     }

//   // Use iota to fill an array with entries [0,1,2,3,4,...rows.size()]
//   std::vector<int> idx(rows.size());
//   Utility::iota (idx.begin(), idx.end(), 0);

//   // Construct index sets
//   ierr = ISCreateGeneral(libMesh::COMM_WORLD,
// 			 rows.size(),
// 			 (int*) &rows[0],
// 			 &parent_is); CHKERRABORT(libMesh::COMM_WORLD,ierr);

//   ierr = ISCreateGeneral(libMesh::COMM_WORLD,
// 			 rows.size(),
// 			 (int*) &idx[0],
// 			 &subvector_is); CHKERRABORT(libMesh::COMM_WORLD,ierr);

//   // Construct the scatter object
//   ierr = VecScatterCreate(this->_vec,
// 			  parent_is,
// 			  epetra_subvector->_vec,
// 			  subvector_is,
// 			  &scatter); CHKERRABORT(libMesh::COMM_WORLD,ierr);

//   // Actually perform the scatter
// #if EPETRA_VERSION_LESS_THAN(2,3,3)
//   ierr = VecScatterBegin(this->_vec,
// 			 epetra_subvector->_vec,
// 			 INSERT_VALUES,
// 			 SCATTER_FORWARD,
// 			 scatter); CHKERRABORT(libMesh::COMM_WORLD,ierr);

//   ierr = VecScatterEnd(this->_vec,
// 		       epetra_subvector->_vec,
// 		       INSERT_VALUES,
// 		       SCATTER_FORWARD,
// 		       scatter); CHKERRABORT(libMesh::COMM_WORLD,ierr);
// #else
//   // API argument order change in Epetra 2.3.3
//   ierr = VecScatterBegin(scatter,
// 			 this->_vec,
// 			 epetra_subvector->_vec,
// 			 INSERT_VALUES,
// 			 SCATTER_FORWARD); CHKERRABORT(libMesh::COMM_WORLD,ierr);

//   ierr = VecScatterEnd(scatter,
// 		       this->_vec,
// 		       epetra_subvector->_vec,
// 		       INSERT_VALUES,
// 		       SCATTER_FORWARD); CHKERRABORT(libMesh::COMM_WORLD,ierr);
// #endif

//   // Clean up
//   ierr = LibMeshISDestroy(parent_is);       CHKERRABORT(libMesh::COMM_WORLD,ierr);
//   ierr = LibMeshISDestroy(subvector_is);    CHKERRABORT(libMesh::COMM_WORLD,ierr);
//   ierr = LibMeshVecScatterDestroy(scatter); CHKERRABORT(libMesh::COMM_WORLD,ierr);
}


/*********************************************************************
 * The following were copied (and slightly modified) from
 * Epetra_FEVector.h in order to allow us to use a standard
 * Epetra_Vector... which is more compatible with other Trilinos
 * packages such as NOX.  All of this code is originally under LGPL
 *********************************************************************/

//----------------------------------------------------------------------------
template <typename T>
int EpetraVector<T>::SumIntoGlobalValues(int numIDs, const int* GIDs,
			                 const double* values)
{
  return( inputValues( numIDs, GIDs, values, true) );
}

//----------------------------------------------------------------------------
template <typename T>
int EpetraVector<T>::SumIntoGlobalValues(const Epetra_IntSerialDenseVector& GIDs,
			                 const Epetra_SerialDenseVector& values)
{
  if (GIDs.Length() != values.Length()) {
    return(-1);
  }

  return( inputValues( GIDs.Length(), GIDs.Values(), values.Values(), true) );
}

//----------------------------------------------------------------------------
template <typename T>
int EpetraVector<T>::SumIntoGlobalValues(int numIDs, const int* GIDs,
					 const int* numValuesPerID,
			                 const double* values)
{
  return( inputValues( numIDs, GIDs, numValuesPerID, values, true) );
}

//----------------------------------------------------------------------------
template <typename T>
int EpetraVector<T>::ReplaceGlobalValues(int numIDs, const int* GIDs,
			                 const double* values)
{
  return( inputValues( numIDs, GIDs, values, false) );
}

//----------------------------------------------------------------------------
template <typename T>
int EpetraVector<T>::ReplaceGlobalValues(const Epetra_IntSerialDenseVector& GIDs,
			                 const Epetra_SerialDenseVector& values)
{
  if (GIDs.Length() != values.Length()) {
    return(-1);
  }

  return( inputValues( GIDs.Length(), GIDs.Values(), values.Values(), false) );
}

//----------------------------------------------------------------------------
template <typename T>
int EpetraVector<T>::ReplaceGlobalValues(int numIDs, const int* GIDs,
					 const int* numValuesPerID,
			                 const double* values)
{
  return( inputValues( numIDs, GIDs, numValuesPerID, values, false) );
}

//----------------------------------------------------------------------------
template <typename T>
int EpetraVector<T>::inputValues(int numIDs,
                                 const int* GIDs,
                                 const double* values,
                                 bool accumulate)
{
 //Important note!! This method assumes that there is only 1 point
 //associated with each element.

  for(int i=0; i<numIDs; ++i) {
    if (_vec->Map().MyGID(GIDs[i])) {
      if (accumulate) {
        _vec->SumIntoGlobalValue(GIDs[i], 0, 0, values[i]);
      }
      else {
        _vec->ReplaceGlobalValue(GIDs[i], 0, 0, values[i]);
      }
    }
    else {
      if (!ignoreNonLocalEntries_) {
	EPETRA_CHK_ERR( inputNonlocalValue(GIDs[i], values[i], accumulate) );
      }
    }
  }

  return(0);
}

//----------------------------------------------------------------------------
template <typename T>
int EpetraVector<T>::inputValues(int numIDs,
                                 const int* GIDs,
				 const int* numValuesPerID,
                                 const double* values,
                                 bool accumulate)
{
  int offset=0;
  for(int i=0; i<numIDs; ++i) {
    int numValues = numValuesPerID[i];
    if (_vec->Map().MyGID(GIDs[i])) {
      if (accumulate) {
	for(int j=0; j<numValues; ++j) {
	  _vec->SumIntoGlobalValue(GIDs[i], j, 0, values[offset+j]);
	}
      }
      else {
	for(int j=0; j<numValues; ++j) {
	  _vec->ReplaceGlobalValue(GIDs[i], j, 0, values[offset+j]);
	}
      }
    }
    else {
      if (!ignoreNonLocalEntries_) {
	EPETRA_CHK_ERR( inputNonlocalValues(GIDs[i], numValues,
					    &(values[offset]), accumulate) );
      }
    }
    offset += numValues;
  }

  return(0);
}

//----------------------------------------------------------------------------
template <typename T>
int EpetraVector<T>::inputNonlocalValue(int GID, double value, bool accumulate)
{
  int insertPoint = -1;

  //find offset of GID in nonlocalIDs_
  int offset = Epetra_Util_binary_search(GID, nonlocalIDs_, numNonlocalIDs_,
					 insertPoint);
  if (offset >= 0) {
    //if offset >= 0
    //  put value in nonlocalCoefs_[offset][0]

    if (accumulate) {
      nonlocalCoefs_[offset][0] += value;
    }
    else {
      nonlocalCoefs_[offset][0] = value;
    }
  }
  else {
    //else
    //  insert GID in nonlocalIDs_
    //  insert 1   in nonlocalElementSize_
    //  insert value in nonlocalCoefs_

    int tmp1 = numNonlocalIDs_;
    int tmp2 = allocatedNonlocalLength_;
    int tmp3 = allocatedNonlocalLength_;
    EPETRA_CHK_ERR( Epetra_Util_insert(GID, insertPoint, nonlocalIDs_,
				       tmp1, tmp2) );
    --tmp1;
    EPETRA_CHK_ERR( Epetra_Util_insert(1, insertPoint, nonlocalElementSize_,
				       tmp1, tmp3) );
    double* values = new double[1];
    values[0] = value;
    EPETRA_CHK_ERR( Epetra_Util_insert(values, insertPoint, nonlocalCoefs_,
				       numNonlocalIDs_, allocatedNonlocalLength_) );
  }

  return(0);
}

//----------------------------------------------------------------------------
template <typename T>
int EpetraVector<T>::inputNonlocalValues(int GID, int numValues,
					 const double* values, bool accumulate)
{
  int insertPoint = -1;

  //find offset of GID in nonlocalIDs_
  int offset = Epetra_Util_binary_search(GID, nonlocalIDs_, numNonlocalIDs_,
					 insertPoint);
  if (offset >= 0) {
    //if offset >= 0
    //  put value in nonlocalCoefs_[offset][0]

    if (numValues != nonlocalElementSize_[offset]) {
      cerr << "Epetra_FEVector ERROR: block-size for GID " << GID << " is "
	   << numValues<<" which doesn't match previously set block-size of "
	   << nonlocalElementSize_[offset] << endl;
      return(-1);
    }

    if (accumulate) {
      for(int j=0; j<numValues; ++j) {
	nonlocalCoefs_[offset][j] += values[j];
      }
    }
    else {
      for(int j=0; j<numValues; ++j) {
	nonlocalCoefs_[offset][j] = values[j];
      }
    }
  }
  else {
    //else
    //  insert GID in nonlocalIDs_
    //  insert numValues   in nonlocalElementSize_
    //  insert values in nonlocalCoefs_

    int tmp1 = numNonlocalIDs_;
    int tmp2 = allocatedNonlocalLength_;
    int tmp3 = allocatedNonlocalLength_;
    EPETRA_CHK_ERR( Epetra_Util_insert(GID, insertPoint, nonlocalIDs_,
				       tmp1, tmp2) );
    --tmp1;
    EPETRA_CHK_ERR( Epetra_Util_insert(numValues, insertPoint, nonlocalElementSize_,
				       tmp1, tmp3) );
    double* newvalues = new double[numValues];
    for(int j=0; j<numValues; ++j) {
      newvalues[j] = values[j];
    }
    EPETRA_CHK_ERR( Epetra_Util_insert(newvalues, insertPoint, nonlocalCoefs_,
				       numNonlocalIDs_, allocatedNonlocalLength_) );
  }

  return(0);
}

//----------------------------------------------------------------------------
template <typename T>
int EpetraVector<T>::GlobalAssemble(Epetra_CombineMode mode)
{
  //In this method we need to gather all the non-local (overlapping) data
  //that's been input on each processor, into the (probably) non-overlapping
  //distribution defined by the map that 'this' vector was constructed with.

  //We don't need to do anything if there's only one processor or if
  //ignoreNonLocalEntries_ is true.
  if (_vec->Map().Comm().NumProc() < 2 || ignoreNonLocalEntries_) {
    return(0);
  }



  //First build a map that describes the data in nonlocalIDs_/nonlocalCoefs_.
  //We'll use the arbitrary distribution constructor of Map.

  Epetra_BlockMap sourceMap(-1, numNonlocalIDs_,
                            nonlocalIDs_, nonlocalElementSize_,
			    _vec->Map().IndexBase(), _vec->Map().Comm());

  //Now build a vector to hold our nonlocalCoefs_, and to act as the source-
  //vector for our import operation.
  Epetra_MultiVector nonlocalVector(sourceMap, 1);

  int i,j;
  for(i=0; i<numNonlocalIDs_; ++i) {
    for(j=0; j<nonlocalElementSize_[i]; ++j) {
      nonlocalVector.ReplaceGlobalValue(nonlocalIDs_[i], j, 0,
					nonlocalCoefs_[i][j]);
    }
  }

  Epetra_Export exporter(sourceMap, _vec->Map());

  EPETRA_CHK_ERR( _vec->Export(nonlocalVector, exporter, mode) );

  destroyNonlocalData();

  return(0);
}


//----------------------------------------------------------------------------
template <typename T>
void EpetraVector<T>::FEoperatorequals(const EpetraVector& source)
{
  (*_vec) = *(source._vec);

  destroyNonlocalData();

  if (source.allocatedNonlocalLength_ > 0) {
    allocatedNonlocalLength_ = source.allocatedNonlocalLength_;
    numNonlocalIDs_ = source.numNonlocalIDs_;
    nonlocalIDs_ = new int[allocatedNonlocalLength_];
    nonlocalElementSize_ = new int[allocatedNonlocalLength_];
    nonlocalCoefs_ = new double*[allocatedNonlocalLength_];
    for(int i=0; i<numNonlocalIDs_; ++i) {
      int elemSize = source.nonlocalElementSize_[i];
      nonlocalCoefs_[i] = new double[elemSize];
      nonlocalIDs_[i] = source.nonlocalIDs_[i];
      nonlocalElementSize_[i] = elemSize;
      for(int j=0; j<elemSize; ++j) {
	nonlocalCoefs_[i][j] = source.nonlocalCoefs_[i][j];
      }
    }
  }
}


//----------------------------------------------------------------------------
template <typename T>
void EpetraVector<T>::destroyNonlocalData()
{
  if (allocatedNonlocalLength_ > 0) {
    delete [] nonlocalIDs_;
    delete [] nonlocalElementSize_;
    nonlocalIDs_ = NULL;
    nonlocalElementSize_ = NULL;
    for(int i=0; i<numNonlocalIDs_; ++i) {
      delete [] nonlocalCoefs_[i];
    }
    delete [] nonlocalCoefs_;
    nonlocalCoefs_ = NULL;
    numNonlocalIDs_ = 0;
    allocatedNonlocalLength_ = 0;
  }
  return;
}


//------------------------------------------------------------------
// Explicit instantiations
template class EpetraVector<Number>;

} // namespace libMesh

#endif // #ifdef HAVE_EPETRA
