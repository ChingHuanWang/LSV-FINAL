#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include "match.h"
#include "genAag.h"
#include "cirMgr.h"
#include "SolverTypes.h"


void Match::parseInput(char* inFilePath, vector<string>& cirFileList)
{
   ifstream inFile(inFilePath);
   string cirFilePath, tmp, busName;
   stringstream ss("");
   vector<string> abus;
   vector<vector<string>> buses;
   int M, busNum;
   // circuit 1
   getline(inFile, cirFilePath);
   getline(inFile, tmp);
   M = stoi(tmp);
   cirFileList.push_back(genAagFile(cirFilePath));
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
   cirFileList.push_back(genAagFile(cirFilePath));
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

void Match::solve() {
   

   // output solver
   size_t poNum[2] = {cirMgr->getCir(1)->getPoNum(), cirMgr->getCir(2)->getPoNum()};
   vector<vector<Var>> Mo(poNum[1], vector<Var>(poNum[0] * 2, 0));

   for (size_t i = 0; i < poNum[0] * 2; ++i) {
      for (size_t j = 0; j < poNum[1]; ++j) {
         Mo[j][i] = _outputSolver->newVar();
      }
   }

   for (size_t i = 0; i < poNum[1]; ++i) {
      _outputSolver->addAloCnf(Mo[i]);
      _outputSolver->addAmoCnf(Mo[i]);
   }

   if (_outputSolver->solve()) {
      for (size_t i = 0; i < poNum[0] * 2; ++i) {
         for (size_t j = 0; j < poNum[1]; ++j) {
            printf("%5d ", Mo[j][i]);
         }
         printf("\n");
      }
   }

}