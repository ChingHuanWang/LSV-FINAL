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
        }

        void parseInput(char*, vector<string>&);

        int getScore(const vector<vector<Var>>&);
        void solve();

    private:
        vector<vector<vector<string>>>  _bus;
        SatSolver                       _outputSolver;     // M_O (c, d, ...)
        SatSolver                       _inputSolver;      // M_I (a, b, ...)
        int                             _optimal;
        int                             _K;

};

#endif  // MATCHING_H