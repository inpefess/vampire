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
 * @file LiteralIndexingStructure.hpp
 * Defines class LiteralIndexingStructure.
 */


#ifndef __LiteralIndexingStructure__
#define __LiteralIndexingStructure__

#include "Forwards.hpp"
#include "Index.hpp"

namespace Indexing {

class LiteralIndexingStructure {
public:
  virtual ~LiteralIndexingStructure() {}

  virtual void handle(Literal* lit, Clause* cls, bool insert) = 0;
  void insert(Literal* lit, Clause* cls) { handle(lit, cls, /* insert = */ true ); }
  void remove(Literal* lit, Clause* cls) { handle(lit, cls, /* insert = */ false); }

  virtual SLQueryResultIterator getAll() { NOT_IMPLEMENTED; }
  virtual SLQueryResultIterator getUnifications(Literal* lit,
	  bool complementary, bool retrieveSubstitutions = true) { NOT_IMPLEMENTED; }
  virtual SLQueryResultIterator getUnificationsWithConstraints(Literal* lit,
          bool complementary, bool retrieveSubstitutions = true) { NOT_IMPLEMENTED; }
  virtual SLQueryResultIterator getGeneralizations(Literal* lit,
	  bool complementary, bool retrieveSubstitutions = true) { NOT_IMPLEMENTED; }
  virtual SLQueryResultIterator getInstances(Literal* lit,
	  bool complementary, bool retrieveSubstitutions = true) { NOT_IMPLEMENTED; }
  virtual SLQueryResultIterator getVariants(Literal* lit,
	  bool complementary, bool retrieveSubstitutions = true) { NOT_IMPLEMENTED; }

  virtual size_t getUnificationCount(Literal* lit, bool complementary)
  {
    CALL("LiteralIndexingStructure::getUnificationCount");
    return countIteratorElements(getUnifications(lit, complementary, false));
  }

};

};

#endif /* __LiteralIndexingStructure__ */
