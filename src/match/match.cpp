#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include "match.h"
#include "genAag.h"
#include "cirMgr.h"
#include "cirGate.h"
#include "SolverTypes.h"
#include "util.h"
#include "outputSolver.h"
#include "inputSolver.h"


void Match::parseInput(char* inFilePath, vector<string>& cirFileList)
{
   ifstream inFile(inFilePath);
   string filePath(inFilePath), prefix;
   prefix = getDirName(filePath);

   string cirFilePath, line;
   int M, busNum;
   // circuit 1
   getline(inFile, cirFilePath);
   getline(inFile, line);
   busNum = stoi(line);
   cirFileList.push_back(genAagFile(prefix, cirFilePath));
   for (int i = 0 ; i < busNum; i++)
      getline(inFile, line);

   // circuit 2
   getline(inFile, cirFilePath);
   getline(inFile, line);
   busNum = stoi(line);
   cirFileList.push_back(genAagFile(prefix, cirFilePath));
   for (int i = 0 ; i < busNum; i++)
      getline(inFile, line);

}

void Match::solve() {
   
   for (size_t i = 1; i <= 2; ++i) {
      
      cirMgr->getCir(i)->collectPoFuncSupp();
      cirMgr->getCir(i)->collectPoGateCount();
      cirMgr->getCir(i)->collectPoOrder();
      cirMgr->getCir(i)->reorderPoBus();

      cirMgr->getCir(i)->collectPiFuncSupp();
      cirMgr->getCir(i)->collectPiGateCount();
      cirMgr->getCir(i)->collectPiOrder();
      cirMgr->getCir(i)->reorderPiBus();
   }

   // output solver initialization
   _outputSolver.init();
   cout << "OK57\n";
   _outputSolver.poFuncSuppConstraint();
   cout << "OK58\n";
   _outputSolver.piFuncSuppConstraint();
   _outputSolver.poBusConstraint();
   _outputSolver.piBusConstraint();


   // input solver initialization
   _inputSolver.init();

   vec<Lit> lits;
   Lit lf, la, lb, lc;
   Var vf, va, vb, vc;
   size_t i0, i1;


   // resize resultMi, resultMo
   vector<CirPiGate*> piList[2];
   vector<CirPoGate*> poList[2];
   for (size_t i = 0; i < 2; ++i) {
      piList[i] = cirMgr->getCir(i + 1)->getPiList();
      poList[i] = cirMgr->getCir(i + 1)->getPoList();
   }
   _resultMo.resize(poList[1].size(), vector<bool>(poList[0].size() * 2, false));
   _resultMi.resize(piList[1].size(), vector<bool>((piList[0].size() + 1) * 2, false));


   // ================== solve ==================
   size_t optimal = 0, score;
   bool useBus = true;
   _outputSolver.assumeRelease();
   _outputSolver.assumeBus(useBus);
   calOptimal();

   for (size_t x = 0; optimal != _optimal; ++x) {

      // ========== output solver ==========
      if (_outputSolver.solve()) score = _outputSolver.calScore();
      else {
         if (useBus) {
            cout << "change to not use bus information" << endl;
            useBus = false;
            _outputSolver.assumeRelease();
            _outputSolver.assumeBus(useBus);
         }
         else break;
      }

      if (score <= optimal) continue;
      cout << "current step: " << x << ", optimal = " << optimal << endl;

      // ========== input solver ==========
      _inputSolver.assumeRelease();
      _inputSolver.assumeMi(_outputSolver.getMiValue());
      _inputSolver.assumeMo(_outputSolver.getMoValue());

      // if input solver unsat -> no counter example -> record the sol of MI/MO as final answer
      if (!_inputSolver.solve()) {
         
         // record optimal value and optimal result
         if (optimal < score) {
            optimal = score;

            cout << "find a solution, new optimal = " << optimal << endl;
            recordOptimalSol();
            continue;
         }
      }
      else {
         _outputSolver.forbidCounterExample(_inputSolver);
         _outputSolver.forbidCurrentMapping();
      }

   }
   // ================== solve ==================
   write();

}

void
Match::calOptimal() {
   vector<CirPoGate*> poList = cirMgr->getCir(2)->getPoList();
   size_t poSize = poList.size();
   _optimal = (_outputSolver.getK() + 1) * poSize;
}

void
Match::recordOptimalSol() {
   _resultMi = _outputSolver.getMiValue();
   _resultMo = _outputSolver.getMoValue();
}

void 
Match::printMatchedMiInvFuncSupp() const
{
   vector<CirPiGate*> cir1Pi = cirMgr->getCir(1)->getPiList();
   vector<CirPiGate*> cir2Pi = cirMgr->getCir(2)->getPiList();
   vector<vector<size_t>> cir1Inv = cirMgr->getCir(1)->getPiFuncSupp();
   vector<vector<size_t>> cir2Inv = cirMgr->getCir(2)->getPiFuncSupp();

   for (size_t i = 0 ; i < cir2Pi.size() ; i++) {
      for (size_t j = 0 ; j < cir1Pi.size() ; j++) {
         if (_resultMi[i][j*2] || _resultMi[i][j*2+1]) {
            cout << "(" << cir2Pi[i]->getName() << "," << cir1Pi[j]->getName()
                 << " : " << cir2Inv[i].size() << "," << cir1Inv[j].size() << ")" << endl;
         }
      }
   }
}

void Match::write() {

   cout << "optimal = " << _optimal << endl;
   cout << "final mapping result:" << endl;
   cout << "output:" << endl;
   cout << "po size = " << (_resultMo[0].size()-2) / 2 << " " << _resultMo.size() << endl;
   cout << "pi size = " << (_resultMi[0].size()-2) / 2 << " " << _resultMi.size() << endl;

   // return;
   for (size_t j = 0; j < _resultMo.size(); ++j) {
      for (size_t i = 0; i < _resultMo[j].size(); ++i) {
         cout << _resultMo[j][i] << " ";
      }
      cout << endl;
   }
   cout << endl;

   cout << "input:" << endl;
   for (size_t j = 0; j < _resultMi.size(); ++j) {
      for (size_t i = 0; i < _resultMi[j].size(); ++i) {
         cout << _resultMi[j][i] << " ";
      }
      cout << endl;
   }
   cout << endl;

   // Todo: write file
}

// bool 
// Match::checkSol() const 
// {
//    SatSolver checker;
//    checker.initialize();

//    vector<CirGate*> dfs1 = cirMgr->getCir(1)->getDfsList();
//    vector<CirGate*> dfs2 = cirMgr->getCir(2)->getDfsList();
//    vector<CirPiGate*> pi1 = cirMgr->getCir(1)->getPiList();
//    vector<CirPiGate*> pi2 = cirMgr->getCir(2)->getPiList();
//    vector<CirPoGate*> po1 = cirMgr->getCir(1)->getPoList();
//    vector<CirPoGate*> po2 = cirMgr->getCir(2)->getPoList();

//    Var va, vb;
//    Lit la, lb, lh;
//    vec<Lit> lits;
//    vector<Var> vh;
//    // init var for cir1, cir 2 and vh
//    for (CirGate* g : dfs1) {
//       g->setVar(checker.newVar());
//    }
//    for (CirGate* g : dfs2) {
//       g->setVar(checker.newVar());
//    }
//    for (size_t i = 0 ; i < po2.size() ; i++) 
//       vh.push_back(checker.newVar());

//    // gen aig cnf for cir1 and cir 2
//    // cir 1
//    for (CirGate* g : dfs1) {
//       if (g->isAig()) {
//          checker.addAigCNF(g->getVar(), g->getIn0Gate()->getVar(), g->isIn0Inv(),
//                            g->getIn1Gate()->getVar(), g->isIn1Inv());
//       }
//       else if (g->isPo()) {
//          va = g->getVar();
//          vb = g->getIn0Gate()->getVar();
//          la = g->isIn0Inv() ? ~Lit(va) : Lit(va);
//          lb = Lit(vb);
//          lits.push(la); lits.push(~lb);
//          checker.addCNF(lits); lits.clear();
//          lits.push(~la); lits.push(lb);
//          checker.addCNF(lits); lits.clear();
//       }
//    }

//    // cir 2
//    for (CirGate* g : dfs2) {
//       if (g->isAig()) {
//          checker.addAigCNF(g->getVar(), g->getIn0Gate()->getVar(), g->isIn0Inv(),
//                            g->getIn1Gate()->getVar(), g->isIn1Inv());
//       }
//       else if (g->isPo()) {
//          va = g->getVar();
//          vb = g->getIn0Gate()->getVar();
//          la = g->isIn0Inv() ? ~Lit(va) : Lit(va);
//          lb = Lit(vb);
//          lits.push(la); lits.push(~lb);
//          checker.addCNF(lits); lits.clear();
//          lits.push(~la); lits.push(lb);
//          checker.addCNF(lits); lits.clear();
//       }
//    }

//    // add constraint from result mi/mo
//    // add pi constraint
//    for (size_t i = 0 ; i < pi2.size() ; i++) {
//       for (size_t j = 0 ; j < pi1.size() ; j++) {
//          // xi == yj -> (~xi+yj)(xi+~yj)
//          if (_resultMi[i][j*2]) {
//             la = ~Lit(pi2[i]->getVar());
//             lb = Lit(pi1[j]->getVar());
//             lits.push(la); lits.push(lb);
//             checker.addCNF(lits); lits.clear();
//             la = Lit(pi2[i]->getVar());
//             lb = ~Lit(pi1[j]->getVar());
//             lits.push(la); lits.push(lb);
//             checker.addCNF(lits); lits.clear();
//          }
//          // xi == ~yj -> (~xi+~yj)(xi+yj)
//          else if (_resultMi[i][j*2+1]) {
//             la = Lit(pi2[i]->getVar());
//             lb = Lit(pi1[j]->getVar());
//             lits.push(la); lits.push(lb);
//             checker.addCNF(lits); lits.clear();
//             la = ~Lit(pi2[i]->getVar());
//             lb = ~Lit(pi1[j]->getVar());
//             lits.push(la); lits.push(lb);
//             checker.addCNF(lits); lits.clear();
//          }
//       }
//    }
//    // add po constraint
//    for (size_t i = 0 ; i < po2.size() ; i++) {
//       for (size_t j = 0 ; j < po1.size() ; j++) {
//          // fi xor gj + vh -> (fi+gj+vh)(~fi+~gj+vh)
//          if (_resultMo[i][j*2]) {
//             la = Lit(po2[i]->getVar());
//             lb = Lit(po1[j]->getVar());
//             lh = Lit(vh[i]);
//             lits.push(la); lits.push(lb); lits.push(lh);
//             checker.addCNF(lits); lits.clear();
//             la = ~Lit(po2[i]->getVar());
//             lb = ~Lit(po1[j]->getVar());
//             lh = Lit(vh[i]);
//             lits.push(la); lits.push(lb); lits.push(lh);
//             checker.addCNF(lits); lits.clear();
//          }
//          // fi xor ~gj -> (fi+~gj)(~fi+gj)
//          else if (_resultMo[i][j*2+1]) {
//             la = Lit(po2[i]->getVar());
//             lb = ~Lit(po1[j]->getVar());
//             lh = Lit(vh[i]);
//             lits.push(la); lits.push(lb); lits.push(lh);
//             checker.addCNF(lits); lits.clear();
//             la = ~Lit(po2[i]->getVar());
//             lb = Lit(po1[j]->getVar());
//             lh = Lit(vh[i]);
//             lits.push(la); lits.push(lb); lits.push(lh);
//             checker.addCNF(lits); lits.clear();
//          }
//       }
//    }

//    for (size_t i = 0 ; i < po2.size() ; i++) {
//       checker.assumeRelease();
//       for (size_t j = 0 ; j < pi2.size() ; j++) {
//          if (_resultMi[j][pi1.size()*2]) {
//             checker.assumeProperty(pi2[j]->getVar(), false);
//          }
//          if (_resultMi[j][pi1.size()*2+1]) {
//             checker.assumeProperty(pi2[j]->getVar(), true);
//          }
//       }
//       for (size_t j = 0 ; j < po2.size() ; j++) {
//          if (j != i) checker.assertProperty(vh[j], true);
//          else checker.assumeProperty(vh[j], false);
//       }
//       bool isSat = checker.assumpSolve();
//       if (isSat) return false;
//    }

//    return true;
// }

// bool
// Match::partialSolvePoMatch(size_t f, size_t g, bool inv)
// {
//    SatSolver checker;
//    checker.initialize();
//    vector<CirGate*> dfs1 = cirMgr->getCir(1)->getDfsList();
//    vector<CirGate*> dfs2 = cirMgr->getCir(2)->getDfsList();
//    vector<CirPiGate*> pi1 = cirMgr->getCir(1)->getPiList();
//    vector<CirPiGate*> pi2 = cirMgr->getCir(2)->getPiList();
//    vector<CirPoGate*> po1 = cirMgr->getCir(1)->getPoList();
//    vector<CirPoGate*> po2 = cirMgr->getCir(2)->getPoList();
   

//    Var va, vb;
//    Lit la, lb;
//    vec<Lit> lits;

//    // init var for cir1, cir 2 and vh
//    for (CirGate* g : dfs1) {
//       g->setVar(checker.newVar());
//    }
//    for (CirGate* g : dfs2) {
//       g->setVar(checker.newVar());
//    }

//    // gen aig cnf for cir1 and cir 2
//    // cir 1
//    for (CirGate* g : dfs1) {
//       if (g->isAig()) {
//          checker.addAigCNF(g->getVar(), g->getIn0Gate()->getVar(), g->isIn0Inv(),
//                            g->getIn1Gate()->getVar(), g->isIn1Inv());
//       }
//       else if (g->isPo()) {
//          va = g->getVar();
//          vb = g->getIn0Gate()->getVar();
//          la = g->isIn0Inv() ? ~Lit(va) : Lit(va);
//          lb = Lit(vb);
//          lits.push(la); lits.push(~lb);
//          checker.addCNF(lits); lits.clear();
//          lits.push(~la); lits.push(lb);
//          checker.addCNF(lits); lits.clear();
//       }
//    }

//    // cir 2
//    for (CirGate* g : dfs2) {
//       if (g->isAig()) {
//          checker.addAigCNF(g->getVar(), g->getIn0Gate()->getVar(), g->isIn0Inv(),
//                            g->getIn1Gate()->getVar(), g->isIn1Inv());
//       }
//       else if (g->isPo()) {
//          va = g->getVar();
//          vb = g->getIn0Gate()->getVar();
//          la = g->isIn0Inv() ? ~Lit(va) : Lit(va);
//          lb = Lit(vb);
//          lits.push(la); lits.push(~lb);
//          checker.addCNF(lits); lits.clear();
//          lits.push(~la); lits.push(lb);
//          checker.addCNF(lits); lits.clear();
//       }
//    }

//    // output match
//    if (!inv) {
//       la = Lit(po2[g]->getVar());
//       lb = Lit(po1[f]->getVar());
//       lits.push(la); lits.push(lb);
//       checker.addCNF(lits); lits.clear();
//       la = ~Lit(po2[g]->getVar());
//       lb = ~Lit(po1[f]->getVar());
//       lits.push(la); lits.push(lb);
//       checker.addCNF(lits); lits.clear();
//    }
//    else {
//       la = Lit(po2[g]->getVar());
//       lb = ~Lit(po1[f]->getVar());
//       lits.push(la); lits.push(lb);
//       checker.addCNF(lits); lits.clear();
//       la = ~Lit(po2[g]->getVar());
//       lb = Lit(po1[f]->getVar());
//       lits.push(la); lits.push(lb);
//       checker.addCNF(lits); lits.clear();
//    }

//    return checker.solve();
// }

// void 
// Match::printMatch() const 
// {
//    vector<CirPiGate*> pi1 = cirMgr->getCir(1)->getPiList();
//    vector<CirPiGate*> pi2 = cirMgr->getCir(2)->getPiList();
//    vector<CirPoGate*> po1 = cirMgr->getCir(1)->getPoList();
//    vector<CirPoGate*> po2 = cirMgr->getCir(2)->getPoList();
//    // print po match
//    cout << "print po match\n";
//    for (size_t i = 0 ; i < po2.size() ; i++) {
//       for (size_t j = 0 ; j < po1.size() ; j++) {
//          if (_resultMo[i][2*j] || _resultMo[i][2*j+1]) {
//             cout << "(" << po1[j]->getName() << ", " << po2[i]->getName() << ")\n";
//          }
//       }
//    }
//    // print pi match
//    cout << "print pi match\n";
//    for (size_t i = 0 ; i < pi2.size() ; i++) {
//       for (size_t j = 0 ; j < pi1.size() ; j++) {
//          if (_resultMi[i][2*j] || _resultMi[i][2*j+1]) {
//             cout << "(" << pi1[j]->getName() << ", " << pi2[i]->getName() << ")\n";
//          }
//       }
//       if (_resultMi[i][pi1.size()*2] || _resultMi[i][pi1.size()*2+1]) {
//          cout << "(const, " << pi2[i]->getName() << ")\n"; 
//       }
//    }
// }