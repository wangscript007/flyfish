#include "util/cm_config.h"
#include "util/cm_string.h"
#include <string.h>
#include <iostream>
using namespace std;

CM_ConfigFileParser::CM_ConfigFileParser(const char * fileName)
{
	m_sFileName=fileName;

	m_mapFileSection.clear();

	LoadFile();

}

CM_ConfigFileParser::~CM_ConfigFileParser()
{


}

int CM_ConfigFileParser::LoadFile()
{
	if (!m_sFileName.length())
	{
		return -1;
	}

	FILE* fp = fopen(m_sFileName.c_str(), "r");
	if (!fp)
	{
		printf("can not open %s\n", m_sFileName.c_str());
		return -1;
	}

	char buf[256];
	for (;;)
	{
		buf[256]={0};
		char* p = fgets(buf, 256, fp);
		if (!p)
		{
           break;
		}

		if (0==strlen(buf)||'#' == buf[0] || '\n' == buf[0] || '\0' == buf[0])
		{
			continue;
		}

		char* ch = strchr(buf, '#');	// remove  #
		if (ch)
		{
           *ch = 0;
		}

		if (ParseLine(buf)!=0)
		{
			return -1;
		}
			
	}

	return 0;

}

map<string,string>  CM_ConfigFileParser::GetSection(const char *secname)
{
	SectionMap::iterator  it=m_mapFileSection.find(string(secname));
	if (it!=m_mapFileSection.end())
	{
		return it->second;
	}

	return map<string,string> ();

}

string CM_ConfigFileParser::GetValue(const char *secname,const char *keyname)
{
	if (NULL==secname || NULL==keyname)
	{
		return string("");
	}

	map<string,string> mapSection=GetSection(secname);

#if 0


	if (strcmp(keyname,"endpoint")==0)
	{
		for (auto item:mapSection)
		{
			cout<<item.first<< "|"<<item.second <<"|"<<endl;
		}
	}
	
#endif

	if (mapSection.empty())
	{
		return string("");
	}

	map<string,string>::iterator it=mapSection.find(string(keyname));
	if (it!=mapSection.end())
	{
		return it->second;
	}
	else
	{
		return string("");
	}
}



int  CM_ConfigFileParser::ParseLine(char* line)
{

	if (NULL==line)
	{
		return -1;
	}

	char * pstart=CM_String::trimleftright(line);

	if (NULL==pstart)
	{
		return -1;
	}

	char* pSecStart = strchr(pstart, '[');
	if (NULL!=pSecStart)
	{
	    if ('['!=pstart[0])
	    {
		   return -1;
	    }

		char* pSecEnd = strchr(pstart, ']');
		if (NULL==pSecEnd || pSecEnd==pSecStart+1 )
		{
			return -1;
		}

		int len=pSecEnd-pSecStart-1;

		if (len<=0)
		{
			return -1;
		}
		char secName[256]={0};
		strncpy(secName,pSecStart+1,len);

		m_sCurrentSectionName=secName;

	}
	

	char* pKey= strchr(pstart, '=');

	if (NULL!=pKey)
	{
		
		char key[100]={0};
		char value[100]={0};
		strncpy(key,pstart,pKey-pstart);

		//char *pk=TrimSpace(pKey+1);
		strncpy(value,pKey+1,strlen(pKey+1));

		const char *trimkey=CM_String::trimall(key);
		const char *trimvalue=CM_String::trimleftright(value);

		SectionMap::iterator it=m_mapFileSection.find(m_sCurrentSectionName);
		if (it!=m_mapFileSection.end())
		{
			((it->second))[string(trimkey)]=string(trimvalue);
#if 0
			printf("m_sCurrentSectionName=%s,trimkey=%s,trimvalue=%s\n",m_sCurrentSectionName.c_str(),trimkey,trimvalue);
#endif
			
		}
		else
		{

			m_mapFileSection[m_sCurrentSectionName].insert(make_pair(string(trimkey),string(trimvalue)));
			
		}
		
		
	}
		
	return 0;
}



char* CM_ConfigFileParser::TrimSpace(char* name)
{
	
	char* start_pos = name;
	
	char lastName[256]={0};
	int i=0;
	while(*start_pos!=0)
	{
		if ((*start_pos=='\r')||(*start_pos=='\n'))
		{
			lastName[i]=0;
			break;
		}
		

		if ((*start_pos!=' ') && (*start_pos!='\t') )
		{
			lastName[i]=*start_pos;
		}		
		i++;
		start_pos++;

		if (i>=256)
		{
			break;
		}
		
	}
	
	memset(name,0,strlen(name));
	strncpy(name,lastName,strlen(lastName));
	//*(name+strlen(lastName))=0;

	return name;
}
