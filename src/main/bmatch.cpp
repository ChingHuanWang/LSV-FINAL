
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
    
    start = time(NULL);
    match->solve();
    end = time(NULL);
    double diff = difftime(end, start);
    cout << "solve time: " << setprecision(6) << fixed << diff << "s" << endl;


}
