#ifndef OUTPUTSOLVER_H
#define OUTPUTSOLVER_H
#include <vector>
#include <string>
#include "sat.h"
#include "inputSolver.h"

class OutputSolver {
    public:
        OutputSolver() {
            _solver.initialize();
        }

        // new variable
        void generateMoVar(size_t, size_t);
        void generateMiVar(size_t, size_t);
        void generateMboVar(size_t, size_t);
        void generateMbiVar(size_t, size_t);

        // constraints
        void init();
        void poFuncSuppConstraint();
        void piFuncSuppConstraint();
        void poBusConstraint();
        void piBusConstraint();
        void forbidCounterExample(InputSolver&);
        void forbidCurrentMapping();

        // utilities
        // void printValues();
        void setK(size_t k) { _k = k; }
        size_t getK() { return _k; }
        size_t calScore();

        // solve
        bool solve() { return _solver.assumpSolve(); }

        // assume
        void assumeRelease() { _solver.assumeRelease(); }
        void assumeBus(bool useBus) { _solver.assumeProperty(_useBus, useBus); };

        // get value
        vector<vector<bool>> getMiValue();
        vector<vector<bool>> getMoValue();


    private:
        SatSolver               _solver;
        vector<vector<Var>>     _mo;
        vector<vector<Var>>     _mi;
        vector<vector<Var>>     _mbo;
        vector<vector<Var>>     _mbi;
        Var                     _useBus;
        size_t                  _k;

};


#endif  // OUTPUTSOLVER_H