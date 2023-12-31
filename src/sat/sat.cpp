
#include "sat.h"
#include "cirMgr.h"

void 
SatSolver::addAigCNF(Var vf, Var va, bool fa, Var vb, bool fb)
{
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

void 
SatSolver::addXorCNF(Var vf, Var va, bool fa, Var vb, bool fb) 
{
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

void 
SatSolver::addAloCnf(vector<Var>& vars)
{
    vec<Lit> lits;
    for (size_t i = 0; i < vars.size(); ++i)
        lits.push(Lit(vars[i]));
    _solver->addClause(lits); lits.clear();
}

void 
SatSolver::addAmoCnf(vector<Var>& vars) 
{
    vec<Lit> lits;
    for (size_t i = 0; i < vars.size() - 1; ++i) {
        for (size_t j = i + 1; j < vars.size(); ++j) {
            lits.push(~Lit(vars[i])); lits.push(~Lit(vars[j]));
            _solver->addClause(lits); lits.clear();
        }
    }
}

void 
SatSolver::addCirCNF(CirObj *cirObj, const int& dataLift)
{
    vector<CirPiGate*> piList = cirObj->getPiList();
    vector<CirAigGate*> aigList = cirObj->getAigList();
    vector<CirPoGate*> poList = cirObj->getPoList();
    Var vf, va, vb;
    vec<Lit> lits;
    Lit lf, la, lb;
    size_t i0, i1, num = piList.size() + aigList.size() + poList.size();

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

