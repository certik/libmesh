#ifndef __variant_filter_iterator_h__
#define __variant_filter_iterator_h__



#include <iterator>
#include <iostream>

/**
 * Original Authors: Corwin Joy          * Michael Gradman
 *                   cjoy@houston.rr.com * Michael.Gradman@caminus.com
 * Caminus, Suite 1150, Two Allen Center, 1200 Smith Street, Houston, TX 77002
 * This class is an extension of variant_bidirectional_iterator to a
 * filter_iterator similar to boost's.  The filter iterator is modeled
 * after a forward_iterator since to go backward and forward requires
 * the storage of both a "begin" and "end" iterator to avoid stepping
 * off the end or the beginning.  To reduce complexity, we only allow
 * traversal in one direction.
 *
 * @author John W. Peterson, 2004.
 */
template<class Type, class Predicate>
#if defined(__GNUC__) && (__GNUC__ < 3)  && !defined(__INTEL_COMPILER)
class variant_filter_iterator : public std::forward_iterator<std::forward_iterator_tag, Type>
#else
class variant_filter_iterator : public std::iterator<std::forward_iterator_tag,  Type>
#endif
{
private:

  // Abstract base class for the iterator type.
  struct IterBase
  {
    virtual ~IterBase() {}
    virtual  IterBase* clone() const = 0 ;
    virtual Type& operator*() const = 0;    // <-- CUSTOM INTERFACE METHOD
    virtual void operator++() = 0;          // <-- CUSTOM INTERFACE METHOD
    virtual bool equal(const IterBase *other) const = 0;
  };


  // Abstract base class for the predicate.
  struct PredBase
  {
    virtual ~PredBase() {}
    virtual PredBase* clone() const = 0 ;
    virtual bool operator()(const IterBase* in) const = 0;
  };


  
  // The actual iterator object is held as a template parameter here.
  template<typename IterType>
  struct Iter : IterBase
  {
    // Constructor
    Iter ( IterType v ) : iter_data ( v )
    {}


    // Destructor
    virtual ~Iter () {}

    
    // Returns a copy of this object as a pointer to
    // the base (non-templated) class.
    virtual IterBase* clone() const
    {
      Iter<IterType> *copy = new Iter<IterType>(iter_data);
      return copy;
    }

    
    virtual Type& operator*() const   // <-- CUSTOM INTERFACE METHOD
    {
      return *iter_data;
    }


    
    virtual void operator++()         // <-- CUSTOM INTERFACE METHOD
    {
      //std::cout << "called Iter::op++()" << std::endl;
      ++iter_data;
    }

    // Use dynamic_cast to convert the base pointer
    // passed in to the derived type.  If the cast
    // fails it means you compared two different derived
    // classes.
    virtual bool equal(const IterBase *other) const
    {
      const Iter<IterType>* p = dynamic_cast<const Iter<IterType>*>(other) ;

      // Check for failed cast
      if ( p == NULL )
	{
	  std::cerr << "Dynamic cast failed in Iter::equal(...)" << std::endl;
	  abort();
	}
      
      //std::cout << "called Iter::equal" << std::endl;
      return iter_data == p->iter_data;
    }

    IterType iter_data;
  };




  
  // The actual predicate is held as a template parameter here.
  // There are two template arguments here, one for the actual type
  // of the predicate and one for the iterator type.
  template <typename IterType, typename PredType>
  struct Pred : PredBase
  {
    // Constructor
    Pred ( PredType const& v ) : pred_data ( v ) {}

    // Destructor
    virtual ~Pred () {}

    // Returns a copy of this object as a pointer to the base class.
    virtual PredBase* clone() const
    {
      Pred<IterType,PredType> *copy = new Pred<IterType,PredType>(pred_data);
      return copy;
    }
    
    // Re-implementation of op()
    virtual bool operator() (const IterBase* in) const
    {
      //std::cout << "called Pred::op()" << std::endl;
      assert (in != NULL);
      
      // Attempt downcast (full qualification of the type is only necessary for GCC 2.95.3)
#if defined(__GNUC__) && (__GNUC__ < 3)  && !defined(__INTEL_COMPILER)
      const variant_filter_iterator::Iter<IterType>* p =
	dynamic_cast<const variant_filter_iterator::Iter<IterType>* >(in);
#else
      const Iter<IterType>* p =
	dynamic_cast<const Iter<IterType>* >(in);
#endif
      
      // Check for failure
      if ( p == NULL )
	{
	  std::cerr << "Dynamic cast failed in Pred::op()" << std::endl;
	  std::cerr << "typeid(IterType).name()=" << typeid(IterType).name() << std::endl;
	  abort();
	}
      
      // Return result of op() for the user's predicate.
      return pred_data(p->iter_data);
    }
    
    // This is the predicate passed in by the user.
    PredType pred_data;
  };



  
  // Polymorphic pointer to the object.  Don't confuse
  // with the data pointer located in the Iter!
  IterBase* data;

  // Also have a polymorphic pointer to the end object,
  // this prevents iterating past the end.
  IterBase* end;

  // The predicate object.  Must have op() capable of
  // operating on IterBase* pointers.  Therefore it has
  // to follow the same paradigm as IterBase.
  PredBase* pred;




  
public:
  typedef variant_filter_iterator<Type, Predicate> Iterator;

  // Templated Constructor.  Allows you to construct the iterator
  // and predicate from any types.  Also advances the data pointer
  // to the first entry which satisfies the predicate.
  template<typename IterType, class PredType>
  variant_filter_iterator ( IterType const& v, IterType const& e, PredType const& p )
    : data ( new Iter<IterType>(v) ),
      end  ( new Iter<IterType>(e) ),
      pred ( new Pred<IterType,PredType>(p) )
  {
    this->satisfy_predicate();
  }


  
  // Default Constructor.
  variant_filter_iterator () : data(NULL), end(NULL), pred(NULL) {}


  
  // Copy Constructor.
  // * Copy the internal data instead of sharing it.
  variant_filter_iterator( const Iterator& rhs ) :
    data ( rhs.data != NULL ? rhs.data->clone() : NULL),
    end  ( rhs.end  != NULL ? rhs.end->clone()  : NULL),
    pred ( rhs.pred != NULL ? rhs.pred->clone() : NULL)
  {}

  

  // Destructor
  ~variant_filter_iterator()
  {
    delete data;
    delete end;
    delete pred;
  }


  
  // unary op*() forwards on to Iter::op*()
  Type& operator*() const
  {
    return **data;
  }


  // op->() 
  Type* operator->() const
  {
    return (&**this);
  }


  
  // op++() forwards on to Iter::op++()
  Iterator& operator++()
  {
    ++*data;
    this->satisfy_predicate();
    return (*this);
  }

  // postfix op++() creates a temporary!
  const Iterator operator++(int) // const here to prevent iterator++++ type operations
  {
    Iterator oldValue(*this); // standard is to return old value
    ++*data;
    this->satisfy_predicate();
    return oldValue;
  }


  // forwards on the the equal function defined for the
  // IterBase pointer.  Possibly also compare the end pointers,
  // but this is usually not important and would require an
  // additional dynamic_cast.
  bool equal(const variant_filter_iterator& other) const
  {
    return data->equal(other.data);
  }


  


  // swap, used to implement op=
  void swap(Iterator& lhs, Iterator& rhs)
  {
    IterBase *temp;

    // Swap the data pointers
    temp = lhs.data;
    lhs.data = rhs.data;
    rhs.data = temp;

    // Swap the end pointers
    temp = lhs.end;
    lhs.end = rhs.end;
    rhs.end = temp;

    // Also swap the predicate objects.
    PredBase *tempp;
    tempp    = lhs.pred;
    lhs.pred = rhs.pred;
    rhs.pred = tempp; 
  }


  // op=
  Iterator& operator=(const Iterator& rhs)
  {
    Iterator temp(rhs);
    swap(temp, *this);
    return *this;
  }


  
private:
  // Advances the data pointer until it reaches
  // the end or the predicate is satisfied.
  void satisfy_predicate()
  {
    while ( !(data->equal(end)) && ( !(*pred)(data) ) )
      ++(*data);
  }
  
};






// op==
template<class Type, class Predicate>
inline
bool operator==(const variant_filter_iterator<Type, Predicate>& x,
		const variant_filter_iterator<Type, Predicate>& y)
{
  //std::cout << "Called op==() for variant_filter_iterators." << std::endl;
  return x.equal(y);
}



// op!=
template<class Type, class Predicate>
inline
bool operator!=(const variant_filter_iterator<Type, Predicate>& x,
		const variant_filter_iterator<Type, Predicate>& y)
{
  //std::cout << "Called op!=() for variant_filter_iterators." << std::endl;
  return (!(x == y));
}



#endif
