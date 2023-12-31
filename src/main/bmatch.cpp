
#include <iostream> 
#include <fstream>
#include <vector>
#include "util.h"
#include "cirMgr.h"
#include "match.h"
#include <ctime>
#include <chrono>
#include <iomanip>

using namespace std;
using namespace std::chrono;

int main(int argc, char** argv)
{
    Match *match = new Match;
    vector<string> cirFileList;

    match->parseInput(argv[1], cirFileList);
    cirMgr->readCircuit(argv[1], cirFileList[0], cirFileList[1]);    

    auto start = high_resolution_clock::now();
    match->solve();
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(end - start);
    cout << "solve time: " << setprecision(6) << duration.count() / 1e6 << "s" << endl;
}
