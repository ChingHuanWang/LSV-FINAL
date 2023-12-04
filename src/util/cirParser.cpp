

#include "cirParser.h"
#include <sstream>
#include <fstream>


void genVerFile(string inFilePath, string outFileName)
{
    ofstream outFile(outFileName);
    ifstream inFile(inFilePath);
    string line, tmp;
    while(getline(inFile, line)) {
        stringstream ss(line);
        ss >> tmp;
        if (tmp == "module" || tmp == "endmodule" || tmp == "input" || tmp == "output" || tmp == "wire") 
            outFile << line << endl;
        else {
            outFile << string(4, ' ') << tmp << " ";
            ss >> tmp;
            while(ss >> tmp) {
                if (tmp != ";")
                    outFile << tmp << " ";
                else
                    outFile << tmp << endl;
            }
        }
        ss.clear();
    }
    outFile.close();
    inFile.close();
    return;
}



#if defined(ABC_NAMESPACE)
namespace ABC_NAMESPACE
{
#elif defined(__cplusplus)
extern "C"
{
#endif

// procedures to start and stop the ABC framework
// (should be called before and after the ABC procedures are called)
void   Abc_Start();
void   Abc_Stop();

// procedures to get the ABC framework and execute commands in it
typedef struct Abc_Frame_t_ Abc_Frame_t;

Abc_Frame_t * Abc_FrameGetGlobalFrame();
int    Cmd_CommandExecute( Abc_Frame_t * pAbc, const char * sCommand );

#if defined(ABC_NAMESPACE)
}
using namespace ABC_NAMESPACE;
#elif defined(__cplusplus)
}
#endif


int genAigFile(string verFile)
{
    Abc_Frame_t * pAbc;
    const char * pFilePath;
    char Command[1000];

    pFilePath = verFile.c_str();

    //////////////////////////////////////////////////////////////////////////
    // start the ABC framework
    Abc_Start();
    pAbc = Abc_FrameGetGlobalFrame();

    //////////////////////////////////////////////////////////////////////////
    // read the file
    sprintf( Command, "read %s", pFilePath );
    if ( Cmd_CommandExecute( pAbc, Command ) )
    {
        fprintf( stdout, "Cannot execute command \"%s\".\n", Command );
        return 0;
    }

    //////////////////////////////////////////////////////////////////////////
    // strash
    sprintf( Command, "strash" );
    if ( Cmd_CommandExecute( pAbc, Command ) )
    {
        fprintf( stdout, "Cannot execute command \"%s\".\n", Command );
        return 0;
    }

    //////////////////////////////////////////////////////////////////////////
    // write_aiger
    string aigFile = verFile.substr(0, verFile.find_first_of('.')) + ".aig";
    string cmd = string("write_aiger -s ") + string(aigFile);
    sprintf( Command, cmd.c_str() );
    if ( Cmd_CommandExecute( pAbc, Command ) )
    {
        fprintf( stdout, "Cannot execute command \"%s\".\n", Command );
        return 0;
    }
    return 0;
}
