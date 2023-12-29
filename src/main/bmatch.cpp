
#include <iostream> 
#include <fstream>
#include <vector>
#include "util.h"
#include "cirMgr.h"
#include "match.h"
#include <ctime>
#include <iomanip>

using namespace std;

int main(int argc, char** argv)
{
    Match *match = new Match;
    vector<string> cirFileList;
    time_t start, end;

    match->parseInput(argv[1], cirFileList);
    cirMgr->readCircuit(cirFileList[0], cirFileList[1]);
    // match->parseBus();
    cirMgr->getCir(1)->collectStrucSupp();
    cirMgr->getCir(2)->collectStrucSupp();

    
    // cirMgr->getCir(1)->collectFuncSupp();
    // cirMgr->getCir(2)->collectFuncSupp();

    cirMgr->getCir(1)->printStrucSupp();
    cirMgr->getCir(2)->printStrucSupp();
    // cirMgr->getCir(1)->printFuncSupp();
    // cirMgr->getCir(2)->printFuncSupp();

    // cirMgr->getCir(1)->collectInvFuncSupp();
    // cirMgr->getCir(2)->collectInvFuncSupp();

    
    // start = time(NULL);
    // match->solve();
    // end = time(NULL);
    // double diff = difftime(end, start);
    // cout << "solve time: " << setprecision(6) << fixed << diff << "s" << endl;

    // match->printMatch();
    cirMgr->getCir(1)->poLongestPath();
    cirMgr->getCir(2)->poLongestPath();

    // vector<size_t> long1 = cirMgr->getCir(1)->getPoLongestPathList();
    // vector<size_t> long2 = cirMgr->getCir(2)->getPoLongestPathList();

    // float avg1 = 0, avg2 = 0;
    // size_t s = 0;
    // for (size_t l : long1) s += l;
    // avg1 = (float)s / long1.size();
    // s = 0;
    // for (size_t l : long2) s += l; 
    // avg2 = (float)s / long2.size();

    // cout << avg1 << " " << avg2 << endl;
    
    // if (match->checkSol()) {
    //     cout << "right sol" << endl;
    // }
    // else cout << "wrong sol" << endl;
}
