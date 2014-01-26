
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

#ifndef SIZZRECORD_HELPERS_H
#define SIZZRECORD_HELPERS_H

class IBaseClientDLL;
class CTeamplayRoundBasedRules;
class RecvTable;
class RecvProp;

namespace Helpers
{
	//---------------------------------------------------------------------------------
	// Purpose: used by the GetPropOffsetFromTable func to get a specific table
	//---------------------------------------------------------------------------------
	RecvTable *GetDataTable( const char *pTableName, RecvTable *pTable );
	
	//---------------------------------------------------------------------------------
	// Purpose: returns the specified prop from the class and table provided.
	//			if prop or table not found, pointer returns NULL
	//---------------------------------------------------------------------------------
	RecvProp *GetPropFromClassAndTable(IBaseClientDLL *pBaseClientDLL, const char *szClassName, const char *szTableName, const char *szPropName);

	//---------------------------------------------------------------------------------
	// Purpose: returns the specified prop offset relative to the table provided.
	//			if offset or table not found, bErr returns true and offset returned is 0
	//---------------------------------------------------------------------------------
	unsigned int GetPropOffsetFromTable(IBaseClientDLL *pBaseClientDLL, const char *pTableName, const char *pPropName);
	
	//---------------------------------------------------------------------------------
	// Purpose: gets the gamerules pointer from the datatables
	//---------------------------------------------------------------------------------
	CTeamplayRoundBasedRules *GetTeamplayRoundBasedGameRulesPointer(IBaseClientDLL *pBaseClientDLL);
}

#endif // SIZZRECORD_HELPERS_H
