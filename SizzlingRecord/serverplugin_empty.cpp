//===== Copyright © 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose: 
//
// $NoKeywords: $
//
//===========================================================================//

#include "PluginDefines.h"

#include <stdio.h>
#include <time.h>

#include "interface.h"
#include "filesystem.h"
#include "eiface.h"
#include "engine/iserverplugin.h"
#include "game/server/iplayerinfo.h"
#include "convar.h"
#include "igameevents.h"

#include "NetPropUtils.h"
#include "PlayerMessage.h"
#include "Helpers.h"
#include "SC_helpers.h"

#include "cdll_int.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CTeamplayRoundBasedRules;

static ConVar enabled("sizz_record_enable", "1", FCVAR_NONE, "If nonzero, enables tournament match demo recording.");

//===========================================================================//

namespace DemoRecorder
{
	inline bool CanRecordDemo( IBaseClientDLL *pBaseClientDLL );
	inline void StartRecording( IVEngineClient *pEngineClient, IBaseClientDLL *pBaseClientDLL, const char *szMapName );
	inline void StopRecording( IVEngineClient *pEngineClient );
}

bool DemoRecorder::CanRecordDemo( IBaseClientDLL *pBaseClientDLL )
{
	return pBaseClientDLL->CanRecordDemo(NULL, 0);
}

void DemoRecorder::StartRecording( IVEngineClient *pEngineClient, IBaseClientDLL *pBaseClientDLL, const char *szMapName )
{
	if ( !!enabled.GetInt() && CanRecordDemo(pBaseClientDLL) )
	{
		// get the time as an int64
		time_t t = time(NULL);

		// convert it to a struct of time values
		struct tm ltime = *localtime(&t);

		// normalize the year and month
		uint32 year = ltime.tm_year + 1900;
		uint32 month = ltime.tm_mon + 1;

		// stop recording the current demo if there is one
		StopRecording(pEngineClient);
		
		// create the record string
		char recordstring[128] = {};
		V_snprintf(recordstring, 128, "record %d%d%d_%d%d_%s\n", year, month, ltime.tm_mday, ltime.tm_hour, ltime.tm_min, szMapName );

		// start recording our demo
		pEngineClient->ClientCmd_Unrestricted( recordstring );
	}
}

void DemoRecorder::StopRecording( IVEngineClient *pEngineClient )
{
	pEngineClient->ClientCmd_Unrestricted( "stop\n" );
}

//===========================================================================//

void VersionChangeCallback( IConVar *var, const char *pOldValue, float flOldValue );
static ConVar version("sizz_record_version", PLUGIN_VERSION_STRING, FCVAR_NOTIFY, "The version of SizzlingRecord running.", &VersionChangeCallback);

void VersionChangeCallback( IConVar *var, const char *pOldValue, float flOldValue )
{
    if (strcmp(version.GetString(), PLUGIN_VERSION_STRING))
    {
        var->SetValue(PLUGIN_VERSION_STRING);
    }
}

static char *UTIL_VarArgs( char *format, ... )
{
    va_list     argptr;
    static char     string[1024];
    
    va_start (argptr, format);
    Q_vsnprintf(string, sizeof(string), format,argptr);
    va_end (argptr);

    return string;  
}

//===========================================================================//

//---------------------------------------------------------------------------------
// Purpose: a sample 3rd party plugin class
//---------------------------------------------------------------------------------
class CEmptyServerPlugin: public IServerPluginCallbacks, public IGameEventListener2, public IRecvPropHookCallback
{
public:
	CEmptyServerPlugin();
	~CEmptyServerPlugin();

	// IServerPluginCallbacks methods
	virtual bool			Load(	CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory );
	virtual void			Unload( void );
	virtual void			Pause( void );
	virtual void			UnPause( void );
	virtual const char     *GetPluginDescription( void );      
	virtual void			LevelInit( char const *pMapName );
	virtual void			ServerActivate( edict_t *pEdictList, int edictCount, int clientMax );
	virtual void			GameFrame( bool simulating );
	virtual void			LevelShutdown( void );
	virtual void			ClientActive( edict_t *pEntity );
	virtual void			ClientDisconnect( edict_t *pEntity );
	virtual void			ClientPutInServer( edict_t *pEntity, char const *playername );
	virtual void			SetCommandClient( int index );
	virtual void			ClientSettingsChanged( edict_t *pEdict );
	virtual PLUGIN_RESULT	ClientConnect( bool *bAllowConnect, edict_t *pEntity, const char *pszName, const char *pszAddress, char *reject, int maxrejectlen );
	virtual PLUGIN_RESULT	ClientCommand( edict_t *pEntity, const CCommand &args );
	virtual PLUGIN_RESULT	NetworkIDValidated( const char *pszUserName, const char *pszNetworkID );
	virtual void			OnQueryCvarValueFinished( QueryCvarCookie_t iCookie, edict_t *pPlayerEntity, EQueryCvarValueStatus eStatus, const char *pCvarName, const char *pCvarValue );
	
	// IGameEventListener2 Interface
	virtual void FireGameEvent( IGameEvent *event );
	
	virtual bool RecvPropHookCallback( const CRecvProxyData *pData, void *pStruct, void *pOut );
	
	// Additions
	bool WaitingForPlayersChangeCallback( const CRecvProxyData *pData, void *pStruct, void *pOut );
	
	int GetCommandIndex() { return m_iClientCommandIndex; }
	
	bool ConfirmInterfaces( void );
	
	void HookProps();
	void UnhookProps();

private:
	CDllDemandLoader m_BaseClientDLL;
	CRecvPropHook m_bInWaitingForPlayersHook;
	ConVarRef m_refTournamentMode;
	
	// Interfaces from the engine
	IGameEventManager2 *m_pGameEventManager;
	IBaseClientDLL *m_pBaseClientDLL;
	IVEngineClient *m_pEngineClient;
	IFileSystem	*m_pFullFileSystem;
	CGlobalVars	*m_pGlobals;
	
	int	*m_iRoundState;
	bool *m_bInWaitingForPlayers;
	int m_iClientCommandIndex;
	bool m_bTournamentMatchStarted;
};

// 
// The plugin is a static singleton that is exported as an interface
//
CEmptyServerPlugin g_EmptyServerPlugin;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CEmptyServerPlugin, IServerPluginCallbacks, INTERFACEVERSION_ISERVERPLUGINCALLBACKS, g_EmptyServerPlugin );

//---------------------------------------------------------------------------------
// Purpose: constructor/destructor
//---------------------------------------------------------------------------------
CEmptyServerPlugin::CEmptyServerPlugin():
	m_BaseClientDLL("tf/bin/client" PLUGIN_EXTENSION),
	m_refTournamentMode((IConVar*)NULL),
	m_pGameEventManager(NULL),
	m_pBaseClientDLL(NULL),
	m_pEngineClient(NULL),
	m_pFullFileSystem(NULL),
	m_pGlobals(NULL),
	m_iRoundState(NULL),
	m_bInWaitingForPlayers(NULL),
	m_iClientCommandIndex(0),
	m_bTournamentMatchStarted(false)
{
}

CEmptyServerPlugin::~CEmptyServerPlugin()
{
}

//---------------------------------------------------------------------------------
// Purpose: called when the plugin is loaded, load the interface we need from the engine
//---------------------------------------------------------------------------------
bool CEmptyServerPlugin::Load(	CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory )
{
	/*g_pFullFileSystem = (IFileSystem *)interfaceFactory(FILESYSTEM_INTERFACE_VERSION, NULL);
	if (!g_pFullFileSystem)
	{
		Warning( "Unable to load g_pFullFileSystem, aborting load\n" );
		return false;
	}*/

	if ( !cvar )
	{
	    //Warning( "[SizzlingStats]: cvar is null.\n" );
	    cvar = (ICvar*)interfaceFactory(CVAR_INTERFACE_VERSION, NULL);
	    if ( !cvar )
	    {
	        //Warning( "[SizzlingStats]: Couldn't get cvar, aborting load.\n" );
	        return false;
	    }
		//Warning( "[SizzlingStats]: got cvar.\n" );
	}

	if ( !g_pCVar )
	{
	    Warning( "linking g_pCVar to cvar\n" );
	    g_pCVar = cvar;
	}
	
	m_pGameEventManager = (IGameEventManager2*)interfaceFactory(INTERFACEVERSION_GAMEEVENTSMANAGER2, NULL);
	m_pEngineClient = (IVEngineClient *)interfaceFactory(VENGINE_CLIENT_INTERFACE_VERSION, NULL);
	m_pBaseClientDLL = (IBaseClientDLL*)m_BaseClientDLL.GetFactory()(CLIENT_DLL_INTERFACE_VERSION, NULL);
	
	IPlayerInfoManager *pPlayerInfoManager = (IPlayerInfoManager *)gameServerFactory(INTERFACEVERSION_PLAYERINFOMANAGER,NULL);
	if (pPlayerInfoManager)
	{
		m_pGlobals = pPlayerInfoManager->GetGlobalVars();
	}

	if ( !ConfirmInterfaces() )
	{
		return false;
	}
	
	//GetPropOffsets();
	HookProps();
	
	// hook this event so we can get the new gamerules pointer each time
	m_pGameEventManager->AddListener( this, "game_newmap", false );
	
	m_refTournamentMode.Init( "mp_tournament", false );

	MathLib_Init( 2.2f, 2.2f, 0.0f, 2 );
	ConVar_Register( 0 );
	return true;
}

//---------------------------------------------------------------------------------
// Purpose: called when the plugin is unloaded (turned off)
//---------------------------------------------------------------------------------
void CEmptyServerPlugin::Unload( void )
{
	if (m_pGameEventManager)
	{
		m_pGameEventManager->RemoveListener( this );
	}
	
	UnhookProps();
	
	if (cvar)
	{
		ConVar_Unregister( );
	}
	
	m_BaseClientDLL.Unload();
}

//---------------------------------------------------------------------------------
// Purpose: called when the plugin is paused (i.e should stop running but isn't unloaded)
//---------------------------------------------------------------------------------
void CEmptyServerPlugin::Pause( void )
{
}

//---------------------------------------------------------------------------------
// Purpose: called when the plugin is unpaused (i.e should start executing again)
//---------------------------------------------------------------------------------
void CEmptyServerPlugin::UnPause( void )
{
}

//---------------------------------------------------------------------------------
// Purpose: the name of this plugin, returned in "plugin_print" command
//---------------------------------------------------------------------------------
const char *CEmptyServerPlugin::GetPluginDescription( void )
{
	return "SizzlingRecord v" PLUGIN_VERSION ", SizzlingCalamari. " __DATE__;
}

//---------------------------------------------------------------------------------
// Purpose: called on level start
//---------------------------------------------------------------------------------
void CEmptyServerPlugin::LevelInit( char const *pMapName )
{
}

//---------------------------------------------------------------------------------
// Purpose: called on level start, when the server is ready to accept client connections
//		edictCount is the number of entities in the level, clientMax is the max client count
//---------------------------------------------------------------------------------
void CEmptyServerPlugin::ServerActivate( edict_t *pEdictList, int edictCount, int clientMax )
{
	// will this function even run on the client?
	//GetPropOffsets();
}

//---------------------------------------------------------------------------------
// Purpose: called once per server frame, do recurring work here (like checking for timeouts)
//---------------------------------------------------------------------------------
void CEmptyServerPlugin::GameFrame( bool simulating )
{
}

//---------------------------------------------------------------------------------
// Purpose: called on level end (as the server is shutting down or going to a new map)
//---------------------------------------------------------------------------------
void CEmptyServerPlugin::LevelShutdown( void ) // !!!!this can get called multiple times per map change
{
	// will this function even run on the client?
	if (m_bTournamentMatchStarted)
	{
		DemoRecorder::StopRecording(m_pEngineClient);
		m_bTournamentMatchStarted = false;
	}
}

//---------------------------------------------------------------------------------
// Purpose: called when a client spawns into a server (i.e as they begin to play)
//---------------------------------------------------------------------------------
void CEmptyServerPlugin::ClientActive( edict_t *pEdict )
{
}

//---------------------------------------------------------------------------------
// Purpose: called when a client leaves a server (or is timed out)
//---------------------------------------------------------------------------------
void CEmptyServerPlugin::ClientDisconnect( edict_t *pEdict )
{
}

//---------------------------------------------------------------------------------
// Purpose: called on 
//---------------------------------------------------------------------------------
void CEmptyServerPlugin::ClientPutInServer( edict_t *pEntity, char const *playername )
{
}

//---------------------------------------------------------------------------------
// Purpose: called on level start
//---------------------------------------------------------------------------------
void CEmptyServerPlugin::SetCommandClient( int index )
{
	m_iClientCommandIndex = index;
}

//---------------------------------------------------------------------------------
// Purpose: called on level start
//---------------------------------------------------------------------------------
void CEmptyServerPlugin::ClientSettingsChanged( edict_t *pEdict )
{
}

//---------------------------------------------------------------------------------
// Purpose: called when a client joins a server
//---------------------------------------------------------------------------------
PLUGIN_RESULT CEmptyServerPlugin::ClientConnect( bool *bAllowConnect, edict_t *pEntity, const char *pszName, const char *pszAddress, char *reject, int maxrejectlen )
{
	return PLUGIN_CONTINUE;
}

//---------------------------------------------------------------------------------
// Purpose: called when a client types in a command (only a subset of commands however, not CON_COMMAND's)
//---------------------------------------------------------------------------------
PLUGIN_RESULT CEmptyServerPlugin::ClientCommand( edict_t *pEntity, const CCommand &args )
{
	return PLUGIN_CONTINUE;
}

//---------------------------------------------------------------------------------
// Purpose: called when a client is authenticated
//---------------------------------------------------------------------------------
PLUGIN_RESULT CEmptyServerPlugin::NetworkIDValidated( const char *pszUserName, const char *pszNetworkID )
{
	//pEngine->LogPrint(UTIL_VarArgs( "NetworkIDValidated: %s, %s\n", pszUserName, pszNetworkID ));
	return PLUGIN_CONTINUE;
}

//---------------------------------------------------------------------------------
// Purpose: called when a cvar value query is finished
//---------------------------------------------------------------------------------
void CEmptyServerPlugin::OnQueryCvarValueFinished( QueryCvarCookie_t iCookie, edict_t *pPlayerEntity, EQueryCvarValueStatus eStatus, const char *pCvarName, const char *pCvarValue )
{
	//Msg( "Cvar query (cookie: %d, status: %d) - name: %s, value: %s\n", iCookie, eStatus, pCvarName, pCvarValue );
}

//---------------------------------------------------------------------------------
// Purpose: confirm the validity of the interface pointers
//---------------------------------------------------------------------------------
bool CEmptyServerPlugin::ConfirmInterfaces( void )
{
	if (!m_pGameEventManager)
	{
		Warning( "Unable to load pGameEventManager, aborting load\n" );
		return false;
	}
	
	if (!m_pEngineClient)
	{
		Warning( "Unable to load pEngineClient, aborting load\n" );
		return false;
	}

	if (!m_pBaseClientDLL)
	{
		Warning( "Unable to load pBaseClientDLL, aborting load\n" );
		return false;
	}
	
	if (!m_pGlobals)
	{
		Warning( "Unable to load pGlobals, aborting load\n" );
		return false;
	}

	Msg( "All interfaces sucessfully loaded\n" );
	return true;
}

void CEmptyServerPlugin::FireGameEvent( IGameEvent *event )
{
	const char *name = event->GetName();
	
	if (SCHelpers::FStrEq(name, "game_newmap"))
	{
		// get the new gamerules pointer in here
		// do i really need it for anything though?
	}
}

bool CEmptyServerPlugin::RecvPropHookCallback( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	const char *name = pData->m_pRecvProp->GetName();
	
	if (SCHelpers::FStrEq(name, "m_bInWaitingForPlayers"))
	{
		return WaitingForPlayersChangeCallback(pData, pStruct, pOut);
	}
	else
	{
		return true;
	}
}

bool CEmptyServerPlugin::WaitingForPlayersChangeCallback( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	bool bWaitingForPlayers = !!(pData->m_Value.m_Int);
	static bool oldWaitingForPlayers = bWaitingForPlayers;

	if (oldWaitingForPlayers != bWaitingForPlayers)
	{
		//using namespace Teamplay_GameRule_States;

		//Msg( UTIL_VarArgs( "round state is %s\n", GetStateName((gamerules_roundstate_t)roundstate) ) );
		bool bTournamentMode = m_refTournamentMode.GetInt() == 1;

		if (bWaitingForPlayers == true)
		{
			if (m_bTournamentMatchStarted)
			{
				DemoRecorder::StopRecording(m_pEngineClient);
				m_bTournamentMatchStarted = false;
			}
		}
		else
		{
			// i don't think i need to check the round state for client stuff
			//int roundstate = *m_iRoundState;
			
			if (bTournamentMode && !m_bTournamentMatchStarted/* && (roundstate != GR_STATE_PREGAME)*/)
			{
				DemoRecorder::StartRecording(m_pEngineClient, m_pBaseClientDLL, m_pGlobals->mapname.ToCStr());
				m_bTournamentMatchStarted = true;
			}
		}

		oldWaitingForPlayers = bWaitingForPlayers;
	}
	return true;
}

void CEmptyServerPlugin::HookProps()
{
	// is there a possibility that the hooking will fail?
	//unsigned int gamerulesoffset = GetPropOffsetFromTable( "DT_TFGameRulesProxy", "baseclass", bError ) +
	//	GetPropOffsetFromTable( "DT_TeamplayRoundBasedRulesProxy", "teamplayroundbased_gamerules_data", bError );
	
	//SendProp *piRoundState = GetPropFromClassAndTable( "CTeamplayRoundBasedRulesProxy", "DT_TeamplayRoundBasedRules", "m_iRoundState" );
	//if (piRoundState)
	//{
	//	m_iRoundStateOffset = gamerulesoffset + piRoundState->GetOffset();
	//	m_iRoundStateHook.Hook( piRoundState, this );
	//}
	
	RecvProp *pbInWaitingForPlayers = Helpers::GetPropFromClassAndTable( m_pBaseClientDLL, "CTeamplayRoundBasedRulesProxy", "DT_TeamplayRoundBasedRules", "m_bInWaitingForPlayers" );
	if (pbInWaitingForPlayers)
	{
		m_bInWaitingForPlayersHook.Hook( pbInWaitingForPlayers, this );
	}
}

void CEmptyServerPlugin::UnhookProps()
{
    //m_iRoundStateHook.Unhook();
    m_bInWaitingForPlayersHook.Unhook();
}
/*
void RecurseClientTable( RecvTable *pTable, int &spacing )
{
	RecvTable *pRecvTable = pTable;
	if (pRecvTable == NULL)
	{
		spacing--;
		return;
	}
	
	char TableName[128];
	int size = sizeof(TableName);

	memset( TableName, 0, size );
	for (int i = 0; i < spacing; i++)
	{
		V_strcat( TableName, "  |", size );
	}
	V_strcat( TableName, pRecvTable->GetName(), size );
	Msg( "%s\n", TableName );

	spacing++;
	int num = pRecvTable->GetNumProps();
	for (int i = 0; i < num; i++)
	{
		RecvProp *pProp = pRecvTable->GetProp(i);

		memset( TableName, 0, sizeof(TableName) );
		for (int j = 0; j < spacing; j++)
		{
			V_strcat( TableName, "  |", size );
		}
		V_strcat( TableName, pProp->GetName(), size );
		Msg( "%s\n", TableName );

		RecurseClientTable( pProp->GetDataTable(), ++spacing );
	}
	spacing-=2;
}

CON_COMMAND ( printclienttables, "prints the client tables ya" )
{
	ClientClass *pClass = pBaseClientDLL->GetAllClasses();
	while ( pClass )
	{
		Msg("%s\n", pClass->m_pNetworkName );
		RecvTable *pTable = pClass->m_pRecvTable;
		int i = 1;
		RecurseClientTable( pTable, i );
		Msg("\n");
		pClass = pClass->m_pNext;
	}
}
*/

