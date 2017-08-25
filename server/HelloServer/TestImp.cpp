#include "TestImp.h"


using namespace std;
using namespace taf;



//////////////////////////////////////////////////////
void TestImp::initialize()
{
    //initialize servant here:
    //...
}

//////////////////////////////////////////////////////
void TestImp::destroy()
{
    //destroy servant here:
    //...
}


int TestImp::doRequest( JceCurrentPtr current, vector<char> &buffer )
{
    //current->setResponse( false );
    vector<char> rebuffer = current->getRequestBuffer();
    auto iType = rebuffer.back();
    rebuffer.pop_back();
    
	buffer={'o','k'};

	return 0;
}
