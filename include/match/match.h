#ifndef MATCHING_H
#define MATCHING_H
#include <vector>
#include <string>
#include "sat.h"

class Match {
    public:
        Match() {
            _outputSolver.initialize();
            _inputSolver.initialize();
            _outFile = 0;
        }

        void parseInput(char*, vector<string>&);

        int getScore(const vector<vector<Var>>&);

        void outputConstraint();
        void solve();
        void outputSolverInit();
        void inputSolverInit();

        void write();

    private:
        char*                           _outFile;
        vector<vector<vector<string>>>  _bus;
        SatSolver                       _outputSolver;     // M_O (c, d, ...)
        SatSolver                       _inputSolver;      // M_I (a, b, ...)
        int                             _optimal;
        int                             _K;
        vector<vector<int>>             _resultMo;
        vector<vector<int>>             _resultMi;

};

#endif  // MATCHING_H