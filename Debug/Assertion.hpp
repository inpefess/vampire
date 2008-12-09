/**
 * @file Assertion.hpp
 * Defines several functions that
 * could be used to make assertions.
 */

#ifndef __Assertion__
#define __Assertion__

#if VDEBUG

#include <iostream>
#include <ostream>

#include "Tracer.hpp"

namespace Debug {

// any function. It can be declared in one module and called in
// another
extern void debug(void*);

class Assertion
{
public:
  static void violated(const char* file,int line,const char* condition);

  static void checkType(const char* file,int line,const void* ptr,
	  const char* assumed, const char* ptrStr);

  template<typename T,typename U>
  static void violatedEquality(const char* file,int line,const char* val1Str,
	  const char* val2Str, const T& val1, const U& val2);
  template<typename T,typename U>
  static void violatedNonequality(const char* file,int line,const char* val1Str,
	  const char* val2Str, const T& val1, const U& val2);

  static void violatedStrEquality(const char* file,int line,const char* val1Str,
  	  const char* val2Str, const char* val1, const char* val2);


  static void reportAssertValidException(const char* file,int line,const char* obj);
private:
  static bool _violated;
};

/**
 * Class AssertionViolationException. It is thrown when any assertion
 * is violated.
 */
class AssertionViolationException
{
public:
  AssertionViolationException (const char* file, int line);
  ~AssertionViolationException () {}
  void cry (std::ostream&);
private:
  void outputFileAndLine (std::ostream&) const;
  /** file in which violatin occurred */
  const char* _file;
  /** line number in the file */
  int _line;
}; // AssertionViolationException

} // namespace Debug

#define ASS(Cond)                                               \
  if (! (Cond)) {                                               \
    Debug::Assertion::violated(__FILE__,__LINE__,#Cond);		\
    throw Debug::AssertionViolationException(__FILE__,__LINE__);	\
  }
#define ALWAYS(Cond) ASS(Cond)
#define NEVER(Cond) ASS(!(Cond))

#define ASS_EQ(VAL1,VAL2)                                               \
  if (! ((VAL1)==(VAL2)) ) {                                               \
    Debug::Assertion::violatedEquality(__FILE__,__LINE__,#VAL1,#VAL2,VAL1,VAL2); \
    throw Debug::AssertionViolationException(__FILE__,__LINE__);	\
  }

#define ASS_NEQ(VAL1,VAL2)                                               \
  if (! ((VAL1)!=(VAL2)) ) {                                               \
    Debug::Assertion::violatedNonequality(__FILE__,__LINE__,#VAL1,#VAL2,VAL1,VAL2); \
    throw Debug::AssertionViolationException(__FILE__,__LINE__);	\
  }

#define ASS_STR_EQ(VAL1,VAL2)                                               \
  if (strcmp((VAL1),(VAL2)) ) {                                               \
    Debug::Assertion::violatedStrEquality(__FILE__,__LINE__,#VAL1,#VAL2,VAL1,VAL2); \
    throw Debug::AssertionViolationException(__FILE__,__LINE__);	\
  }

#define ASS_ALLOC_TYPE(PTR,TYPE)						\
  Debug::Assertion::checkType(__FILE__,__LINE__,(PTR),(TYPE), #PTR)



#define ASSERT_VALID(obj) try { (obj).assertValid(); } catch(...) \
  { Debug::Assertion::reportAssertValidException(__FILE__,__LINE__,#obj); throw; }

#define ASSERTION_VIOLATION \
  Debug::Assertion::violated(__FILE__,__LINE__,"true");		\
  throw Debug::AssertionViolationException(__FILE__,__LINE__);
#else // ! VDEBUG

#define ASS(Cond)
#define ASSERT(Cond)
#define ALWAYS(Cond) Cond
#define NEVER(Cond) Cond

#define ASS_EQ(VAL1,VAL2)
#define ASS_NEQ(VAL1,VAL2)
#define ASS_STR_EQ(VAL1,VAL2)
#define ASS_ALLOC_TYPE(PTR,TYPE)


#define ASSERTION_VIOLATION

#define ASSERT_VALID(obj)

#endif // VDEBUG

#if VDEBUG

template<typename T,typename U>
void Debug::Assertion::violatedEquality(const char* file,int line,const char* val1Str,
	  const char* val2Str, const T& val1, const U& val2)
{
  if (_violated) {
    return;
  }

  _violated = true;
  std::cout << "Condition "<<val1Str<<" == "<<val2Str<<" in file " << file << ", line " << line
       << " was violated, as:\n" << val1Str<<" == "<<val1 <<"\n" << val2Str<<" == "<<val2 << "\n"
       << "----- stack dump -----\n";
  Tracer::printStack(cout);
  std::cout << "----- end of stack dump -----\n";
} // Assertion::violatedEquality

template<typename T,typename U>
void Debug::Assertion::violatedNonequality(const char* file,int line,const char* val1Str,
	  const char* val2Str, const T& val1, const U& val2)
{
  if (_violated) {
    return;
  }

  _violated = true;
  std::cout << "Condition "<<val1Str<<" != "<<val2Str<<" in file " << file << ", line " << line
       << " was violated, as:\n" << val1Str<<" == "<<val1 <<"\n" << val2Str<<" == "<<val2 << "\n"
       << "----- stack dump -----\n";
  Tracer::printStack(cout);
  std::cout << "----- end of stack dump -----\n";
} // Assertion::violatedEquality

#endif

#endif // __Assertion__

