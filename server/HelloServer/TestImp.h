#ifndef _TEST_IMP_H_
#define _TEST_IMP_H_


#include "util/Servant.h"
#include "util/JceCurrent.h"
/**
 *
 *
 */

using namespace taf;
class TestImp : public taf::Servant
{
public:
	
    virtual ~TestImp()
    {
    }
    
    virtual void initialize();
    
    virtual void destroy();
    virtual int doRequest( JceCurrentPtr current, vector<char> &buffer );

};

/////////////////////////////////////////////////////
#endif
