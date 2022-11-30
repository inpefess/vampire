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
 * @file TypeSubstitutionTree.cpp
 * Implements class TypeSubstitutionTree.
 */

#include "Lib/Environment.hpp"
#include "Lib/Metaiterators.hpp"
#include "Lib/Random.hpp"
#include "Lib/SmartPtr.hpp"

#include "Kernel/TermIterators.hpp"
#include "Kernel/Matcher.hpp"
#include "Kernel/Signature.hpp"
#include "Kernel/Term.hpp"
#include "Kernel/SortHelper.hpp"
#include "Kernel/ApplicativeHelper.hpp"

#include "Shell/Options.hpp"

#include "TypeSubstitutionTree.hpp"

namespace Indexing
{

using namespace Lib;
using namespace Kernel;

TypeSubstitutionTree::TypeSubstitutionTree()
: SubstitutionTree()
{
}

void TypeSubstitutionTree::insert(TermList sort, LeafData ld)
{
  CALL("TypeSubstitutionTree::insert");
  handleTerm(sort,ld,true);
}

void TypeSubstitutionTree::remove(TermList sort, LeafData ld)
{
  CALL("TypeSubstitutionTree::remove");
  handleTerm(sort,ld,false);
}

struct TypeSubstitutionTree::ToTypeSubFn
{

  ToTypeSubFn(TermList queryTerm)
  : _queryTerm(queryTerm) {}

  TermQueryResult operator() (TermQueryResult tqr) {
    if(!_queryTerm.isVar() && !tqr.term.isVar()){
      tqr.isTypeSub = true;
    } else {
      RobSubstitution* subst = tqr.substitution->tryGetRobSubstitution();
      ALWAYS(subst->unify(_queryTerm, SubstitutionTree::QRS_QUERY_BANK, tqr.term, SubstitutionTree::QRS_RESULT_BANK));      
    }
    return tqr;
  }

private:
  TermList _queryTerm;
};

/**
 * According to value of @b insert, insert or remove term.
 */
void TypeSubstitutionTree::handleTerm(TermList sort, LeafData ld, bool insert)
{
  CALL("TypeSubstitutionTree::handleTerm");

  Renaming normalizer;
  normalizer.normalizeVariables(ld.term);

  auto normSort = normalizer.apply(sort);

  BindingMap svBindings;
  svBindings.insert(0, normSort);
  _nextVar = max(_nextVar, 1);

  if(insert) {
    SubstitutionTree::insert(svBindings, ld);
  } else {
    SubstitutionTree::remove(svBindings, ld);
  }
}

//TODO use sorts and delete non-shared
TermQueryResultIterator TypeSubstitutionTree::getUnifications(TermList sort, TermList trm,
	  bool retrieveSubstitutions)
{
  CALL("TypeSubstitutionTree::getUnifications");
 
  return pvi(iterTraits(SubstitutionTree::iterator<UnificationsIterator>(trm, retrieveSubstitutions, 
          /* withConstraints */ false, /* extra */ false, /* functionalSubtermMap */ nullptr))
           .map(ToTypeSubFn(trm)));
}

}
