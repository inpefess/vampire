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
 * @file Induction.hpp
 * Defines class Induction
 *
 */

#ifndef __Induction__
#define __Induction__

#include <cmath>

#include "Forwards.hpp"

#include "Indexing/InductionFormulaIndex.hpp"
#include "Indexing/LiteralIndex.hpp"
#include "Indexing/TermIndex.hpp"

#include "Kernel/TermTransformer.hpp"
#include "Kernel/Theory.hpp"

#include "Lib/DHMap.hpp"
#include "Lib/DHSet.hpp"
#include "Lib/List.hpp"

#include "InductionHelper.hpp"
#include "InferenceEngine.hpp"

namespace Inferences
{

using namespace Kernel;
using namespace Saturation;

using ClauseToLiteralMap = vunordered_map<Clause*, LiteralStack>;

class TermReplacement : public TermTransformer {
public:
  TermReplacement(Term* o, TermList r) : _o(o), _r(r) {} 
  TermList transformSubterm(TermList trm) override;
protected:
  Term* _o;
  TermList _r;
};

class SkolemSquashingTermReplacement : public TermReplacement {
public:
  SkolemSquashingTermReplacement(Term* o, TermList r, unsigned& var)
    : TermReplacement(o, r), _v(var) {}
  TermList transformSubterm(TermList trm) override;
  DHMap<Term*, unsigned> _tv; // maps terms to their variable replacement
private:
  unsigned& _v;               // fresh variable counter supported by caller
};

class LiteralSubsetReplacement : TermTransformer {
public:
  LiteralSubsetReplacement(Literal* lit, Term* o, TermList r, const unsigned maxSubsetSize)
      : _lit(lit), _o(o), _r(r), _maxSubsetSize(maxSubsetSize) {
    _occurrences = _lit->countSubtermOccurrences(TermList(_o));
    _maxIterations = pow(2, _occurrences);
  }

  // Returns transformed lit for the first 2^(_occurences) - 1 calls, then returns nullptr.
  Literal* transformSubset();

  List<Literal*>* getListOfTransformedLiterals();

protected:
  virtual TermList transformSubterm(TermList trm);

private:
  // _iteration serves as a map of occurrences to replace
  unsigned _iteration = 0;
  unsigned _maxIterations;
  // Counts how many occurrences were already encountered in one transformation
  unsigned _matchCount = 0;
  unsigned _occurrences;
  const unsigned _maxOccurrences = 20;
  Literal* _lit;
  Term* _o;
  TermList _r;
  const unsigned _maxSubsetSize;
};

struct InductionContext {
  explicit InductionContext(Term* t)
    : _indTerm(t) {}
  InductionContext(Term* t, Literal* l, Clause* cl)
    : InductionContext(t)
  {
    insert(cl, l);
  }

  void insert(Clause* cl, Literal* lit) {
    // this constructs an empty inner map if cl is not yet mapped
    auto node = _cls.emplace(cl, LiteralStack()).first;
    node->second.push(lit);
  }

  Formula* getFormula(TermList r, bool opposite, Substitution* subst = nullptr) const;
  Formula* getFormulaWithSquashedSkolems(TermList r, bool opposite, unsigned& var,
    VList** varList = nullptr, Substitution* subst = nullptr) const;

  vstring toString() const {
    vstringstream str;
    str << *_indTerm << endl;
    for (const auto& kv : _cls) {
      str << *kv.first << endl;
      for (const auto& lit : kv.second) {
        str << *lit << endl;
      }
    }
    return str.str();
  }

  Term* _indTerm = nullptr;
  ClauseToLiteralMap _cls;
private:
  Formula* getFormula(TermReplacement& tr, bool opposite) const;
};

class ContextSubsetReplacement
  : public TermTransformer, public IteratorCore<InductionContext> {
public:
  ContextSubsetReplacement(InductionContext context, bool noGen);

  bool hasNext() override {
    return _iteration+1 < _maxIterations;
  }
  InductionContext next() override;

protected:
  TermList transformSubterm(TermList trm) override;

private:
  // _iteration serves as a map of occurrences to replace
  unsigned _iteration = 0;
  unsigned _maxIterations;
  // Counts how many occurrences were already encountered in one transformation
  unsigned _matchCount = 0;
  InductionContext _context;
  TermList _r;
};


class Induction
: public GeneratingInferenceEngine
{
public:
  CLASS_NAME(Induction);
  USE_ALLOCATOR(Induction);

  void attach(SaturationAlgorithm* salg) override;
  void detach() override;

  ClauseIterator generateClauses(Clause* premise) override;

#if VDEBUG
  void setTestIndices(const Stack<Index*>& indices) override {
    _comparisonIndex = static_cast<LiteralIndex*>(indices[0]);
    _inductionTermIndex = static_cast<TermIndex*>(indices[1]);
    _structInductionTermIndex = static_cast<TermIndex*>(indices[2]);
  }
#endif // VDEBUG

private:
  // The following pointers can be null if int induction is off.
  LiteralIndex* _comparisonIndex = nullptr;
  TermIndex* _inductionTermIndex = nullptr;
  TermIndex* _structInductionTermIndex = nullptr;
  InductionFormulaIndex _formulaIndex;
};

class InductionClauseIterator
{
public:
  // all the work happens in the constructor!
  InductionClauseIterator(Clause* premise, InductionHelper helper, const Options& opt,
    TermIndex* structInductionTermIndex, InductionFormulaIndex& formulaIndex)
      : _helper(helper), _opt(opt), _structInductionTermIndex(structInductionTermIndex),
      _formulaIndex(formulaIndex)
  {
    processClause(premise);
  }

  CLASS_NAME(InductionClauseIterator);
  USE_ALLOCATOR(InductionClauseIterator);
  DECL_ELEMENT_TYPE(Clause*);

  inline bool hasNext() { return _clauses.isNonEmpty(); }
  inline OWN_ELEMENT_TYPE next() { 
    return _clauses.pop();
  }

private:
  void processClause(Clause* premise);
  void processLiteral(Clause* premise, Literal* lit);
  void processIntegerComparison(Clause* premise, Literal* lit);

  ClauseStack produceClauses(Formula* hypothesis, InferenceRule rule, const InductionContext& context);
  void resolveClauses(InductionContext context, InductionFormulaIndex::Entry* e, const TermQueryResult* bound1, const TermQueryResult* bound2);
  void resolveClauses(const ClauseStack& cls, const InductionContext& context, Substitution& subst, RobSubstitution* rsubst = nullptr);

  void performIntInduction(const InductionContext& context, InductionFormulaIndex::Entry* e, bool increasing, const TermQueryResult& bound1, const TermQueryResult* optionalBound2);

  void performStructInductionOne(const InductionContext& context, InductionFormulaIndex::Entry* e);
  void performStructInductionTwo(const InductionContext& context, InductionFormulaIndex::Entry* e);
  void performStructInductionThree(const InductionContext& context, InductionFormulaIndex::Entry* e);

  bool notDoneInt(InductionContext context, Literal* bound1, Literal* bound2, InductionFormulaIndex::Entry*& e);

  Stack<Clause*> _clauses;
  InductionHelper _helper;
  const Options& _opt;
  TermIndex* _structInductionTermIndex;
  InductionFormulaIndex& _formulaIndex;
};

};

#endif /*__Induction__*/
