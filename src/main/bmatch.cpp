
#include <iostream> 
#include <fstream>
#include <vector>
#include "util.h"
#include "cirMgr.h"
#include "match.h"
#include <ctime>
#include <iomanip>
#include <chrono>

using namespace std;
using namespace std::chrono;

int main(int argc, char** argv)
{
    Match *match = new Match(argv[2]);
    vector<string> cirFileList;

    match->parseInput(argv[1], cirFileList);
    cirMgr->readCircuit(cirFileList[0], cirFileList[1]);
    match->parseBus();
    
    

    cirMgr->getCir(1)->collectStrucSupp();
    cirMgr->getCir(2)->collectStrucSupp();
    cirMgr->getCir(1)->collectFuncSupp();
    cirMgr->getCir(2)->collectFuncSupp();
    cirMgr->getCir(1)->collectInvFuncSupp();
    cirMgr->getCir(2)->collectInvFuncSupp();
    cirMgr->getCir(1)->piLongestPath();
    cirMgr->getCir(2)->piLongestPath();
    // cirMgr->getCir(1)->poLongestPath();
    // cirMgr->getCir(2)->poLongestPath();
    for (size_t i = 0; i < 2; ++i) {
        cirMgr->getCir(i + 1)->collectSym();
    }
    return 0;
    vector<size_t> long1 = cirMgr->getCir(1)->getPiLongestPathList();
    vector<size_t> long2 = cirMgr->getCir(2)->getPiLongestPathList();

    vector<vector<size_t>> inv1 = cirMgr->getCir(1)->getInvFuncSupp();
    vector<vector<size_t>> inv2 = cirMgr->getCir(2)->getInvFuncSupp();
    // vector<size_t> inv1sum(cirMgr->getCir(1)->getPoNum(), 0);
    // vector<size_t> inv2sum(cirMgr->getCir(2)->getPoNum(), 0);

    // for (size_t i = 0 ; i < inv1.size() ; i++) {
    //     inv1sum[inv1[i].size()-1]++;
    // }

    // for (size_t i = 0 ; i < inv2.size() ; i++) {
    //     inv2sum[inv2[i].size()-1]++;
    // }

    // for (size_t i = 0 ; i < inv1sum.size() ; i++) {
    //     cout << i+1 << " : " << inv1sum[i] << " " << inv2sum[i] << endl;
    // }

    // for (size_t i = 0 ; i < long1.size() ; i++) {
    //     cout << "max len = " << long1[i] << ", invsupp num = " << inv1[i].size() << endl;
    // }

    // for (size_t i = 0 ; i < long2.size() ; i++) {
    //     cout << "max len = " << long2[i] << ", invsupp num = " << inv2[i].size() << endl;
    // }

    // vector<size_t> long1 = cirMgr->getCir(1)->getPoLongestPathList();
    // vector<size_t> long2 = cirMgr->getCir(2)->getPoLongestPathList();
    // vector<vector<CirGate*>> func1 = cirMgr->getCir(1)->getStrucSupp();
    // vector<vector<CirGate*>> func2 = cirMgr->getCir(2)->getStrucSupp();

    // for (size_t i = 0 ; i < long1.size() ; i++) {
    //     cout << "path len = " << long1[i] << ", " << "func supp num = " << func1[i].size() << endl;
    //     if (func1[i].size() == 1) cout << "fanout num = " << func1[i][0]->numFout() << endl;
    // }

    // for (size_t i = 0 ; i < long2.size() ; i++) {
    //     cout << "path len = " << long2[i] << ", " << "func supp num = " << func2[i].size() << endl;
    //     if (func2[i].size() == 1) cout << "fanout num = " << func2[i][0]->numFout() << endl;;
    // }
    
    // cirMgr->getCir(1)->printStrucSupp();
    // cirMgr->getCir(2)->printStrucSupp();
    // cirMgr->getCir(1)->printFuncSupp();
    // cirMgr->getCir(2)->printFuncSupp();

    // cirMgr->getCir(1)->collectInvFuncSupp();
    // cirMgr->getCir(2)->collectInvFuncSupp();

    
    auto start = high_resolution_clock::now();
    match->solve();
    auto end = high_resolution_clock::now();
    auto diff = duration_cast<microseconds>(end - start);
    cout << "solve time: " << setprecision(6) << fixed << diff.count() / 1e6 << "s" << endl;
    // match->printMatch();
}
