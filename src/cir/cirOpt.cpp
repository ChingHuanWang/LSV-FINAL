/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir optimization functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"
#include <algorithm>

using namespace std;

// TODO: Please keep "CirMgr::sweep()" and "CirMgr::optimize()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/**************************************************/
/*   Public member functions about optimization   */
/**************************************************/
// Remove unused gates
// DFS list should NOT be changed
// UNDEF, float and unused list may be changed

void
CirGate::deleteFout(CirGate* fout)
{
   for (size_t i = 0 ; i < numFout() ; i++) {
      if (fout == _foutList[i].getGate()) {
         _foutList[i] = _foutList.back();
         _foutList.pop_back();
         break;
      }
   }
}


void
CirUndefGate::deleteUnused(vector<CirGate*>& idList)
{
   if (numFout() == 0) {
      idList[getId()] = 0;
      cirMgr->recycle(this);
   }
}


void
CirAigGate::deleteUnused(vector<CirGate*>& idList)
{
   if (numFout() == 0) {
      getIn0Gate()->deleteFout(this);
      getIn1Gate()->deleteFout(this);
      getIn0Gate()->deleteUnused(idList);
      if (getIn0Gate() != getIn1Gate())
         getIn1Gate()->deleteUnused(idList);
      idList[getId()] = 0;
      cirMgr->recycle(this);
   }
}

void
CirObj::sweep()
{
   vector<CirGate*> tmp;
   for (CirGate* g : _aigList) 
      if (g->numFout() == 0) tmp.push_back(g);
   for (CirGate* g : tmp) 
      g->deleteUnused(_idList);
   
   _fltList.clear();
   _unusedList.clear();
   _aigList.clear();
   // reset flt, unused, aig list
   for (CirGate* g : _idList) {
      if (g && g->numFltFin())
         _fltList.push_back(g);
      else if (g && !g->isPo() && !g->isConst() && !g->numFout())
         _unusedList.push_back(g);
      else if (g && g->isAig())
         _aigList.push_back(static_cast<CirAigGate*>(g));
   }
   sort(_fltList.begin(), _fltList.end(), [](CirGate* a, CirGate* b) {
      return a->getId() < b->getId();
   });
   sort(_unusedList.begin(), _unusedList.end(), [](CirGate* a, CirGate* b) {
      return a->getId() < b->getId();
   });

}

// Recursively simplifying from POs;
// _dfsList needs to be reconstructed afterwards
// UNDEF gates may be delete if its fanout becomes empty...
void
CirGate::reconnect(size_t gateV)
{
   for (CirGateV v : _foutList) {
      CirGate* g = v.getGate();
      gateV = v.isInv() ? (gateV ^ 0x1) : gateV;
      if (g->getIn0Gate() == this) g->setIn0(gateV);
      else g->setIn1(gateV);
   }
}

void
CirPoGate::updateFanin(vector<CirGate*>& idList)
{
   setToGlobalRef();
   if (!getIn0Gate()->isGlobalRef())
      getIn0Gate()->updateFanin(idList);
   _in0 = _in0.getLitId();
}

void
CirAigGate::updateFanin(vector<CirGate*>& idList)
{
   setToGlobalRef();
   if (!getIn0Gate()->isGlobalRef())
      getIn0Gate()->updateFanin(idList);
   if (!getIn1Gate()->isGlobalRef())
      getIn1Gate()->updateFanin(idList);

   bool flag = false;
   if (getIn0Gate()->isConst()) {
      if (isIn0Inv()) reconnect(_in1());
      else reconnect(_in0());
      flag = true;
   }
   else if (getIn1Gate()->isConst()) {
      if (isIn1Inv()) reconnect(_in0());
      else reconnect(_in1());
      flag = true;
   }
   else if (getIn0Gate() == getIn1Gate()) {
      if (isIn0Inv() == isIn1Inv()) reconnect(_in0());
      else reconnect( (size_t)cirMgr->getConst0() );
      flag = true;
   }
   // reset in0 in1
   if (flag) {
      idList[getId()] = 0;
   }
   else {
      _in0 = _in0.getLitId();
      _in1 = _in1.getLitId();
      clearFouts();
   }

}

void
CirObj::optimize()
{
   CirGate::incrementGlobalRef();
   for (CirGate* g : _dfsList) {
      if (g->isPo())
         g -> updateFanin(_idList);
   }
   _fltList.clear();
   _unusedList.clear();
   _aigList.clear();

   // reset those list
   // for (CirGate* g : _idList) {
   //    if (g)
   //       cout << g->getType() << " id = " << g->getId() << " in0 = " << g->getIn0GateV() << " in1 = " << g->getIn1GateV() << endl;
   // }
   for (CirGate* g : _dfsList) {
      if (_idList[g->getId()]) g->genConnection(_objIdx);
      else cirMgr->recycle(g);
   }
   _dfsList.clear();
   genDfsList();
   for (CirGate* g : _idList) {
      if (g && g->numFltFin())
         _fltList.push_back(g);
      else if (g && !g->isPo() && !g->isConst() && !g->numFout())
         _unusedList.push_back(g);
      else if (g && g->isAig())
         _aigList.push_back(static_cast<CirAigGate*>(g));
   }
   sort(_fltList.begin(), _fltList.end(), [](CirGate* a, CirGate* b) {
      return a->getId() < b->getId();
   });
   sort(_unusedList.begin(), _unusedList.end(), [](CirGate* a, CirGate* b) {
      return a->getId() < b->getId();
   });
}

/***************************************************/
/*   Private member functions about optimization   */
/***************************************************/
