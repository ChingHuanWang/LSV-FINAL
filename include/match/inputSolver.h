#ifndef INPUTSOLVER_H
#define INPUTSOLVER_H
#include <vector>
#include <string>
#include "sat.h"

class InputSolver {
    public:
        InputSolver() {
            _solver.initialize();
        }

        // new variable
        void generateAigVar();
        void generateMoVar(size_t, size_t);
        void generateMiVar(size_t, size_t);
        void generateHVar(size_t, size_t);

        // constraints
        void init();

        // utilities
        // void printValues();

        // solve
        bool solve() { return _solver.assumpSolve(); }

        // assume
        void assumeRelease() { _solver.assumeRelease(); }
        void assumeMo(vector<vector<bool>>);
        void assumeMi(vector<vector<bool>>);

        // get value
        vector<size_t> getPiValue(size_t cirId);
        vector<size_t> getPoValue(size_t cirId);


    private:
        SatSolver               _solver;
        vector<vector<Var>>     _mo;
        vector<vector<Var>>     _mi;
        vector<vector<Var>>     _h;

};


#endif  // INPUTSOLVER_H