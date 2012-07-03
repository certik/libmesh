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



#ifndef __type_tensor_h__
#define __type_tensor_h__

// Local includes
#include "libmesh_common.h"
#include "type_vector.h"

// C++ includes
#include <cmath>

namespace libMesh
{

// Forward declarations
template <typename T> class TypeTensorColumn;
template <typename T> class ConstTypeTensorColumn;
template <unsigned int N, typename T> class TypeNTensor;



/**
 * This class defines a tensor in \p LIBMESH_DIM dimensional space of type T.
 * T may either be Real or Complex.  The default constructor for
 * this class is protected, suggesting that you should not instantiate
 * one of these directly.
 *
 * \author Roy Stogner, 2004.
 */

template <typename T>
class TypeTensor
{
template <typename T2>
friend class TypeTensor;

protected:

  /**
   * Empty constructor.  Gives the tensor 0 in \p LIBMESH_DIM
   * dimensions.
   */
  TypeTensor  ();

  /**
   * Constructor.  By default sets higher dimensional entries to 0.
   * This is a poor constructor for 2D tensors - if the default
   * arguments are to be overridden it requires that the "xz = 0."
   * etc. arguments also be given explicitly.
   */
  template <typename Scalar>
  explicit TypeTensor  (const Scalar xx,
	                const Scalar xy=0,
	                const Scalar xz=0,
	                const Scalar yx=0,
	                const Scalar yy=0,
	                const Scalar yz=0,
	                const Scalar zx=0,
	                const Scalar zy=0,
                        typename
                          boostcopy::enable_if_c<ScalarTraits<Scalar>::value,
                                                 const Scalar>::type zz=0);

  /**
   * Constructor.  Assigns each vector to a different row of the
   * tensor.  We're in LIBMESH_DIM space dimensions and so LIBMESH_DIM
   * many vectors are needed.
   */
  template<typename T2>
  TypeTensor(const TypeVector<T2>& vx);

  template<typename T2>
  TypeTensor(const TypeVector<T2>& vx, const TypeVector<T2> &vy);

  template<typename T2>
  TypeTensor(const TypeVector<T2>& vx, const TypeVector<T2> &vy, const TypeVector<T2> &vz);

public:

  /**
   * Copy-constructor.
   */
  template<typename T2>
  TypeTensor(const TypeTensor<T2>& p);

  /**
   * Destructor.
   */
  ~TypeTensor();

  /**
   * Assign to a tensor without creating a temporary.
   */
  template<typename T2>
  void assign (const TypeTensor<T2> &);

  /**
   * Assignment-from-scalar operator.  Used only to zero out vectors.
   */
  template <typename Scalar>
  typename boostcopy::enable_if_c<
    ScalarTraits<Scalar>::value,
    TypeTensor&>::type
  operator = (const Scalar& p)
  { libmesh_assert(p == Scalar(0)); this->zero(); return *this; }

  /**
   * Return the \f$ i,j^{th} \f$ element of the tensor.
   */
  const T & operator () (const unsigned int i, const unsigned int j) const;

  /**
   * Return a writeable reference to the \f$ i,j^{th} \f$ element of the
   * tensor.
   */
  T & operator () (const unsigned int i, const unsigned int j);

  /**
   * Return a proxy for the \f$ i^{th} \f$ column of the tensor.
   */
  ConstTypeTensorColumn<T> slice (const unsigned int i) const;

  /**
   * Return a writeable proxy for the \f$ i^{th} \f$ column of the tensor.
   */
  TypeTensorColumn<T> slice (const unsigned int i);

  /**
   * Return one row of the tensor as a TypeVector.
   */
  TypeVector<T> row(const unsigned int r);

  /**
   * Add two tensors.
   */
  template<typename T2>
  TypeTensor<typename CompareTypes<T, T2>::supertype>
  operator + (const TypeTensor<T2> &) const;

  /**
   * Add to this tensor.
   */
  template<typename T2>
  const TypeTensor<T> & operator += (const TypeTensor<T2> &);

  /**
   * Add to this tensor without creating a temporary.
   */
  template<typename T2>
  void add (const TypeTensor<T2> &);

  /**
   * Add a scaled tensor to this tensor without
   * creating a temporary.
   */
  template <typename T2>
  void add_scaled (const TypeTensor<T2> &, const T);

  /**
   * Subtract two tensors.
   */
  template<typename T2>
  TypeTensor<typename CompareTypes<T, T2>::supertype>
  operator - (const TypeTensor<T2> &) const;

  /**
   * Subtract from this tensor.
   */
  template<typename T2>
  const TypeTensor<T> & operator -= (const TypeTensor<T2> &);

  /**
   * Subtract from this tensor without creating a temporary.
   */
  template<typename T2>
  void subtract (const TypeTensor<T2> &);

  /**
   * Subtract a scaled value from this tensor without
   * creating a temporary.
   */
  template <typename T2>
  void subtract_scaled (const TypeTensor<T2> &, const T);

  /**
   * Return the opposite of a tensor
   */
  TypeTensor<T> operator - () const;

  /**
   * Multiply a tensor by a number, i.e. scale.
   */
  template <typename Scalar>
  typename boostcopy::enable_if_c<
    ScalarTraits<Scalar>::value,
    TypeTensor<typename CompareTypes<T, Scalar>::supertype> >::type
  operator * (const Scalar) const;

  /**
   * Multiply this tensor by a number, i.e. scale.
   */
  template <typename Scalar>
  const TypeTensor<T> & operator *= (const Scalar);

  /**
   * Divide a tensor by a number, i.e. scale.
   */
  template <typename Scalar>
  typename boostcopy::enable_if_c<
    ScalarTraits<Scalar>::value,
    TypeTensor<typename CompareTypes<T, Scalar>::supertype> >::type
  operator / (const Scalar) const;

  /**
   * Divide this tensor by a number, i.e. scale.
   */
  const TypeTensor<T> & operator /= (const T);

  /**
   * Multiply 2 tensors together, i.e. matrix product.
   * The tensors may be of different types.
   */
  template <typename T2>
  TypeTensor<T> operator * (const TypeTensor<T2> &) const;

  /**
   * Multiply 2 tensors together, i.e. dyadic product
   * sum_ij Aij*Bij.
   * The tensors may be of different types.
   */
  template <typename T2>
  typename CompareTypes<T,T2>::supertype
  contract (const TypeTensor<T2> &) const;

  /**
   * Multiply a tensor and vector together, i.e. matrix-vector product.
   * The tensor and vector may be of different types.
   */
  template <typename T2>
  TypeVector<typename CompareTypes<T,T2>::supertype>
  operator * (const TypeVector<T2> &) const;

  /**
   * The transpose (with complex numbers not conjugated) of the tensor.
   */
  TypeTensor<T> transpose() const;

  /**
   * Returns the Frobenius norm of the tensor, i.e. the square-root of
   * the sum of the elements squared.
   */
  Real size() const;

  /**
   * Returns the Frobenius norm of the tensor squared, i.e.  sum of the
   * element magnitudes squared.
   */
  Real size_sq() const;

  /**
   * Returns the determinant of the tensor.  Because these are 3x3
   * tensors at most, we don't do an LU decomposition like DenseMatrix
   * does.
   */
  T det() const;

  /**
   * Returns the trace of the tensor.
   */
  T tr() const;

  /**
   * Zero the tensor in any dimension.
   */
  void zero();

  /**
   * @returns \p true if two tensors are equal valued.
   */
  bool operator == (const TypeTensor<T>& rhs) const;

  /**
   * @returns \p true if this tensor is "less"
   * than another.  Useful for sorting.
   */
  bool operator < (const TypeTensor<T>& rhs) const;

  /**
   * @returns \p true if this tensor is "greater"
   * than another.
   */
  bool operator > (const TypeTensor<T>& rhs) const;

  /**
   * Formatted print, by default to \p libMesh::out.
   */
  void print(std::ostream& os = libMesh::out) const;

  /**
   * Formatted print as above but allows you to do
   * std::cout << t << std::endl;
   */
  friend std::ostream& operator << (std::ostream& os, const TypeTensor<T>& t)
  {
    t.print(os);
    return os;
  }

  /**
   * Unformatted print to the stream \p out.  Simply prints the elements
   * of the tensor separated by spaces and newlines.
   */
  void write_unformatted (std::ostream &out, const bool newline = true) const;

  // protected:

  /**
   * The coordinates of the \p TypeTensor
   */
  T _coords[LIBMESH_DIM*LIBMESH_DIM];
};


template <typename T>
class TypeTensorColumn
{
public:

  TypeTensorColumn(TypeTensor<T> &tensor,
                   unsigned int j) :
     _tensor(&tensor), _j(j) {}

  /**
   * Return a writeable reference to the \f$ i,this \f$ element of the
   * tensor.
   */
  T & operator () (const unsigned int i)
    { return (*_tensor)(i,_j); }

  T & slice (const unsigned int i)
    { return (*_tensor)(i,_j); }

  /**
   * Assign values to this column of the tensor.
   */
  TypeTensorColumn<T>& operator = (const TypeVector<T>& rhs)
    { 
      for (unsigned int i=0; i != LIBMESH_DIM; ++i)
        (*this)(i) = rhs(i);
      return *this;
    }

private:
  TypeTensor<T> *_tensor;
  const unsigned int _j;
};


template <typename T>
class ConstTypeTensorColumn
{
public:

  ConstTypeTensorColumn(const TypeTensor<T> &tensor,
                        unsigned int j) :
     _tensor(&tensor), _j(j) {}

  /**
   * Return the \f$ i,this \f$ element of the tensor.
   */
  const T & operator () (const unsigned int i) const
    { return (*_tensor)(i,_j); }

  const T & slice (const unsigned int i) const
    { return (*_tensor)(i,_j); }

private:
  const TypeTensor<T> *_tensor;
  const unsigned int _j;
};


template <typename T>
struct IncrementRank<TypeTensor<T> >
{
  typedef TypeNTensor<3,T> type;
};


template <typename T>
struct IncrementRank<TensorValue<T> >
{
  typedef TypeNTensor<3,T> type;
};


template <typename T>
struct MakeNumber<TypeTensor<T> >
{
  typedef TypeTensor<typename MakeNumber<T>::type> type;
};

//------------------------------------------------------
// Inline functions
template <typename T>
inline
TypeTensor<T>::TypeTensor ()
{
  _coords[0] = 0;

#if LIBMESH_DIM > 1
  _coords[1] = 0;
  _coords[2] = 0;
  _coords[3] = 0;
#endif

#if LIBMESH_DIM > 2
  _coords[4] = 0;
  _coords[5] = 0;
  _coords[6] = 0;
  _coords[7] = 0;
  _coords[8] = 0;
#endif
}



template <typename T>
template <typename Scalar>
inline
TypeTensor<T>::TypeTensor
  (const Scalar xx,
   const Scalar xy,
   const Scalar xz,
   const Scalar yx,
   const Scalar yy,
   const Scalar yz,
   const Scalar zx,
   const Scalar zy,
   typename
     boostcopy::enable_if_c<ScalarTraits<Scalar>::value,
                            const Scalar>::type zz)
{
  _coords[0] = xx;

#if LIBMESH_DIM == 2
  _coords[1] = xy;
  _coords[2] = yx;
  _coords[3] = yy;
#endif

#if LIBMESH_DIM == 3
  _coords[1] = xy;
  _coords[2] = xz;
  _coords[3] = yx;
  _coords[4] = yy;
  _coords[5] = yz;
  _coords[6] = zx;
  _coords[7] = zy;
  _coords[8] = zz;
#endif
}



template <typename T>
template<typename T2>
inline
TypeTensor<T>::TypeTensor (const TypeTensor <T2> &p)
{
  // copy the nodes from vector p to me
  for (unsigned int i=0; i<LIBMESH_DIM*LIBMESH_DIM; i++)
    _coords[i] = p._coords[i];
}


template <typename T>
template <typename T2>
TypeTensor<T>::TypeTensor(const TypeVector<T2>& vx)
{
  libmesh_assert(LIBMESH_DIM == 1);
  _coords[0] = vx(0);
}

template <typename T>
template <typename T2>
TypeTensor<T>::TypeTensor(const TypeVector<T2>& vx, const TypeVector<T2> &vy)
{
  libmesh_assert(LIBMESH_DIM == 2);
  _coords[0] = vx(0);
  _coords[1] = vx(1);
  _coords[2] = vy(0);
  _coords[3] = vy(1);
}

template <typename T>
template <typename T2>
TypeTensor<T>::TypeTensor(const TypeVector<T2>& vx, const TypeVector<T2> &vy, const TypeVector<T2> &vz)
{
  libmesh_assert(LIBMESH_DIM == 3);
  _coords[0] = vx(0);
  _coords[1] = vx(1);
  _coords[2] = vx(2);
  _coords[3] = vy(0);
  _coords[4] = vy(1);
  _coords[5] = vy(2);
  _coords[6] = vz(0);
  _coords[7] = vz(1);
  _coords[8] = vz(2);
}




template <typename T>
inline
TypeTensor<T>::~TypeTensor ()
{
}



template <typename T>
template<typename T2>
inline
void TypeTensor<T>::assign (const TypeTensor<T2> &p)
{
  for (unsigned int i=0; i<LIBMESH_DIM*LIBMESH_DIM; i++)
    _coords[i] = p._coords[i];
}



template <typename T>
inline
const T & TypeTensor<T>::operator () (const unsigned int i,
			      const unsigned int j) const
{
  libmesh_assert (i<3);
  libmesh_assert (j<3);

#if LIBMESH_DIM < 3
  const static T my_zero = 0;
  if (i >= LIBMESH_DIM || j >= LIBMESH_DIM)
    return my_zero;
#endif

  return _coords[i*LIBMESH_DIM+j];
}



template <typename T>
inline
T & TypeTensor<T>::operator () (const unsigned int i,
				const unsigned int j)
{
#if LIBMESH_DIM < 3

  if (i >= LIBMESH_DIM || j >= LIBMESH_DIM)
    {
//       libMesh::err << "ERROR:  You are assigning to a tensor component" << std::endl
// 		<< "that is out of range for the compiled LIBMESH_DIM!"      << std::endl
// 		<< " LIBMESH_DIM=" << LIBMESH_DIM << " , i=" << i << " , j=" << j << std::endl;
      libmesh_error();
    }

#endif

  libmesh_assert (i<LIBMESH_DIM);
  libmesh_assert (j<LIBMESH_DIM);

  return _coords[i*LIBMESH_DIM+j];
}


template <typename T>
inline
ConstTypeTensorColumn<T> 
TypeTensor<T>::slice (const unsigned int i) const
{
  libmesh_assert (i < LIBMESH_DIM);
  return ConstTypeTensorColumn<T>(*this, i);
}


template <typename T>
inline
TypeTensorColumn<T> 
TypeTensor<T>::slice (const unsigned int i)
{
  libmesh_assert (i < LIBMESH_DIM);
  return TypeTensorColumn<T>(*this, i);
}


template <typename T>
inline
TypeVector<T>
TypeTensor<T>::row(const unsigned int r)
{
  TypeVector<T> return_vector;

  for(unsigned int j=0; j<LIBMESH_DIM; j++)
    return_vector._coords[j] = _coords[r*LIBMESH_DIM + j];

  return return_vector;
}

template <typename T>
template<typename T2>
inline
TypeTensor<typename CompareTypes<T, T2>::supertype>
TypeTensor<T>::operator + (const TypeTensor<T2> &p) const
{

#if LIBMESH_DIM == 1
  return TypeTensor(_coords[0] + p._coords[0]);
#endif

#if LIBMESH_DIM == 2
  return TypeTensor(_coords[0] + p._coords[0],
		    _coords[1] + p._coords[1],
		    0.,
		    _coords[2] + p._coords[2],
		    _coords[3] + p._coords[3]);
#endif

#if LIBMESH_DIM == 3
  return TypeTensor(_coords[0] + p._coords[0],
		    _coords[1] + p._coords[1],
		    _coords[2] + p._coords[2],
		    _coords[3] + p._coords[3],
		    _coords[4] + p._coords[4],
		    _coords[5] + p._coords[5],
		    _coords[6] + p._coords[6],
		    _coords[7] + p._coords[7],
		    _coords[8] + p._coords[8]);
#endif

}



template <typename T>
template<typename T2>
inline
const TypeTensor<T> & TypeTensor<T>::operator += (const TypeTensor<T2> &p)
{
  this->add (p);

  return *this;
}



template <typename T>
template<typename T2>
inline
void TypeTensor<T>::add (const TypeTensor<T2> &p)
{
  for (unsigned int i=0; i<LIBMESH_DIM*LIBMESH_DIM; i++)
    _coords[i] += p._coords[i];
}



template <typename T>
template <typename T2>
inline
void TypeTensor<T>::add_scaled (const TypeTensor<T2> &p, const T factor)
{
  for (unsigned int i=0; i<LIBMESH_DIM*LIBMESH_DIM; i++)
    _coords[i] += factor*p._coords[i];

}



template <typename T>
template<typename T2>
inline
TypeTensor<typename CompareTypes<T, T2>::supertype>
TypeTensor<T>::operator - (const TypeTensor<T2> &p) const
{

#if LIBMESH_DIM == 1
  return TypeTensor(_coords[0] - p._coords[0]);
#endif

#if LIBMESH_DIM == 2
  return TypeTensor(_coords[0] - p._coords[0],
		    _coords[1] - p._coords[1],
		    0.,
		    _coords[2] - p._coords[2],
		    _coords[3] - p._coords[3]);
#endif

#if LIBMESH_DIM == 3
  return TypeTensor(_coords[0] - p._coords[0],
		    _coords[1] - p._coords[1],
		    _coords[2] - p._coords[2],
		    _coords[3] - p._coords[3],
		    _coords[4] - p._coords[4],
		    _coords[5] - p._coords[5],
		    _coords[6] - p._coords[6],
		    _coords[7] - p._coords[7],
		    _coords[8] - p._coords[8]);
#endif

}



template <typename T>
template <typename T2>
inline
const TypeTensor<T> & TypeTensor<T>::operator -= (const TypeTensor<T2> &p)
{
  this->subtract (p);

  return *this;
}



template <typename T>
template <typename T2>
inline
void TypeTensor<T>::subtract (const TypeTensor<T2>& p)
{
  for (unsigned int i=0; i<LIBMESH_DIM*LIBMESH_DIM; i++)
    _coords[i] -= p._coords[i];
}



template <typename T>
template <typename T2>
inline
void TypeTensor<T>::subtract_scaled (const TypeTensor<T2> &p, const T factor)
{
  for (unsigned int i=0; i<LIBMESH_DIM*LIBMESH_DIM; i++)
    _coords[i] -= factor*p._coords[i];
}



template <typename T>
inline
TypeTensor<T> TypeTensor<T>::operator - () const
{

#if LIBMESH_DIM == 1
  return TypeTensor(-_coords[0]);
#endif

#if LIBMESH_DIM == 2
  return TypeTensor(-_coords[0],
		    -_coords[1],
		    -_coords[2],
		    -_coords[3]);
#endif

#if LIBMESH_DIM == 3
  return TypeTensor(-_coords[0],
		    -_coords[1],
		    -_coords[2],
		    -_coords[3],
		    -_coords[4],
		    -_coords[5],
		    -_coords[6],
		    -_coords[7],
		    -_coords[8]);
#endif

}



template <typename T>
template <typename Scalar>
inline
typename boostcopy::enable_if_c<
  ScalarTraits<Scalar>::value,
  TypeTensor<typename CompareTypes<T, Scalar>::supertype> >::type
TypeTensor<T>::operator * (const Scalar factor) const
{
  typedef typename CompareTypes<T, Scalar>::supertype TS;


#if LIBMESH_DIM == 1
  return TypeTensor<TS>(_coords[0]*factor);
#endif

#if LIBMESH_DIM == 2
  return TypeTensor<TS>(_coords[0]*factor,
		        _coords[1]*factor,
		        _coords[2]*factor,
		        _coords[3]*factor);
#endif

#if LIBMESH_DIM == 3
  return TypeTensor<TS>(_coords[0]*factor,
		        _coords[1]*factor,
		        _coords[2]*factor,
		        _coords[3]*factor,
		        _coords[4]*factor,
		        _coords[5]*factor,
		        _coords[6]*factor,
		        _coords[7]*factor,
		        _coords[8]*factor);
#endif
}



template <typename T, typename Scalar>
inline
typename boostcopy::enable_if_c<
  ScalarTraits<Scalar>::value,
  TypeTensor<typename CompareTypes<T, Scalar>::supertype> >::type
operator * (const Scalar factor,
	    const TypeTensor<T> &t)
{
  return t * factor;
}



template <typename T>
template <typename Scalar>
inline
const TypeTensor<T> & TypeTensor<T>::operator *= (const Scalar factor)
{
  for (unsigned int i=0; i<LIBMESH_DIM*LIBMESH_DIM; i++)
    _coords[i] *= factor;

  return *this;
}




template <typename T>
template <typename Scalar>
inline
typename boostcopy::enable_if_c<
  ScalarTraits<Scalar>::value,
  TypeTensor<typename CompareTypes<T, Scalar>::supertype> >::type
TypeTensor<T>::operator / (const Scalar factor) const
{
  libmesh_assert (factor != static_cast<T>(0.));

  typedef typename CompareTypes<T, Scalar>::supertype TS;

#if LIBMESH_DIM == 1
  return TypeTensor<TS>(_coords[0]/factor);
#endif

#if LIBMESH_DIM == 2
  return TypeTensor<TS>(_coords[0]/factor,
		        _coords[1]/factor,
		        _coords[2]/factor,
		        _coords[3]/factor);
#endif

#if LIBMESH_DIM == 3
  return TypeTensor<TS>(_coords[0]/factor,
		        _coords[1]/factor,
		        _coords[2]/factor,
		        _coords[3]/factor,
		        _coords[4]/factor,
		        _coords[5]/factor,
		        _coords[6]/factor,
		        _coords[7]/factor,
		        _coords[8]/factor);
#endif

}



template <typename T>
inline
TypeTensor<T> TypeTensor<T>::transpose() const
{
#if LIBMESH_DIM == 1
  return TypeTensor(_coords[0]);
#endif

#if LIBMESH_DIM == 2
  return TypeTensor(_coords[0],
		    _coords[2],
		    _coords[1],
		    _coords[3]);
#endif

#if LIBMESH_DIM == 3
  return TypeTensor(_coords[0],
		    _coords[3],
		    _coords[6],
		    _coords[1],
		    _coords[4],
		    _coords[7],
		    _coords[2],
		    _coords[5],
		    _coords[8]);
#endif
}



template <typename T>
inline
const TypeTensor<T> & TypeTensor<T>::operator /= (const T factor)
{
  libmesh_assert (factor != static_cast<T>(0.));

  for (unsigned int i=0; i<LIBMESH_DIM*LIBMESH_DIM; i++)
    _coords[i] /= factor;

  return *this;
}




template <typename T>
template <typename T2>
inline
TypeVector<typename CompareTypes<T,T2>::supertype>
TypeTensor<T>::operator * (const TypeVector<T2> &p) const
{
  TypeVector<typename CompareTypes<T,T2>::supertype> returnval;
  for (unsigned int i=0; i<LIBMESH_DIM; i++)
    for (unsigned int j=0; j<LIBMESH_DIM; j++)
      returnval(i) += (*this)(i,j)*p(j);

  return returnval;
}



template <typename T>
template <typename T2>
inline
TypeTensor<T> TypeTensor<T>::operator * (const TypeTensor<T2> &p) const
{
  TypeTensor<T> returnval;
  for (unsigned int i=0; i<LIBMESH_DIM; i++)
    for (unsigned int j=0; j<LIBMESH_DIM; j++)
      for (unsigned int k=0; k<LIBMESH_DIM; k++)
        returnval(i,j) += (*this)(i,k)*p(k,j);

  return returnval;
}



  /**
   * Multiply 2 tensors together, i.e. sum Aij*Bij.
   * The tensors may be of different types.
   */
template <typename T>
template <typename T2>
inline
typename CompareTypes<T,T2>::supertype
TypeTensor<T>::contract (const TypeTensor<T2> &t) const
{
  typename CompareTypes<T,T2>::supertype sum = 0.;
  for (unsigned int i=0; i<LIBMESH_DIM*LIBMESH_DIM; i++)
    sum += _coords[i]*t._coords[i];
  return sum;
}



template <typename T>
inline
Real TypeTensor<T>::size() const
{
  return std::sqrt(this->size_sq());
}



template <typename T>
inline
T TypeTensor<T>::det() const
{
#if LIBMESH_DIM == 1
  return _coords[0];
#endif

#if LIBMESH_DIM == 2
  return (_coords[0] * _coords[3]
	  - _coords[1] * _coords[2]);
#endif

#if LIBMESH_DIM == 3
  return (_coords[0] * _coords[4] * _coords[8]
	  + _coords[1] * _coords[5] * _coords[6]
	  + _coords[2] * _coords[3] * _coords[7]
	  - _coords[0] * _coords[5] * _coords[7]
	  - _coords[1] * _coords[3] * _coords[8]
	  - _coords[2] * _coords[4] * _coords[6]);
#endif
}

template <typename T>
inline
T TypeTensor<T>::tr() const
{
#if LIBMESH_DIM == 1
  return _coords[0];
#endif

#if LIBMESH_DIM == 2
  return _coords[0] + _coords[3];
#endif

#if LIBMESH_DIM == 3
  return _coords[0] + _coords[4] + _coords[8];
#endif
}

template <typename T>
inline
void TypeTensor<T>::zero()
{
  for (unsigned int i=0; i<LIBMESH_DIM*LIBMESH_DIM; i++)
    _coords[i] = 0.;
}



template <typename T>
inline
Real TypeTensor<T>::size_sq () const
{
  Real sum = 0.;
  for (unsigned int i=0; i<LIBMESH_DIM*LIBMESH_DIM; i++)
    sum += libmesh_norm(_coords[i]);
  return sum;
}



template <typename T>
inline
bool TypeTensor<T>::operator == (const TypeTensor<T>& rhs) const
{
#if LIBMESH_DIM == 1
  return (std::abs(_coords[0] - rhs._coords[0])
	  < TOLERANCE);
#endif

#if LIBMESH_DIM == 2
  return ((std::abs(_coords[0] - rhs._coords[0]) +
	   std::abs(_coords[1] - rhs._coords[1]) +
	   std::abs(_coords[2] - rhs._coords[2]) +
	   std::abs(_coords[3] - rhs._coords[3]))
	  < 4.*TOLERANCE);
#endif

#if LIBMESH_DIM == 3
  return ((std::abs(_coords[0] - rhs._coords[0]) +
	   std::abs(_coords[1] - rhs._coords[1]) +
	   std::abs(_coords[2] - rhs._coords[2]) +
	   std::abs(_coords[3] - rhs._coords[3]) +
	   std::abs(_coords[4] - rhs._coords[4]) +
	   std::abs(_coords[5] - rhs._coords[5]) +
	   std::abs(_coords[6] - rhs._coords[6]) +
	   std::abs(_coords[7] - rhs._coords[7]) +
	   std::abs(_coords[8] - rhs._coords[8]))
	  < 9.*TOLERANCE);
#endif

}


template <typename T, typename T2>
inline
typename CompareTypes<T, T2>::supertype
libmesh_dot(const TypeTensor<T>& a, const TypeTensor<T2>& b)
{ return a.contract(b); }


} // namespace libMesh

#endif // #define __type_tensor_h__
