/**
 * @file Hash.hpp
 * Defines hash functions for various types.
 */

#ifndef __Hash__
#define __Hash__

#include <string>

namespace Lib {

/**
 * Hash functions for various types.
 */
class Hash
{
public:

  /** Return true if the two objects coincide. */
  template<typename T>
  static bool equals(T o1, T o2)
  { return o1 == o2; }

  static unsigned hash(const char* str);
  /** Hash function for strings */
  static unsigned hash(const std::string& str)
  { return hash(str.c_str()); }

  template<typename T>
  static unsigned hash(T obj)
  { return hash(reinterpret_cast<const unsigned char*>(&obj),sizeof(obj)); }

  static unsigned hash(const unsigned char*,size_t length);
  static unsigned hash(const unsigned char*,size_t length,unsigned begin);
};

struct IdentityHash
{
  template<typename T>
  static bool equals(T o1, T o2)
  { return o1 == o2; }

  template<typename T>
  static unsigned hash(T val)
  { return static_cast<unsigned>(val); }
};

struct PtrIdentityHash {
  static unsigned hash(void* ptr) {
    return static_cast<unsigned>(reinterpret_cast<size_t>(ptr));
  }
};


}

#endif
