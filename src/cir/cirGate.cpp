/****************************************************************************
  FileName     [ cirGate.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define class CirAigGate member functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdarg.h>
#include <cassert>
#include "cirGate.h"
#include "cirMgr.h"
#include "util.h"

using namespace std;

extern CirMgr *cirMgr;
size_t CirGate::_globalRef = 0;

// TODO: Implement memeber functions for class(es) in cirGate.h

/**************************************/
/*   class CirGate member functions   */
/**************************************/
bool 
CirGate::isOutsGlobalRef() const
{
   if (_foutList.size() == 0) return false;
   for (CirGateV v : _foutList) {
      if (!v.getGate()->isGlobalRef()) return false;
   }
   return true;
}

size_t
CirGateV::getGid() const
{
   return getGate()->getId();
}

size_t
CirGateV::getLitId() const
{
   return 2*getGid() + isInv();
}

void
CirGate::backwardDfs(int level, int numSpace, bool isInv)
{
   if (level < 0) return;
   setToGlobalRef();
   for (int i = 0; i < numSpace; i++) cout << " ";
   cout << (isInv ? "!" : "") << getType()
        << " " << getId() << "\n";
}

void 
CirPoGate::backwardDfs(int level, int numSpace, bool isInv)
{
   if (level < 0) return;
   for (int i = 0; i < numSpace; i++) cout << " ";
   cout << (isInv ? "!" : "") << getType()
        << " " << getId();
   if (isGlobalRef() && isIn0GlobalRef()) {
      cout << " (*)\n";
      return;
   } 
   setToGlobalRef();
   cout << "\n";
   getIn0Gate()->backwardDfs(level-1, numSpace+2, _in0.isInv());
}

void 
CirAigGate::backwardDfs(int level, int numSpace, bool isInv)
{
   if (level < 0) return;
   for (int i = 0; i < numSpace; i++) cout << " ";
   cout << (isInv ? "!" : "") << getType()
        << " " << getId();
   if (isGlobalRef() && isIn0GlobalRef() && isIn1GlobalRef()) {
      cout << " (*)\n";
      return;
   }
   setToGlobalRef();
   cout << "\n";
   getIn0Gate()->backwardDfs(level-1, numSpace+2, _in0.isInv());
   getIn1Gate()->backwardDfs(level-1, numSpace+2, _in1.isInv());
}


void 
CirGate::forwardDfs(int level, int numSpace, bool isInv)
{
   if(level < 0) return;
   for (int i = 0; i < numSpace; i++) cout << " ";
   cout << (isInv ? "!" : "") << getType()
        << " " << getId();
   if (isGlobalRef() && isOutsGlobalRef()) {
      cout << " (*)\n";
      return;
   }
   setToGlobalRef();
   cout << "\n";
   for (CirGateV v : _foutList) {
      v.getGate()->forwardDfs(level-1, numSpace+2, v.isInv());
   }
   
}

void
CirGate::reportGate() const
{
   unsigned w = 50;
   for (unsigned i = 0; i < w; ++i) cout << '=';
   cout << endl;

   ostringstream osstr;
   osstr << "= " << getType() << "(" << _id << ")";
   if (getName().size()) osstr << "\"" << getName() << "\"";
   osstr << ", line " << _line+1;
   cout << setw(w - 2) << left << osstr.str() << " =" << endl;

   for (unsigned i = 0; i < w; ++i) cout << '=';
   cout << endl;
}

void
CirGate::reportFanin(int level)
{
   assert (level >= 0);
   CirGate::incrementGlobalRef();
   backwardDfs(level, 0, false);
}

void
CirGate::reportFanout(int level)
{
   assert (level >= 0);
   CirGate::incrementGlobalRef();
   forwardDfs(level, 0, false);
}

