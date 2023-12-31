/****************************************************************************
  FileName     [ CirObj.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir manager functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <cstdio>
#include <ctype.h>
#include <cassert>
#include <cstring>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <string>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"
#include "sat.h"
#include "SolverTypes.h"


using namespace std;

// TODO: Implement memeber functions for class CirObj

/*******************************/
/*   Global variable and enum  */
/*******************************/
CirMgr* cirMgr = new CirMgr;

enum CirParseError {
   EXTRA_SPACE,
   MISSING_SPACE,
   ILLEGAL_WSPACE,
   ILLEGAL_NUM,
   ILLEGAL_IDENTIFIER,
   ILLEGAL_SYMBOL_TYPE,
   ILLEGAL_SYMBOL_NAME,
   MISSING_NUM,
   MISSING_IDENTIFIER,
   MISSING_NEWLINE,
   MISSING_DEF,
   CANNOT_INVERTED,
   MAX_LIT_ID,
   REDEF_GATE,
   REDEF_SYMBOLIC_NAME,
   REDEF_CONST,
   NUM_TOO_SMALL,
   NUM_TOO_BIG,

   DUMMY_END
};

/**************************************/
/*   Static varaibles and functions   */
/**************************************/
static unsigned lineNo = 0;  // in printint, lineNo needs to ++
// static unsigned colNo  = 0;  // in printing, colNo needs to ++
// static char buf[1024];
// static string errMsg;
// static int errInt;
// static CirGate *errGate;


/**************************************************************/
/*   class CirObj member functions for circuit construction   */
/**************************************************************/
void
CirMgr::deleCircuit()
{
   for (CirGate* g : _recycleList)
      delete g;
   _recycleList.clear();
}

void
CirObj::deleteCircuit()
{
   for (CirGate* g : _idList) 
      if (g) delete g;

   _idList.clear();
   _piList.clear();
   _poList.clear();
   _aigList.clear();
   _undefList.clear();
   _fltList.clear();
   _unusedList.clear();
   _dfsList.clear();
   
}

void 
CirObj::parsePi(ifstream& aag, size_t numIn)
{
   size_t litIn = 0;
   for (size_t i = 0 ; i < numIn; i++) {
      aag >> litIn; lineNo++;
      CirGate* pi = new CirPiGate(litIn/2, lineNo);
      _piList.push_back(static_cast<CirPiGate*>(pi)); 
      _idList[litIn/2] = pi;
   }
}

void 
CirObj::parsePo(ifstream& aag, size_t numOut)
{
   size_t litIn0 = 0;
   for (size_t i = 0; i < numOut; i++) {
      aag >> litIn0; lineNo++;
      CirGate* po = new CirPoGate(_maxVarIdx+i+1, lineNo, litIn0);
      _poList.push_back(static_cast<CirPoGate*>(po));
      _idList[_maxVarIdx+i+1] = po;
   }
}

void 
CirObj::parseAig(ifstream& aag, size_t numAnd)
{
   size_t litIn0 = 0, litIn1 = 0, litAig = 0;
   for (size_t i = 0; i < numAnd; i++) {
      aag >> litAig >> litIn0 >> litIn1; lineNo++;
      CirGate* aig = new CirAigGate(litAig/2, lineNo, litIn0, litIn1);
      _aigList.push_back(static_cast<CirAigGate*>(aig));
      _idList[litAig/2] = aig;
   }

}

void
CirObj::parseName(ifstream& aag)
{
   string tmp;
   while(getline(aag, tmp)){
      if(tmp[0] == 'i' || tmp[0] == 'o'){
         size_t idx = tmp.find_first_of(" ");
         int gid = 0; 
         myStr2Int(tmp.substr(1, idx-1), gid);
         tmp[0] == 'i' ? _piList[gid] -> setName(tmp.substr(idx+1)) : _poList[gid] -> setName(tmp.substr(idx+1)); 
      }
   }
}

size_t 
CirObj::checkGate(size_t litId)
{
   
   CirGate* gate = getGate(litId/2);
   size_t gateV = 0, gateId = litId/2;
   if (!gate) {
      _idList[gateId] = new CirUndefGate(gateId, 0);
      _undefList.push_back(static_cast<CirUndefGate*>(_idList[gateId]));
   }
   gateV = _idList[gateId]->isUndef() ? (size_t)(_idList[gateId]) + CirGateV::FLT : (size_t)(_idList[gateId]);
   gateV += litId % 2 == 1 ? CirGateV::NEG : 0;
   return gateV;

} 

void
CirPoGate::genConnection(size_t idx)
{
   _in0 = cirMgr -> getCir(idx) -> checkGate(_in0());
   getIn0Gate() -> addFout((size_t)(this)+_in0.isInv());
}

void 
CirAigGate::genConnection(size_t idx)
{
   _in0 = cirMgr -> getCir(idx) -> checkGate(_in0());
   getIn0Gate() -> addFout((size_t)(this)+_in0.isInv());
   _in1 = cirMgr -> getCir(idx) -> checkGate(_in1());
   getIn1Gate() -> addFout((size_t)(this)+_in1.isInv());
}

void
CirObj::genConnection()
{
   for (CirGate* gate : _idList) {
      if (gate) gate -> genConnection(_objIdx); 
   }
}

void 
CirObj::parseFlt()
{
   for (CirGate* gate : _idList) {
      if(gate && gate->numFltFin()) 
         _fltList.push_back(gate);
   }
   sort(_fltList.begin(), _fltList.end(), [](CirGate* a, CirGate* b) {
      return a->getId() < b->getId();
   });
}

void
CirObj::parseUnused()
{
   for (CirGate* gate : _idList) {
      if (gate && !gate->isPo() 
         && !gate->isConst() && !gate->numFout()) {
         _unusedList.push_back(gate);
      }
   }
   sort(_unusedList.begin(), _unusedList.end(), [](CirGate* a, CirGate* b) {
      return a->getId() < b->getId();
   });
}

bool
CirObj::readCircuit(const string& fileName)
{
   ifstream aag; aag.open(fileName);
   string tmp;
   size_t maxVarIdx, numIn, numLatch, numOut, numAnd;
   aag >> tmp >> maxVarIdx >> numIn >> numLatch >> numOut >> numAnd;
   _maxVarIdx = maxVarIdx; 
   _idList.assign(_maxVarIdx*2+1, 0);
   _const = new CirConstGate(0, 0);
   _idList[0] = _const;

   parsePi(aag, numIn);
   parsePo(aag, numOut);
   parseAig(aag, numAnd);
   parseName(aag);
   genConnection();
   parseFlt();
   parseUnused();
   genDfsList();
   aag.close();


   return true;
}

bool
CirObj::readBus(vector<vector<string>>& bus) {

   unordered_map<string, size_t> piMap, poMap;
   vector<size_t> tmpBus;
   bool isPiBus;

   // construct PI name to ID
   for (size_t i = 0; i < _piList.size(); ++i)
      piMap[_piList[i]->getName()] = _piList[i]->getId();

   // construct PO name to ID
   for (size_t i = 0; i < _poList.size(); ++i)
      poMap[_poList[i]->getName()] = _poList[i]->getId();
   
   // construct _piBus and _poBus
   for (size_t i = 0; i < bus.size(); ++i) {

      tmpBus.resize(bus[i].size());
      isPiBus = piMap.find(bus[i][0]) != piMap.end();

      for (size_t j = 0; j < bus[i].size(); ++j)
         tmpBus[j] = (isPiBus)? piMap[bus[i][j]]:poMap[bus[i][j]];
      
      if (isPiBus) _piBus.push_back(tmpBus);
      else _poBus.push_back(tmpBus);
      tmpBus.clear();
   }

   // ================== check for _piBus ==================
   cout << "PI bus of circuit " << _objIdx << ":\n";
   for (size_t i = 0; i < _piBus.size(); ++i) {
      for (size_t j = 0; j < _piBus[i].size(); ++j) {
         cout << _piBus[i][j] << " ";
      }
      cout << endl;
   }
   // ================== check for _piBus ==================

   // ================== check for _poBus ==================
   cout << "PO bus of circuit " << _objIdx << ":\n";
   for (size_t i = 0; i < _poBus.size(); ++i) {
      for (size_t j = 0; j < _poBus[i].size(); ++j) {
         cout << _poBus[i][j] << " ";
      }
      cout << endl;
   }
   // ================== check for _poBus ==================
   return true;
}

bool
CirMgr::readCircuit(const string& input, const string& cirFile1, const string& cirFile2)
{
   _const = new CirConstGate(0, 0);
   cirMgr->getCir(1)->readCircuit(cirFile1);
   cirMgr->getCir(2)->readCircuit(cirFile2);

   // parse bus
   ifstream inFile(input);
   size_t busNum, portNum;
   string line, portName;
   stringstream ss("");
   vector<string> bus;
   vector<vector<string>> cir1Bus, cir2Bus;

   // parse bus of circuit 1
   getline(inFile, line); // get circuit file (circuit_1.v)
   getline(inFile, line); // get circuit bus
   busNum = stoi(line);
   for (int i = 0 ; i < busNum; ++i) {
      getline(inFile, line); // bus information
      ss << line;
      ss >> portNum;
      for (int j = 0; j < portNum; ++j) {
         ss >> portName;
         bus.push_back(portName);
      }
      cir1Bus.push_back(bus);
      bus.clear();
      ss.clear();
      ss.str("");
   }
   
   // ================== check for cir1Bus ==================
   // for (size_t i = 0; i < cir1Bus.size(); ++i) {
   //    for (size_t j = 0; j < cir1Bus[i].size(); ++j) {
   //       cout << cir1Bus[i][j] << " ";
   //    }
   //    cout << endl;
   // }
   // ================== check for cir1Bus ==================

   // circuit 2
   getline(inFile, line); // get circuit file (circuit_1.v)
   getline(inFile, line); // get circuit bus
   busNum = stoi(line);
   for (int i = 0 ; i < busNum; ++i) {
      getline(inFile, line); // bus information
      ss << line;
      ss >> portNum;
      for (int j = 0; j < portNum; ++j) {
         ss >> portName;
         bus.push_back(portName);
      }
      cir2Bus.push_back(bus);
      bus.clear();
      ss.clear();
      ss.str("");
   }

   // ================== check for cir1Bus ==================
   // for (size_t i = 0; i < cir2Bus.size(); ++i) {
   //    for (size_t j = 0; j < cir2Bus[i].size(); ++j) {
   //       cout << cir2Bus[i][j] << " ";
   //    }
   //    cout << endl;
   // }
   // ================== check for cir1Bus ==================

   cirMgr->getCir(1)->readBus(cir1Bus);
   cirMgr->getCir(2)->readBus(cir2Bus);

   return true;
}

/**********************************************************/
/*   class CirObj member functions for circuit printing   */
/**********************************************************/
/*********************
Circuit Statistics
==================
  PI          20
  PO          12
  AIG        130
------------------
  Total      162
*********************/

void
CirGateV::reportGateV() const
{
   cout << (isFlt() ? "*" : "")
        << (isInv() ? "!" : "")
        << getGid();
}

void 
CirPiGate::printGate() const 
{
   cout << getType() << "  " << getId();
   if(getName().size())
      cout << " (" <<  getName() << ")";
   cout << "\n";
}

void 
CirPoGate::printGate() const 
{
   cout << getType() << "  " << getId() << " "; 
   _in0.reportGateV();
   if(getName().size())
      cout << " (" <<  getName() << ")";
   cout << "\n";
}

void 
CirAigGate::printGate() const 
{
   cout << getType() << " " << getId() << " ";
   _in0.reportGateV();
   cout << " ";
   _in1.reportGateV();
   cout << "\n";
}

void
CirPiGate::genDfsList(vector<CirGate*>& dfsList)
{
   setToGlobalRef();
   dfsList.push_back(this);
}

void 
CirConstGate::genDfsList(vector<CirGate*>& dfsList)
{
   setToGlobalRef();
   dfsList.push_back(this);
}

void 
CirPoGate::genDfsList(vector<CirGate*>& dfsList)
{
   setToGlobalRef();
   if (!_in0.getGate() -> isGlobalRef())
      _in0.getGate() -> genDfsList(dfsList);
   dfsList.push_back(this);
}

void
CirAigGate::genDfsList(vector<CirGate*>& dfsList)
{
   setToGlobalRef();
   if (!_in0.getGate() -> isGlobalRef())
      _in0.getGate() -> genDfsList(dfsList);

   if (!_in1.getGate() -> isGlobalRef())
      _in1.getGate() -> genDfsList(dfsList);

   dfsList.push_back(this);
}

void 
CirObj::genDfsList()
{
   CirGate::incrementGlobalRef();
   
   for (CirGate* gate : _poList) {
      gate -> genDfsList(_dfsList);
   }
}

void
CirObj::printSummary() const
{
   cout << endl;
   cout << "Circuit Statistics" << endl
        << "==================" << endl;
   unsigned tot = 0;
   tot += _piList.size();
   cout << "  " << setw(7) << left << "PI"
        << setw(7) << right << _piList.size() << endl;
   tot += _poList.size();
   cout << "  " << setw(7) << left << "PO"
        << setw(7) << right << _poList.size() << endl;
   tot += _aigList.size();
   cout << "  " << setw(7) << left << "AIG"
        << setw(7) << right << _aigList.size() << endl;
   cout << "------------------" << endl;
   cout << "  Total  " << setw(7) << right << tot << endl;
}

void
CirObj::printNetlist() const
{
   size_t l = 0;
   for (CirGate* gate : _dfsList) {
      cout << "[" << l << "] ";
      gate -> printGate();
      l++;
   }
}  

void
CirObj::printPIs() const
{
   cout << "PIs of the circuit:";
   for (CirGate* gate : _piList) {
      cout << " " << gate -> getId();
   }
   cout << endl;
}

void
CirObj::printPOs() const
{
   cout << "POs of the circuit:";
   for (CirGate* gate : _poList) {
      cout << " " << gate -> getId();
   }
   cout << endl;
}

void
CirObj::printFloatGates() const
{
   if (_fltList.size()) {
      cout << "Gates with floating fanin(s):";
      for (CirGate* gate : _fltList) {
         cout << " " << gate -> getId();
      }
   }

   cout << "\n";
   if (_unusedList.size()) {
      cout << "Gates defined but not used  :";
      for (CirGate* gate : _unusedList) {
         cout << " " << gate -> getId();
      }
   }
}

void
CirObj::writeAag(ostream& outfile) const
{
   size_t aigNum = 0;
   for (CirGate* gate : _dfsList) { if (gate->isAig()) { aigNum++; }}
   
   outfile << "aag " << _maxVarIdx << " " << _piList.size() << " "
           << "0 "   << _poList.size() << " " << aigNum << "\n";

   for (CirGate* pi : _piList) 
      outfile << pi -> getId()*2 << "\n";
   
   for (CirGate* po : _poList) 
      outfile << po->getIn0Gate()->getId()*2 + po->isIn0Inv() << "\n";
   
   for (CirGate* gate : _dfsList) {
      if(!gate->isAig()) continue;
      outfile << gate->getId()*2 << " "
              << gate->getIn0Gate()->getId()*2+gate->isIn0Inv() << " "
              << gate->getIn1Gate()->getId()*2+gate->isIn1Inv() << "\n";
   }

   int idx = 0;
   for (CirGate* pi : _piList)
      if (pi->getName().size() > 0) { outfile << "i" << idx << " " << pi->getName() << "\n"; idx++;}
   
   idx = 0;
   for (CirGate* po : _poList)
      if (po->getName().size() > 0) { outfile << "o" << idx << " " << po->getName() << "\n"; idx++;}
   
}


void
CirObj::writeGate(ostream& outfile, CirGate *g) const
{
}



void
CirObj::printFECPairs() const
{
   size_t grpNum = 0;
   for (const FecGrp& grp : *_fecGrps) {
      cout << "[" << grpNum << "] ";
      for (size_t i = 0; i < grp.size(); i++) {
         cout << (grp[i] % 2 == 1 ? "!" : "")
              << grp[i] / 2 << " ";
      }
      cout << '\n';
      grpNum++;
   }
}

void
CirObj::getRedundant(vector<size_t>& input, vector<size_t>& output, vector<vector<bool>>& redundant) {

   assert (_poList.size() == output.size());

   redundant.resize(output.size(), vector<bool>(input.size(), true));

   // ===== check input and output is correct =====
   // cout << "input: ";
   // for (size_t i = 0; i < input.size(); ++i)
   //    cout << input[i];
   // cout << endl;
   // cout << "output: ";
   // for (size_t i = 0; i < output.size(); ++i)
   //    cout << output[i];
   // cout << endl;
   // getchar();
   // ===== check input and output is correct =====

   for (size_t i = 0; i < redundant.size(); ++i)
      for (size_t j = 0; j < _poFuncSupp[i].size(); ++j)
         redundant[i][_poFuncSupp[i][j] - 1] = false;

   SatSolver s;
   Var vf, va, vb;
   vector<Var> vh(output.size(), 0);
   vec<Lit> lits;
   Lit lf, la, lb;
   size_t i0, i1, num = _dfsList.size();

   s.initialize();
   s.addCirCNF(this, 0);
   
   for (size_t i = 0; i < _poList.size(); ++i)
      vh[i] = s.newVar();
   
   for (size_t i = 0; i < _poList.size(); ++i) {
      vf = _poList[i]->getId(); lf = (output[i])? ~Lit(vf):Lit(vf);
      la = ~Lit(vh[i]);
      lits.push(lf); lits.push(la);
      s.addCNF(lits); lits.clear();
   }
   
   for (size_t i = 0; i < output.size(); ++i) {
      for (size_t j = 0; j < _poFuncSupp[i].size(); ++j) {
         redundant[i][_poFuncSupp[i][j] - 1] = true;
         s.assumeRelease();
         s.assumeProperty(vh[i], true);
         for (size_t k = 0; k < input.size(); ++k) {
            if (!redundant[i][k]) s.assumeProperty(k + 1, input[k]);
         }
         if (s.assumpSolve()) redundant[i][_poFuncSupp[i][j] - 1] = false;
      }
   }
}

void
CirObj::collectPoFuncSupp() {

   if (_strucSupp.empty()) collectStrucSupp();

   Var vf, va, vb;
   Lit lf, la, lb;
   vec<Lit> lits;
   vector<Var> h(_piList.size(), 0), g(_poList.size(), 0);
   size_t gateNum = _piList.size() + _aigList.size() + _poList.size();
   SatSolver s;
   s.initialize();
   s.addCirCNF(this, 0);
   s.addCirCNF(this, gateNum);

   for (size_t i = 0; i < h.size(); ++i)
      h[i] = s.newVar();
   for (size_t i = 0; i < g.size(); ++i)
      g[i] = s.newVar();
   
   for (size_t i = 0; i < h.size(); ++i) {
      vf = h[i]; lf = Lit(vf);
      va = _piList[i]->getId(); la = Lit(va);
      vb = va + gateNum; lb = Lit(vb);
      lits.push(lf); lits.push(la); lits.push(~lb);
      s.addCNF(lits); lits.clear();
      lits.push(lf); lits.push(~la); lits.push(lb);
      s.addCNF(lits); lits.clear();
      lits.push(~lf); lits.push(la); lits.push(lb);
      s.addCNF(lits); lits.clear();
      lits.push(~lf); lits.push(~la); lits.push(~lb);
      s.addCNF(lits); lits.clear();
   }

   for (size_t i = 0; i < g.size(); ++i) {
      vf = g[i]; lf = Lit(vf);
      va = _poList[i]->getId(); la = Lit(va);
      vb = va + gateNum; lb = Lit(vb);
      lits.push(~lf); lits.push(la); lits.push(lb);
      s.addCNF(lits); lits.clear();
      lits.push(~lf); lits.push(~la); lits.push(~lb);
      s.addCNF(lits); lits.clear();
   }

   _poFuncSupp.resize(_poList.size());

   for (size_t i = 0; i < _poList.size(); ++i) {
      vector<size_t> tmp;
      for (set<CirGate*>::iterator gate = _poList[i]->suppBegin(); gate != _poList[i]->suppEnd(); ++gate) {
         s.assumeRelease();
         for (size_t k = 0; k < _poList.size(); ++k)
            s.assumeProperty(g[k], k == i);
         for (size_t k = 0; k < _piList.size(); ++k)
            s.assumeProperty(h[k], k == (*gate)->getId() - 1);
         if (s.assumpSolve()) tmp.push_back((*gate)->getId());
      }
      _poFuncSupp[i] = tmp;
   }

   // ========== check ==========
   cout << "PO functional support of circuit " << _objIdx << ":\n";
   for (size_t i = 0; i < _poFuncSupp.size(); ++i) {
      for (size_t j = 0; j < _poFuncSupp[i].size(); ++j) {
         cout << _poFuncSupp[i][j] << " ";
      }
      cout << endl;
   }
   // ========== check ==========
}

void
CirObj::printPoFuncSupp() const {

   cout << "functional support of cir " << _objIdx << ":\n";
   for (size_t j = 0; j < _poFuncSupp.size(); ++j) {
      cout << _poFuncSupp[j].size() << " ";
      for (size_t k = 0; k < _poFuncSupp[j].size(); ++k) {
         cout << _poFuncSupp[j][k] << " ";
      }
      cout << endl;
   }
}

void
CirObj::collectPiFuncSupp()
{
   _piFuncSupp.assign(_piList.size(), vector<size_t>(0, 0));
   for (size_t i = 0 ; i < _poList.size() ; i++) {
      for (size_t piId : _poFuncSupp[i]) {
         for (size_t j = 0 ; j < _piList.size() ; j++) {
            if (_piList[j]->getId() == piId) 
               _piFuncSupp[j].push_back(_poList[i]->getId());
         }
      }
   }
}

void 
CirObj::printPiFuncSupp() const
{
   for (size_t i = 0 ; i < _piList.size() ; i++) {
      cout << "PI " << _piList[i]->getName() << " : ";
      for (size_t poId : _piFuncSupp[i]) {
         cout << _idList[poId]->getName() << " ";
      }
      cout << endl;
   }
}

void
CirObj::collectSym() {

   if (_poFuncSupp.empty()) collectPoFuncSupp();

   Var vf, va, vb;
   Lit lf, la, lb;
   vec<Lit> lits;
   vector<Var> h(_piList.size(), 0), g(_poList.size(), 0);
   size_t gateNum = _dfsList.size();
   SatSolver s;
   
   s.initialize();
   s.addCirCNF(this, 0);
   s.addCirCNF(this, gateNum);

   for (size_t i = 0; i < h.size(); ++i)
      h[i] = s.newVar();
   for (size_t i = 0; i < g.size(); ++i)
      g[i] = s.newVar();
   
   // h[i] = 0 => x[i] == y[i]
   // h[i] = 1 => x[i] != y[i]
   for (size_t i = 0; i < h.size(); ++i) {
      vf = h[i]; lf = ~Lit(vf);
      va = _piList[i]->getId(); la = Lit(va);
      vb = va + gateNum; lb = Lit(vb);
      lits.push(lf); lits.push(la); lits.push(~lb);
      s.addCNF(lits); lits.clear();
      lits.push(lf); lits.push(~la); lits.push(lb);
      s.addCNF(lits); lits.clear();
      lits.push(~lf); lits.push(la); lits.push(lb);
      s.addCNF(lits); lits.clear();
      lits.push(~lf); lits.push(~la); lits.push(~lb);
      s.addCNF(lits); lits.clear();
   }

   for (size_t i = 0; i < g.size(); ++i) {
      vf = g[i]; lf = Lit(vf);
      va = _poList[i]->getId(); la = Lit(va);
      vb = va + gateNum; lb = Lit(vb);
      lits.push(~lf); lits.push(la); lits.push(lb);
      s.addCNF(lits); lits.clear();
      lits.push(~lf); lits.push(~la); lits.push(~lb);
      s.addCNF(lits); lits.clear();
   }
   
   _sym.resize(_poList.size());

   for (size_t i = 0; i < _poList.size(); ++i) {
      vector<vector<size_t>> symGroup;
      vector<bool> grouped(_poFuncSupp[i].size(), false);

      for (size_t j = 0; j < _poFuncSupp[i].size(); ++j) {
         if (grouped[j]) continue;

         vector<size_t> group = {_poFuncSupp[i][j]};
         for (size_t k = j + 1; k < _poFuncSupp[i].size(); ++k) {
            s.assumeRelease();
            for (size_t l = 0; l < _poList.size(); ++l)
               s.assumeProperty(g[l], l == i);
            for (size_t l = 0; l < _piList.size(); ++l) {
               s.assumeProperty(h[l], (l == _poFuncSupp[i][j] - 1) || (l == _poFuncSupp[i][k] - 1));
               if (l == _poFuncSupp[i][j] - 1) s.assumeProperty(_piList[l]->getId(), true);
               else if (l == _poFuncSupp[i][k] - 1) s.assumeProperty(_piList[l]->getId(), false);
            }

            if (!s.assumpSolve()) {
               group.push_back(_poFuncSupp[i][k]);
               grouped[k] = true;
            }
         }
         grouped[j] = true;

         symGroup.push_back(group);
      }
      _sym[i] = symGroup;
   }
}

void
CirObj::printSym() const {
   cout << "symmetric group of cir " << _objIdx << ":\n";
   for (size_t i = 0; i < _sym.size(); ++i) {
      cout << "PO " << i << ":\n";
      for (size_t j = 0; j < _sym[i].size(); ++j) {
         for (size_t k = 0; k < _sym[i][j].size(); ++k) {
            cout << _sym[i][j][k] << " ";
         }
         cout << endl;
      }
   }   
}


/**********************************************************/
/* class CirGate member functions for collecting struc supp */
/**********************************************************/

void 
CirObj::printStrucSupp() const
{
   for (CirPoGate* g : _poList) {
      cout << g->getName() << endl;
      g->printStrucSupp();
   }
}

void
CirObj::collectStrucSupp()
{
   
   CirGate::incrementGlobalRef();
   for (CirPoGate* g : _poList) {
      g->collectStrucSupp();
      _strucSupp.push_back(vector<CirGate*>(g->suppBegin(), g->suppEnd()));
   }
}

void 
CirPoGate::printStrucSupp() const 
{
   cout << "size = " << _strucSupp.size() << endl;
   // for (CirGate* g : _strucSupp) {
   //    cout << g->getName() << " ";
   // }
   // cout << endl;
   
}

void 
CirPiGate::collectStrucSupp() 
{
   if (!isGlobalRef()) {
      CirGate::setToGlobalRef();
      _strucSupp.insert(this);
   }
   return;
}

void 
CirPoGate::collectStrucSupp()
{
   CirGate::setToGlobalRef();
   if (!isIn0GlobalRef()) {
      getIn0Gate()->collectStrucSupp();
   }
   _strucSupp.insert(getIn0Gate()->suppBegin(), getIn0Gate()->suppEnd());
   return;
}

void
CirAigGate::collectStrucSupp()
{
   CirGate::setToGlobalRef();
   if (!isIn0GlobalRef()) {
      getIn0Gate()->collectStrucSupp();
   }
   if (!isIn1GlobalRef()) {
      getIn1Gate()->collectStrucSupp();
   }

   _strucSupp.insert(getIn0Gate()->suppBegin(), getIn0Gate()->suppEnd());
   _strucSupp.insert(getIn1Gate()->suppBegin(), getIn1Gate()->suppEnd());
   return;
}

void
CirUndefGate::collectStrucSupp()
{
   if (!isGlobalRef()) {
      CirGate::setToGlobalRef();
      _strucSupp.insert(this);
   }
   return;
}

void 
CirConstGate::collectStrucSupp()
{
   if (!isGlobalRef()) {
      CirGate::setToGlobalRef();
      _strucSupp.insert(this);
   }
   return;
}

/**********************************************************/
/* class CirMgr member functions for collecting unate var */
/**********************************************************/

void
CirObj::collectUnate()
{
   
   _posUnateTable.assign(_poList.size(), vector<size_t>(0, 0));
   _negUnateTable.assign(_poList.size(), vector<size_t>(0, 0));
   
   Var va, vb, vh;
   Lit la, lb, lh;
   vec<Lit> lits;
   vector<string> choice{ "pos", "neg" };

   for (string unateType : choice) {
      // setting the var value of the circuit
      for (size_t i = 0 ; i < _dfsList.size() ; i++)
         _dfsList[i]->setVar(_sat.newVar());

      // datalift var num for finding unate var
      int dataLift = _dfsList.size();
      for (size_t i = 0 ; i < dataLift*2 ; i++)
         _sat.newVar();

      // gen aig cnf for f_a (i = 0), f_b (i = 1), f_h (i = 2)
      for (size_t i = 0 ; i < 2 ; i++) {
         for (CirAigGate* aig : _aigList) {
            _sat.addAigCNF(aig->getVar()+dataLift*i, aig->getIn0Gate()->getVar()+dataLift*i, aig->isIn0Inv(),
                           aig->getIn1Gate()->getVar()+dataLift*i, aig->isIn1Inv());
         }
      }

      for (size_t i = 0 ; i < 2 ; i++) {
         for (CirPoGate* po : _poList) {
            va = po->getVar()+dataLift*i;
            vb = po->getIn0Gate()->getVar()+dataLift*i;
            la = po->isIn0Inv() ? ~Lit(va) : Lit(va);
            lb = Lit(vb);
            lits.push(la); lits.push(~lb);
            _sat.addCNF(lits); lits.clear();
            lits.push(~la); lits.push(lb);
            _sat.addCNF(lits); lits.clear();
         }
      }

      // v_a*v_b + v_h => (v_a+v_h)(v_b+v_h)
      for (CirPoGate* po : _poList) {
         va = po->getVar(); vb = va+dataLift; vh = vb+dataLift;
         la = unateType == "pos" ? Lit(va) : ~Lit(va); 
         lb = unateType == "pos" ? ~Lit(vb) : Lit(vb);
         lh = Lit(vh);
         lits.push(la); lits.push(lh);
         _sat.addCNF(lits); lits.clear();
         lits.push(lb); lits.push(lh);
         _sat.addCNF(lits); lits.clear();
      }


      // collect unate var
      for (size_t i = 0 ; i < _poList.size() ; i++) {

         // (Va = Vb)+Vh = (Va + ~Vb + Vh)(~Va + Vb + Vh)
         for (size_t piId : _poFuncSupp[i]) {
            va = _idList[piId]->getVar(); vb = va+dataLift; vh = vb+dataLift;
            la = Lit(va); lb = ~Lit(vb); lh = Lit(vh);
            lits.push(la); lits.push(lb); lits.push(lh);
            _sat.addCNF(lits); lits.clear();

            la = ~Lit(va); lb = Lit(vb); lh = Lit(vh);
            lits.push(la); lits.push(lb); lits.push(lh);
            _sat.addCNF(lits); lits.clear();
         }

         // solve sat
         for (size_t piId : _poFuncSupp[i]) {
            _sat.assumeRelease();
            va = _idList[piId]->getVar(); vb = va+dataLift; vh = vb+dataLift;
            _sat.assumeProperty(va, false);
            _sat.assumeProperty(vb, true);
            _sat.assumeProperty(vh, true);
            _sat.assumeProperty(_poList[i]->getVar()+dataLift*2, false);
            bool isSat = _sat.assumpSolve();
            // cout << "isSat = " << isSat << endl;
            if(!isSat) {
               if (unateType == "pos") {
                  _posUnateTable[i].push_back(piId);
               }
               else {
                  _negUnateTable[i].push_back(piId);
               }
            }
         }
      }
      // reset _sat
      _sat.reset();
   }
}

void 
CirObj::printUnate() const 
{
   cout << "positive case" << endl;

   for (size_t i = 0 ; i < _poList.size() ; i++) {
      cout << _poList[i]->getName() << " : ";
      for (size_t piId : _posUnateTable[i]) {
         cout << _idList[piId]->getName() << " ";
      }
      cout << endl;
   }

   cout << "negative case" << endl;

   for (size_t i = 0 ; i < _poList.size() ; i++) {
      cout << _poList[i]->getName() << " : ";
      for (size_t piId : _negUnateTable[i]) {
         cout << _idList[piId]->getName() << " ";
      }
      cout << endl;
   }
}

void
CirObj::collectPiGateCount() {

   // check if already count
   if (_piGateCount.size() != 0) return;

   _piGateCount.resize(_piList.size());
   for (size_t i = 0; i < _piGateCount.size(); ++i)
      _piGateCount[i] = _piList[i]->piToPoGateCount();
   
}

void
CirObj::collectPoGateCount() {

   // check if already count
   if (_poGateCount.size() != 0) return;

   _poGateCount.resize(_poList.size());
   for (size_t i = 0; i < _poGateCount.size(); ++i)
      _poGateCount[i] = _poList[i]->poToPiGateCount();
   
   // ========== check ==========
   cout << "PO gate count of circuit " << _objIdx << ":\n";
   for (size_t i = 0; i < _poList.size(); ++i) {
      cout << _poGateCount[i] << " ";
   }
   cout << endl;
   // ========== check ==========

}

void
CirObj::collectPoOrder() {

   size_t count = 0;
   // ===== order PO according to =====
   // 1. functional support size
   // 2. TPI gate number

   // step 1. generate index vector of PO
   _poOrder.resize(_poList.size());
   generate(_poOrder.begin(), _poOrder.end(), [&](){ return count++; });

   // step 2. order PO
   sort(_poOrder.begin(), _poOrder.end(), [&_poFuncSupp=_poFuncSupp, &_poGateCount=_poGateCount](size_t idx1, size_t idx2) {
      if (_poFuncSupp[idx1].size() < _poFuncSupp[idx2].size()) return true;
      if (_poFuncSupp[idx1].size() > _poFuncSupp[idx2].size()) return false;
      return _poGateCount[idx1] < _poGateCount[idx2];
   });

   // step 3. re-order _poFuncSupp and _poGateCount
   vector<size_t> newPoGateCount;
   vector<vector<size_t>> newPoFuncSupp;
   vector<CirPoGate*> newPoList;
   newPoGateCount.resize(_poList.size());
   newPoFuncSupp.resize(_poList.size());
   for (size_t i = 0; i < _poList.size(); ++i) {
      newPoGateCount[i] = _poGateCount[_poOrder[i]];
      newPoFuncSupp[i] = _poFuncSupp[_poOrder[i]];
      newPoList.push_back(_poList[_poOrder[i]]);
   }
   _poGateCount = newPoGateCount;
   _poFuncSupp = newPoFuncSupp;
   for (size_t i = 0; i < _poList.size(); ++i)
      _poList[i] = newPoList[i];

   // ========== check ==========
   cout << "new PO order of circuit " << _objIdx << ":\n";
   for (size_t i = 0; i < _poList.size(); ++i) {
      cout << _poList[i]->getId() << " ";
   }
   cout << endl;
   cout << "new PO functional support of circuit " << _objIdx << ":\n";
   for (size_t i = 0; i < _poList.size(); ++i) {
      for (size_t j = 0; j < _poFuncSupp[i].size(); ++j) {
         cout << _poFuncSupp[i][j] << " ";
      }
      cout << endl;
   }
   cout << "new PO gate count of circuit " << _objIdx << ":\n";
   for (size_t i = 0; i < _poList.size(); ++i) {
      cout << _poGateCount[i] << " ";
   }
   cout << endl;
   // ========== check ==========
}

void
CirObj::collectPiOrder() {

   size_t count = 0;
   // ===== order PI according to =====
   // 1. functional support size
   // 2. TPO gate number

   // step 1. generate index vector of PO
   _piOrder.resize(_piList.size());
   generate(_piOrder.begin(), _piOrder.end(), [&](){ return count++; });

   // step 2. order PO
   sort(_piOrder.begin(), _piOrder.end(), [&_piFuncSupp=_piFuncSupp, &_piGateCount=_piGateCount](size_t idx1, size_t idx2) {
      if (_piFuncSupp[idx1].size() < _piFuncSupp[idx2].size()) return true;
      if (_piFuncSupp[idx1].size() > _piFuncSupp[idx2].size()) return false;
      return _piGateCount[idx1] < _piGateCount[idx2];
   });

   // step 3. re-order _poFuncSupp and _poGateCount
   vector<size_t> newPiGateCount;
   vector<vector<size_t>> newPiFuncSupp;
   vector<CirPiGate*> newPiList;
   newPiGateCount.resize(_piList.size());
   newPiFuncSupp.resize(_piList.size());
   for (size_t i = 0; i < _piList.size(); ++i) {
      newPiGateCount[i] = _piGateCount[_piOrder[i]];
      newPiFuncSupp[i] = _piFuncSupp[_piOrder[i]];
      newPiList.push_back(_piList[_piOrder[i]]);
   }
   _piGateCount = newPiGateCount;
   _piFuncSupp = newPiFuncSupp;
   for (size_t i = 0; i < _piList.size(); ++i)
      _piList[i] = newPiList[i];
}

void
CirObj::reorderPoBus() {
   
   // construct mapping from PO ID -> funcSupp idx
   unordered_map<size_t, size_t> map;
   for (size_t i = 0; i < _poList.size(); ++i)
      map[_poList[i]->getId()] = i;

   // reorder _poBus
   size_t count = 0;
   vector<size_t> idx;
   vector<double> avgFuncSuppSize;
   idx.resize(_poBus.size());
   generate(idx.begin(), idx.end(), [&](){ return count++; });

   // calculate average functional support size of each bus and order
   avgFuncSuppSize.resize(_poBus.size());
   double avg;
   for (size_t i = 0; i < _poBus.size(); ++i) {
      avg = 0;
      for (size_t j = 0; j < _poBus[i].size(); ++j)
         avg += (double)_poFuncSupp[map[_poBus[i][j]]].size();
      avg /= (double)_poBus[i].size();
      avgFuncSuppSize[i] = avg;
   }
   sort(idx.begin(), idx.end(), [&](size_t idx1, size_t idx2) {
      return avgFuncSuppSize[idx1] < avgFuncSuppSize[idx2];
   });

   vector<vector<size_t>> newPoBus;
   newPoBus.resize(_poBus.size());
   for (size_t i = 0; i < idx.size(); ++i)
      newPoBus[i] = _poBus[idx[i]];
   // for (size_t i = 0; i < newPoBus.size(); ++i)
   //    for (size_t j = 0; j < newPoBus[i].size(); ++j)
   //       newPoBus[i][j] = map[newPoBus[i][j]];
   _poBus = newPoBus;

   // ========== check ==========
   cout << "new PO bus of circuit " << _objIdx << ":\n";
   for (size_t i = 0; i < _poBus.size(); ++i) {
      for (size_t j = 0; j < _poBus[i].size(); ++j) {
         cout << _poBus[i][j] << " ";
      }
      cout << endl;
   }
   // ========== check ==========

}

void
CirObj::reorderPiBus() {

   // construct mapping from PO ID -> funcSupp idx
   unordered_map<size_t, size_t> map;
   for (size_t i = 0; i < _piList.size(); ++i)
      map[_piList[i]->getId()] = i;

   // reorder _poBus
   size_t count = 0;
   vector<size_t> idx;
   vector<double> avgFuncSuppSize;
   idx.resize(_piBus.size());
   generate(idx.begin(), idx.end(), [&](){ return count++; });

   // calculate average functional support size of each bus and order
   avgFuncSuppSize.resize(_piBus.size());
   double avg;
   for (size_t i = 0; i < _piBus.size(); ++i) {
      avg = 0;
      for (size_t j = 0; j < _piBus[i].size(); ++j)
         avg += (double)_piFuncSupp[map[_piBus[i][j]]].size();
      avg /= (double)_piBus[i].size();
      avgFuncSuppSize[i] = avg;
   }
   sort(idx.begin(), idx.end(), [&](size_t idx1, size_t idx2) {
      return avgFuncSuppSize[idx1] < avgFuncSuppSize[idx2];
   });

   vector<vector<size_t>> newPiBus;
   newPiBus.resize(_piBus.size());
   for (size_t i = 0; i < idx.size(); ++i)
      newPiBus[i] = _piBus[idx[i]];
   // for (size_t i = 0; i < newPiBus.size(); ++i)
   //    for (size_t j = 0; j < newPiBus[i].size(); ++j)
   //       newPiBus[i][j] = map[newPiBus[i][j]];
   _piBus = newPiBus;

   // ========== check ==========
   cout << "new PI bus of circuit " << _objIdx << ":\n";
   for (size_t i = 0; i < _piBus.size(); ++i) {
      for (size_t j = 0; j < _piBus[i].size(); ++j) {
         cout << _piBus[i][j] << " ";
      }
      cout << endl;
   }
   // ========== check ==========
}