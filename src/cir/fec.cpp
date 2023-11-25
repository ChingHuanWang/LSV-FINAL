#include "fec.h"
#include "cirGate.h"

FecGrp::FecGrp(const vector<CirGate*>& dfsList) 
{
    // cout << "aigList size = " << aigList.size() << endl;
    // cout << "vec size = " << _vec.size() << endl;
    for (size_t i = 0; i < dfsList.size(); i++)
        if (dfsList[i]->isAig()) 
            _vec.push_back(dfsList[i]->getId()*2); 
}