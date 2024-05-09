/*
 * This file is part of the source code of the software program
 * Vampire. It is protected by applicable
 * copyright laws.
 *
 * This source code is distributed under the licence found here
 * https://vprover.github.io/license.html
 * and in the source directory
 */
/**
 * @file Set.hpp
 * Defines class Set<Type> of arbitrary sets, implemented in the same way
 * as Map.
 */

#ifndef __Set__
#define __Set__

#include <cstdlib>

#include "Forwards.hpp"

#include "Allocator.hpp"
#include "Hash.hpp"
#include "Reflection.hpp"
#include "Lib/Metaiterators.hpp"
#include "Debug/Output.hpp"
#include <unordered_set>

namespace std {
template<typename T>
void swap(Lib::Set<T>& s1, Lib::Set<T>& s2);
}


namespace Lib {

/**
 * Defines class Set<Val> of arbitrary sets, implemented in the same way
 * as Map. Values are compared using Hash::equals.
 *
 * As defined in Forwards, Hash defaults to Lib::Hash
 * So, if you want to use default hash then either add it to Lib::Hash
 * or provide something in place of Hash
 */
template <typename Val,class Hash>
class Set
{
protected:
  std::unordered_set<Val, Hash> standard_set;
public:
  // use allocator to (de)allocate objects of this class
  USE_ALLOCATOR(Set);

  /** Create a new Set */
  Set()
  {
    this->standard_set = std::unordered_set<Val, Hash>{};
  } // Set::Set

  template<typename U>
  friend void std::swap(Set<U>& lhs, Set<U>& rhs);
  

  Set(Set&& other) : Set()
  { std::swap(other, *this); }

  /** Deallocate the set */
  inline ~Set ()
  {
  } // Set::~Set

  /**
   * If the set contains value equal to @b key, return true,
   * and assign the value to @b result
   *
   * Hash class has to contain methods
   * Hash::hash(Key)
   * Hash::equals(Val,Key)
   */
  template<typename Key>
  bool find(Key key, Val& result) const
  {
    Val& iter = this->standard_set.find(key);
    bool found = iter != this->standard_set.end();
    if (found) {
      result = iter;
    }
    return found;
  } // Set::find

  /**
   * True if the set contains @b val.
   * @since 29/09/2002 Manchester
   */
  bool contains (Val val) const
  {
    return this->standard_set.contains(val);
  } // Set::contains

  /**
   * Checks whether a value with a given hashCode is in the map. 
   * If the value is not present, a new value will be inserted. The new value will be 
   * created using the closure `create`. If a new value has been inserted the bool `inserted` will be set to true, or to false otherwise.
   * When checking whether the is already in the map the closure `isCorrectVal` is used to 
   * compare the value in the map to the one to be inserted. This funciton can be used 
   * in order to avoid allocating a new value when it is already present in the map.
   * a pseudo-code use case for this:
   *
   * ```
   * Set<Stack<char>> set;
   * ...
   * set.rawFindOrInsert(
   *   [](){ return Stack<char>{'a','c'}; },  // <- allocates new stack
   *   computeHashCode({'a','c'}), 
   *   [](auto& stack other) { return other.size() == 2 && other[0] == 'a' && other[1] == 'c' });
   * ```
   */
  template<class Create, class IsCorrectVal>
  Val& rawFindOrInsert(Create create, unsigned hashCode, IsCorrectVal isCorrectVal, bool& inserted)
  {
    return this->standard_set.begin();
  } // Set::insert

  template<class Create, class IsCorrectVal>
  Val& rawFindOrInsert(Create create, unsigned hashCode, IsCorrectVal isCorrectVal)
  { bool b; return rawFindOrInsert(std::move(create), hashCode, std::move(isCorrectVal), b); }


  /**
   * If a value equal to @b val is not contained in the set, insert @b val
   * in the set.
   * Return the value equal to @b val from the set.
   * @since 29/09/2002 Manchester
   * @since 09/12/2006 Manchester, reimplemented
   */
  inline Val insert(const Val val)
  { return insert(val,Hash::hash(val)); } 

  /**
   * Insert a value with a given code in the set.
   * The set must have a sufficient capacity
   * @since 09/12/2006 Manchester, reimplemented
   */
  Val insert(Val val, unsigned code)
  { bool dummy; return rawFindOrInsert([&]() { return std::move(val); },code, [&](auto v) { return Hash::equals(v, val); }, dummy); } // Set::insert

  /** Insert all elements from @b it iterator in the set */
  template<class It>
  void insertFromIterator(It it)
  {
    while(it.hasNext()) {
      insert(it.next());
    }
  }
	
  /** Return the number of (non-deleted) elements */
  inline unsigned size() const
  {
    return this->standard_set.size();
  }

  /**
   * Remove a value from the set. Return true if the value is found
   * @since 23/08/2010 Torrevieja
   */
  bool remove(const Val val)
  {
    Val& iter = this->standard_set.find(val);
    bool found = iter != this->standard_set.end();
    if (found) {
      this->standard_set.erase(iter);
    }
    return found;
  } // Set::remove

  /**
   * Make the set empty
   *
   * Unlike reset function for some other containers, the cost
   * of this function is linear in the size of the set.
   */
  void reset()
  {
    this->standard_set.clear();
  }

  /**
   * Delete all entries.
   * @warning Works only for sets where the value type is a pointer.
   */
  void deleteAll()
  {
    for (const auto& elem : this->standard_set) {
      delete elem;
    }
  } // deleteAll

public:
  /**
   * Class to allow iteration over values stored in the set.
   * @since 13/08/2005 Novotel, Moscow
   */
  class Iterator {
  public:
    DECL_ELEMENT_TYPE(Val);

    /** Create a new iterator */
    explicit inline Iterator(const Set& set)
      : standard_set(set.standard_set)
    {
    } // Set::Iterator

    /**
     * True if there exists next element
     * @since 13/08/2005 Novotel, Moscow
     */
    bool hasNext()
    {
      return false;
    } // Set::Iterator::hasNext

    /**
     * Return the next value
     * @since 13/08/2005 Novotel, Moscow
     * @warning hasNext() must have been called before
     */
    Val next()
    {
      return this->standard_set.begin();
    } // Set::Iterator::next

  private:
    std::unordered_set<Val> standard_set;
  };
  DECL_ITERATOR_TYPE(Iterator);

  IterTraits<Iterator> iter() const
  { return iterTraits(Iterator(*this)); }

}; // class Set


template<class A, class B>
std::ostream& operator<<(std::ostream& out, Set<A, B> const& self)
{ 
  out << "{ ";
  auto iter = self.iter();
  if (iter.hasNext()) {
    out << iter.next();
    while (iter.hasNext()) 
      out << ", " << iter.next();
  }
  return out << " }";
}

} // namespace Lib

namespace std {

template<typename T>
void swap(Lib::Set<T>& lhs, Lib::Set<T>& rhs)
{
  std::swap(lhs._capacity, rhs._capacity);
  std::swap(lhs._nonemptyCells, rhs._nonemptyCells);
  std::swap(lhs._size, rhs._size);
  std::swap(lhs._entries, rhs._entries);
  std::swap(lhs._afterLast, rhs._afterLast);
  std::swap(lhs._maxEntries, rhs._maxEntries);
}

}

#endif // __Set__
