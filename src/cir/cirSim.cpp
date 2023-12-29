/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir simulation functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Keep "CirMgr::randimSim()" and "CirMgr::fileSim()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/************************************************/
/*   Public member functions about Simulation   */
/************************************************/
void 
CirPiGate::simulate(const CirSimV& simV)
{
   CirGate::setToGlobalRef();
   _sim0 = simV;
   for (CirGateV gateV : _foutList) 
      gateV.getGate()->simulate(gateV.isInv() ? ~simV : simV);

} 


void
CirPoGate::simulate(const CirSimV& simV)
{
   CirGate::setToGlobalRef();
   _sim0 = simV;
}



void 
CirAigGate::simulate(const CirSimV& simV)
{
   if (!CirGate::isGlobalRef()) {
      CirGate::setToGlobalRef();
      _sim0 = simV; return;
   }

   _sim1 = simV;
   for (CirGateV gateV : _foutList) 
      gateV.getGate()->simulate(gateV.isInv() ? ~(_sim0 & _sim1) : (_sim0 & _sim1));
}


void
CirObj::simulate()
{
   // for (size_t i = 0; i < _piList.size(); i++) 
   //    _piList[i]->simulate(word[i]);
      
   
   // for (CirGate* gate : _dfsList) {
   //    if (gate->isPi()) {
   //       gate->setSim0(word[gate->getId()-1]);
   //       gate->simulate();
   //    }
   //    else 
   //       gate->simulate();
   // }
   
   for (CirGate* gate : _dfsList) {
      if (gate->isAig()) {
         gate->printGate();
         gate->setSim0(gate->isIn0Inv() ? ~gate->getIn0Gate()->getSimResult() : gate->getIn0Gate()->getSimResult());
         gate->setSim1(gate->isIn1Inv() ? ~gate->getIn1Gate()->getSimResult() : gate->getIn1Gate()->getSimResult());
         // gate->simulate();
      }
      else if (gate->isPo()) {
         gate->printGate();
         gate->setSim0(gate->isIn0Inv() ? ~gate->getIn0Gate()->getSimResult() : gate->getIn0Gate()->getSimResult());
      }
      // else if (gate->isPi()) {
      //    gate->setSim0(word[gate->getId() - 1]);
      // }
   }
}

void 
CirObj::writeLog(size_t wordLen)
{
   
   for (size_t i = 0; i < wordLen; i++) {
      for (size_t j = 0; j < _piList.size(); j++) 
         *_simLog << _piList[j]->getSimBit(i);
      
      *_simLog << " ";

      for (CirPoGate* gate: _poList)
         *_simLog << gate->getSimBit(i);

      *_simLog << endl;
   }
}


void 
CirObj::collectFecGrps()
{
   vector<FecGrp>* newFecGrps = new vector<FecGrp>;
   for (FecGrp& grp : *_fecGrps) {
      unordered_map<size_t, FecGrp, HashInt> fecGrpsMap;
      for (size_t i = 0; i < grp.size(); i++) {
         size_t gateId = grp[i]/2;
         CirGate* gate = _idList[gateId];
         size_t simVal = gate->getSimResult();
         unordered_map<size_t, FecGrp, HashInt>::iterator tmp = fecGrpsMap.find(simVal);
         if (tmp != fecGrpsMap.end()) {
            tmp->second.add(gateId*2); continue;
         }
            
         tmp = fecGrpsMap.find(~(simVal));
         if (tmp != fecGrpsMap.end()) {
            tmp->second.add(gateId*2+1); continue;
         }
            
         FecGrp& tmpGrp = fecGrpsMap[simVal];
         tmpGrp.add(gateId*2);
      }  
      
      
      for (auto& tmpGrp : fecGrpsMap) {
         if (tmpGrp.second.size() > 1) 
            newFecGrps->push_back(tmpGrp.second);
      }
   }
   delete _fecGrps;
   _fecGrps = newFecGrps;
}

void
CirObj::randomSim()
{

}

void
CirObj::fileSim(ifstream& patternFile)
{
   string str;
   size_t bitNum = 0, piNum = _piList.size(), wordLen = patternLen;
   _fecGrps->clear(); 

   while (getline(patternFile, str)) {
      if (str.length() == 0) continue;
      str = str.substr(str.find_first_not_of(" "));
      if (str.length() != piNum) {
         cerr << "Pattern(" << str << ") length(" << str.length() << ") does not match the number of inputs("
              << piNum << ") in a circuit!!\n";
         return;
      }
      for (size_t i = 0; i < piNum; i++) {
         if (str[i] == '0') 
            _piList[i]->flipToZero(~(const1 << bitNum));
         
         else if (str[i] == '1') 
            _piList[i]->flipToOne(const1 << bitNum);
         else {
            cerr << "Pattern(" << str << ") contains a non-0/1 character('" << str[i] << "').\n";
            return;
         }
      }
      
      bitNum++;
      if (bitNum == wordLen) {
         if (_fecGrps->size() == 0) {
            _fecGrps->push_back(_dfsList);
            (*_fecGrps)[0].add(0);
         }
         simulate();
         collectFecGrps();
         bitNum = 0;
         if (_simLog) writeLog(wordLen);
      }         
   }

   if (bitNum > 0 && bitNum < wordLen) {
      if (_fecGrps->size() == 0) {
         _fecGrps->push_back(_dfsList);
         (*_fecGrps)[0].add(0);
      } 
      simulate();
      collectFecGrps();
      if (_simLog) writeLog(bitNum);
   }

   if (_simLog) _simLog->close();

   
}

/*************************************************/
/*   Private member functions about Simulation   */
/*************************************************/



/*************************************************/
/*   Public member functions about grouping   */
/*************************************************/

void
CirObj::initInputGrp()
{  
   for (size_t piIdx = 0 ; piIdx < _piList.size() ; piIdx++) {      
      bool isInsert = false;
      for (vector<size_t>* itr : _inputGrp) {
         if (_invFuncSupp[piIdx].size() == (*itr)[0]) {
            itr->push_back(piIdx); isInsert = true;
            break;
         }
      }
      if (!isInsert) {
         vector<size_t>* tmp = new vector<size_t>;
         tmp->push_back(_invFuncSupp[piIdx].size()); tmp->push_back(piIdx); 
         _inputGrp.push_back(tmp);
      }
   }

   sort(_inputGrp.begin(), _inputGrp.end(), [](vector<size_t>* a, vector<size_t>* b) {
      return (*a)[0] < (*b)[0];
   });

   // debug code
   for (vector<size_t>* itr : _inputGrp) {
      for (size_t i = 0 ; i < (*itr).size() ; i++) {
         if (i == 0) continue;
         cout << _invFuncSupp[(*itr)[i]].size() << " ";
      }
      cout << endl;
   }
}

void 
CirObj::initOutputGrp()
{
   for (size_t poIdx = 0 ; poIdx < _poList.size() ; poIdx++) {      
      bool isInsert = false;
      for (vector<size_t>* itr : _outputGrp) {
         if (_funcSupp[poIdx].size() == (*itr)[0]) {
            itr->push_back(poIdx); isInsert = true;
            break;
         }
      }
      if (!isInsert) {
         vector<size_t>* tmp = new vector<size_t>;
         tmp->push_back(_funcSupp[poIdx].size()); tmp->push_back(poIdx); 
         _outputGrp.push_back(tmp);
      }
   }

   sort(_outputGrp.begin(), _outputGrp.end(), [](vector<size_t>* a, vector<size_t>* b) {
      return (*a)[0] < (*b)[0];
   });
   
   // debug code
   for (vector<size_t>* itr : _outputGrp) {
      for (size_t i = 0 ; i < (*itr).size() ; i++) {
         if (i == 0) continue;
         cout << _funcSupp[(*itr)[i]].size() << " ";
      }
      cout << endl;
   }
}

void
CirObj::type1Sim()
{
   
}


// aig structure parsing
size_t
CirGate::piToPoGateCount()
{
   size_t gateCount = 0;
   for (CirGateV v : _foutList) {
      CirGate* fanout = v.getGate();
      gateCount += fanout->piToPoGateCount();
   }
   return gateCount;
}

size_t 
CirPoGate::piToPoGateCount()
{
   return 1;
}

size_t
CirPoGate::poToPiGateCount()
{
   return _in0.getGate()->poToPiGateCount();
}

size_t
CirGate::poToPiGateCount()
{
   if (this->isAig())
      return getIn0Gate()->poToPiGateCount() + getIn1Gate()->poToPiGateCount();
   if (this->isPo())
      return getIn0Gate()->poToPiGateCount();
}

size_t
CirAigGate::poToPiGateCount()
{
   return _in0.getGate()->poToPiGateCount() + _in1.getGate()->poToPiGateCount();;
}

size_t 
CirPiGate::poToPiGateCount()
{
   return 1;
}


size_t 
CirObj::piToPoGateCount()
{
   size_t gateCount = 0;
   for (CirPiGate* pi : _piList) {
      gateCount = pi->piToPoGateCount();
      cout << "pi name : " << pi->getName() 
           << ", gate count = " << gateCount << endl;
   }
}

size_t 
CirObj::poToPiGateCount()
{

}

size_t 
CirGate::piLongestPath()
{
   size_t maxLen = 0;
   for (CirGateV v : _foutList) {
      CirGate* g = v.getGate();
      size_t tmp = g->piLongestPath();
      if (maxLen <= tmp) maxLen = tmp;
   }
   return maxLen + 1;
}

size_t 
CirPoGate::piLongestPath()
{
   return 1;
}

void
CirObj::piLongestPath()
{
   size_t maxLen = 0;
   for (CirPiGate* pi : _piList) {
      maxLen = pi->piLongestPath();
      _piLongestPathList.push_back(maxLen);
      cout << "pi name : " << pi->getName() 
           << ", path len = " << maxLen << endl; 
   }
}

size_t 
CirPoGate::poLongestPath()
{
   size_t maxLen = getIn0Gate()->poLongestPath();
   return maxLen+1;
}

size_t 
CirAigGate::poLongestPath()
{
   size_t leftLen = getIn0Gate()->poLongestPath();
   size_t rightLen = getIn1Gate()->poLongestPath();
   return leftLen > rightLen ? leftLen+1 : rightLen+1;
}

size_t
CirPiGate::poLongestPath()
{
   return 1;
}

void
CirObj::poLongestPath()
{
   size_t maxLen = 0;
   for (CirPoGate* po : _poList) {
      maxLen = po->poLongestPath();
      _poLongestPathList.push_back(maxLen);
      // cout << "po name : " << po->getName() 
      //      << ", path len = " << maxLen << endl; 
   }
}