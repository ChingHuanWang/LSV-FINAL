
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
    cirMgr->getCir(1)->printNetlist();
    cirMgr->getCir(2)->printNetlist();
    match->parseBus();
    cirMgr->getCir(1)->collectStrucSupp();
    cirMgr->getCir(2)->collectStrucSupp();

    // cirMgr->getCir(1)->collectFuncSupp();
    // cirMgr->getCir(2)->collectFuncSupp();
    // cirMgr->getCir(1)->printFuncSupp();
    // cirMgr->getCir(2)->printFuncSupp();

    // cirMgr->getCir(1)->collectSym();
    // cirMgr->getCir(2)->collectSym();
    // cirMgr->getCir(1)->printSym();
    // cirMgr->getCir(2)->printSym();
    // getchar();
    // cirMgr->getCir(1)->collectFuncSupp();
    // cirMgr->getCir(2)->collectFuncSupp();

    // cirMgr->collectUnate();
    // cirMgr->printUnate();

    // cirMgr->getCir(1)->printStrucSupp();
    // cirMgr->getCir(2)->printStrucSupp();
    
    start = time(NULL);
    match->solve();
    end = time(NULL);
    double diff = difftime(end, start);
    cout << "solve time: " << setprecision(6) << fixed << diff << "s" << endl;
}
