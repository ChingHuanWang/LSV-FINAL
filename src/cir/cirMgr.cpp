/****************************************************************************
  FileName     [ cirMgr.cpp ]
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
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"


using namespace std;

// TODO: Implement memeber functions for class CirMgr

/*******************************/
/*   Global variable and enum  */
/*******************************/
CirMgr* cirMgr = 0;

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

// static bool
// parseError(CirParseError err)
// {
//    switch (err) {
//       case EXTRA_SPACE:
//          cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
//               << ": Extra space character is detected!!" << endl;
//          break;
//       case MISSING_SPACE:
//          cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
//               << ": Missing space character!!" << endl;
//          break;
//       case ILLEGAL_WSPACE: // for non-space white space character
//          cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
//               << ": Illegal white space char(" << errInt
//               << ") is detected!!" << endl;
//          break;
//       case ILLEGAL_NUM:
//          cerr << "[ERROR] Line " << lineNo+1 << ": Illegal "
//               << errMsg << "!!" << endl;
//          break;
//       case ILLEGAL_IDENTIFIER:
//          cerr << "[ERROR] Line " << lineNo+1 << ": Illegal identifier \""
//               << errMsg << "\"!!" << endl;
//          break;
//       case ILLEGAL_SYMBOL_TYPE:
//          cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
//               << ": Illegal symbol type (" << errMsg << ")!!" << endl;
//          break;
//       case ILLEGAL_SYMBOL_NAME:
//          cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
//               << ": Symbolic name contains un-printable char(" << errInt
//               << ")!!" << endl;
//          break;
//       case MISSING_NUM:
//          cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
//               << ": Missing " << errMsg << "!!" << endl;
//          break;
//       case MISSING_IDENTIFIER:
//          cerr << "[ERROR] Line " << lineNo+1 << ": Missing \""
//               << errMsg << "\"!!" << endl;
//          break;
//       case MISSING_NEWLINE:
//          cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
//               << ": A new line is expected here!!" << endl;
//          break;
//       case MISSING_DEF:
//          cerr << "[ERROR] Line " << lineNo+1 << ": Missing " << errMsg
//               << " definition!!" << endl;
//          break;
//       case CANNOT_INVERTED:
//          cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
//               << ": " << errMsg << " " << errInt << "(" << errInt/2
//               << ") cannot be inverted!!" << endl;
//          break;
//       case MAX_LIT_ID:
//          cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
//               << ": Literal \"" << errInt << "\" exceeds maximum valid ID!!"
//               << endl;
//          break;
//       case REDEF_GATE:
//          cerr << "[ERROR] Line " << lineNo+1 << ": Literal \"" << errInt
//               << "\" is redefined, previously defined as "
//               << errGate->getTypeStr() << " in line " << errGate->getLineNo()
//               << "!!" << endl;
//          break;
//       case REDEF_SYMBOLIC_NAME:
//          cerr << "[ERROR] Line " << lineNo+1 << ": Symbolic name for \""
//               << errMsg << errInt << "\" is redefined!!" << endl;
//          break;
//       case REDEF_CONST:
//          cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
//               << ": Cannot redefine constant (" << errInt << ")!!" << endl;
//          break;
//       case NUM_TOO_SMALL:
//          cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
//               << " is too small (" << errInt << ")!!" << endl;
//          break;
//       case NUM_TOO_BIG:
//          cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
//               << " is too big (" << errInt << ")!!" << endl;
//          break;
//       default: break;
//    }
//    return false;
// }

/**************************************************************/
/*   class CirMgr member functions for circuit construction   */
/**************************************************************/
void
CirMgr::deleteCircuit()
{
   for (CirGate* g : _idList) 
      if (g) delete g;
   for (CirGate* g : _recycleList)
      delete g;

   _idList.clear();
   _piList.clear();
   _poList.clear();
   _aigList.clear();
   _undefList.clear();
   _fltList.clear();
   _unusedList.clear();
   _dfsList.clear();
   _recycleList.clear();
}

void 
CirMgr::parsePi(ifstream& aag, size_t numIn)
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
CirMgr::parsePo(ifstream& aag, size_t numOut)
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
CirMgr::parseAig(ifstream& aag, size_t numAnd)
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
CirMgr::parseName(ifstream& aag)
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
CirMgr::checkGate(size_t litId)
{
   
   CirGate* gate = cirMgr -> getGate(litId/2);
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
CirPoGate::genConnection()
{
   _in0 = cirMgr -> checkGate(_in0());
   getIn0Gate() -> addFout((size_t)(this)+_in0.isInv());
}

void 
CirAigGate::genConnection()
{
   _in0 = cirMgr -> checkGate(_in0());
   getIn0Gate() -> addFout((size_t)(this)+_in0.isInv());
   _in1 = cirMgr -> checkGate(_in1());
   getIn1Gate() -> addFout((size_t)(this)+_in1.isInv());
}

void
CirMgr::genConnection()
{
   for (CirGate* gate : _idList) {
      if (gate) gate -> genConnection(); 
   }
}

void 
CirMgr::parseFlt()
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
CirMgr::parseUnused()
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
CirMgr::readCircuit(const string& fileName)
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

/**********************************************************/
/*   class CirMgr member functions for circuit printing   */
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
CirMgr::genDfsList()
{
   CirGate::incrementGlobalRef();
   
   for (CirGate* gate : _poList) {
      gate -> genDfsList(_dfsList);
   }
}

void
CirMgr::printSummary() const
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
CirMgr::printNetlist() const
{
   size_t l = 0;
   for (CirGate* gate : _dfsList) {
      cout << "[" << l << "] ";
      gate -> printGate();
      l++;
   }
}  

void
CirMgr::printPIs() const
{
   cout << "PIs of the circuit:";
   for (CirGate* gate : _piList) {
      cout << " " << gate -> getId();
   }
   cout << endl;
}

void
CirMgr::printPOs() const
{
   cout << "POs of the circuit:";
   for (CirGate* gate : _poList) {
      cout << " " << gate -> getId();
   }
   cout << endl;
}

void
CirMgr::printFloatGates() const
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
CirMgr::writeAag(ostream& outfile) const
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
CirMgr::writeGate(ostream& outfile, CirGate *g) const
{
}



void
CirMgr::printFECPairs() const
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
