/****************************************************************************
  FileName     [ cirGate.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic gate data structures ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_GATE_H
#define CIR_GATE_H

#include <string>
#include <vector>
#include <set>
#include <iostream>
#include <bitset>
#include <unordered_map>
#include "cirDef.h"
#include "myHashMap.h"
#include "fec.h"

using namespace std;

class CirGate;

//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------
// TODO: Define your own data members and member functions, or classes



class CirGateV
{
public:
   CirGateV(size_t v): _v(v) {}
   CirGateV(const CirGateV& obj): _v(obj._v) {}
   ~CirGateV() {}
   
   size_t operator() () { return _v; }
   size_t operator() () const { return _v; }
   void setInv() { _v |= NEG; }
   void setFlt() { _v |= FLT; }
   void reportGateV() const;
   CirGate* getGate() const { return (CirGate*)(_v & PTR_MSK); }
   bool isInv() const { return _v & NEG; }
   bool isFlt() const { return _v & FLT; }
   size_t getGid() const;
   size_t getLitId() const;

   static const size_t NEG     = 0X1;
   static const size_t FLT     = 0x2;
   static const size_t PTR_MSK = ((~(size_t(0)) >> 2) << 2);

private:
   size_t _v;
   
};



class CirGate
{
public:
   CirGate(size_t id, size_t line): _id(id), _line(line), _ref(0) {}
   virtual ~CirGate() {}

   // Basic access methods
   string getTypeStr() const { return ""; }
   unsigned getLineNo() const { return 0; }

   // Printing functions
   virtual string getType() const { return ""; }
   virtual string getName() const { return ""; }
   virtual void printGate() const = 0;
   virtual void genDfsList(vector<CirGate*>&) = 0;
   virtual void genConnection(size_t) { }
   virtual void deleteUnused(vector<CirGate*>&) { }
   virtual void updateFanin(vector<CirGate*>&) { }
   virtual bool isPi() const { return false; }
   virtual bool isPo() const { return false; }
   virtual bool isAig() const { return false; }
   virtual bool isUndef() const { return false; }
   virtual bool isConst() const { return false; }
   virtual void backwardDfs(int, int, bool);
   virtual void forwardDfs(int, int, bool);
   virtual bool isOutsGlobalRef() const;
   virtual bool isIn0Inv() const { return false; }
   virtual bool isIn1Inv() const { return false; }
   virtual void setIn0(const size_t in0) {}
   virtual void setIn1(const size_t in1) {}
   virtual bool merge(unordered_map<HashKey, CirGate*, HashString>&) { return false; }
   virtual void simulate(const CirSimV& simV) {} 
   virtual size_t getSimResult() const { return 0; }
   virtual void setSim0(const CirSimV& simV) { }
   virtual void setSim1(const CirSimV& simV) { }

   virtual CirGate* getIn0Gate() const { return 0; }
   virtual CirGate* getIn1Gate() const { return 0; }
   
   virtual size_t getId() const { return _id; }
   virtual size_t getLine() const { return _line; }
   virtual size_t numFltFin() const { return 0; }
   virtual size_t getIn0GateV() const { return 0; }
   virtual size_t getIn1GateV() const { return 0; }
   virtual size_t getIn0LitId() const { return 0; }
   virtual size_t getIn1LitId() const { return 0; }

   virtual void reconnect(size_t);
   size_t numFout() const { return _foutList.size(); }
   void addFout(CirGateV v) { _foutList.push_back(v); }
   void clearFouts() { _foutList.clear(); }
   void reportGate() const;
   void reportFanin(int level);
   void reportFanout(int level);
   void deleteFout(CirGate*);
   

   void setToGlobalRef() { _ref = _globalRef; }
   bool isGlobalRef() const { return _ref == _globalRef; }
   
   vector<CirGateV>& getFoutList() { return _foutList; } 
   static void incrementGlobalRef() { _globalRef++; }

   // function for boolean matching
   virtual void collectStrucSupp() {};
   const set<CirGate*>::iterator suppBegin() { return _strucSupp.begin(); }
   const set<CirGate*>::iterator suppEnd() { return _strucSupp.end(); }
   void setVar(int v)  { _var = v; }
   int  getVar() const { return _var; }
   virtual size_t piToPoGateCount();

private:
   size_t _id;
   size_t _line;
   size_t _ref;
   static size_t _globalRef;
   

protected:
   vector<CirGateV> _foutList;
   set<CirGate*>    _strucSupp;
   int              _var;
   // CirSimV _sim0;
   // CirSimV _sim1;
};

class CirPiGate : public CirGate
{
public:
   CirPiGate(size_t id, size_t line)
   : CirGate(id, line), _name(""), _sim0(0) { }
   ~CirPiGate() {}

   void printGate() const;
   void setName(const string& name) { _name = name; }
   void genDfsList(vector<CirGate*>&);
   void updateFanin(vector<CirGate*>&) { clearFouts(); }
   bool isPi() const { return true; }
   string getType() const { return "PI"; }
   string getName() const { return _name; }
   void setSim0(const CirSimV& sim0) { _sim0 = sim0; }
   void simulate(const CirSimV&); 
   void flipToOne(const size_t& val) { _sim0 |= val; }
   void flipToZero(const size_t& val) { _sim0 &= val; }
   size_t getSimResult() const { return _sim0(); }
   size_t getSimBit(const size_t i) { return (_sim0() >> i) & const1; }

   // function for boolean matching
   void collectStrucSupp();

private:
   string  _name;
   CirSimV _sim0;
};

class CirPoGate : public CirGate
{
public:
   CirPoGate(size_t id, size_t line, size_t in0)
   :CirGate(id, line), _in0(in0), _name("") {}

   ~CirPoGate() {}

   void genConnection(size_t);
   void genDfsList(vector<CirGate*>&);
   void backwardDfs(int, int, bool);
   void setName(const string& name) { _name = name; }
   void updateFanin(vector<CirGate*>&);
   bool merge(unordered_map<HashKey, CirGate*, HashString>&);
   void printGate() const;
   void setIn0(const size_t in0) { _in0 = in0; }
   bool isPo() const { return true; }
   bool isIn0GlobalRef() { return _in0.getGate()->isGlobalRef(); }
   bool isIn0Inv() const { return _in0.isInv(); }
   size_t numFltFin() const { return _in0.isFlt(); }
   size_t getIn0LitId() const { return _in0.getLitId(); }
   string getType() const { return "PO"; }
   string getName() const { return _name; }
   size_t getSimBit(const size_t i) { return (_sim0() >> i) & const1; }
   void setSim0(const CirSimV& sim0) { _sim0 = sim0; }
   void simulate(const CirSimV&);
   CirGate* getIn0Gate() const { return _in0.getGate(); }

   // function for boolean matching
   void collectStrucSupp();
   void printStrucSupp() const;
   size_t piToPoGateCount();

private:
   CirGateV _in0;
   CirSimV  _sim0;
   string  _name;
   
};

class CirAigGate : public CirGate
{
public: 
   CirAigGate(size_t id, size_t line, size_t in0, size_t in1)
   :CirGate(id, line), _in0(in0), _in1(in1), _sim0(0), _sim1(0) {}

   ~CirAigGate() {}

   void genConnection(size_t);
   void genDfsList(vector<CirGate*>&);
   void deleteUnused(vector<CirGate*>&);
   void backwardDfs(int, int, bool);
   void updateFanin(vector<CirGate*>&);
   bool merge(unordered_map<HashKey, CirGate*, HashString>&);
   void printGate() const;
   void setIn0(const size_t in0) { _in0 = in0; }
   void setIn1(const size_t in1) { _in1 = in1; }
   bool isAig() const { return true; }
   bool isIn0GlobalRef() { return _in0.getGate()->isGlobalRef(); }
   bool isIn1GlobalRef() { return _in1.getGate()->isGlobalRef(); }
   bool isIn0Inv() const { return _in0.isInv(); }
   bool isIn1Inv() const { return _in1.isInv(); }
   size_t numFltFin() const { return _in0.isFlt()+_in1.isFlt(); }
   size_t getIn0GateV() const { return _in0(); }
   size_t getIn1GateV() const { return _in1(); }; 
   size_t getIn0LitId() const { return _in0.getLitId(); }
   size_t getIn1LitId() const { return _in1.getLitId(); }
   string getType() const { return "AIG"; }
   void setSim0(const CirSimV& sim0) { _sim0 = sim0; }
   void setSim1(const CirSimV& sim1) { _sim1 = sim1; }
   void simulate(const CirSimV&);
   CirGate* getIn0Gate() const { return _in0.getGate(); }
   CirGate* getIn1Gate() const { return _in1.getGate(); }
   size_t getSimResult() const { return (_sim0() & _sim1()); }

   // function for boolean matching
   void collectStrucSupp();

private:
   CirGateV _in0;
   CirGateV _in1;
   CirSimV _sim0;
   CirSimV _sim1;
};

class CirUndefGate : public CirGate
{
public:
   CirUndefGate(size_t id, size_t line): CirGate(id, line), _sim0(0) {}
   ~CirUndefGate() {}

   bool isUndef() const { return true; }
   void genDfsList(vector<CirGate*>&) {}
   void deleteUnused(vector<CirGate*>&);
   void updateFanin(vector<CirGate*>&) { clearFouts(); }
   void printGate() const {};
   string getType() const { return "UNDEF"; }

   // function for boolean matching
   void collectStrucSupp();

private:
   CirSimV _sim0;
};

class CirConstGate : public CirGate
{
public:
   CirConstGate(size_t id, size_t line): CirGate(id, line), _sim0(0) {}
   ~CirConstGate() {}

   bool isConst() const { return true; }
   void genDfsList(vector<CirGate*>&);
   void updateFanin(vector<CirGate*>&) { clearFouts(); }
   void printGate() const { cout << getType() << "\n"; };
   string getType() const { return "CONST0"; }

   // function for boolean matching
   void collectStrucSupp();

private:
   CirSimV _sim0;
};



#endif // CIR_GATE_H
