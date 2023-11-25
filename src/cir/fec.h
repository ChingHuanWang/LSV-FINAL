
#ifndef CIR_FEC_H
#define CIR_FEC_H

#include <vector>
#include <bitset>
#include "cirDef.h"

class CirSimV
{
public:
   CirSimV(size_t v = 0) { _simV = v; }

   size_t operator()() const { return _simV; }
   size_t operator ~() const { return ~_simV; }
   CirSimV operator &(const CirSimV& p) const {
      CirSimV v = _simV & p._simV; return v; }
   CirSimV& operator &=(const CirSimV& p) {
      _simV &= p._simV; return *this; }
   CirSimV operator =(const CirSimV& p) {
      _simV = p._simV; return *this; }
   bool operator ==(const CirSimV& p) const {
      return (_simV == p._simV); }

   CirSimV& operator |=(const size_t& val) {
      _simV |= val; return *this;
   }

   CirSimV& operator &=(const size_t& val) {
      _simV &= val; return *this;
   }
   
   
private:
   size_t _simV;
};

class HashInt
{
public:
   size_t operator() (const size_t& k) const
   {
      return k ^ (k >> 32);
   }
};

class FecGrp
{
public:
   FecGrp() {}
   FecGrp(const vector<CirGate*>&);
   FecGrp(const FecGrp& k) { _vec = k._vec; }
   ~FecGrp() {}
   size_t operator[] (const size_t i) const { return _vec[i]; }
   size_t size() const { return _vec.size(); }
   void add(size_t litId) { _vec.push_back(litId); }

private:
   vector<size_t> _vec;
};

#endif // CIR_GATE_H