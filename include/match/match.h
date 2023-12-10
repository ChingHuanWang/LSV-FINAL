#ifndef MATCHING_H
#define MATCHING_H
#include <vector>
#include <string>
#include "sat.h"

class Match {
    public:
        void parseInput(char*, vector<string>&);

        void solve();

    private:
        vector<vector<vector<string>>>  _bus;
        SatSolver                       *_outputSolver;     // M_O (c, d, ...)
        SatSolver                       *_inputSolver;      // M_I (a, b, ...)

};

#endif  // MATCHING_H