/****************************************************************************
  FileName     [ sat.h ]
  PackageName  [ sat ]
  Synopsis     [ Define miniSat solver interface functions ]
  Author       [ Chung-Yang (Ric) Huang, Cheng-Yin Wu ]
  Copyright    [ Copyleft(c) 2010-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef SAT_H
#define SAT_H

#include <cassert>
#include <iostream>
#include <vector>
#include "Solver.h"
#include "cirMgr.h"
#include "cirGate.h"

using namespace std;

/********** MiniSAT_Solver **********/
class SatSolver
{
   public : 
      SatSolver():_solver(0) { }
      ~SatSolver() { if (_solver) delete _solver; }

      // Solver initialization and reset
      void initialize() {
         reset();
         if (_curVar == 0) { _solver->newVar(); ++_curVar; }
      }
      void reset() {
         if (_solver) delete _solver;
         _solver = new Solver();
         _assump.clear(); _curVar = 0;
      }

      // Constructing proof model
      // Return the Var ID of the new Var
      inline Var newVar() { _solver->newVar(); return _curVar++; }
      // fa/fb = true if it is inverted
      void addAigCNF(Var vf, Var va, bool fa, Var vb, bool fb) {
         vec<Lit> lits;
         Lit lf = Lit(vf);
         Lit la = fa? ~Lit(va): Lit(va);
         Lit lb = fb? ~Lit(vb): Lit(vb);
         lits.push(la); lits.push(~lf);
         _solver->addClause(lits); lits.clear();
         lits.push(lb); lits.push(~lf);
         _solver->addClause(lits); lits.clear();
         lits.push(~la); lits.push(~lb); lits.push(lf);
         _solver->addClause(lits); lits.clear();
      }
      // fa/fb = true if it is inverted
      void addXorCNF(Var vf, Var va, bool fa, Var vb, bool fb) {
         vec<Lit> lits;
         Lit lf = Lit(vf);
         Lit la = fa? ~Lit(va): Lit(va);
         Lit lb = fb? ~Lit(vb): Lit(vb);
         lits.push(~la); lits.push( lb); lits.push( lf);
         _solver->addClause(lits); lits.clear();
         lits.push( la); lits.push(~lb); lits.push( lf);
         _solver->addClause(lits); lits.clear();
         lits.push( la); lits.push( lb); lits.push(~lf);
         _solver->addClause(lits); lits.clear();
         lits.push(~la); lits.push(~lb); lits.push(~lf);
         _solver->addClause(lits); lits.clear();
      }

      void addAloCnf(vector<Var>& vars) {
         vec<Lit> lits;
         for (size_t i = 0; i < vars.size(); ++i)
            lits.push(Lit(vars[i]));
         _solver->addClause(lits); lits.clear();
      }

      void addAmoCnf(vector<Var>& vars) {
         vec<Lit> lits;
         for (size_t i = 0; i < vars.size() - 1; ++i) {
            for (size_t j = i + 1; j < vars.size(); ++j) {
               lits.push(~Lit(vars[i])); lits.push(~Lit(vars[j]));
               _solver->addClause(lits); lits.clear();
            }
         }
      }

      void addCNF(vec<Lit>& lits) {
         _solver->addClause(lits);
      }

      void addCirCNF(CirObj *cirObj, const int& dataLift = 0) {

         Var vf, va, vb;
         vec<Lit> lits;
         Lit lf, la, lb;
         size_t i0, i1, num = cirObj->getGateNum();
         vector<CirAigGate*> aigList = cirObj->getAigList();
         vector<CirPoGate*> poList = cirObj->getPoList();

         for (size_t i = 0; i < num; ++i)
            this->newVar();
         
         for (size_t i = 0; i < aigList.size(); ++i) {
            // aigList[0][i]->printGate();
            vf = aigList[i]->getId() + dataLift;
            i0 = aigList[i]->getIn0LitId();
            i1 = aigList[i]->getIn1LitId();
            va = i0 / 2 + dataLift; vb = i1 / 2 + dataLift;
            // cout << "vf = " << vf << ", va = " << va << ", inv_a = " << (i0 & 1) << ", vb = " << vb << ", inv_b = " << (i1 & 1) << endl;
            this->addAigCNF(vf, va, i0 & 1, vb, i1 & 1);
         }

         for (size_t i = 0; i < poList.size(); ++i) {
            // poList[0][i]->printGate();
            vf = poList[i]->getId() + dataLift; lf = Lit(vf);
            i0 = poList[i]->getIn0LitId();
            va = i0 / 2 + dataLift; la = (i0 & 1)? ~Lit(va):Lit(va);
            // cout << "vf = " << vf << ", va = " << va << ", inv_a = " << (i0 & 1) << endl;
            lits.push(lf); lits.push(~la);
            this->addCNF(lits); lits.clear();
            lits.push(~lf); lits.push(la);
            this->addCNF(lits); lits.clear();
         }
      }

      // For incremental proof, use "assumeSolve()"
      void assumeRelease() { _assump.clear(); }
      void assumeProperty(Var prop, bool val) {
         _assump.push(val? Lit(prop): ~Lit(prop));
      }
      bool assumpSolve() { return _solver->solve(_assump); }

      // For one time proof, use "solve"
      void assertProperty(Var prop, bool val) {
         _solver->addUnit(val? Lit(prop): ~Lit(prop));
      }
      bool solve() { _solver->solve(); return _solver->okay(); }

      // Functions about Reporting
      // Return 1/0/-1; -1 means unknown value
      int getValue(Var v) const {
         return (_solver->modelValue(v)==l_True?1:
                (_solver->modelValue(v)==l_False?0:-1)); }
      void printStats() const { const_cast<Solver*>(_solver)->printStats(); }

   private : 
      Solver           *_solver;    // Pointer to a Minisat solver
      Var               _curVar;    // Variable currently
      vec<Lit>          _assump;    // Assumption List for assumption solve
};

#endif  // SAT_H

