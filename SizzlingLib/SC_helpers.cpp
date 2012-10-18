//

#include "SC_helpers.h"
#include "ServerPluginHandler.h"
#include "VFuncs.h"

#include "eiface.h"

#include "steam/steamclientpublic.h"
#include "UserIdTracker.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern IVEngineServer	*pEngine; // helper functions (messaging clients, loading content, making entities, running commands, etc)
extern CGlobalVars		*gpGlobals;
extern IServerGameEnts	*pServerEnts;
extern IServerGameDLL	*pServerDLL;
//extern s_ServerPlugin	*g_pServerPluginHandler;
extern UserIdTracker 	*g_pUserIdTracker;

namespace SCHelpers
{
	edict_t *UserIDToEdict( int userid )
	{
		int index = g_pUserIdTracker->GetEntIndex(userid);
		return pEngine->PEntityOfEntIndex(index);
		/*edict_t *pEntity = NULL;
		for ( int i = 1; i <= gpGlobals->maxClients; i++ )
		{
			pEntity = pEngine->PEntityOfEntIndex( i );
			if ( !pEntity || pEntity->IsFree() )
				continue;
			if ( pEngine->GetPlayerUserId( pEntity ) == userid )
			{
				return pEntity;
			}
		}
		return NULL;*/
	}

	unsigned int UserIDToSteamID( int userid )
	{
		int index = g_pUserIdTracker->GetEntIndex(userid);
		edict_t *pEdict = pEngine->PEntityOfEntIndex(index);
		const CSteamID *pSteamID = pEngine->GetClientSteamID(pEdict);
		if ( pSteamID )
		{
			return pSteamID->GetAccountID();
		}
		else
		{
			return 0;
		}
		/*edict_t *pEntity = NULL;
		for ( int i = 1; i <= gpGlobals->maxClients; i++ )
		{
			pEntity = pEngine->PEntityOfEntIndex( i );
			if ( !pEntity || pEntity->IsFree() )
				continue;
			if ( pEngine->GetPlayerUserId( pEntity ) == userid )
			{
				const CSteamID *pSteamID = pEngine->GetClientSteamID( pEntity );
				if ( pSteamID )
					return pSteamID->GetAccountID();
			}
		}
		return 0;*/
	}

	CBaseEntity *GetEntityByClassname( const char *pszClassname )	// i'd like to than df's source code and a late night of stripping away bs for this one
	{
		for ( int i = gpGlobals->maxClients; i < gpGlobals->maxEntities; ++i )
		{
			edict_t *pEdict = pEngine->PEntityOfEntIndex( i );
			if ( !pEdict )
			{
				continue;
				Msg( "bad edict\n" );
			}
			CBaseEntity *pEntity = pServerEnts->EdictToBaseEntity( pEdict );
			if ( !pEntity )
			{
				continue;
				Msg( "bad entity\n" );
			}
			datamap_t *pDatamap = GetDataDescMap( pEntity );
			if ( !pDatamap )
			{
				continue;
				Msg( "bad datamap\n" );
			}
			Msg( "%s\n", pDatamap->dataClassName );
			//Msg( "%s\n", pszClassname );
			if ( FStrEq( pDatamap->dataClassName, pszClassname ) )
			{
				return pEntity;
			}
			//for ( int j = 0; j < pDatamap->dataNumFields; j++ )
			//{
			//	typedescription_t *pType = &pDatamap->dataDesc[j];
			//	if ( pType->fieldType != FIELD_VOID )
			//	{
			//		//Msg( "%s\n", pType->fieldName );
			//		if ( FStrEq( pszClassname, pType->fieldName ) )
			//		{
			//			int offset = pType->fieldOffset[TD_OFFSET_NORMAL];
			//			Msg( "%i\n", offset );
			//			string_t *m_iClassname = (string_t *)(((unsigned int)pEntity)+(offset));
			//			Msg( "%s\n", STRING(*m_iClassname) );
			//			if ( FStrEq( STRING(*m_iClassname), pszClassname ) )
			//			{
			//				Msg( "%s\n", m_iClassname );
			//				return pEntity;
			//			}
			//		}
			//	}
			//}
		}
		Msg( "CLASSNAME NOT FOUND NOOOOOOO\n" );
		return NULL;
	}

	//CBaseEntity *GetBaseFromID(int id) {
	//   edict_t *pEntity = NULL;
	//   for(int i = 1; i <= ppEngineClient->GetMaxClients(); i++) { 
	//      pEntity = pEngine->PEntityOfEntIndex(i); 
	//      if(!pEntity || pEntity->IsFree()) 
	//		  continue; 
	//      if(pEngine->GetPlayerUserId(pEntity) == id) 
	//		  return pEngine->PEntityOfEntIndex(i)->GetUnknown()->GetBaseEntity();
	//   } 
	//   return 0; 
	//}

	int UserIDToEntIndex( int userid )
	{
		return g_pUserIdTracker->GetEntIndex(userid);
		/*edict_t *pEntity = NULL;
		for ( int i = 1; i <= gpGlobals->maxClients; i++ )
		{
			pEntity = pEngine->PEntityOfEntIndex( i );
			if ( !pEntity || pEntity->IsFree() )
				continue;
			if ( pEngine->GetPlayerUserId( pEntity ) == userid )
			{
				return i;
			}
		}
		return 0;*/
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

	unsigned int GetThisPluginIndex( const char *pszDescriptionPart )
	{
		for ( unsigned int i = 0; i < g_pServerPluginHandler->nLoadedPlugins; ++i )
		{
			//Msg( "plugin %i, %s\n", i, *((const char **)(g_pServerPluginHandler->pszInfo + i)) );
			if ( V_strstr( *((const char **)(g_pServerPluginHandler->pszInfo + i)), pszDescriptionPart ) )
			{
				return i;
			}
		}
		return 0;
	}

	//---------------------------------------------------------------------------------
	// Purpose: used by the GetPropOffsetFromTable func to get a specific table
	//---------------------------------------------------------------------------------
	SendTable *GetDataTable( const char *pTableName, SendTable *pTable )
	{
		SendTable *pSendTable = pTable;
		if (pSendTable == NULL)
			return NULL;
		if ( FStrEq( pTableName, pSendTable->GetName() ) )
			return pSendTable;
		int num = pSendTable->GetNumProps();
		for (int i = 0; i < num; i++){
			SendProp *pProp = pSendTable->GetProp(i);
			SendTable *pSubTable = GetDataTable( pTableName, pProp->GetDataTable() );
			if (pSubTable == NULL)
				continue;
			if ( FStrEq(pSubTable->GetName(), pTableName) )
				return pSubTable;
		}
		return NULL;
	}

	//---------------------------------------------------------------------------------
	// Purpose: returns the specified prop from the table provided.
	//			if prop or table not found, bErr returns true and pointer returns NULL
	//---------------------------------------------------------------------------------
	SendProp *GetPropFromTable(const char *pTableName, const char *pPropName, bool *bErr )
	{
		ServerClass *pClass = pServerDLL->GetAllServerClasses();
		if (!pClass)
		{
			if ( bErr )
				*bErr = true;
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
					if ( bErr )
						*bErr = false;
					return pProp;
				}
			}
			pClass = pClass->m_pNext;
		}
		Warning("prop %s not found in %s or table name incorrect\n", pPropName, pTableName);
		if ( bErr )
			*bErr = true;
		return 0;
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

	void *GetTeamplayRoundBasedGameRulesPointer()
	{
		SendProp *pSendProp = SCHelpers::GetPropFromTable( "DT_TeamplayRoundBasedRulesProxy", "teamplayroundbased_gamerules_data" );
		if ( pSendProp )
		{
			SendTableProxyFn proxyfn = pSendProp->GetDataTableProxyFn();
			if ( proxyfn )
			{
				CSendProxyRecipients recp;
				return proxyfn( NULL, NULL, NULL, &recp, 0 );
			}
		}
		return NULL;
	}

} // namespace SCHelpers

