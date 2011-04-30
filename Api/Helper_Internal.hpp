/**
 * @file Helper_Internal.hpp
 * Defines classes that do not need to be exposed to the API user.
 */

#ifndef __Api_Helper_Internal__
#define __Api_Helper_Internal__

#include "Forwards.hpp"

#include "FormulaBuilder.hpp"

#include "Helper.hpp"

#include "Lib/Environment.hpp"

#include "Kernel/BDD.hpp"
#include "Kernel/Clause.hpp"
#include "Kernel/Connective.hpp"
#include "Kernel/Formula.hpp"
#include "Kernel/FormulaUnit.hpp"
#include "Kernel/Inference.hpp"
#include "Kernel/Signature.hpp"
#include "Kernel/Term.hpp"
#include "Kernel/TermIterators.hpp"
#include "Kernel/Unit.hpp"

#include "Shell/Parser.hpp"
#include "Shell/TPTP.hpp"
#include "Shell/VarManager.hpp"

namespace Api {

using namespace Shell;

typedef Kernel::Formula::VarList VarList;

class DefaultHelperCore
{
public:
  DefaultHelperCore() {}
  virtual ~DefaultHelperCore() {
  }

  static DefaultHelperCore* instance();
  virtual string getVarName(Var v) const;
  string toString(Kernel::TermList t) const;
  string toString(const Kernel::Term* t0) const;
  string toString(const Kernel::Formula* f) const;
  string toString(const Kernel::Clause* clause) const;
  string toString (const Kernel::Unit* unit) const;

  virtual VarManager::VarFactory* getVarFactory() { return 0; };

  virtual bool isFBHelper() const { return false; }
private:
  struct Var2NameMapper;
public:
  StringIterator getVarNames(VarList* l);
};

class FBHelperCore
: public DefaultHelperCore
{
public:
  FBHelperCore() : nextVar(0), refCtr(0), varFact(*this), _unaryPredicate(0)
  {
  }

  void incRef()
  {
    CALL("ApiHelperCore::incRef");

    refCtr++;
  }

  /**
   * Decrease the reference counter of the object and destroy it if it
   * becomes zero
   *
   * After the return from this function, the object may not exist any more.
   */
  void decRef()
  {
    CALL("ApiHelperCore::decRef");
    ASS_G(refCtr,0);

    refCtr--;
    if(refCtr==0) {
      delete this;
    }
  }

  virtual bool isFBHelper() const { return true; }


  Term term(const Function& f,const Term* args, unsigned arity);
  Formula atom(const Predicate& p, bool positive, const Term* args, unsigned arity);
  virtual string getVarName(Var v) const;
  Sort getVarSort(Var v) const;
  Var getVar(string varName, Sort varSort);

  virtual VarManager::VarFactory* getVarFactory()
  { return &varFact; }

  /** indicates whether we shall check names of functions,
   * predicates and variables */
  bool _checkNames;
  /** indicates whether we shall check that we do not bind
   * variables that are already bound in a formula */
  bool _checkBindingBoundVariables;

  /** Return arbitrary uninterpreted unary predicate */
  unsigned getUnaryPredicate();

  Sort getSort(const Api::Term t);
  void ensureArgumentsSortsMatch(BaseType* type, const Api::Term* args);
  void ensureEqualityArgumentsSortsMatch(const Api::Term arg1, const Api::Term arg2);
private:

  struct FBVarFactory : public VarManager::VarFactory
  {
    explicit FBVarFactory(FBHelperCore& parent) : _parent(parent) {}
    virtual unsigned getVarAlias(unsigned var);
    virtual string getVarName(unsigned var);

    FBHelperCore& _parent;
  };

  /** Map from variable names to their numbers */
  Map<string,Var> vars;
  /** Map from variable names to their numbers */
  Map<Var,string> varNames;
  /** Map from variable names to their sorts */
  Map<Var,Sort> varSorts;
  /** next available variable number */
  Var nextVar;

  int refCtr;

  FBVarFactory varFact;

  /** Can contain an un-interpreted unary predicate, or zero in case
   * it is uninitialized
   *
   * Is used in @c FormulaBuilder::replaceConstant() */
  unsigned _unaryPredicate;
};

class SingleVarApplicator
{
public:
  SingleVarApplicator(unsigned var, TermList term) : _srcVar(var), _tgtTerm(term) {}
  TermList apply(unsigned var)
  {
    if(var!=_srcVar) {
      return TermList(var, false);
    }
    return _tgtTerm;
  }
private:
  unsigned _srcVar;
  TermList _tgtTerm;
};


}

#endif // __Api_Helper_Internal__
