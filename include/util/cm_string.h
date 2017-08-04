#ifndef  __CM_STRING__
#define  __CM_STRING__
#include <string>

using namespace std;
//#define        ISSPACE(x)        ((x)==' '||(x)=='\t'||(x)=='\r'||(x)=='\n'||(x)=='\f'||(x)=='\b')
#define        ISSPACE(x)        ((x)==' '||(x)=='\t')
#define        ISEOF(x)        ((x)=='\r'||(x)=='\n')

class CM_String
{
	static char* trimall(char* line);
	static char* trimlr(char* line);
	static char* tolower(char* _s);
	static char* touper(char* _s);
	static string strtolower(const string &str);
	static string strtoupper(const string &str);
};

#endif