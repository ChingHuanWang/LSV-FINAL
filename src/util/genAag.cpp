#include "genAag.h"
#include "cirParser.h"
#include <cstring>
#include <cstdio>
extern "C" int aigToAag(int argc, char** argv);

void genAagFile(string filePath)
{
    size_t start = filePath.find_last_of('/');
    start = start == string::npos ? 0 : start+1;
    string verFile = filePath.substr(start);

    genVerFile(filePath, verFile);
    genAigFile(verFile);

    string aigFile = verFile.substr(0, verFile.find_first_of('.')) + ".aig";
    string aagFile = verFile.substr(0, verFile.find_first_of('.')) + ".aag";

    char* aig = new char[aigFile.length()+1];
    char* aag = new char[aagFile.length()+1];

    strcpy(aig, aigFile.c_str());
    strcpy(aag, aagFile.c_str());

    char* v[] = { aig, aag };
    int c = 2;

    aigToAag(c, v);
    delete aig;
    delete aag;
    remove(verFile.c_str());
    remove(aigFile.c_str());
}