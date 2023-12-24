#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include "match.h"
#include "genAag.h"
#include "cirMgr.h"
#include "cirGate.h"
#include "SolverTypes.h"
#include "util.h"


void Match::parseInput(char* inFilePath, vector<string>& cirFileList)
{
   ifstream inFile(inFilePath);
   string filePath(inFilePath), prefix;
   prefix = getDirName(filePath);

   string cirFilePath, tmp, busName;
   stringstream ss("");
   vector<string> abus;
   vector<vector<string>> buses;
   int M, busNum;
   // circuit 1
   getline(inFile, cirFilePath);
   getline(inFile, tmp);
   M = stoi(tmp);
   cirFileList.push_back(genAagFile(prefix, cirFilePath));
   for (int i = 0 ; i < M ; i++) {
      getline(inFile, tmp);
      // todo: how to store the bus information?
      ss << tmp;
      ss >> busNum;
      for (int j = 0; j < busNum; ++j) {
         ss >> busName;
         abus.push_back(busName);
      }
      buses.push_back(abus);
      abus.clear();
      ss.clear();
      ss.str("");
   }
   _bus.push_back(buses);
   buses.clear();

   // circuit 2
   getline(inFile, cirFilePath);
   getline(inFile, tmp);
   M = stoi(tmp);
   cirFileList.push_back(genAagFile(prefix, cirFilePath));
   for (int i = 0 ; i < M ; i++) {
      getline(inFile, tmp);
      // todo: how to store the bus information?
      ss << tmp;
      ss >> busNum;
      for (int j = 0; j < busNum; ++j) {
         ss >> busName;
         abus.push_back(busName);
      }
      buses.push_back(abus);
      abus.clear();
      ss.clear();
      ss.str("");
   }
   _bus.push_back(buses);
   buses.clear();

   // ====== check buses are stored correctly
   // for (size_t i = 0; i < _bus.size(); ++i) {
   //    for (size_t j = 0; j < _bus[i].size(); ++j) {
   //       for (size_t k = 0; k < _bus[i][j].size(); ++k) {
   //          printf("%s ", _bus[i][j][k].c_str());
   //       }
   //       printf("\n");
   //    }
   //    printf("\n");
   // }
   // ====== check buses are stored correctly

}

void Match::parseBus() {
   // for (size_t i = 0; i < _bus.size(); ++i) {
   //    for (size_t j = 0; j < _bus[i].size(); ++j) {
   //       for (size_t k = 0; k < _bus[i][j].size(); ++k) {

   //       }
   //    }
   // }
}

int Match::getScore(const vector<vector<Var>>& sol) {

   int score = 0, tmp;
   for (size_t i = 0; i < sol[0].size(); i += 2) {
      tmp = 0;
      for (size_t j = 0; j < sol.size(); ++j) {
         tmp += _outputSolver.getValue(sol[j][i]) + _outputSolver.getValue(sol[j][i + 1]);
      }
      score += (tmp > 0)? _K + tmp:tmp;
   }
   return score;
}

void Match::outputSolverInit(vector<vector<Var>>& Mo, vector<vector<Var>>& Mi, Var& allowProj) {

   size_t poNum[2] = {cirMgr->getCir(1)->getPoNum(), cirMgr->getCir(2)->getPoNum()};
   size_t piNum[2] = {cirMgr->getCir(1)->getPiNum(), cirMgr->getCir(2)->getPiNum()};
   size_t gateNum[2] = {cirMgr->getCir(1)->getGateNum(), cirMgr->getCir(2)->getGateNum()};


   vector<vector<bool>> funcSupp1(poNum[0], vector<bool>(piNum[0], false));
   vector<vector<bool>> funcSupp2(poNum[1], vector<bool>(piNum[1], false));
   vector<vector<int>> funcSuppSize(2, vector<int>(poNum[0], 0));
   int count;

   vec<Lit> lits;
   Lit lf, la, lb;
   Var vf, va, vb;
   size_t i0, i1;

   cirMgr->getCir(1)->collectFuncSupp();
   cirMgr->getCir(2)->collectFuncSupp();
   cirMgr->getCir(1)->printFuncSupp();
   cirMgr->getCir(2)->printFuncSupp();
   vector<vector<vector<size_t>>> funcSupp = {cirMgr->getCir(1)->getFuncSupp(), cirMgr->getCir(2)->getFuncSupp()};

   // ==================== output solver variable ====================
   // 1. output mapping matrix
   // 2. input mapping matrix
   // 3. allow projection variable
   for (size_t i = 0; i < Mi.size(); ++i) {
      for (size_t j = 0; j < Mi[i].size(); ++j) {
         Mi[i][j] = _outputSolver.newVar();
      }
   }

   for (size_t i = 0; i < Mo.size(); ++i) {
      for (size_t j = 0; j < Mo[i].size(); ++j) {
         Mo[i][j] = _outputSolver.newVar();
      }
   }
   allowProj = _outputSolver.newVar();
   // ==================== output solver variable ====================

   // ==================== output solver constraint ====================

   // sum of every row = 1, because problem demand that PI/PO of cir2 need to be matched 
   for (size_t i = 0; i < Mo.size(); ++i) {
      _outputSolver.addAloCnf(Mo[i]);
      // _outputSolver.addAmoCnf(Mo[i]);
   }
   // for (size_t i = 0; i < Mo.size(); ++i) {
   //    for (size_t j = 0; j < Mo[i].size(); ++j) {
   //       vf = Mo[i][j]; lf = Lit(vf); lits.push(lf);
   //    }
   // }
   // _outputSolver.addCNF(lits); lits.clear();

   for (size_t i = 0; i < Mi.size(); ++i) {
      _outputSolver.addAloCnf(Mi[i]);
      _outputSolver.addAmoCnf(Mi[i]);
   }

   // every two column of the Mo matrix
   // (1) every two entry of C/D coulmn can not both be 1
   // (2) ci and dj can not both be 1  
   for (size_t i = 0; i < Mo[0].size(); i += 2) {
      for (size_t j = 0; j < Mo.size() - 1; ++j) {
         for (size_t k = j + 1; k < Mo.size(); ++k) {
            vf = allowProj; lf = Lit(vf);
            va = Mo[j][i]; la = Lit(va);
            vb = Mo[k][i]; lb = Lit(vb);
            lits.push(lf); lits.push(~la); lits.push(~lb);
            _outputSolver.addCNF(lits); lits.clear();
         }
      }

      for (size_t j = 0; j < Mo.size(); ++j) {
         for (size_t k = 0; k < Mo.size(); ++k) {
            vf = allowProj; lf = Lit(vf);
            va = Mo[j][i]; la = Lit(va);
            vb = Mo[k][i + 1]; lb = Lit(vb);
            lits.push(lf); lits.push(~la); lits.push(~lb);
            _outputSolver.addCNF(lits); lits.clear();
         }
      }

      for (size_t j = 0; j < Mo.size() - 1; ++j) {
         for (size_t k = j + 1; k < Mo.size(); ++k) {
            vf = allowProj; lf = Lit(vf);
            va = Mo[j][i + 1]; la = Lit(va);
            vb = Mo[k][i + 1]; lb = Lit(vb);
            lits.push(lf); lits.push(~la); lits.push(~lb);
            _outputSolver.addCNF(lits); lits.clear();
         }
      }
   }


   // ===== functional support constraint =====
   // if |FuncSupp(f_i)| > |FuncSupp(g_j)|, then f_i cannot map to g_j
   // else { if x_i in FuncSupp(f_i), then x_i is mapped to y_j in FuncSupp(g_j) }
   for (size_t j = 0; j < poNum[1]; ++j) {
      for (size_t i = 0; i < poNum[0]; ++i) {
         if (funcSupp[0][i].size() > funcSupp[1][j].size()) {
            vf = Mo[j][i * 2]; lf = ~Lit(vf); lits.push(lf);
            _outputSolver.addCNF(lits); lits.clear();
            vf = Mo[j][i * 2 + 1]; lf = ~Lit(vf); lits.push(lf);
            _outputSolver.addCNF(lits); lits.clear();
         }
         // else {
         //    for (size_t k = 0; k < funcSupp1[i].size(); ++k) {
         //       if (!funcSupp1[i][k]) continue;
         //       for (size_t l = 0; l < funcSupp2[j].size(); ++l) {
         //          if (!funcSupp2[j][l]) continue;
         //          va = Mi[l][k * 2]; la = Lit(va); lits.push(la);
         //          vb = Mi[l][k * 2 + 1]; lb = Lit(vb); lits.push(lb);
         //       }
         //       vf = Mo[j][i * 2]; lf = Lit(vf); lits.push(~lf);
         //       _outputSolver.addCNF(lits); lits.pop();
         //       vf = Mo[j][i * 2 + 1]; lf = Lit(vf); lits.push(~lf);
         //       _outputSolver.addCNF(lits); lits.clear();
         //    }
         // }
      }
   }
   // ===== functional support constraint =====

   // ==================== output solver constraint ====================
}

void Match::inputSolverInit(vector<vector<Var>>& mi, vector<vector<Var>>& h, vector<vector<Var>>& mo) {


   vec<Lit> lits;
   Lit lf, la, lb, lc;
   Var vf, va, vb, vc;
   size_t i0, i1;
   size_t poNum[2] = {cirMgr->getCir(1)->getPoNum(), cirMgr->getCir(2)->getPoNum()};
   size_t piNum[2] = {cirMgr->getCir(1)->getPiNum(), cirMgr->getCir(2)->getPiNum()};
   size_t gateNum[2] = {cirMgr->getCir(1)->getGateNum(), cirMgr->getCir(2)->getGateNum()};
   vector<vector<CirPoGate*>> poList = {cirMgr->getCir(1)->getPoList(), cirMgr->getCir(2)->getPoList()};
   vector<vector<CirAigGate*>> aigList = {cirMgr->getCir(1)->getAigList(), cirMgr->getCir(2)->getAigList()};


   // ================== input solver variable ==================
   // 1. aig constraint
   // 2. input mapping matrix
   // 3. intermediate variable h
   // 4. output mapping matrix
   // recieve output solver solution
   _inputSolver.addCirCNF(cirMgr->getCir(1), 0);
   _inputSolver.addCirCNF(cirMgr->getCir(2), gateNum[0]);

   for (size_t i = 0; i < mi.size(); ++i)
      for (size_t j = 0; j < mi[i].size(); ++j)
         mi[i][j] = _inputSolver.newVar();

   for (size_t i = 0; i < h.size(); ++i)
      for (size_t j = 0; j < h[i].size(); ++j)
         h[i][j] = _inputSolver.newVar();
      
   for (size_t i = 0; i < mo.size(); ++i)
      for (size_t j = 0; j < mo[i].size(); ++j)
         mo[i][j] = _inputSolver.newVar();
   // ================== input solver variable ==================

   // ================== input solver constraint ==================

   // circuit 1 AIG constraint
   // for (size_t i = 0; i < aigList[0].size(); ++i) {
   //    // aigList[0][i]->printGate();
   //    vf = aigList[0][i]->getId();
   //    i0 = aigList[0][i]->getIn0LitId();
   //    i1 = aigList[0][i]->getIn1LitId();
   //    va = i0 / 2; vb = i1 / 2;
   //    // cout << "vf = " << vf << ", va = " << va << ", inv_a = " << (i0 & 1) << ", vb = " << vb << ", inv_b = " << (i1 & 1) << endl;
   //    _inputSolver.addAigCNF(vf, va, i0 & 1, vb, i1 & 1);
   // }

   // for (size_t i = 0; i < poList[0].size(); ++i) {
   //    // poList[0][i]->printGate();
   //    vf = poList[0][i]->getId(); lf = Lit(vf);
   //    i0 = poList[0][i]->getIn0LitId();
   //    va = i0 / 2; la = (i0 & 1)? ~Lit(va):Lit(va);
   //    // cout << "vf = " << vf << ", va = " << va << ", inv_a = " << (i0 & 1) << endl;
   //    lits.push(lf); lits.push(~la);
   //    _inputSolver.addCNF(lits); lits.clear();
   //    lits.push(~lf); lits.push(la);
   //    _inputSolver.addCNF(lits); lits.clear();
   // }

   // circuit 2 constraint
   // for (size_t i = 0; i < aigList[1].size(); ++i) {
   //    // aigList[1][i]->printGate();
   //    vf = aigList[1][i]->getId() + gateNum[0];
   //    i0 = aigList[1][i]->getIn0LitId();
   //    i1 = aigList[1][i]->getIn1LitId();
   //    va = i0 / 2 + gateNum[0]; vb = i1 / 2 + gateNum[0];
   //    // cout << "vf = " << vf << ", va = " << va << ", inv_a = " << (i0 & 1) << ", vb = " << vb << ", inv_b = " << (i1 & 1) << endl;
   //    _inputSolver.addAigCNF(vf, va, i0 & 1, vb, i1 & 1);
   // }

   // for (size_t i = 0; i < poList[1].size(); ++i) {
   //    // poList[1][i]->printGate();
   //    vf = poList[1][i]->getId() + gateNum[0]; lf = Lit(vf);
   //    i0 = poList[1][i]->getIn0LitId();
   //    va = i0 / 2 + gateNum[0]; la = (i0 & 1)? ~Lit(va):Lit(va);
   //    // cout << "vf = " << vf << ", va = " << va << ", inv_a = " << (i0 & 1) << endl;
   //    lits.push(lf); lits.push(~la);
   //    _inputSolver.addCNF(lits); lits.clear();
   //    lits.push(~lf); lits.push(la);
   //    _inputSolver.addCNF(lits); lits.clear();
   // }

   // x_i and y_j pair
   // aij -> xi == yj
   for (size_t j = 0; j < mi.size(); ++j) {
      for (size_t i = 0; i < piNum[0] * 2; ++i) {
         vf = mi[j][i]; lf = Lit(vf);
         va = i / 2 + 1; la = (i & 1)? ~Lit(va):Lit(va);
         vb = j + 1 + gateNum[0]; lb = Lit(vb);
         // cout << "vf = " << vf << ", va = " << va << ", vb = " << vb << endl;
         lits.push(~lf); lits.push(~la); lits.push(lb);
         _inputSolver.addCNF(lits); lits.clear();
         lits.push(~lf); lits.push(la); lits.push(~lb);
         _inputSolver.addCNF(lits); lits.clear();
      }
   }

   // 0 or 1 and y_j pair
   // a_1_m_I+1 -> 0 == yj
   for (size_t j = 0; j < mi.size(); ++j) {
      for (size_t i = piNum[0] * 2; i < mi[j].size(); ++i) {
         vf = mi[j][i]; lf = Lit(vf);
         va = j + 1 + gateNum[0]; la = (i & 1)? ~Lit(va):Lit(va);
         // cout << "vf = " << vf << ", va = " << va << endl;
         lits.push(~lf); lits.push(~la);
         _inputSolver.addCNF(lits); lits.clear();
      }
   }

   // h
   // c11 -> h11 == (f1 XOR g1)
   // exclude the case of cij = 0
   for (size_t j = 0; j < mo.size(); ++j) {
      for (size_t i = 0; i < mo[j].size(); ++i) {
         vf = mo[j][i]; lf = Lit(vf);
         va = h[j][i]; la = Lit(va);
         vb = poList[0][i/2]->getId(); lb = (i & 1)? ~Lit(vb):Lit(vb);
         vc = poList[1][j]->getId() + gateNum[0]; lc = Lit(vc);
         // cout << "vf = " << vf << ", va = " << va << ", vb = " << vb << ", vc = " << vc << endl;
         lits.push(~lf); lits.push(~la); lits.push(lb); lits.push(lc);
         _inputSolver.addCNF(lits); lits.clear();
         lits.push(~lf); lits.push(~la); lits.push(~lb); lits.push(~lc);
         _inputSolver.addCNF(lits); lits.clear();
         lits.push(~lf); lits.push(la); lits.push(lb); lits.push(~lc);
         _inputSolver.addCNF(lits); lits.clear();
         lits.push(~lf); lits.push(la); lits.push(~lb); lits.push(lc);
         _inputSolver.addCNF(lits); lits.clear();
      }
   }

   // c, d and h
   // ~c11 -> ~h11
   for (size_t j = 0; j < mo.size(); ++j) {
      for (size_t i = 0; i < mo[j].size(); ++i) {
         vf = mo[j][i]; lf = Lit(vf);
         va = h[j][i]; la = Lit(va);
         // cout << "vf = " << vf << ", va = " << va << endl;
         lits.push(lf); lits.push(~la);
         _inputSolver.addCNF(lits); lits.clear();
      }
   }

   // sum hij
   for (size_t j = 0; j < h.size(); ++j) {
      for (size_t i = 0; i < h[j].size(); ++i) {
         vf = h[j][i]; lf = Lit(vf);
         lits.push(lf);
      }
   }
   _inputSolver.addCNF(lits); lits.clear();
   // ================== input solver constraint ==================
}

void Match::solve() {
   
   // output solver
   size_t poNum[2] = {cirMgr->getCir(1)->getPoNum(), cirMgr->getCir(2)->getPoNum()};
   size_t piNum[2] = {cirMgr->getCir(1)->getPiNum(), cirMgr->getCir(2)->getPiNum()};
   size_t gateNum[2] = {cirMgr->getCir(1)->getGateNum(), cirMgr->getCir(2)->getGateNum()};
   vector<vector<CirPoGate*>> poList = {cirMgr->getCir(1)->getPoList(), cirMgr->getCir(2)->getPoList()};
   vector<vector<CirAigGate*>> aigList = {cirMgr->getCir(1)->getAigList(), cirMgr->getCir(2)->getAigList()};

   vector<vector<Var>> outputMo(poNum[1], vector<Var>(poNum[0] * 2, 0));
   vector<vector<Var>> outputMi(piNum[1], vector<Var>((piNum[0] + 1) * 2, 0));
   Var allowProj;

   vector<vector<Var>> inputMi(piNum[1], vector<Var>((piNum[0] + 1) * 2, 0));
   vector<vector<Var>> inputH(poNum[1], vector<Var>(poNum[0] * 2, 0));
   vector<vector<Var>> inputMo(poNum[1], vector<Var>(poNum[0] * 2, 0));

   // vector<int> fi = {9, 1, 11, 7, 8, 6, 5, 10, 4, 3, 2, 12};
   // vector<int> gi = {8, 5, 12, 11, 2, 1, 6, 7, 10, 9, 3, 4};
   // vector<int> fo = {0, 1, 2, 3};
   // vector<int> go = {0, 3, 1, 2};
   // vector<vector<Var>> answerMo(poNum[1], vector<Var>(poNum[0] * 2, 0));
   // vector<vector<Var>> answerMi(piNum[1], vector<Var>((piNum[0] + 1) * 2, 0));
   // for (size_t i = 0; i < answerMo.size(); ++i) {
   //    answerMo[go[i]][fo[i] * 2] = 1;
   // }
   // for (size_t i = 0; i < answerMi.size(); ++i) {
   //    answerMi[gi[i] - 1][(fi[i] - 1) * 2] = 1;
   // }

   vec<Lit> lits;
   Lit lf, la, lb, lc;
   Var vf, va, vb, vc;
   size_t i0, i1;

   outputSolverInit(outputMo, outputMi, allowProj);
   inputSolverInit(inputMi, inputH, inputMo);

   _K = poNum[1] + 1;
   _optimal = (_K + 1) * poNum[1];
   cout << "_K = " << _K << ", _optimal = " << _optimal << endl;
   // cout << "gateNum: " <<  gateNum[0] << " " << gateNum[1] << endl;

   // ================== solve ==================
   int optimal = 0, score;
   bool proj = false;
   char inputcheck;
   vector<size_t> counterExampleIn, counterExampleOut;
   vector<vector<bool>> red1, red2;
   _outputSolver.assumeRelease();
   _outputSolver.assumeProperty(allowProj, proj);

   _resultMo.resize(poNum[1], vector<int>(poNum[0] * 2, 0));
   _resultMi.resize(piNum[1], vector<int>((piNum[0] + 1) * 2, 0));
   for (size_t x = 0; optimal != _optimal; ++x) {

      if (_outputSolver.assumpSolve()) score = getScore(outputMo);
      else {
         cout << "unsat" << endl;
         cout << score << endl;
         if (!proj) {
            cout << "allow projection\n";
            cout << "paused to check: If there is no problem, please press Enter:\n";
            getchar();
            proj = true;
            _outputSolver.assumeRelease();
            _outputSolver.assumeProperty(allowProj, proj);
            continue;
         }
         else break;
      }

      cout << "x = " << x << ", optimal = " << optimal << ", score = " << score << endl;
      if (score <= optimal) continue;

      // printOutputSolverValue(outputMi, outputMi, allowProj);

      // ==================== add input solver assumption ====================

      // if output solver sat, return the value of MI and MO for mi and mo use
      _inputSolver.assumeRelease();
      for (size_t j = 0; j < outputMo.size(); ++j) {
         for (size_t i = 0; i < outputMo[j].size(); ++i) {
            vf = inputMo[j][i];
            _inputSolver.assumeProperty(vf, _outputSolver.getValue(outputMo[j][i]));
         }
      }
      for (size_t j = 0; j < inputMi.size(); ++j) {
         for (size_t i = 0; i < inputMi[j].size(); ++i) {
            vf = inputMi[j][i];
            _inputSolver.assumeProperty(vf, _outputSolver.getValue(outputMi[j][i]));
         }
      }
      // ==================== add input solver assumption ====================

      // if input solver unsat -> no counter example -> record the sol of MI/MO as final answer
      if (!_inputSolver.assumpSolve()) {
         optimal = score;
         cout << "step:" << x << ", unsat, optimal = " << optimal << endl;
         for (size_t j = 0; j < outputMo.size(); ++j) {
            for (size_t i = 0; i < outputMo[j].size(); ++i) {
               _resultMo[j][i] = _outputSolver.getValue(outputMo[j][i]);
            }
         }

         for (size_t j = 0; j < outputMi.size(); ++j) {
            for (size_t i = 0; i < outputMi[j].size(); ++i) {
               _resultMi[j][i] = _outputSolver.getValue(outputMi[j][i]);
            }
         }
         continue;
      }
      // else printInputSolverValue(inputMi, inputH, inputMo);
      
      // calculate the redundant var, if x3 is redunda, then x3 is recorded in red
      counterExampleIn.resize(piNum[0], 0); counterExampleOut.resize(poNum[0], 0);
      for (size_t i = 0; i < piNum[0]; ++i)
         counterExampleIn[i] = (size_t)_inputSolver.getValue(i + 1);
      for (size_t i = 0; i < poNum[0]; ++i)
         counterExampleOut[i] = (size_t)_inputSolver.getValue(gateNum[0] - poNum[0] + i + 1);
      cirMgr->getCir(1)->getRedundant(counterExampleIn, counterExampleOut, red1);

      counterExampleIn.resize(piNum[1], 0); counterExampleOut.resize(poNum[1], 0);
      for (size_t i = 0; i < piNum[1]; ++i)
         counterExampleIn[i] = (size_t)_inputSolver.getValue(i + 1 + gateNum[0]);
      for (size_t i = 0; i < poNum[1]; ++i)
         counterExampleOut[i] = (size_t)_inputSolver.getValue(gateNum[0] + gateNum[1] - poNum[1] + i + 1);
      cirMgr->getCir(2)->getRedundant(counterExampleIn, counterExampleOut, red2);

      // cout << "==================== add learned clause ====================" << endl;
      // refer to strenthen learning ex5
      for (size_t q = 0; q < poList[1].size(); ++q) {
         for (size_t p = 0; p < poList[0].size(); ++p) {
            // cout << "p = " << p << ", q = " << q << endl;
            vf = (_inputSolver.getValue(poList[0][p]->getId()) != _inputSolver.getValue(poList[1][q]->getId() + gateNum[0]))? outputMo[q][2 * p]:outputMo[q][2 * p + 1];
            lf = Lit(vf); lits.push(~lf);
            // cout << "literal: " << vf;
            for (size_t j = 0; j < piNum[1]; ++j) {
               for (size_t i = 0; i < piNum[0]; ++i) {
                  if (red1[p][i] || red2[q][j]) continue;
                  // cout << "i = " << i << ", j = " << j << endl;
                  va = (_inputSolver.getValue(i + 1) != _inputSolver.getValue(j + 1 + gateNum[0]))? outputMi[j][2 * i]:outputMi[j][2 * i + 1];
                  la = Lit(va); lits.push(la);
                  // cout << " + " << va;
               }
            }
            for (size_t i = 0; i < piNum[1]; ++i) {
               if (red2[q][i]) continue;

               va = (_inputSolver.getValue(i + 1 + gateNum[0]))? outputMi[i][2 * piNum[0]]:outputMi[i][2 * piNum[0] + 1];
               la = Lit(va); lits.push(la);
               // cout << " + " << va;
            }
            // cout << endl;
            _outputSolver.addCNF(lits); lits.clear();
         }
      }

      // add clause to prevent to use the current MI/MO
      for (size_t j = 0; j < outputMo.size(); ++j) {
         for (size_t i = 0; i < outputMo[j].size(); ++i) {
            vf = outputMo[j][i]; lf = (_outputSolver.getValue(outputMo[j][i]))? ~Lit(vf):Lit(vf);
            lits.push(lf);
         }
      }
      for (size_t j = 0; j < outputMi.size(); ++j) {
         for (size_t i = 0; i < outputMi[j].size(); ++i) {
            vf = outputMi[j][i]; lf = (_outputSolver.getValue(outputMi[j][i]))? ~Lit(vf):Lit(vf);
            lits.push(lf);
         }
      }
      _outputSolver.addCNF(lits); lits.clear();
      // ==============================================
      // _outputSolver.assumeRelease();
      // for (size_t j = 0; j < outputMo.size(); ++j) {
      //    for (size_t i = 0; i < outputMo[j].size(); ++i) {
      //       vf = outputMo[j][i];
      //       _outputSolver.assumeProperty(vf, answerMo[j][i]);
      //    }
      // }
      // for (size_t j = 0; j < outputMi.size(); ++j) {
      //    for (size_t i = 0; i < outputMi[j].size(); ++i) {
      //       vf = outputMi[j][i];
      //       _outputSolver.assumeProperty(vf, answerMi[j][i]);
      //    }
      // }
      // if (!_outputSolver.assumpSolve()) {cout << "There is some problem." << endl; getchar();}
      // _outputSolver.assumeRelease();
      // _outputSolver.assumeProperty(allowProj, proj);
      // getchar();
      // cout << "==================== add learned clause ====================" << endl;
   }
   // ================== solve ==================

   write();

}

void 
Match::printMatchedMiInvFuncSupp() const
{
   vector<CirPiGate*> cir1Pi = cirMgr->getCir(1)->getPiList();
   vector<CirPiGate*> cir2Pi = cirMgr->getCir(2)->getPiList();
   vector<vector<size_t>> cir1Inv = cirMgr->getCir(1)->getInvFuncSupp();
   vector<vector<size_t>> cir2Inv = cirMgr->getCir(2)->getInvFuncSupp();

   for (size_t i = 0 ; i < cir2Pi.size() ; i++) {
      for (size_t j = 0 ; j < cir1Pi.size() ; j++) {
         if (_resultMi[i][j*2] || _resultMi[i][j*2+1]) {
            cout << "(" << cir2Pi[i]->getName() << "," << cir1Pi[j]->getName()
                 << " : " << cir2Inv[i].size() << "," << cir1Inv[j].size() << ")" << endl;
         }
      }
   }
}

void Match::printOutputSolverValue(vector<vector<Var>>& Mi, vector<vector<Var>>& Mo, Var& allowProj) {
   cout << "==================== variable of output solver ====================" << endl;
      cout << "input mapping matrix:\n";
      for (size_t i = 0; i < Mi.size(); ++i) {
         for (size_t j = 0; j < Mi[i].size(); ++j) {
            cout << _outputSolver.getValue(Mi[i][j]) << " ";
         }
         cout << endl;
      }
      cout << "output mapping matrix:\n";
      for (size_t i = 0; i < Mo.size(); ++i) {
         for (size_t j = 0; j < Mo[i].size(); ++j) {
            cout << _outputSolver.getValue(Mo[i][j]) << " ";
         }
         cout << endl;
      }
      cout << "allowProj: " << _outputSolver.getValue(allowProj) << endl;
   cout << "==================== variable of output solver ====================" << endl;
}

void Match::printInputSolverValue(vector<vector<Var>>& Mi, vector<vector<Var>>& H, vector<vector<Var>>& Mo) {

   size_t gateNum[2] = {cirMgr->getCir(1)->getGateNum(), cirMgr->getCir(2)->getGateNum()};

   cout << "==================== variable of input solver ====================" << endl;
   cout << "AIG" << endl;
   int count = 0;
   for (size_t i = 0; i < 2; ++i) {
      for (size_t j = 0; j < gateNum[i]; ++j) {
         // cout << (count + 1) << ": " << _inputSolver.getValue(++count) << " ";
         cout << _inputSolver.getValue(++count) << " ";
      }
      cout << endl;
   }
   cout << "Mi" << endl;
   for (size_t i = 0; i < Mi.size(); ++i) {
      for (size_t j = 0; j < Mi[i].size(); ++j) {
         cout << _inputSolver.getValue(Mi[i][j]) << " ";
      }
      cout << endl;
   }

   cout << "H" << endl;
   for (size_t i = 0; i < H.size(); ++i) {
      for (size_t j = 0; j < H[i].size(); ++j) {
         cout << _inputSolver.getValue(H[i][j]) << " ";
      }
      cout << endl;
   }
   cout << "Mo:\n";
   for (size_t i = 0; i < Mo.size(); ++i){
      for (size_t j = 0; j < Mo[i].size(); ++j){
         cout << _inputSolver.getValue(Mo[i][j]) << " ";
      }
      cout << endl;
   }
   cout << "==================== variable of input solver ====================" << endl;
}

void Match::write() {

   cout << "optimal = " << _optimal << endl;
   cout << "final mapping result:" << endl;
   cout << "output:" << endl;
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