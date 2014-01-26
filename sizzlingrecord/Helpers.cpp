
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

#include "Helpers.h"

#include "SC_helpers.h"
#include "client_class.h"
#include "dt_recv.h"

#include "cdll_int.h"

//---------------------------------------------------------------------------------
// Purpose: used by the GetPropOffsetFromTable func to get a specific table
//---------------------------------------------------------------------------------
RecvTable *Helpers::GetDataTable( const char *pTableName, RecvTable *pTable )
{
	if (pTable)
	{
		if ( SCHelpers::FStrEq( pTableName, pTable->GetName() ) )
		{
			return pTable;
		}
	
		int num = pTable->GetNumProps();
		for (int i = 0; i < num; i++)
		{
			RecvProp *pProp = pTable->GetProp(i);
			if (pProp)
			{
				RecvTable *pSubTable = GetDataTable( pTableName, pProp->GetDataTable() );
				if (pSubTable && SCHelpers::FStrEq(pSubTable->GetName(), pTableName))
				{
					return pSubTable;
				}
			}
		}
	}
	return NULL;
}

//---------------------------------------------------------------------------------
// Purpose: returns the specified prop from the class and table provided.
//			if prop or table not found, pointer returns NULL
//---------------------------------------------------------------------------------
RecvProp *Helpers::GetPropFromClassAndTable(IBaseClientDLL *pBaseClientDLL, const char *szClassName, const char *szTableName, const char *szPropName)
{
	ClientClass *pClass = pBaseClientDLL->GetAllClasses();
	if (pClass)
	{
		do
		{
			if ( SCHelpers::FStrEq(szClassName, pClass->GetName()) )
			{
				RecvTable *pTable = GetDataTable( szTableName, pClass->m_pRecvTable );
				if (pTable)
				{
					int numprops = pTable->GetNumProps();
					for (int i = 0; i < numprops; ++i)
					{
						RecvProp *pProp = pTable->GetProp(i);
						if (pProp && SCHelpers::FStrEq(szPropName, pProp->GetName()) )
						{
							return pProp;
						}
					}
				}
			}
			pClass = pClass->m_pNext;
		} while(pClass);
	}
	Warning("prop %s not found in %s => %s\n", szPropName, szClassName, szTableName);
	return NULL;
}

//---------------------------------------------------------------------------------
// Purpose: returns the specified prop offset relative to the table provided.
//			if offset or table not found, bErr returns true and offset returned is 0
//---------------------------------------------------------------------------------
unsigned int Helpers::GetPropOffsetFromTable(IBaseClientDLL *pBaseClientDLL, const char *pTableName, const char *pPropName)
{
	ClientClass *pClass = pBaseClientDLL->GetAllClasses();
	if (pClass)
	{
		do
		{
			RecvTable *pTable = GetDataTable( pTableName, pClass->m_pRecvTable );
			if (pTable)
			{
				int num = pTable->GetNumProps();
				for (int i = 0; i < num; i++)
				{
					RecvProp *pProp = pTable->GetProp(i);
					if ( SCHelpers::FStrEq( pPropName, pProp->GetName() ) )
					{
						return pProp->GetOffset();
					}
				}
			}
			pClass = pClass->m_pNext;
		} while(pClass);
	}
	else
	{
		Warning("GetAllClasses() returned null\n");
		return 0;
	}
	Warning("prop %s not found in %s or table name incorrect\n", pPropName, pTableName);
	return 0;
}

// gets the gamerules pointer from the datatables
CTeamplayRoundBasedRules *Helpers::GetTeamplayRoundBasedGameRulesPointer(IBaseClientDLL *pBaseClientDLL)
{
	RecvProp *pProp = GetPropFromClassAndTable(pBaseClientDLL, "CTFGameRulesProxy", "DT_TeamplayRoundBasedRulesProxy", "teamplayroundbased_gamerules_data");
	if ( pProp )
	{
		DataTableRecvVarProxyFn proxyfn = pProp->GetDataTableProxyFn();
		if ( proxyfn )
		{
			void *pGameRules = NULL;
			proxyfn( NULL, &pGameRules, NULL, 0 );
			return reinterpret_cast<CTeamplayRoundBasedRules*>(pGameRules);
		}
	}
	return NULL;
}

