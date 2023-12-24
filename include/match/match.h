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
        void parseBus();

        int getScore(const vector<vector<Var>>&);

        void solve();
        void outputSolverInit(vector<vector<Var>>&, vector<vector<Var>>&, Var&);
        void inputSolverInit(vector<vector<Var>>&, vector<vector<Var>>&, vector<vector<Var>>&);
        void printOutputSolverValue(vector<vector<Var>>&, vector<vector<Var>>&, Var&);
        void printInputSolverValue(vector<vector<Var>>&, vector<vector<Var>>&, vector<vector<Var>>&);

        void write();

    private:
        char*                           _outFile;
        vector<vector<vector<string>>>  _bus;
        vector<vector<vector<size_t>>>  _piBus;
        vector<vector<vector<size_t>>>  _poBus;
        SatSolver                       _outputSolver;     // M_O (c, d, ...)
        SatSolver                       _inputSolver;      // M_I (a, b, ...)
        int                             _optimal;
        int                             _K;
        vector<vector<int>>             _resultMo;
        vector<vector<int>>             _resultMi;

};

#endif  // MATCHING_H