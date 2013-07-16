
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

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern IVEngineServer	*pEngine; // helper functions (messaging clients, loading content, making entities, running commands, etc)
extern CGlobalVars		*gpGlobals;
extern IServerGameEnts	*pServerEnts;
extern IServerGameDLL	*pServerDLL;
//extern s_ServerPlugin	*g_pServerPluginHandler;

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

	CBaseEntity *BaseHandleToBaseEntity( const CBaseHandle *pHandle )
	{
		if (pHandle && pHandle->IsValid())
		{
			int entindex = pHandle->GetEntryIndex();
			edict_t *pEdict = pEngine->PEntityOfEntIndex(entindex);
			return EdictToBaseEntity(pEdict);
		}
		
		return NULL;
	}

	const char **GetClassname( CBaseEntity * const pEnt )
	{
		static uint32 classname_offset = 0xffff;
		if (classname_offset == 0xffff)
		{
			classname_offset = GetOffsetForDatamapVar(pEnt, "m_iClassname");
		}
		return ByteOffsetFromPointer<const char*>(pEnt, classname_offset);
	}

	CBaseEntity *GetEntityByClassname( const char *pszClassname, int start_index /*= 0*/, int *ent_index_out /*= NULL*/ )
	{
		for ( int i = start_index; i < gpGlobals->maxEntities; ++i )
		{
			edict_t *pEdict = pEngine->PEntityOfEntIndex( i );
			if ( !pEdict )
			{
				//Msg( "bad edict\n" );
				continue;
			}
			IServerNetworkable *pNetworkable = pEdict->GetNetworkable();
			if (!pNetworkable)
			{
				continue;
			}
			ServerClass *pClass = pNetworkable->GetServerClass();
			if (pClass)
			{
				//Msg( "%s\n", pClass->GetName() );
				if ( FStrEq( pClass->GetName(), pszClassname ) )
				{
					if (ent_index_out)
					{
						*ent_index_out = i+1;
					}
					return pNetworkable->GetBaseEntity();
				}
			}
			/*
			CBaseEntity *pEntity = pServerEnts->EdictToBaseEntity( pEdict );
			if ( !pEntity )
			{
				//Msg( "bad entity\n" );
				continue;
			}
			datamap_t *pDatamap = GetDataDescMap( pEntity );
			if ( !pDatamap )
			{
				//Msg( "bad datamap\n" );
				continue;
			}
			Msg( "%s\n", pDatamap->dataClassName );
			if ( FStrEq( pDatamap->dataClassName, pszClassname ) )
			{
				if (ent_index_out)
				{
					*ent_index_out = i;
				}
				return pEntity;
			}*/
		}
		//Msg( "CLASSNAME NOT FOUND NOOOOOOO\n" );
		return NULL;
	}

	// gets the blu and red team entities and puts them in the pointers passed in
	void GetTeamEnts( CBaseEntity **ppBluTeam, CBaseEntity **ppRedTeam, uint32 team_num_offset )
	{
		int cur_index = gpGlobals->maxClients;
		*ppBluTeam = NULL;
		*ppRedTeam = NULL;

		CBaseEntity *pTeam = NULL;
		do
		{
			pTeam = GetEntityByClassname( "CTFTeam", cur_index, &cur_index );
			//CBaseEntity *pTeam = GetEntityByClassname( "", cur_index, &cur_index );
			if (pTeam)
			{
				int *team_num = ByteOffsetFromPointer<int>(pTeam, team_num_offset);
				if (*team_num == 2)
				{
					*ppRedTeam = pTeam;
				}
				else if (*team_num == 3)
				{
					*ppBluTeam = pTeam;
				}
			}
		} while ( pTeam && (!(*ppBluTeam) || !(*ppRedTeam)) );
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

	int GetThisPluginIndex( CServerPlugin *pPluginManager, IServerPluginCallbacks *pThisPlugin )
	{
		if (pPluginManager && pThisPlugin)
		{
			int num_plugins = pPluginManager->m_plugins.Count();
			for ( int i = 0; i < num_plugins; ++i )
			{
				CPlugin *pPlugin = pPluginManager->m_plugins[i];
				if ( pPlugin && (pPlugin->m_pPlugin == pThisPlugin) )
				{
					return i;
				}
			}
		}
		return -1;
	}

	int GetPluginIndex( CServerPlugin *pPluginManager, const char *pszDescriptionPart )
	{
		if (pPluginManager && pszDescriptionPart)
		{
			int num_plugins = pPluginManager->m_plugins.Count();
			for ( int i = 0; i < num_plugins; ++i )
			{
				CPlugin *pPlugin = pPluginManager->m_plugins[i];
				//Msg( "plugin %i, %s\n", i, pPlugin->m_szName );
				if ( pPlugin && V_strstr(pPlugin->m_szName, pszDescriptionPart) )
				{
					return i;
				}
			}
		}
		return 0xffff;
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
	unsigned int GetPropOffsetFromTable(const char *pTableName, const char *pPropName, bool &bErr)
	{
		ServerClass *pClass = pServerDLL->GetAllServerClasses();
		if (!pClass)
		{
			bErr = true;
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
					bErr = false;
					return pProp->GetOffset();
				}
			}
			pClass = pClass->m_pNext;
		}
		Warning("prop %s not found in %s or table name incorrect\n", pPropName, pTableName);
		bErr = true;
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

	void RegisterForAllEvents( IGameEventManager2 *pEventMgr, IGameEventListener2 *pListener, bool bServerSize /*= true*/ )
	{
		if (pEventMgr && pListener)
		{
			const static int NUM_EVENTS_OFFSET = 16;
			const static int EVENT_NAMES_OFFSET = 4;
			const static int EVENT_NAMES_MAXSIZE = 64;

			int *p_num_events = ByteOffsetFromPointer<int>(pEventMgr, NUM_EVENTS_OFFSET);
			const char **p_event_name = ByteOffsetFromPointer<const char*>(pEventMgr, EVENT_NAMES_OFFSET);

			if (p_num_events && p_event_name)
			{
				int num_events = *p_num_events;
				const char *event_name = *p_event_name;

				pEventMgr->RemoveListener(pListener);
				for (int i = 0; i < num_events; ++i, event_name += EVENT_NAMES_MAXSIZE)
				{
					//Msg("registering for event %i: '%s'\n", i, event_name);
					pEventMgr->AddListener(pListener, event_name, true);
				}
			}
		}
	}

} // namespace SCHelpers

