
#include <iostream> 
#include <fstream>
#include <vector>
#include "util.h"
#include "cirMgr.h"
#include "src/base/abc/abc.h"
#include "src/base/abc/abc.h"
#include "src/base/main/main.h"
#include "src/base/main/mainInt.h"
#include "src/bdd/cudd/cudd.h"
#include "src/sat/cnf/cnf.h"
// #include "base/main/main.h"
// #include "base/main/mainInt.h"
// #include "bdd/cudd/cudd.h"
// #include "sat/cnf/cnf.h"

typedef struct Abc_Frame_t_ Abc_Frame_t;
Abc_Frame_t * Abc_FrameGetGlobalFrame();
int    Cmd_CommandExecute( Abc_Frame_t * pAbc, const char * sCommand );

using namespace std;

int main(int argc, char** argv)
{
    // vector<string> cirFileList;
    // parseInput(argv[1], cirFileList);
    // cirMgr->readCircuit(cirFileList[0], cirFileList[1]);
    // cirMgr->getCir(1)->printNetlist();
    // cirMgr->getCir(2)->printNetlist();
    // Abc_Obj_t* pObj = 0;
    // Abc_Ntk_t* pNtk = 0;

    Abc_Frame_t* pAbc = 0;
    pAbc = Abc_FrameGetGlobalFrame();
    char Command[1000];

    sprintf( Command, "read %s", argv[1] );
    if ( Cmd_CommandExecute( pAbc, Command ) )
    {
        fprintf( stdout, "Cannot execute command \"%s\".\n", Command );
        return 0;
    }
    Abc_Ntk_t* pNtk = Abc_FrameReadNtk(pAbc);

}
