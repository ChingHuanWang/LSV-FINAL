/****************************************************************************
  FileName     [ cirMgr.h ]
  PackageName  [ cir ]
  Synopsis     [ Define circuit manager ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_MGR_H
#define CIR_MGR_H

#include <unordered_map>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <bitset>
#include <unordered_map>
#include <list>
#include "sat.h"

using namespace std;

#include "cirDef.h"
#include "fec.h"


extern CirMgr* cirMgr;

// TODO: Define your own data members and member functions
class CirObj
{
public:
   CirObj(size_t objIdx): _objIdx(objIdx), _maxVarIdx(0), _const(0), _simLog(0), _fecGrps(new vector<FecGrp>) 
   { _sat.initialize(); }
   ~CirObj() { deleteCircuit(); }

   // Access functions
   // return '0' if "gid" corresponds to an undefined gate.
   CirGate* getGate(unsigned gid) const { return _idList[gid]; }


   // Member functions about circuit construction
   bool readCircuit(const string&);

   // Member functions about circuit optimization
   void sweep();
   void optimize();

   // Member functions about simulation
   void simulate();
   void randomSim();
   void fileSim(ifstream&);
   void setSimLog(ofstream* logFile) { _simLog = logFile; }
   void writeLog(size_t);
   void collectFecGrps();


   // Member functions about fraig
   void strash();
   void printFEC() const;
   void fraig();

   // Member functions about circuit reporting
   void printSummary() const;
   void printNetlist() const;
   void printPIs() const;
   void printPOs() const;
   void printFloatGates() const;
   void printFECPairs() const;
   void writeAag(ostream&) const;
   void writeGate(ostream&, CirGate*) const;

   void parsePi(ifstream&, size_t);
   void parsePo(ifstream&, size_t);
   void parseAig(ifstream&, size_t);
   void parseName(ifstream&);
   void parseFlt();
   void parseUnused();
   void genConnection();
   void genDfsList();
   void deleteCircuit();
   size_t checkGate(size_t);

   // functions for boolean matching
   size_t getPoNum() const { return _poList.size(); }
   size_t getPiNum() const { return _piList.size(); }
   size_t getGateNum() const { return _piList.size() + _aigList.size() + _poList.size(); }
   vector<CirPiGate*> getPiList() const { return _piList; }
   vector<CirPoGate*> getPoList() const { return _poList; }
   vector<CirAigGate*> getAigList() const { return _aigList; }
   vector<vector<size_t>> getFuncSupp() const { return _funcSupp; }
   vector<vector<size_t>> getInvFuncSupp() const { return _invFuncSupp; }
   void getRedundant(vector<size_t>&, vector<size_t>&, vector<vector<bool>>&);
   void collectStrucSupp();
   void collectFuncSupp();
   void collectInvFuncSupp();
   void printStrucSupp() const;
   void printFuncSupp() const;
   void printInvFuncSupp() const;

   // functions for constructing unate table and symmetry table
   void collectUnate();
   void printUnate() const;
   void collectSym();
   void printSym() const;

   // member function for grouping input/output 
   void initInputGrp();
   void initOutputGrp();
   void type1Sim();
   void type2Sim();
   void type3Sim();

private:
   size_t                    _objIdx;
   size_t                    _maxVarIdx;
   CirConstGate*             _const;
   vector<CirGate*>          _idList;
   vector<CirPiGate*>        _piList;
   vector<CirPoGate*>        _poList;
   vector<CirAigGate*>       _aigList;
   vector<CirUndefGate*>     _undefList;
   vector<CirGate*>          _fltList;
   vector<CirGate*>          _unusedList;
   vector<CirGate*>          _dfsList;
   ofstream                  *_simLog;
   vector<FecGrp>*           _fecGrps;

   // member for solving unate and symmetry 
   SatSolver                                               _sat;
   vector<vector<size_t>>                                  _posUnateTable;
   vector<vector<size_t>>                                  _negUnateTable;
   vector<vector<size_t>>                                  _funcSupp;
   vector<vector<size_t>>                                  _invFuncSupp;
   vector<vector<vector<size_t>>>                          _sym;

   // memeber for grouping input/output
   // first of each vector<size_t>* is the supp size of that group
   // rest of the elements are pi/po idx 
   vector<vector<size_t>*>                                  _inputGrp;
   vector<vector<size_t>*>                                  _outputGrp;

};

class CirMgr
{

public:
   CirMgr()
   { 
      _objList.push_back(new CirObj(1)); 
      _objList.push_back(new CirObj(2));
      _const = 0;
      
   } 
   ~CirMgr()
   {
      deleCircuit();
   }
   void deleCircuit();

   // Member functions about circuit construction
   bool readCircuit(const string&, const string&);
   void recycle(CirGate* g) { _recycleList.push_back(g); }
   CirObj* getCir(size_t idx) const { return _objList[idx-1]; }
   CirConstGate* getConst0() const { return _const; }

   // member function about unate and symmetry
   void collectUnate() { _objList[0]->collectUnate(); _objList[1]->collectUnate(); }
   void printUnate() const { _objList[0]->printUnate(); _objList[1]->printUnate();}

   // member function about doing I/O grouping simulation
   void initInputGrp();
   void initOutputGrp();
   void inputGrpRefineSim();
   void outputGrpRefineSim();

private:
   CirConstGate*        _const;
   vector<CirObj*>      _objList;
   vector<CirGate*>     _recycleList;
   vector<vector<int>>  _aigVarList;
   
};

#endif // CIR_MGR_H
