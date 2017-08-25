#ifndef __CM_CONFIG_FILE_PARSER_H__
#define __CM_CONFIG_FILE_PARSER_H__

#include <map>
#include <string>

using namespace std;

typedef  map<string,map<string,string>>  SectionMap;

class CM_ConfigFileParser
{
public:
	CM_ConfigFileParser(){};
	CM_ConfigFileParser(const char * fileName);
	~CM_ConfigFileParser();

	int LoadFile();
	int ParseLine(char* line);
	char* TrimSpace(char* name);
	map<string,string>  GetSection(const char *secname);
	string GetValue(const char *secname,const char *keyname);

private:
	string m_sFileName;
	string m_sCurrentSectionName;
	map<string,map<string,string> >  m_mapFileSection;


};





#endif

