/**
 * @file SMTFormula.cpp
 * Implements class SMTFormula.
 */

#include <ostream>
#include <sstream>

#include "Lib/Int.hpp"
#include "Lib/Metaiterators.hpp"
#include "Lib/Sort.hpp"
#include "Lib/System.hpp"

#include "SMTFormula.hpp"

namespace Shell
{

SMTConstant SMTFormula::unsignedValue(unsigned val)
{
  CALL("SMTFormula::unsignedValue");

  return SMTConstant(Int::toString(val)+".0");
}

SMTConstant SMTFormula::name(string name)
{
  CALL("SMTFormula::name/1");

  return SMTConstant(name);
}

SMTConstant SMTFormula::name(string n1, string n2)
{
  CALL("SMTFormula::name/2");

  return SMTConstant(n1 + "_" + n2);
}


SMTFormula SMTFormula::condNumber(SMTFormula condition, unsigned value)
{
  CALL("SMTFormula::condNumber");

  return SMTFormula("(ite " + condition._val + " " + unsignedValue(value)._val + " 0.0)");
}


SMTFormula SMTFormula::conjunction(const SMTFormula& f1, const SMTFormula& f2)
{
  CALL("SMTFormula::conjunction");

  if(f1.isTrue()) { return f2; }
  if(f2.isTrue()) { return f1; }
  if(f1.isFalse() || f2.isFalse()) { return getFalse(); }
  return SMTFormula("(and " + f1._val + " " + f2._val + ")");
}

SMTFormula SMTFormula::disjunction(const SMTFormula& f1, const SMTFormula& f2)
{
  CALL("SMTFormula::disjunction");

  if(f1.isFalse()) { return f2; }
  if(f2.isFalse()) { return f1; }
  if(f1.isTrue() || f2.isTrue()) { getTrue(); }
  return SMTFormula("(or " + f1._val + " " + f2._val + ")");
}



//helper operator implementation for pseudo-operators --> and -=-

SMTFormula__HalfImpl SMTFormula::operator--(int) const { return SMTFormula__HalfImpl(*this); }

SMTFormula__HalfEquiv SMTFormula::operator-() const { return SMTFormula__HalfEquiv(*this); }
SMTFormula SMTFormula::operator-=(const SMTFormula__HalfEquiv& r) const
{ return SMTFormula("(= " + _val + " " + r.pf._val + ")"); }




///////////////////////
// SMTBenchmark
//

void SMTBenchmark::addFormula(const SMTFormula& f, string comment)
{
  CALL("SMTBenchmark::addFormula");
  ASS_EQ(_formulas.size(), _formulaComments.size());

  _formulas.push(f);
  _formulaComments.push(comment);
}

/**
 * Remove the most recently added formula
 */
void SMTBenchmark::popFormula()
{
  CALL("SMTBenchmark::popFormula");
  ASS_EQ(_formulas.size(), _formulaComments.size());

  _formulas.pop();
  _formulaComments.pop();
}

void SMTBenchmark::declarePropositionalConstant(SMTConstant pred)
{
  CALL("SMTBenchmark::declarePropositionalConstant");
  ASS(!_funDecls.find(pred.toString()));

  _predDecls.insert(pred.toString());
}

void SMTBenchmark::declareRealConstant(SMTConstant pred)
{
  CALL("SMTBenchmark::declareRealConstant");
  ASS(!_predDecls.find(pred.toString()));

  _funDecls.insert(pred.toString(), "Real");
}

void SMTBenchmark::output(ostream& out)
{
  CALL("SMTBenchmark::output");

  out << "(benchmark VampireGeneratedBenchmark" << endl;

  StringMap::Iterator fdeclIt(_funDecls);
  while(fdeclIt.hasNext()) {
    string func, fType;
    fdeclIt.next(func, fType);
    out << ":extrafuns ((" << func << " " << fType << "))" <<endl;
  }

  static Stack<string> predDeclStack;
  predDeclStack.reset();
  predDeclStack.loadFromIterator(_predDecls.iterator());
  sort<DefaultComparator>(predDeclStack.begin(), predDeclStack.end());

  Stack<string>::BottomFirstIterator pdeclIt(predDeclStack);
  while(pdeclIt.hasNext()) {
    string pred = pdeclIt.next();
    out << ":extrapreds ((" << pred << "))" <<endl;
  }

  out << ":formula ( (and " << endl;

  Stack<SMTFormula>::BottomFirstIterator fit(_formulas);
  Stack<string>::BottomFirstIterator fCommentIt(_formulaComments);

  if(!fit.hasNext()) { out << "  true" << endl; }

  while(fit.hasNext()) {
    ALWAYS(fCommentIt.hasNext());

    SMTFormula form = fit.next();
    string comment = fCommentIt.next();
    out << "  " << form.toString();
    if(comment!="") {
      out << " ; " << comment;
    }
    out << endl;
  }

  out << ") )" << endl;
  out << ")" << endl;
}

///////////////////////
// SMTSolver
//

void SMTSolver::minimize(SMTBenchmark& problem, SMTConstant costFn, SMTSolverResult& res)
{
  CALL("SMTSolver::minimize");

  unsigned left = 1;
  unsigned guess = left;
  while(!tryUpperBound(problem, costFn, guess, res)) {
    unsigned newGuess = guess*2;
    if(newGuess<=guess) {
      //we had an overflow
      return;
    }
    left = guess;
    guess = newGuess;
  }
  unsigned right = guess;

  while(left!=right) {
    unsigned middle = left + (right-left)/2;
    if(tryUpperBound(problem, costFn, middle, res)) {
      right = middle;
    }
    else {
      left = middle+1;
    }
  }
  tryUpperBound(problem, costFn, left, res);
}

bool SMTSolver::tryUpperBound(SMTBenchmark& problem, SMTConstant costFn, unsigned val, SMTSolverResult& res)
{
  CALL("SMTSolver::tryUpperBound");

  SMTFormula valFormula = SMTFormula::unsignedValue(val);
  SMTFormula bound = SMTFormula::less(costFn, valFormula);

  problem.addFormula(bound);

  run(problem, res);

  problem.popFormula();

  return res.status==SMTSolverResult::SAT;
}

///////////////////////
// YicesSolver
//

void YicesSolver::run(SMTBenchmark& problem, SMTSolverResult& res)
{
  CALL("YicesSolver::run");

  stringstream problemStm;
  problem.output(problemStm);

  static Stack<string> proverOut;
  proverOut.reset();

  string execName = System::guessExecutableDirectory()+"/yices";
  if(!System::fileExists(execName)) {
    USER_ERROR("Executable "+execName+" does not exist");
  }

  System::executeCommand(execName+" -smt -e", problemStm.str(), proverOut);

  res.reset();

  Stack<string>::Iterator oit(proverOut);
  while(oit.hasNext()) {
    string line = oit.next();
    if(line=="") { continue; }
    if(line=="sat") { res.status = SMTSolverResult::SAT; continue; }
    if(line=="unsat") { res.status = SMTSolverResult::UNSAT; continue; }

    if(line.substr(0,3)!="(= " || line.substr(line.size()-1,1)!=")") {
      LOG("unrecognized Yices output line: \"" << line << "\"");
      continue;
    }
    string lineCore = line.substr(3, line.size()-4);
    size_t sep = lineCore.find(' ');
    string element = lineCore.substr(0, sep);
    string value = lineCore.substr(sep+1);

    res.assignment.insert(element, value);
  }
}


}