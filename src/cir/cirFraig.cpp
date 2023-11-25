/****************************************************************************
  FileName     [ cirFraig.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir FRAIG functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2012-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include <unordered_map>
#include "cirMgr.h"
#include "cirGate.h"
#include "sat.h"
#include "myHashMap.h"
#include "util.h"

using namespace std;

// TODO: Please keep "CirMgr::strash()" and "CirMgr::fraig()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/*******************************************/
/*   Public member functions about fraig   */
/*******************************************/
// _floatList may be changed.
// _unusedList and _undefList won't be changed
bool 
CirAigGate::merge(unordered_map<HashKey, CirGate*, HashString>& gateHashTable)
{
   HashKey key = HashKey(_in0(), _in1());
   unordered_map<HashKey, CirGate*>::const_iterator tmp = gateHashTable.find(key);
   if (tmp == gateHashTable.end()) {
      setIn0(getIn0LitId());
      setIn1(getIn1LitId());
      gateHashTable[key] = this;
      _foutList.clear();
   }
   else {
      for (CirGateV gatev : _foutList) {
         CirGate* fout = gatev.getGate();
         if (fout->getIn0Gate() == this) 
            fout->setIn0((size_t)(tmp->second)+gatev.isInv());
         else
            fout->setIn1((size_t)(tmp->second)+gatev.isInv());
      }
      return true;
   }
   return false;
}

bool
CirPoGate::merge(unordered_map<HashKey, CirGate*, HashString>& gateHashTable)
{
   setIn0(getIn0LitId());
   return false;
}

void
CirMgr::strash()
{
   unordered_map<HashKey, CirGate*, HashString> gateHashTable;

   for (CirGate* g : _dfsList) {
      if (g->merge(gateHashTable) && g->isAig()) 
         _idList[g->getId()] = 0;     
   }

   // gen connection
   for (CirGate* g : _dfsList) {
      if (_idList[g->getId()]) g->genConnection();
      else cirMgr->recycle(g);
   }

   // reset list
   _dfsList.clear();
   _aigList.clear();
   _fltList.clear();
   genDfsList();
   for (CirGate* g : _idList) {
      if (g && g->numFltFin())
         _fltList.push_back(g);
      if (g && g->isAig())
         _aigList.push_back(static_cast<CirAigGate*>(g));
   }
}

void
CirMgr::fraig()
{
}

/********************************************/
/*   Private member functions about fraig   */
/********************************************/
