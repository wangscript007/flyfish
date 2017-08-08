#include "util/cm_string.h"
#include <string.h>



//去掉行中的所有空格
char* CM_String::trimall(char* line)
{
	char* start_pos = line;
	
	char lastline[256]={0};
	int i=0;
	while(*start_pos!=0)
	{
		if ((ISEOF(*start_pos)))
		{
			lastline[i]=0;
			break;
		}
		

		if ((*start_pos!=' ') && (*start_pos!='\t') )
		{
			lastline[i]=*start_pos;
		}		
		i++;
		start_pos++;

		if (i>=256)
		{
			break;
		}
		
	}

	if (strlen(lastline))
	{
		memset(line,0,strlen(line));
		strncpy(line,lastline,strlen(lastline));
	}
	


	return line;
}


//去掉行中头和尾的空格
char* CM_String::trimleftright(char* line)
{
	char        *Tail, *Head;

	for ( Tail = line + strlen( line ) - 1; Tail >= line; Tail -- )
	{
		if ( !ISSPACE( *Tail ) )
			break;
	}
		

	for ( Head = line; Head <= Tail; Head ++ )
	{
		if ( !ISSPACE( *Head ) )
			break;
	}
		
	if ( Head != line )
	{
		memmove( line, Head, ( Tail - Head + 2 ) * sizeof( char ) );
	}
		
	return line;
}

//去掉行中头和尾的空格
char* CM_String::trimlr(char* _s)
{
	int len;
	char* end;

	/* Null pointer, there is nothing to do */
	if (!_s) return _s;
	if (!strlen(_s)) return _s;

	/* Remove spaces and tabs from the begining of string */
	while (ISSPACE(*_s)) _s++;
	if (ISEOF(*_s))
	{
		return NULL;
	}

	len = strlen(_s);

	end = _s + len - 1;

	/* Remove trailing spaces and tabs */
	while (ISSPACE(*end)) end--;
	if (end != (_s + len - 1)) 
	{
		*(end+1) = '\0';
	}

	return _s;
}


char* CM_String::tolower(char* _s)
{
	char *tmp=_s;

    if (!tmp || !strlen(tmp))
    {
		return _s;
    }

	while(*tmp)
	{
		if (*tmp>='A' && *tmp<='Z')
		{
			*tmp+=32;
		}

		tmp++;
	}

	return _s;
}

char* CM_String::touper(char* _s)
{
	char *tmp=_s;

    if (!tmp || !strlen(tmp))
    {
		return _s;
    }

	while(*tmp)
	{
		if (*tmp>='a' && *tmp<='z')
		{
			*tmp-=32;
		}

		tmp++;
	}

	return _s;
}


string CM_String::strtolower(const string &str)
{

	char strtmp[256]={0};

	strncpy(strtmp,str.c_str(),str.length());

	tolower(strtmp);

	return string(strtmp);
}


string CM_String::strtoupper(const string &str)
{

	char strtmp[256]={0};

	strncpy(strtmp,str.c_str(),str.length());

	touper(strtmp);

	return string(strtmp);
}