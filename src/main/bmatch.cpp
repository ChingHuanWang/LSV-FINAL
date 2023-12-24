
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
    // cirMgr->getCir(1)->printNetlist();
    // cirMgr->getCir(2)->printNetlist();
    // parseBus();
    cirMgr->getCir(1)->collectStrucSupp();
    cirMgr->getCir(2)->collectStrucSupp();

    
    cirMgr->getCir(1)->collectFuncSupp();
    cirMgr->getCir(2)->collectFuncSupp();

    cirMgr->getCir(1)->collectInvFuncSupp();
    cirMgr->getCir(2)->collectInvFuncSupp();

    cirMgr->getCir(1)->printInvFuncSupp();
    cirMgr->getCir(2)->printInvFuncSupp();

    // cirMgr->collectUnate();
    // cirMgr->printUnate();

    // cirMgr->getCir(1)->printStrucSupp();
    // cirMgr->getCir(2)->printStrucSupp();
    
    start = time(NULL);
    match->solve();
    match->printMatchedMiInvFuncSupp();
    end = time(NULL);
    double diff = difftime(end, start);
    cout << "solve time: " << setprecision(6) << fixed << diff << "s" << endl;
}
