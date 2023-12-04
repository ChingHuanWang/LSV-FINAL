
#include <iostream> 
#include <fstream>
#include <vector>
#include "util.h"
#include "cirMgr.h"

using namespace std;

int main(int argc, char** argv)
{
    vector<string> cirFileList;
    parseInput(argv[1], cirFileList);
    cirMgr->readCircuit(cirFileList[0], cirFileList[1]);
    cirMgr->getCir(1)->printNetlist();
    cirMgr->getCir(2)->printNetlist();


}
