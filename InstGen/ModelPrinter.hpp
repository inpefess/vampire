/**
 * @file ModelPrinter.hpp
 * Defines class ModelPrinter.
 */

#ifndef __ModelPrinter__
#define __ModelPrinter__

#include "Forwards.hpp"

#include "Lib/DHMap.hpp"
#include "Lib/Stack.hpp"

#include "Kernel/Term.hpp"


namespace InstGen {

using namespace Kernel;
using namespace SAT;

class ModelPrinter {
public:
  ModelPrinter(IGAlgorithm& iga);

  bool tryOutput(ostream& stm);

private:

  struct InstLitComparator;
  struct PredNumComparator;

  static bool isEprProblem();
  static bool isEquality(Literal* lit);

  void collectTrueLits();
  void getInstances(LiteralStack& trueLits, TermStack& domain, LiteralStack& instanceAcc);
  void generateNewInstances(Literal* base, TermStack& domain, DHSet<Literal*>& instSet, LiteralStack& instAcc);
  void analyzeEqualityAndPopulateDomain();
  void rewriteLits(LiteralStack& lits);

  void outputDomainSpec(ostream& out);
  void outputFunInterpretations(ostream& out);
  void outputPredInterpretations(ostream& out);

  Stack<TermList> _domain;


  typedef DHMap<TermList,TermList> EqMap;
  /** Maps terms to representants of their equivalence classes */
  EqMap _rewrites;

  LiteralStack _trueLits;
  LiteralStack _trueEqs;

  IGAlgorithm& _iga;
};

}

#endif // __ModelPrinter__