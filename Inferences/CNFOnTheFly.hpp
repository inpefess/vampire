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
 * @file ProxyElimination.hpp
 * Defines class ProxyElimination.
 */

#ifndef __CNFOnTheFly__
#define __CNFOnTheFly__

#include "Forwards.hpp"
#include "InferenceEngine.hpp"
#include "Kernel/Term.hpp"
#include "Kernel/Signature.hpp"
#include "Kernel/Inference.hpp"

#include "Indexing/TermIndex.hpp"

#include "InferenceEngine.hpp"

#include <memory>

namespace Inferences {
using namespace Indexing;

class IFFXORRewriterISE
  : public ImmediateSimplificationEngine
{
public:

  USE_ALLOCATOR(IFFXORRewriterISE);

  Clause* simplify(Clause* c);
};

class EagerClausificationISE
  : public ImmediateSimplificationEngine
{
public:

  USE_ALLOCATOR(EagerClausificationISE);

  ClauseIterator simplifyMany(Clause* c);
  Clause* simplify(Clause* c){ NOT_IMPLEMENTED; }

};

class LazyClausification
  : public SimplificationEngine
{
public:
  USE_ALLOCATOR(LazyClausification);

  LazyClausification(){
    _formulaIndex = 0;
  }

  ClauseIterator perform(Clause* c) override;

  void attach(SaturationAlgorithm* salg) override;
  void detach() override;

private:
  SkolemisingFormulaIndex* _formulaIndex;
};

class LazyClausificationGIE
  : public GeneratingInferenceEngine
{
public:

  USE_ALLOCATOR(LazyClausificationGIE);

  LazyClausificationGIE(){
    _formulaIndex = 0;
  }

  void attach(SaturationAlgorithm* salg) override;
  void detach() override;

  ClauseIterator generateClauses(Clause* c) override;

private:
  SkolemisingFormulaIndex* _formulaIndex;
};

/*class NotProxyISE
  : public ImmediateSimplificationEngine
{
public:
  USE_ALLOCATOR(NotProxyISE);
  
  Kernel::Clause* simplify(Kernel::Clause* c);
};


class EqualsProxyISE
   : public ImmediateSimplificationEngine
{

public:
  USE_ALLOCATOR(EqualsProxyISE);
  
  Kernel::Clause* simplify(Kernel::Clause* c);        
};


class OrImpAndProxyISE
  : public ImmediateSimplificationEngine
{

public:
  USE_ALLOCATOR(OrImpAndProxyISE);
  
  Kernel::Clause* simplify(Kernel::Clause* c);
};


class PiSigmaProxyISE
   : public ImmediateSimplificationEngine
{
  
public:
  USE_ALLOCATOR(PiSigmaProxyISE);
  
  Kernel::Clause* simplify(Kernel::Clause* c);     
};


class ProxyISE 
  : public ImmediateSimplificationEngine {
  public:
    USE_ALLOCATOR(ProxyISE);
    ClauseIterator simplifyMany(Clause* c);
    Clause* simplify(Clause* c){ NOT_IMPLEMENTED; }
};*/


}

#endif
