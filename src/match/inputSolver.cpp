#include "inputSolver.h"
#include "cirMgr.h"

void
InputSolver::generateAigVar() {
    vector<CirPiGate*> piList[2];
    vector<CirAigGate*> aigList[2];
    vector<CirPoGate*> poList[2];
    for (size_t i = 0; i < 2; ++i) {
        piList[i] = cirMgr->getCir(i + 1)->getPiList();
        aigList[i] = cirMgr->getCir(i + 1)->getAigList();
        poList[i] = cirMgr->getCir(i + 1)->getPoList();
        
        for (CirPiGate* gate: piList[i])
            gate->setVar(_solver.newVar());
        for (CirAigGate* gate: aigList[i])
            gate->setVar(_solver.newVar());
        for (CirPoGate* gate: poList[i])
            gate->setVar(_solver.newVar());
    }
}

void
InputSolver::generateMoVar(size_t nRow, size_t nCol)
{
    _mo.resize(nRow);
    for (size_t i = 0; i < nRow; ++i)
        _mo[i].resize(nCol);
    
    for (size_t i = 0; i < nRow; ++i)
        for (size_t j = 0; j < nCol; ++j)
            _mo[i][j] = _solver.newVar();
    
}

void
InputSolver::generateMiVar(size_t nRow, size_t nCol)
{
    _mi.resize(nRow);
    for (size_t i = 0; i < nRow; ++i)
        _mi[i].resize(nCol);
    
    for (size_t i = 0; i < nRow; ++i)
        for (size_t j = 0; j < nCol; ++j)
            _mi[i][j] = _solver.newVar();
}

void
InputSolver::generateHVar(size_t nRow, size_t nCol)
{
    _h.resize(nRow);
    for (size_t i = 0; i < nRow; ++i)
        _h[i].resize(nCol);
    
    for (size_t i = 0; i < nRow; ++i)
        for (size_t j = 0; j < nCol; ++j)
            _h[i][j] = _solver.newVar();
}

void
InputSolver::init() {

    Var vf, va, vb, vc;
    Lit lf, la, lb, lc;
    vec<Lit> lits;
    
    // 1. AIG constraint
    vector<CirPiGate*> piList[2];
    vector<CirAigGate*> aigList[2];
    vector<CirPoGate*> poList[2];
    for (size_t i = 0; i < 2; ++i) {
        piList[i] = cirMgr->getCir(i + 1)->getPiList();
        aigList[i] = cirMgr->getCir(i + 1)->getAigList();
        poList[i] = cirMgr->getCir(i + 1)->getPoList();
    }

    // ========== add new variable ==========
    generateAigVar();
    generateMoVar(poList[1].size(), poList[0].size() * 2);
    generateMiVar(piList[1].size(), (piList[0].size() + 1) * 2);
    generateHVar(poList[1].size(), poList[0].size() * 2);

    // ========== add constraint ==========

    for (size_t i = 0; i < 2; ++i) {

        for (CirAigGate* gate: aigList[i]) {
            _solver.addAigCNF(
                gate->getVar(),
                gate->getIn0Gate()->getVar(),
                gate->isIn0Inv(),
                gate->getIn1Gate()->getVar(),
                gate->isIn1Inv()
            );
        }

        for (CirPoGate* gate: poList[i]) {
            vf = gate->getVar(); lf = Lit(vf);
            va = gate->getIn0Gate()->getVar(); la = gate->isIn0Inv()? ~Lit(va):Lit(va);
            lits.push(lf); lits.push(~la);
            _solver.addCNF(lits); lits.clear();
            lits.push(~lf); lits.push(la);
            _solver.addCNF(lits); lits.clear();
        }
    }

    // 2. input mapping
    for (size_t j = 0; j < _mi.size(); ++j) {

        // yj to xi
        for (size_t i = 0; i < _mi[j].size() - 2; ++i) {
            vf = _mi[j][i]; lf = Lit(vf);
            va = piList[0][i / 2]->getVar(); la = (i & 1)? ~Lit(va):Lit(va);
            vb = piList[1][j]->getVar(); lb = Lit(vb);
            lits.push(~lf); lits.push(~la); lits.push(lb);
            _solver.addCNF(lits); lits.clear();
            lits.push(~lf); lits.push(la); lits.push(~lb);
            _solver.addCNF(lits); lits.clear();
        }

        // yj to const.
        for (size_t i = _mi[j].size() - 2; i < _mi[j].size(); ++i) {
            vf = _mi[j][i]; lf = Lit(vf);
            va = piList[1][j]->getVar(); la = (i & 1)? ~Lit(va):Lit(va);
            lits.push(~lf); lits.push(~la);
            _solver.addCNF(lits); lits.clear();
        }
    }

    // 3. output mapping

    // f xor g
    // c11 -> h11 == (f1 XOR g1)
    // exclude the case of cij = 0
    for (size_t j = 0; j < _mo.size(); ++j) {
        for (size_t i = 0; i < _mo[j].size(); ++i) {
            vf = _mo[j][i]; lf = Lit(vf);
            va = _h[j][i]; la = Lit(va);
            vb = poList[0][i / 2]->getVar(); lb = (i & 1)? ~Lit(vb):Lit(vb);
            vc = poList[1][j]->getVar(); lc = Lit(vc);
            lits.push(~lf); lits.push(~la); lits.push(lb); lits.push(lc);
            _solver.addCNF(lits); lits.clear();
            lits.push(~lf); lits.push(~la); lits.push(~lb); lits.push(~lc);
            _solver.addCNF(lits); lits.clear();
            lits.push(~lf); lits.push(la); lits.push(lb); lits.push(~lc);
            _solver.addCNF(lits); lits.clear();
            lits.push(~lf); lits.push(la); lits.push(~lb); lits.push(lc);
            _solver.addCNF(lits); lits.clear();
        }
    }

   // c, d and h
   // ~c11 -> ~h11
    for (size_t j = 0; j < _mo.size(); ++j) {
        for (size_t i = 0; i < _mo[j].size(); ++i) {
            vf = _mo[j][i]; lf = Lit(vf);
            va = _h[j][i]; la = Lit(va);
            lits.push(lf); lits.push(~la);
            _solver.addCNF(lits); lits.clear();
        }
    }

   // sum hij
    for (size_t j = 0; j < _h.size(); ++j) {
        for (size_t i = 0; i < _h[j].size(); ++i) {
            vf = _h[j][i]; lf = Lit(vf);
            lits.push(lf);
        }
    }
    _solver.addCNF(lits); lits.clear();

}


void
InputSolver::assumeMo(vector<vector<bool>> mo) {

    for (size_t i = 0; i < _mo.size(); ++i)
        for (size_t j = 0; j < _mo[i].size(); ++j)
            _solver.assumeProperty(_mo[i][j], mo[i][j]);
}

void
InputSolver::assumeMi(vector<vector<bool>> mi) {
    for (size_t i = 0; i < _mi.size(); ++i)
        for (size_t j = 0; j < _mi[i].size(); ++j)
            _solver.assumeProperty(_mi[i][j], mi[i][j]);
}

vector<size_t>
InputSolver::getPiValue(size_t cirId) {

    vector<size_t> values;
    vector<CirPiGate*> piList = cirMgr->getCir(cirId)->getPiList();

    values.resize(piList.size());
    for (size_t i = 0; i < piList.size(); ++i)
        values[i] = _solver.getValue(piList[i]->getVar());
    
    return values;
}

vector<size_t>
InputSolver::getPoValue(size_t cirId) {

    vector<size_t> values;
    vector<CirPoGate*> poList = cirMgr->getCir(cirId)->getPoList();

    values.resize(poList.size());
    for (size_t i = 0; i < poList.size(); ++i)
        values[i] = _solver.getValue(poList[i]->getVar());
    
    return values;
}