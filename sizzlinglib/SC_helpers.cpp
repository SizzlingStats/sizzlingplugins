
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

//

#include "SC_helpers.h"
#include "ServerPluginHandler.h"
#include "VFuncs.h"

#include "igameevents.h"
#include "eiface.h"

#include "steam/steamclientpublic.h"
#include "UserIdTracker.h"

#include "basehandle.h"
#include "SizzPluginContext.h"
#include "TFTeamWrapper.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern IServerGameDLL	*pServerDLL;

namespace SCHelpers
{
	CBaseEntity *EdictToBaseEntity( edict_t *pEdict )
	{
		if (pEdict)
		{
			IServerUnknown *pUnk = pEdict->GetUnknown();
			if (pUnk)
			{
				return pUnk->GetBaseEntity();
			}
		}
		return nullptr;
	}

	int EntIndexFromEdict( const edict_t *pEdict )
	{
		int ent_index = -1;
		if (pEdict)
		{
			ent_index = pEdict->m_EdictIndex;
		}
		return ent_index;
	}

	const char *GetClassName( CBaseEntity * const pEnt )
	{
		static uint32 classname_offset = GetOffsetForDatamapVar(pEnt, "m_iClassname");
		return *ByteOffsetFromPointer<const char**>(pEnt, classname_offset);
	}

	// gets the blu and red team entities and puts them in the pointers passed in
	void GetTeamEnts( CSizzPluginContext *context, CBaseEntity **ppBluTeam, CBaseEntity **ppRedTeam )
	{
		if (context && ppBluTeam && ppRedTeam)
		{
			*ppBluTeam = NULL;
			*ppRedTeam = NULL;

			edict_t *pTeamEdict = context->GetEntityByClassName("CTFTeam", context->GetMaxClients());
			while (pTeamEdict && (!(*ppBluTeam) || !(*ppRedTeam)))
			{
				CBaseEntity *pTeam = SCHelpers::EdictToBaseEntity(pTeamEdict);
				if (pTeam)
				{
					CTFTeamWrapper team(pTeam);
					unsigned int team_num = team.GetTeamID();
					if (team_num == 2)
					{
						*ppRedTeam = pTeam;
					}
					else if (team_num == 3)
					{
						*ppBluTeam = pTeam;
					}
				}
				int ent_index = SCHelpers::EntIndexFromEdict(pTeamEdict);
				pTeamEdict = context->GetEntityByClassName("CTFTeam", ent_index+1);
			}
		}
	}

	//-----------------------------------------------------------------------------
	// Purpose: Returns the 4 bit nibble for a hex character
	// Input  : c - 
	// Output : unsigned char
	//-----------------------------------------------------------------------------
	unsigned char S_nibble( char c )
	{
		if ( ( c >= '0' ) &&
			( c <= '9' ) )
		{
			return (unsigned char)(c - '0');
		}

		if ( ( c >= 'A' ) &&
			( c <= 'F' ) )
		{
			return (unsigned char)(c - 'A' + 0x0a);
		}

		if ( ( c >= 'a' ) &&
			( c <= 'f' ) )
		{
			return (unsigned char)(c - 'a' + 0x0a);
		}

		return '0';
	}

	//-----------------------------------------------------------------------------
	// Purpose: 
	// Input  : *in - 
	//			numchars - 
	//			*out - 
	//			maxoutputbytes - 
	//-----------------------------------------------------------------------------
	void S_bigendianhextobinary( char const *in, int numchars, byte *out, int maxoutputbytes )
	{
		int len = V_strlen( in );
		numchars = MIN( len, numchars );
		// Make sure it's even
		numchars = ( numchars ) & ~0x1;

		// Must be an even # of input characters (two chars per output byte)
		Assert( numchars >= 2 );

		memset( out, 0x00, maxoutputbytes );

		byte *p;
		int i;

		p = out;
		for ( i = 0; 
			( i < numchars ) && ( ( p - out ) < maxoutputbytes ); 
			i+=2, p++ )
		{
			*p = ( S_nibble( in[i] ) << 4 ) | S_nibble( in[i+1] );		
		}
	}

	//-----------------------------------------------------------------------------
	// Purpose: 
	// Input  : *in - 
	//			numchars - 
	//			*out - 
	//			maxoutputbytes - 
	//-----------------------------------------------------------------------------
	void S_littleendianhextobinary( char const *in, int numchars, byte *out, int maxoutputbytes )
	{
		int len = V_strlen( in );
		numchars = MIN( len, numchars );
		// Make sure it's even
		numchars = ( numchars ) & ~0x1;

		// Must be an even # of input characters (two chars per output byte)
		Assert( numchars >= 2 );

		memset( out, 0x00, maxoutputbytes );

		byte *p;
		int i;

		p = out;
		for ( i = numchars-2; 
			( i >= 0 ) && ( ( p - out ) < maxoutputbytes ); 
			i-=2, p++ )
		{
			*p = ( S_nibble( in[i] ) << 4 ) | S_nibble( in[i+1] );		
		}
	}

	//---------------------------------------------------------------------------------
	// Purpose: used by the GetPropOffsetFromTable func to get a specific table
	//---------------------------------------------------------------------------------
	SendTable *GetDataTable( const char *pTableName, SendTable *pTable )
	{
		if (!pTable)
			return NULL;
		
		if ( FStrEq( pTableName, pTable->GetName() ) )
			return pTable;
		
		int num = pTable->GetNumProps();
		for (int i = 0; i < num; i++)
		{
			SendProp *pProp = pTable->GetProp(i);
			if (pProp)
			{
				SendTable *pSubTable = GetDataTable( pTableName, pProp->GetDataTable() );
				if (pSubTable == NULL)
					continue;
				if ( FStrEq(pSubTable->GetName(), pTableName) )
					return pSubTable;
			}
		}
		return NULL;
	}

	//---------------------------------------------------------------------------------
	// Purpose: returns the specified prop from the class and table provided.
	//			if prop or table not found, pointer returns NULL
	//---------------------------------------------------------------------------------
	SendProp *GetPropFromClassAndTable(const char *szClassName, const char *szTableName, const char *szPropName)
	{
		ServerClass *pServerClass = pServerDLL->GetAllServerClasses();
		if (!pServerClass)
		{
			Warning("servergamedll->GetAllServerClasses() returned null\n");
			return NULL;
		}
		while (pServerClass)
		{
			if ( FStrEq(szClassName, pServerClass->GetName()) )
			{
				SendTable *pTable = GetDataTable( szTableName, pServerClass->m_pTable );
				if (pTable)
				{
					int numprops = pTable->GetNumProps();
					for (int i = 0; i < numprops; ++i)
					{
						SendProp *pProp = pTable->GetProp(i);
						if (pProp && FStrEq(szPropName, pProp->GetName()) )
						{
							return pProp;
						}
					}
				}
			}
			pServerClass = pServerClass->m_pNext;
		}
		Warning("prop %s not found in %s => %s\n", szPropName, szClassName, szTableName);
		return NULL;
	}

	//---------------------------------------------------------------------------------
	// Purpose: returns the specified prop offset relative to the table provided.
	//			if offset or table not found, bErr returns true and offset returned is 0
	//---------------------------------------------------------------------------------
	unsigned int GetPropOffsetFromTable(const char *pTableName, const char *pPropName)
	{
		ServerClass *pClass = pServerDLL->GetAllServerClasses();
		if (!pClass)
		{
			Warning("servergamedll->GetAllServerClasses() returned null\n");
			return 0;
		}
		while (pClass)
		{
			SendTable *pTable = GetDataTable( pTableName, pClass->m_pTable );
			if (pTable == NULL)
			{
				pClass = pClass->m_pNext;
				continue;
			}
			int num = pTable->GetNumProps();
			for (int i = 0; i < num; i++)
			{
				SendProp *pProp = pTable->GetProp(i);
				if ( FStrEq( pPropName, pProp->GetName() ) )
				{
					return pProp->GetOffset();
				}
			}
			pClass = pClass->m_pNext;
		}
		Warning("prop %s not found in %s or table name incorrect\n", pPropName, pTableName);
		return 0;
	}

	CTeamplayRoundBasedRules *GetTeamplayRoundBasedGameRulesPointer()
	{
		SendProp *pSendProp = SCHelpers::GetPropFromClassAndTable( "CTFGameRulesProxy", "DT_TeamplayRoundBasedRulesProxy", "teamplayroundbased_gamerules_data" );
		if ( pSendProp )
		{
			SendTableProxyFn proxyfn = pSendProp->GetDataTableProxyFn();
			if ( proxyfn )
			{
				CSendProxyRecipients recp;
				void *pGameRules = proxyfn( NULL, NULL, NULL, &recp, 0 );
				return reinterpret_cast<CTeamplayRoundBasedRules*>(pGameRules);
			}
		}
		return NULL;
	}


	int GetDatamapVarOffset( datamap_t *pDatamap, const char *szVarName )
	{
		while (pDatamap)
		{
			int numFields = pDatamap->dataNumFields;
			for (int i = 0; i < numFields; ++i)
			{
				typedescription_t *pTypeDesc = &pDatamap->dataDesc[i];
				// TODO: check if FIELD_VOID types can have additional data tables inside
				if (pTypeDesc/* && (pTypeDesc->fieldType != FIELD_VOID)*/)
				{
					if ( pTypeDesc->fieldName && FStrEq(pTypeDesc->fieldName, szVarName) )
					{
						return pTypeDesc->fieldOffset[TD_OFFSET_NORMAL];
					}
					else
					{
						// there can be additional data tables inside this type description
						GetDatamapVarOffset(pTypeDesc->td, szVarName);
					}
				}
			}
			pDatamap = pDatamap->baseMap;
		}
		return 0;
	}

	int GetOffsetForDatamapVar( const CBaseEntity *pEntity, const char *szVarName )
	{
		if (pEntity)
		{
			datamap_t *pDatamap = GetDataDescMap( pEntity );
			return GetDatamapVarOffset( pDatamap, szVarName );
		}
		return 0;
	}

} // namespace SCHelpers

