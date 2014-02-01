
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

#include "SizzlingProtect.h"
#include "convar.h"

static bool IsValidString( const char **pChar );

bool SizzlingProtect::IsChatExploit( const CCommand &args, int ClientCommandIndex )
{
	const char *pChar = V_strstr( args.ArgS(), "%" );
	while ( pChar )
	{
		if ( !IsValidString(&pChar) )
		{
			return true;
		}
		pChar = V_strstr( pChar, "%" );
	}
	return false;
}

static bool IsValidString( const char **pChar )
{
	if ( *pChar )
	{
		while ( **pChar != '\0' )
		{
			switch ( **pChar )
			{
			case ' ':
			case '\n':
			case '%':
				++(*pChar);
				continue;
			case 'n':
			case 'N':
			case 's':
			case 'S':
				return false;
			default:
				return true;
			}
		}
	}
	return true;
}
