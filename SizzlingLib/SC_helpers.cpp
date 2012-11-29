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
	}

	void PrintSpace( int numspaces )
	{
		for (int i = 0; i < numspaces; ++i)
		{
			Msg(" ");
		}
		Msg("|");
	}

	void PrintDataMap( datamap_t *pDatamap, int spacing )
	{
		if (pDatamap)
		{
			int numFields = pDatamap->dataNumFields;
			for (int i = 0; i < numFields; ++i)
			{
				typedescription_t *pTypeDesc = &pDatamap->dataDesc[i];
				PrintSpace(spacing);
				Msg( "class name: %s\n", pDatamap->dataClassName );
				PrintSpace(spacing);
				Msg( "num fields: %i\n", pDatamap->dataNumFields );
				PrintTypeDescription(pTypeDesc, spacing+1);
			}
		}
	}

	void PrintTypeDescription( typedescription_t *pDesc, int spacing )
	{
		if (pDesc)
		{
			if (pDesc->fieldType == FIELD_VOID)
			{
				PrintSpace(spacing);
				Msg( "field type was void\n" );
			}
			PrintSpace(spacing);
			Msg( "field name   : %s\n", pDesc->fieldName );
			PrintSpace(spacing);
			Msg( "external name: %s\n", pDesc->externalName );
			PrintSpace(spacing);
			Msg( "offset normal: %i\n", pDesc->fieldOffset[0] );
			PrintSpace(spacing);
			Msg( "offset packed: %i\n", pDesc->fieldOffset[1] );
			PrintDataMap( pDesc->td, spacing+1 );
		}
	}

	void PrintEntityDatamap( CBaseEntity *pEntity )
	{
		if (pEntity)
		{
			datamap_t *pDatamap = GetDataDescMap( pEntity );
			PrintDataMap( pDatamap, 0 );
		}
	}

	CBaseEntity *GetEntityByClassname( const char *pszClassname )
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

	int UserIDToEntIndex( int userid )
	{
		return g_pUserIdTracker->GetEntIndex(userid);
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
		if (!pTable)
			return NULL;
		
		SendTable *pSendTable = pTable;
		if ( FStrEq( pTableName, pSendTable->GetName() ) )
			return pSendTable;
		
		int num = pSendTable->GetNumProps();
		for (int i = 0; i < num; i++)
		{
			SendProp *pProp = pSendTable->GetProp(i);
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

} // namespace SCHelpers

