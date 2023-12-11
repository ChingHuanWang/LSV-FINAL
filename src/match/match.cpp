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

void Match::solve() {
   
   // output solver
   size_t poNum[2] = {cirMgr->getCir(1)->getPoNum(), cirMgr->getCir(2)->getPoNum()};
   size_t piNum[2] = {cirMgr->getCir(1)->getPiNum(), cirMgr->getCir(2)->getPiNum()};
   vector<vector<Var>> Mo(poNum[1], vector<Var>(poNum[0] * 2, 0));
   vector<vector<Var>> Mi(piNum[1], vector<Var>((piNum[0] + 1) * 2, 0));
   _K = poNum[1] + 1;
   _optimal = (_K + 1) * poNum[1];
   cout << "_K = " << _K << ", _optimal = " << _optimal << endl;

   for (size_t i = 0; i < Mo.size(); ++i) {
      for (size_t j = 0; j < Mo[i].size(); ++j) {
         Mo[i][j] = _outputSolver.newVar();
      }
   }

   for (size_t i = 0; i < Mi.size(); ++i) {
      for (size_t j = 0; j < Mi[i].size(); ++j) {
         Mi[i][j] = _outputSolver.newVar();
      }
   }

   for (size_t i = 0; i < Mo.size(); ++i) {
      _outputSolver.addAloCnf(Mo[i]);
      _outputSolver.addAmoCnf(Mo[i]);
   }

   for (size_t i = 0; i < Mi.size(); ++i) {
      _outputSolver.addAloCnf(Mi[i]);
      _outputSolver.addAmoCnf(Mi[i]);
   }

   // if (_outputSolver.solve()) {
   //    for (size_t j = 0; j < poNum[1]; ++j) {
   //       for (size_t i = 0; i < poNum[0] * 2; ++i) {
   //          printf("%5d ", _outputSolver.getValue(Mo[j][i]));
   //       }
   //       printf("\n");
   //    }
   // }
   // else {
   //    printf("unsat\n");
   // }

   // ================== input solver ==================
   vector<vector<Var>> mi(piNum[1], vector<Var>((piNum[0] + 1) * 2, 0));
   vector<vector<Var>> h(poNum[1], vector<Var>(poNum[0] * 2, 0));
   vector<vector<Var>> mo(poNum[1], vector<Var>(poNum[0] * 2, 0));

   // for (size_t i = 0; i < cirMgr->getCir(1)-> )
   size_t gateNum[2] = {cirMgr->getCir(1)->getGateNum(), cirMgr->getCir(2)->getGateNum()};
   for (size_t i = 0; i < 2; ++i) {
      for (size_t j = 0; j < gateNum[i]; ++j) {
         _inputSolver.newVar();
      }
   }

   for (size_t i = 0; i < mi.size(); ++i)
      for (size_t j = 0; j < mi[i].size(); ++j)
         mi[i][j] = _inputSolver.newVar();

   for (size_t i = 0; i < h.size(); ++i)
      for (size_t j = 0; j < h[i].size(); ++j)
         h[i][j] = _inputSolver.newVar();
      
   for (size_t i = 0; i < mo.size(); ++i)
      for (size_t j = 0; j < mo[i].size(); ++j)
         mo[i][j] = _inputSolver.newVar();

   for (size_t i = 0; i < mi.size(); ++i) {
      _inputSolver.addAloCnf(mi[i]);
      _inputSolver.addAmoCnf(mi[i]);
   }

   // circuit 1 AIG constraint
   vector<vector<CirPoGate*>> poList = {cirMgr->getCir(1)->getPoList(), cirMgr->getCir(2)->getPoList()};
   vector<vector<CirAigGate*>> aigList = {cirMgr->getCir(1)->getAigList(), cirMgr->getCir(2)->getAigList()};
   vec<Lit> lits;
   Lit lf, la, lb, lc;
   Var vf, va, vb, vc;
   size_t i0, i1;
   for (size_t i = 0; i < aigList[0].size(); ++i) {
      // aigList[0][i]->printGate();
      vf = aigList[0][i]->getId();
      i0 = aigList[0][i]->getIn0LitId();
      i1 = aigList[0][i]->getIn1LitId();
      va = i0 / 2; vb = i1 / 2;
      // cout << "vf = " << vf << ", va = " << va << ", inv_a = " << (i0 & 1) << ", vb = " << vb << ", inv_b = " << (i1 & 1) << endl;
      _inputSolver.addAigCNF(vf, va, i0 & 1, vb, i1 & 1);
   }

   for (size_t i = 0; i < poList[0].size(); ++i) {
      // poList[0][i]->printGate();
      vf = poList[0][i]->getId(); lf = Lit(vf);
      i0 = poList[0][i]->getIn0LitId();
      va = i0 / 2; la = (i0 & 1)? ~Lit(va):Lit(va);
      // cout << "vf = " << vf << ", va = " << va << ", inv_a = " << (i0 & 1) << endl;
      lits.push(lf); lits.push(~la);
      _inputSolver.addCNF(lits); lits.clear();
      lits.push(~lf); lits.push(la);
      _inputSolver.addCNF(lits); lits.clear();
   }

   // circuit 2 constraint
   for (size_t i = 0; i < aigList[1].size(); ++i) {
      // aigList[1][i]->printGate();
      vf = aigList[1][i]->getId() + gateNum[0];
      i0 = aigList[1][i]->getIn0LitId();
      i1 = aigList[1][i]->getIn1LitId();
      va = i0 / 2 + gateNum[0]; vb = i1 / 2 + gateNum[0];
      // cout << "vf = " << vf << ", va = " << va << ", inv_a = " << (i0 & 1) << ", vb = " << vb << ", inv_b = " << (i1 & 1) << endl;
      _inputSolver.addAigCNF(vf, va, i0 & 1, vb, i1 & 1);
   }

   for (size_t i = 0; i < poList[1].size(); ++i) {
      // poList[1][i]->printGate();
      vf = poList[1][i]->getId() + gateNum[0]; lf = Lit(vf);
      i0 = poList[1][i]->getIn0LitId();
      va = i0 / 2 + gateNum[0]; la = (i0 & 1)? ~Lit(va):Lit(va);
      // cout << "vf = " << vf << ", va = " << va << ", inv_a = " << (i0 & 1) << endl;
      lits.push(lf); lits.push(~la);
      _inputSolver.addCNF(lits); lits.clear();
      lits.push(~lf); lits.push(la);
      _inputSolver.addCNF(lits); lits.clear();
   }

   // x_i and y_j pair
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
   for (size_t j = 0; j < mo.size(); ++j) {
      for (size_t i = 0; i < mo[j].size(); ++i) {
         vf = mo[j][i]; lf = Lit(vf);
         va = h[j][i]; la = Lit(va);
         // cout << "vf = " << vf << ", va = " << va << endl;
         lits.push(lf); lits.push(~la);
         _inputSolver.addCNF(lits); lits.clear();
      }
   }

   for (size_t j = 0; j < h.size(); ++j) {
      for (size_t i = 0; i < h[j].size(); ++i) {
         vf = h[j][i]; lf = Lit(vf);
         lits.push(lf);
      }
   }
   _inputSolver.addCNF(lits); lits.clear();


   int optimal = 0, score;
   vector<vector<int>> resultMo(poNum[1], vector<int>(poNum[0], 0));
   vector<vector<int>> resultMi(piNum[1], vector<int>(piNum[0], 0));
   for (size_t x = 0; _outputSolver.solve(); ++x) {

      score = getScore(Mo);
      if (score <= optimal) continue;
      // cout << "x = " << x << ", optimal = " << optimal << endl;

      // cout << "output solver:\n";
      // for (size_t i = 0; i < Mi.size(); ++i) {
      //    for (size_t j = 0; j < Mi[i].size(); ++j) {
      //       cout << Mi[i][j] << ": " << _outputSolver.getValue(Mi[i][j]) << " ";
      //    }
      //    cout << endl;
      // }
      // for (size_t i = 0; i < Mo.size(); ++i) {
      //    for (size_t j = 0; j < Mo[i].size(); ++j) {
      //       cout << Mo[i][j] << ": " << _outputSolver.getValue(Mo[i][j]) << " ";
      //    }
      //    cout << endl;
      // }
      // cout << endl;

      _inputSolver.assumeRelease();
      for (size_t j = 0; j < mo.size(); ++j) {
         for (size_t i = 0; i < mo[j].size(); ++i) {
            vf = mo[j][i];
            _inputSolver.assumeProperty(vf, _outputSolver.getValue(Mo[j][i]));
         }
      }

      for (size_t j = 0; j < mi.size(); ++j) {
         for (size_t i = 0; i < mi[j].size(); ++i) {
            vf = mi[j][i];
            _inputSolver.assumeProperty(vf, _outputSolver.getValue(Mi[j][i]));
         }
      }

      if (!_inputSolver.assumpSolve()) {
         optimal = score;
         cout << "step:" << x << ", unsat, optimal = " << optimal << endl;
         for (size_t j = 0; j < Mo.size(); ++j) {
            for (size_t i = 0; i < Mo[j].size(); ++i) {
               resultMo[j][i] = _outputSolver.getValue(Mo[j][i]);
            }
         }

         for (size_t j = 0; j < Mi.size(); ++j) {
            for (size_t i = 0; i < Mi[j].size(); ++i) {
               resultMi[j][i] = _outputSolver.getValue(Mi[j][i]);
            }
         }
      }
      // else {
      //    cout << "input solver" << endl;
      //    int count = 0;
      //    for (size_t i = 0; i < 2; ++i) {
      //       for (size_t j = 0; j < gateNum[i]; ++j) {
      //          cout << (count + 1) << ": " << _inputSolver.getValue(++count) << " ";
      //       }
      //       cout << endl;
      //    }

      //    for (size_t i = 0; i < mi.size(); ++i) {
      //       for (size_t j = 0; j < mi[i].size(); ++j) {
      //          cout << mi[i][j] << ": " << _inputSolver.getValue(mi[i][j]) << " ";
      //       }
      //       cout << endl;
      //    }

      //    for (size_t i = 0; i < h.size(); ++i) {
      //       for (size_t j = 0; j < h[i].size(); ++j) {
      //          cout << h[i][j] << ": " << _inputSolver.getValue(h[i][j]) << " ";
      //       }
      //       cout << endl;
      //    }
            
      //    for (size_t i = 0; i < mo.size(); ++i){
      //       for (size_t j = 0; j < mo[i].size(); ++j){
      //          cout << mo[i][j] << ": " << _inputSolver.getValue(mo[i][j]) << " ";
      //       }
      //       cout << endl;
      //    }

      //    cout << endl;
      // }
      // for (size_t j = 0; j < poNum[1]; ++j) {
      //    for (size_t i = 0; i < poNum[0] * 2; ++i) {
      //       printf("%5d ", _outputSolver.getValue(Mo[j][i]));
      //    }
      //    printf("\n");
      // }
      // cout << endl;

      if (optimal == _optimal) {
         break;
      }
      // cout << "before" << endl;
      for (size_t j = 0; j < Mo.size(); ++j) {
         for (size_t i = 0; i < Mo[j].size(); ++i) {
            vf = Mo[j][i]; lf = (_outputSolver.getValue(Mo[j][i]))? ~Lit(vf):Lit(vf);
            lits.push(lf);
         }
      }
      for (size_t j = 0; j < Mi.size(); ++j) {
         for (size_t i = 0; i < Mi[j].size(); ++i) {
            vf = Mi[j][i]; lf = (_outputSolver.getValue(Mi[j][i]))? ~Lit(vf):Lit(vf);
            lits.push(lf);
         }
      }
      _outputSolver.addCNF(lits); lits.clear();
      // cout << "after" << endl;
   }

   cout << "optimal = " << optimal << endl;
   cout << "final mapping result:" << endl;
   cout << "output:" << endl;
   for (size_t j = 0; j < Mo.size(); ++j) {
      for (size_t i = 0; i < Mo[j].size(); ++i) {
         cout << resultMo[j][i] << " ";
      }
      cout << endl;
   }
   cout << endl;

   cout << "input:" << endl;
   for (size_t j = 0; j < Mi.size(); ++j) {
      for (size_t i = 0; i < Mi[j].size(); ++i) {
         cout << resultMi[j][i] << " ";
      }
      cout << endl;
   }
   cout << endl;
   

}