#include "cirMgr.h"
#include "outputSolver.h"
#include "inputSolver.h"
#include "SolverTypes.h"

void
OutputSolver::generateMoVar(size_t nRow, size_t nCol)
{
    _mo.resize(nRow);
    for (size_t i = 0; i < nRow; ++i) {
        _mo[i].resize(nCol);
        for (size_t j = 0; j < nCol; ++j)
            _mo[i][j] = _solver.newVar();
    }
}

void
OutputSolver::generateMiVar(size_t nRow, size_t nCol)
{
    _mi.resize(nRow);
    for (size_t i = 0; i < nRow; ++i) {
        _mi[i].resize(nCol);
        for (size_t j = 0; j < nCol; ++j)
            _mi[i][j] = _solver.newVar();
    }
}

void
OutputSolver::generateMboVar(size_t nRow, size_t nCol)
{
    _mbo.resize(nRow);
    for (size_t i = 0; i < nRow; ++i) {
        _mbo[i].resize(nCol);
        for (size_t j = 0; j < nCol; ++j)
            _mbo[i][j] = _solver.newVar();
    }
}

void
OutputSolver::generateMbiVar(size_t nRow, size_t nCol)
{
    _mbi.resize(nRow);
    for (size_t i = 0; i < nRow; ++i) {
        _mbi[i].resize(nCol);
        for (size_t j = 0; j < nCol; ++j)
            _mbi[i][j] = _solver.newVar();
    }
}

void
OutputSolver::init()
{
    // ========== new variable ========== 
    vector<CirPiGate*> piList[2];
    vector<CirAigGate*> aigList[2];
    vector<CirPoGate*> poList[2];
    vector<vector<size_t>> piBus[2];
    vector<vector<size_t>> poBus[2];
    for (size_t i = 0; i < 2; ++i) {
        piList[i] = cirMgr->getCir(i + 1)->getPiList();
        aigList[i] = cirMgr->getCir(i + 1)->getAigList();
        poList[i] = cirMgr->getCir(i + 1)->getPoList();
        piBus[i] = cirMgr->getCir(i + 1)->getPiBus();
        poBus[i] = cirMgr->getCir(i + 1)->getPoBus();
    }

    vector<size_t> size = {
        poList[1].size(), poList[0].size() * 2,
        piList[1].size(), (piList[0].size() + 1) * 2,
        poBus[1].size(), poBus[0].size(),
        piBus[1].size(), piBus[0].size()
    };
    generateMoVar(size[0], size[1]);
    generateMiVar(size[2], size[3]);
    generateMboVar(size[4], size[5]);
    generateMbiVar(size[6], size[7]);
    _useBus = _solver.newVar();

    // ========== setting ==========
    setK(poList[1].size() + 1);

    cout << "_mo size: " << _mo.size() << " x " << _mo[0].size() << endl;
    cout << "_mi size: " << _mi.size() << " x " << _mi[0].size() << endl;
    cout << "_mbo size: " << _mbo.size() << " x " << _mbo[0].size() << endl;
    cout << "_mbi size: " << _mbi.size() << " x " << _mbi[0].size() << endl;

    // ========== add constraint ==========
    Var vf, va, vb;
    Lit lf, la, lb;
    vec<Lit> lits;

    // 1. PO mapping
    // PO mapping is 1-1

    // each row of _mo has exactly one 1
    for (size_t i = 0; i < _mo.size(); ++i) {
      _solver.addAloCnf(_mo[i]);
      _solver.addAmoCnf(_mo[i]);
    }

    // every two column of the Mo matrix
    // (1) every two entry of C/D coulmn can not both be 1
    // (2) ci and dj can not both be 1
    for (size_t i = 0; i < _mo[0].size(); i += 2) {

        for (size_t j = 0; j < _mo.size() - 1; ++j) {
            for (size_t k = j + 1; k < _mo.size(); ++k) {
                va = _mo[j][i]; la = Lit(va); lits.push(~la);
                vb = _mo[k][i]; lb = Lit(vb); lits.push(~lb);
                _solver.addCNF(lits); lits.clear();
            }
        }

        for (size_t j = 0; j < _mo.size(); ++j) {
            for (size_t k = 0; k < _mo.size(); ++k) {
                va = _mo[j][i]; la = Lit(va); lits.push(~la);
                vb = _mo[k][i + 1]; lb = Lit(vb); lits.push(~lb);
                _solver.addCNF(lits); lits.clear();
            }
        }

        for (size_t j = 0; j < _mo.size() - 1; ++j) {
            for (size_t k = j + 1; k < _mo.size(); ++k) {
                va = _mo[j][i + 1]; la = Lit(va); lits.push(~la);
                vb = _mo[k][i + 1]; lb = Lit(vb); lits.push(~lb);
                _solver.addCNF(lits); lits.clear();
            }
        }
    }

    // 2. PI mapping

    // each circuit 2 PI can only map to a circuit 1 PI
    for (size_t i = 0; i < _mi.size(); ++i) {
        _solver.addAloCnf(_mi[i]);
        _solver.addAmoCnf(_mi[i]);
    }

    // each circuit 1 PI maps to at least one circuit 2 PI
    for (size_t i = 0; i < _mi[0].size() - 2; i += 2) {
        for (size_t j = 0; j < _mi.size(); ++j) {
            vf = _mi[j][i]; lf = Lit(vf); lits.push(lf);
            vf = _mi[j][i + 1]; lf = Lit(vf); lits.push(lf);
        }
        _solver.addCNF(lits); lits.clear();
    }

    // 3. Mbo mapping
    // Mbo mapping is 1-1

    // each circuit 2 bus can only map to a circuit 1 bus
    for (size_t i = 0; i < _mbo.size(); ++i) {
        _solver.addAloCnf(_mbo[i]);
        _solver.addAmoCnf(_mbo[i]);
    }

    // each circuit 1 bus maps to at least one circuit 2 bus
    for (size_t i = 0; i < _mbo[0].size(); ++i) {
        for (size_t j = 0; j < _mbo.size(); ++j)
            vf = _mbo[j][i]; lf = Lit(vf); lits.push(lf);
        _solver.addCNF(lits); lits.clear();
    }

    // 4. Mbi mapping
    // Mbi mapping is 1-1.
    // each circuit 2 bus can only map to a circuit 1 bus
    for (size_t i = 0; i < _mbi.size(); ++i) {
        _solver.addAloCnf(_mbi[i]);
        _solver.addAmoCnf(_mbi[i]);
    }

    // each circuit 1 bus maps to at least one circuit 2 bus
    for (size_t i = 0; i < _mbi[0].size(); ++i) {
        for (size_t j = 0; j < _mbi.size(); ++j)
            vf = _mbi[j][i]; lf = Lit(vf); lits.push(lf);
        _solver.addCNF(lits); lits.clear();
    }

}

void
OutputSolver::poFuncSuppConstraint() {

    cout << "test183";
    vector<vector<size_t>> cir1PoFuncSupp, cir2PoFuncSupp;
    cir1PoFuncSupp = cirMgr->getCir(1)->getPoFuncSupp();
    cir2PoFuncSupp = cirMgr->getCir(2)->getPoFuncSupp();
    cout << 1;
    Var vf;
    Lit lf;
    vec<Lit> lits;
    cout << 2;
    // f is a circuit 1 PO and g is a circuit 2 PO
    // if |FuncSupp(f)| > |FuncSupp(g)|, then f cannot map to g
    for (size_t j = 0; j < _mo.size(); ++j) {
        for (size_t i = 0; i < _mo[j].size(); ++i) {
            if (cir1PoFuncSupp[i].size() > cir2PoFuncSupp[j].size()) {
                vf = _mo[j][i * 2]; lf = ~Lit(vf); lits.push(lf);
                _solver.addCNF(lits); lits.clear();
                vf = _mo[j][i * 2 + 1]; lf = ~Lit(vf); lits.push(lf);
                _solver.addCNF(lits); lits.clear();
            }
        }
    }
    cout << 3;
}

void
OutputSolver::piFuncSuppConstraint() {

    vector<vector<size_t>> cir1PiFuncSupp, cir2PiFuncSupp;
    cir1PiFuncSupp = cirMgr->getCir(1)->getPiFuncSupp();
    cir2PiFuncSupp = cirMgr->getCir(2)->getPiFuncSupp();

    Var vf;
    Lit lf;
    vec<Lit> lits;

    // f is a circuit 1 PO and g is a circuit 2 PO
    // if |FuncSupp(f)| > |FuncSupp(g)|, then f cannot map to g
    for (size_t j = 0; j < _mi.size(); ++j) {
        for (size_t i = 0; i < _mi[j].size(); ++i) {
            if (cir1PiFuncSupp[i].size() > cir2PiFuncSupp[j].size()) {
                vf = _mi[j][i * 2]; lf = ~Lit(vf); lits.push(lf);
                _solver.addCNF(lits); lits.clear();
                vf = _mi[j][i * 2 + 1]; lf = ~Lit(vf); lits.push(lf);
                _solver.addCNF(lits); lits.clear();
            }
        }
    }
}

void
OutputSolver::poBusConstraint() {

    vector<vector<size_t>> cir1PoBus, cir2PoBus;
    cir1PoBus = cirMgr->getCir(1)->getPoBus();
    cir2PoBus = cirMgr->getCir(2)->getPoBus();

    Var vf, va, vb;
    Lit lf, la, lb;
    vec<Lit> lits;

    for (size_t j = 0; j < _mbo.size(); ++j) {
        for (size_t i = 0; i < _mbo[j].size(); ++i) {
            for (size_t k = 0; k < cir2PoBus[j].size(); ++k) {
                vf = _useBus; lf = Lit(vf); lits.push(~lf);
                va = _mbo[j][i]; la = Lit(va); lits.push(~la);
                for (size_t l = 0; l > cir1PoBus[i].size(); ++l) {
                    vb = _mo[cir2PoBus[j][k]][cir1PoBus[i][l] * 2]; lb = Lit(vb); lits.push(lb);
                    vb = _mo[cir2PoBus[j][k]][cir1PoBus[i][l] * 2 + 1]; lb = Lit(vb); lits.push(lb);
                }
                _solver.addCNF(lits); lits.clear();
            }
        }
    }
}

void
OutputSolver::piBusConstraint() {

    vector<vector<size_t>> cir1PiBus, cir2PiBus;
    cir1PiBus = cirMgr->getCir(1)->getPiBus();
    cir2PiBus = cirMgr->getCir(2)->getPiBus();

    Var vf, va, vb;
    Lit lf, la, lb;
    vec<Lit> lits;

    for (size_t j = 0; j < _mbi.size(); ++j) {
        for (size_t i = 0; i < _mbi[j].size(); ++i) {
            for (size_t k = 0; k < cir2PiBus[j].size(); ++k) {
                vf = _useBus; lf = Lit(vf); lits.push(~lf);
                va = _mbo[j][i]; la = Lit(va); lits.push(~la);
                for (size_t l = 0; l > cir1PiBus[i].size(); ++l) {
                    vb = _mi[cir2PiBus[j][k]][cir1PiBus[i][l] * 2]; lb = Lit(vb); lits.push(lb);
                    vb = _mi[cir2PiBus[j][k]][cir1PiBus[i][l] * 2 + 1]; lb = Lit(vb); lits.push(lb);
                }
                for (size_t l = _mi[cir2PiBus[j][k]].size() - 2; l < _mi[cir2PiBus[j][k]].size(); ++l)
                    vb = _mi[cir2PiBus[j][k]][l]; lb = Lit(vb); lits.push(lb);
                _solver.addCNF(lits); lits.clear();
            }
        }
    }
}

void
OutputSolver::forbidCounterExample(InputSolver& inputSolver) {


    Var vf, va, vb;
    Lit lf, la, lb;
    vec<Lit> lits;

    vector<CirPiGate*> piList[2];
    vector<CirPoGate*> poList[2];
    for (size_t i = 0; i < 2; ++i) {
        piList[i] = cirMgr->getCir(i + 1)->getPiList();
        poList[i] = cirMgr->getCir(i + 1)->getPoList();
    }

    vector<vector<bool>> redundant[2];
    vector<size_t> counterIn[2], counterOut[2];
    for (size_t i = 0; i < 2; ++i) {
        
        counterIn[i].resize(piList[i].size(), 0);
        counterIn[i] = inputSolver.getPiValue(i + 1);

        counterOut[i].resize(poList[i].size(), 0);
        counterOut[i] = inputSolver.getPoValue(i + 1);
        
        cirMgr->getCir(i + 1)->getRedundant(counterIn[i], counterOut[i], redundant[i]);
    }

    // refer to strenthen learning ex5
    for (size_t q = 0; q < poList[1].size(); ++q) {
        for (size_t p = 0; p < poList[0].size(); ++p) {
            vf = (counterOut[0][p] != counterOut[1][q])? _mo[q][2 * p]:_mo[q][2 * p + 1];
            lf = Lit(vf); lits.push(~lf);

            for (size_t j = 0; j < piList[1].size(); ++j) {
                for (size_t i = 0; i < piList[0].size(); ++i) {
                    if (redundant[0][p][i] || redundant[1][q][j]) continue;
                    va = (counterIn[0][i] != counterIn[1][j])? _mi[j][2 * i]:_mi[j][2 * i + 1];
                    la = Lit(va); lits.push(la);
                }
            }

            for (size_t j = 0; j < piList[1].size(); ++j) {
                if (redundant[1][q][j]) continue;
                va = (counterIn[1][j])? _mi[j][2 * piList[0].size()]:_mi[j][2 * piList[0].size() + 1];
                la = Lit(va); lits.push(la);
            }
            _solver.addCNF(lits); lits.clear();
        }
    }
}

void
OutputSolver::forbidCurrentMapping() {

    Var vf;
    Lit lf;
    vec<Lit> lits;

    for (size_t j = 0; j < _mo.size(); ++j) {
        for (size_t i = 0; i < _mo[j].size(); ++i) {
            vf = _mo[j][i]; lf = (_solver.getValue(_mo[j][i]))? ~Lit(vf):Lit(vf);
            lits.push(lf);
        }
    }
    for (size_t j = 0; j < _mi.size(); ++j) {
        for (size_t i = 0; i < _mi[j].size(); ++i) {
            vf = _mi[j][i]; lf = (_solver.getValue(_mi[j][i]))? ~Lit(vf):Lit(vf);
            lits.push(lf);
        }
    }
    _solver.addCNF(lits); lits.clear();

}

size_t
OutputSolver::calScore() {

    size_t score = 0, tmp;
    for (size_t i = 0; i < _mo[0].size(); i += 2) {
        tmp = 0;
        for (size_t j = 0; j < _mo.size(); ++j) {
            tmp += _solver.getValue(_mo[j][i]) + _solver.getValue(_mo[j][i + 1]);
        }
        score += (tmp > 0)? _k + tmp:tmp;
    }
    return score;
}

vector<vector<bool>>
OutputSolver::getMiValue() {

    vector<vector<bool>> values;
    values.resize(_mi.size());
    for (size_t i = 0; i < _mi.size(); ++i) {
        values[i].resize(_mi[i].size());
        for (size_t j = 0; j < _mi[i].size(); ++j) {
            values[i][j] = _solver.getValue(_mi[i][j]) == 1;
        }
    }

    return values;
}

vector<vector<bool>>
OutputSolver::getMoValue() {

    vector<vector<bool>> values;
    values.resize(_mo.size());
    for (size_t i = 0; i < _mo.size(); ++i) {
        values[i].resize(_mo[i].size());
        for (size_t j = 0; j < _mo[i].size(); ++j) {
            values[i][j] = _solver.getValue(_mo[i][j]) == 1;
        }
    }

    return values;
}