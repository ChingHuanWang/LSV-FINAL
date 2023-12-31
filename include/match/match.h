#ifndef MATCHING_H
#define MATCHING_H
#include <vector>
#include <string>
#include "sat.h"
#include "outputSolver.h"
#include "inputSolver.h"

class Match {
    public:
        Match() {
            _outFile = 0;
        }
        void parseInput(char*, vector<string>&);

        void solve();
        void recordOptimalSol();
        void calOptimal();

        void write();
        // bool checkSol() const;
        // void printMatch() const;
        // bool partialSolvePoMatch(size_t, size_t, bool);
        void printMatchedMiInvFuncSupp() const;

    private:
        char*                           _outFile;
        OutputSolver                    _outputSolver;     // M_O (c, d, ...)
        InputSolver                     _inputSolver;      // M_I (a, b, ...)
        size_t                          _optimal;
        vector<vector<bool>>            _resultMo;
        vector<vector<bool>>            _resultMi;

};

#endif  // MATCHING_H