//===== Copyright © 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose: 
//
// $NoKeywords: $
//
//===========================================================================//

#define GAME_DLL
#define SERVER_BUILD

#include "dbgflag.h"

#include <stdio.h>

#include "interface.h"
#include "filesystem.h"
#include "engine/iserverplugin.h"
#include "game/server/iplayerinfo.h"
#include "eiface.h"
#include "igameevents.h"
#include "convar.h"
#include "Color.h"
#include "vstdlib/random.h"
#include "engine/IEngineTrace.h"
#include "tier2/tier2.h"

#include "tier1/stringpool.h"
#include "tier1/utlmap.h"

#include "tier1/bitbuf.h"
#include "MRecipientFilter.h"
#include "SRecipientFilter.h"

#include "dt_send.h"
#include "server_class.h"
#include "const.h"
#include "playerdata.h"
#include "PlayerMessage.h"
#include "SizzlingStats.h"
#include "ThreadCallQueue.h"
//#include "cbase.h"
#include "SC_helpers.h"
//#include "EventQueue.h"
#ifdef COUNT_CYCLES
	#include "fasttimer.h"
#endif

#include "PluginDefines.h"
#include "autoupdate.h"
#include "UserIdTracker.h"
#include "ServerPluginHandler.h"

#include "curl/curl.h"

#ifndef SERVER_BUILD
	#include "cdll_int.h"
	#include "dt_recv.h"
	#include "client_class.h"
#endif

#include "CommandCallback.h"
#include "teamplay_gamerule_states.h"
#include "NetPropUtils.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//struct s_ServerPlugin;

// Interfaces from the engine
IVEngineServer			*pEngine = NULL; // helper functions (messaging clients, loading content, making entities, running commands, etc)
IGameEventManager2		*gameeventmanager = NULL; // game events interface
IPlayerInfoManager		*playerinfomanager = NULL; // game dll interface to interact with players
//IBotManager			*botmanager = NULL; // game dll interface to interact with bots
IServerPluginHelpers	*helpers = NULL; // special 3rd party plugin helpers from the engine
//s_ServerPlugin			*g_pServerPluginHandler = NULL;
//extern UserIdTracker 	*g_pUserIdTracker;
extern CTSCallQueue		*g_pTSCallQueue;

#ifndef SERVER_BUILD
	IBaseClientDLL			*pBaseClientDLL = NULL;
	IVEngineClient			*pEngineClient = NULL;
#endif
IServerGameDLL			*pServerDLL = NULL;
IServerGameEnts			*pServerEnts = NULL;
IFileSystem			*g_pFullFileSystem = NULL;

CGlobalVars				*gpGlobals = NULL;

/*
static CCountedStringPool g_RefCountedStrPool;
CCountedStringPool *g_pRefCountedStringPool = &g_RefCountedStrPool;
*/

//extern PlayerMessage	*g_pMessage;

//===========================================================================//

void VersionChangeCallback( IConVar *var, const char *pOldValue, float flOldValue );
static ConVar version("sizz_stats_version", PLUGIN_VERSION_STRING, FCVAR_NOTIFY, "The version of SizzlingStats running.", &VersionChangeCallback);

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

// function to initialize any cvars/command in this plugin
//void Bot_RunAll( void ); 

//template <typename T>
//T *GetInterface( const char	*pchModuleName, const char *pszInterfaceVersion );

//void			GetEntityByClassname(const char *szClassname);
//void			PrintInfo( void );
//void			CreateMenu(bf_write* pBuffer, const char* szMessage, int nOptions=10, int iSecondsToStayOpen=-1);

//CBaseEntity *GetBaseFromID(int id);

//void ShowMenu( edict_t *pEntity, int buttons, int time, const char *pText) // STOLE IT FROM TONY AND MODIFIED IT LOL
//{
//	SRecipientFilter filter;
//	filter.AddRecipient( engine->IndexOfEdict( pEntity ) );
//
//	int umsg = 9;
//	if(umsg != -1)
//	{
//		char text[2048];
//		char buf[251];
//		char *p = text;
//		//int limit = strlen(pText);
//
//		strncpy(text, pText, sizeof(text));
//		text[sizeof(text)-1] = '\0';
//
//		// write messages with more option enabled while there is enough data
//		while(strlen(p) > sizeof(buf)-1)
//		{
//			strncpy(buf, p, sizeof(buf));
//			buf[sizeof(buf)-1] = '\0';
//
//			bf_write *pBuffer = engine->UserMessageBegin(&filter, umsg);
//			pBuffer->WriteShort(buttons);       // Sets how many options the menu has
//			pBuffer->WriteChar(time);           // Sets how long the menu stays open -1 for stay until option selected
//			pBuffer->WriteByte(true);           // more?
//			pBuffer->WriteString(buf);          // The text shown on the menu
//			engine->MessageEnd();
//
//			p += sizeof(buf) - 1;
//		}
//		// then send last bit
//		bf_write *pBuffer = engine->UserMessageBegin(&filter, umsg);
//		pBuffer->WriteShort(buttons);       // Sets how many options the menu has
//		pBuffer->WriteChar(time);           // Sets how long the menu stays open -1 for stay until option selected
//		pBuffer->WriteByte(false);          // more?
//		pBuffer->WriteString(p);            // The text shown on the menu
//		engine->MessageEnd();
//	}
//}

//===========================================================================//

// useful helper func
//inline bool FStrEq(const char *sz1, const char *sz2)
//{
//	return(Q_stricmp(sz1, sz2) == 0);
//}

//template<typename T>
//T *UTIL_LoadInterface(CreateInterfaceFn pfnFactory, const char *pszInterfaceVersion, bool &bSuccess)
//{
//	int nResult = 0;
//	T *pInterface = (T*)pfnFactory(pszInterfaceVersion, &nResult);
//
//	if(nResult == IFACE_OK)
//	{
//		DevMsg("UTIL_LoadInterface: factory reported interface \"%s\" loaded successfully\n", pszInterfaceVersion);
//	}
//	else
//	{
//		Warning("UTIL_LoadInterface: pfnFactory(%s, %p) => IFACE_FAIL\n", pszInterfaceVersion, &nResult);
//		bSuccess = false;
//		return NULL;
//	}
//
//	if(pInterface == NULL)
//	{
//		Warning("UTIL_LoadInterface: pfnFactory(%s, %p) => NULL\n", pszInterfaceVersion, &nResult);
//		bSuccess = false;
//		return NULL;
//	}
//
//	Msg("UTIL_LoadInterface: interface \"%s\" pointer appears valid, factory reports that the interface loaded successfully\n", pszInterfaceVersion);
//
//	return pInterface;
//}

//template <typename T>
//T *GetInterface( const char	*pchModuleName, const char *pszInterfaceVersion )
//{
//	int nReturnCode = 0;
//	CDllDemandLoader module = CDllDemandLoader( pchModuleName );
//	T *pInterface = (T*)module.GetFactory()(pszInterfaceVersion, &nReturnCode);
//	module.Unload();
//	//if (nReturnCode == IFACE_FAILED)
//	//	return NULL;
//	//else
//	//	return pInterface;
//
//	return (nReturnCode == IFACE_FAILED) ? NULL : pInterface;
//}

//---------------------------------------------------------------------------------
// Purpose: a sample 3rd party plugin class
//---------------------------------------------------------------------------------
class CEmptyServerPlugin: public IServerPluginCallbacks, public IGameEventListener2, public ICommandCallback
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

	// Additions
	virtual bool			ConfirmInterfaces( void );
	void					LoadCurrentPlayers();
	//virtual bool			GetPropOffset( const char *pClassName, const char *pPropName, unsigned int &offset, bool bServerSide );
	//virtual void			GetPropsFromTable( const char *pTableName );
	//virtual void			GetMessageInts( void );

	virtual void	CommandCallback( const CCommand &command )
	{
		if ( SCHelpers::FStrEq(command.GetCommandString(), "tryend"))
		{
			m_SizzlingStats.SS_EndOfRound();
		}
	}

	// IGameEventListener Interface
	virtual void FireGameEvent( IGameEvent *event );

	virtual int GetCommandIndex() { return m_iClientCommandIndex; }

private:
	void GetGameRules()
	{
		if ( !m_pTeamplayRoundBasedRules )
		{
			m_pTeamplayRoundBasedRules = SCHelpers::GetTeamplayRoundBasedGameRulesPointer();
		}
	}
	
	static bool RoundStateChangeCallback(const SendProp *pProp, const void *pStructBase, const void *pData, DVariant *pOut, int iElement, int objectID)
	{
	    using namespace Teamplay_GameRule_States;
	    
	    Msg( UTIL_VarArgs( "round state is now %s\n", GetStateName(*reinterpret_cast<const gamerules_roundstate_t*>(pData)) ) );
	    return true;
	}
	
	static bool WaitingForPlayersChangeCallback(const SendProp *pProp, const void *pStructBase, const void *pData, DVariant *pOut, int iElement, int objectID)
	{
	    bool bWaiting = *reinterpret_cast<const bool*>(pData);
	    if (bWaiting)
	    {
	        Msg( "waiting for players, if mp_tournament is 1, a tournament game is not active\n" );
	    }
	    else
	    {
	        Msg( "not waiting for players, if mp_tournament is 1, a tournament game is active\n" );
	    }
	    return true;
	}
	
	void HookProps()
	{
	    using namespace SCHelpers;
	    
	    bool bError = false;
	    // is there a possibility that the hooking will fail?
        unsigned int gamerulesoffset = GetPropOffsetFromTable( "DT_TFGameRulesProxy", "baseclass", bError ) +
		    GetPropOffsetFromTable( "DT_TeamplayRoundBasedRulesProxy", "teamplayroundbased_gamerules_data", bError );

        SendProp *piRoundState = GetPropFromTable( "DT_TeamplayRoundBasedRules", "m_iRoundState" );
        if (piRoundState)
        {
            m_iRoundStateOffset = gamerulesoffset + piRoundState->GetOffset();
            m_iRoundStateHook.Hook( piRoundState, &CEmptyServerPlugin::RoundStateChangeCallback );
        }
        
        SendProp *pbInWaitingForPlayers = GetPropFromTable( "DT_TeamplayRoundBasedRules", "m_bInWaitingForPlayers" );
        if (pbInWaitingForPlayers)
        {
            m_bInWaitingForPlayersOffset = gamerulesoffset + pbInWaitingForPlayers->GetOffset();
            m_bInWaitingForPlayersHook.Hook( pbInWaitingForPlayers, &CEmptyServerPlugin::WaitingForPlayersChangeCallback );
        }
	}
	
	void UnhookProps()
	{
	    m_iRoundStateHook.Unhook();
	    m_bInWaitingForPlayersHook.Unhook();
	}

private:
	SizzlingStats m_SizzlingStats;
	CSayHook m_SayHook;
	CSayTeamHook m_SayTeamHook;
	CSendPropHook	m_iRoundStateHook;
	CSendPropHook	m_bInWaitingForPlayersHook;
	CAutoUpdateThread	*m_pAutoUpdater;
	CTeamplayRoundBasedRules *m_pTeamplayRoundBasedRules;
	unsigned int	m_iRoundStateOffset;
	unsigned int	m_bInWaitingForPlayersOffset;
	int m_iClientCommandIndex;
	bool m_bShouldRecord;
#ifdef COUNT_CYCLES
		CCycleCount m_CycleCount;
#endif
	//CUtlMap<char *, int> m_MessageNameToIDMap;
	//const char *m_pPropNames[20];
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
	m_SizzlingStats(),
	m_SayHook(),
	m_SayTeamHook(),
	m_iRoundStateHook(),
	m_bInWaitingForPlayersHook(),
	m_pAutoUpdater(NULL),
	m_pTeamplayRoundBasedRules(NULL),
	m_iRoundStateOffset(0),
	m_bInWaitingForPlayersOffset(0),
	m_iClientCommandIndex(0),
	m_bShouldRecord(false)
{
}

static ConCommand test("tryend", &g_EmptyServerPlugin);

CEmptyServerPlugin::~CEmptyServerPlugin()
{
}

//---------------------------------------------------------------------------------
// Purpose: called when the plugin is loaded, load the interface we need from the engine
//---------------------------------------------------------------------------------
bool CEmptyServerPlugin::Load(	CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory )
{
	//ConnectTier1Libraries( &interfaceFactory, 1 );
	//ConnectTier2Libraries( &interfaceFactory, 1 );

	curl_global_init(CURL_GLOBAL_ALL);

	autoUpdateInfo_t a = { PLUGIN_PATH PLUGIN_NAME, URL_TO_UPDATED, URL_TO_META, PLUGIN_PATH, 0, PLUGIN_VERSION };
	m_pAutoUpdater = new CAutoUpdateThread(a, s_pluginInfo);
	m_pAutoUpdater->StartThread();

	//AutoUpdate();
	g_pFullFileSystem = (IFileSystem *)interfaceFactory(FILESYSTEM_INTERFACE_VERSION, NULL);
	if (!g_pFullFileSystem){
		Warning( "Unable to load g_pFullFileSystem, aborting load\n" );
		return false;
	}

	if ( !cvar )
	{
	    Warning( "[SizzlingStats]: cvar is null.\n" );
	    cvar = (ICvar*)interfaceFactory(CVAR_INTERFACE_VERSION, NULL);
	    if ( !cvar )
	    {
	        Warning( "[SizzlingStats]: Couldn't get cvar, aborting load.\n" );
	        return false;
	    }
		Warning( "[SizzlingStats]: got cvar.\n" );
	}

	if ( !g_pCVar )
	{
	    Warning( "linking g_pCVar to cvar\n" );
	    g_pCVar = cvar;
	}

	g_pUserIdTracker->Load();

	//CSysModule *pClientModule = g_pFullFileSystem->LoadModule("bin/client.dll", "MOD", false);
	//if(!pClientModule)
	//{
	//	Warning("Unable to find client DLL module (are you on a dedicated server?)\n");
	//	return false;
	//}

	//CreateInterfaceFn pfnClientFactory = Sys_GetFactory(pClientModule);
	//if(!interfaceFactory)
	//{
	//	Warning("Unable to retrieve client factory\n");
	//	return false;
	//}

	playerinfomanager = (IPlayerInfoManager *)gameServerFactory(INTERFACEVERSION_PLAYERINFOMANAGER,NULL);
	pEngine = (IVEngineServer*)interfaceFactory(INTERFACEVERSION_VENGINESERVER, NULL);
	gameeventmanager = (IGameEventManager2*)interfaceFactory(INTERFACEVERSION_GAMEEVENTSMANAGER2, NULL);
	helpers = (IServerPluginHelpers*)interfaceFactory(INTERFACEVERSION_ISERVERPLUGINHELPERS, NULL);
	g_pServerPluginHandler = (s_ServerPlugin*)interfaceFactory(INTERFACEVERSION_ISERVERPLUGINHELPERS, NULL);

	pServerEnts = (IServerGameEnts *)gameServerFactory(INTERFACEVERSION_SERVERGAMEENTS, NULL);
	pServerDLL = (IServerGameDLL *)gameServerFactory(INTERFACEVERSION_SERVERGAMEDLL, NULL);
#ifndef SERVER_BUILD
	pEngineClient = (IVEngineClient *)interfaceFactory(VENGINE_CLIENT_INTERFACE_VERSION, NULL);
	//pEngineClient = UTIL_LoadInterface<IVEngineClient>(interfaceFactory, VENGINE_CLIENT_INTERFACE_VERSION, bSucess);
	//pBaseClientDLL = UTIL_LoadInterface<IBaseClientDLL>(pfnClientFactory, CLIENT_DLL_INTERFACE_VERSION, bSucess);

	pBaseClientDLL = GetInterface<IBaseClientDLL>( "tf/bin/client.dll", CLIENT_DLL_INTERFACE_VERSION );
#endif

	if ( !ConfirmInterfaces() ){
		return false;
	}

	gpGlobals = playerinfomanager->GetGlobalVars();

	GetGameRules();
	HookProps();

	gameeventmanager->AddListener( this, "teamplay_round_stalemate", true );
	gameeventmanager->AddListener( this, "teamplay_round_active", true );		// 9:54
	gameeventmanager->AddListener( this, "teamplay_round_win", true );			// end round
	//gameeventmanager->AddListener( this, "teamplay_point_captured", true );		// point captured
	gameeventmanager->AddListener( this, "arena_win_panel", true );
	gameeventmanager->AddListener( this, "teamplay_win_panel", true );			//fix for incorrect round points
	gameeventmanager->AddListener( this, "player_chat", true );			// player types in chat
	gameeventmanager->AddListener( this, "player_changename", true );	// player changes name
	gameeventmanager->AddListener( this, "player_healed", true );		// player healed (not incl buffs)
	gameeventmanager->AddListener( this, "teamplay_game_over", true );	// happens when mp_winlimit is met
	gameeventmanager->AddListener( this, "tf_game_over", true );		// happens when mp_timelimit is met
	gameeventmanager->AddListener( this, "medic_death", true );			// when a medic dies
	gameeventmanager->AddListener( this, "player_say", true );
	gameeventmanager->AddListener( this, "tournament_stateupdate", true ); // for getting team names
	//gameeventmanager->AddListener( this, "player_shoot", true );		// for accuracy stats

	//gameeventmanager->AddListener( this, "player_chargedeployed", true );	// when a medic deploys uber/kritz
	//gameeventmanager->AddListener( this, "player_spawn", true );	// when a player spawns...

	//gameeventmanager->AddListener( this, "teamplay_suddendeath_end", true );
	//gameeventmanager->AddListener( this, "teamplay_overtime_end", true );

	m_SizzlingStats.Load();

	LoadCurrentPlayers();
	//Name: 	player_changename
	//Structure: 	
	//short 	userid 	user ID on server
	//string 	oldname 	players old (current) name
	//string 	newname 	players new name 

	//Name: 	player_chat
	//Structure: 	
	//bool 	teamonly 	true if team only chat
	//short 	userid 	chatting player
	//string 	text 	chat text 

	m_SayHook.Hook(cvar, "say");
	m_SayTeamHook.Hook(cvar, "say_team");

	MathLib_Init( 2.2f, 2.2f, 0.0f, 2 );
	ConVar_Register( 0 );
	return true;
}

//---------------------------------------------------------------------------------
// Purpose: called when the plugin is unloaded (turned off)
//---------------------------------------------------------------------------------
void CEmptyServerPlugin::Unload( void )
{
	m_SayHook.Unhook(cvar);
	m_SayTeamHook.Unhook(cvar);
	if (pEngine)
	{
		pEngine->LogPrint("Unload\n");
	}
	m_SizzlingStats.SS_Msg( "plugin unloading\n" );
	if (gameeventmanager)
	{
		gameeventmanager->RemoveListener( this ); // make sure we are unloaded from the event system
	}

	m_SizzlingStats.SS_DeleteAllPlayerData();

	m_SizzlingStats.Unload();
	
	UnhookProps();

	if (cvar)
	{
		ConVar_Unregister( );
	}

	m_pAutoUpdater->ShutDown();
	delete m_pAutoUpdater;
	m_pAutoUpdater = NULL;

	curl_global_cleanup();
	//DisconnectTier2Libraries( );
	//DisconnectTier1Libraries( );
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

void CEmptyServerPlugin::LoadCurrentPlayers()
{
	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		edict_t *pEdict = pEngine->PEntityOfEntIndex(i);
		g_pUserIdTracker->ClientActive( pEdict );
		m_SizzlingStats.SS_InsertPlayer( pEdict );
	}
}

//---------------------------------------------------------------------------------
// Purpose: the name of this plugin, returned in "plugin_print" command
//---------------------------------------------------------------------------------
const char *CEmptyServerPlugin::GetPluginDescription( void )
{
	return "SizzlingStats v" PLUGIN_VERSION ", SizzlingCalamari. " __DATE__;
}

//---------------------------------------------------------------------------------
// Purpose: called on level start
//---------------------------------------------------------------------------------
void CEmptyServerPlugin::LevelInit( char const *pMapName )
{
	pEngine->LogPrint(UTIL_VarArgs( "LevelInit: %s\n", pMapName ));
	pEngine->LogPrint( "[SizzlingStats]: Attempting update.\n" );
	m_pAutoUpdater->StartThread();
	pEngine->LogPrint( "[SizzlingStats]: Update attempt complete.\n" );
	GetGameRules();
	
	m_SizzlingStats.LevelInit(pMapName);
}

//---------------------------------------------------------------------------------
// Purpose: called on level start, when the server is ready to accept client connections
//		edictCount is the number of entities in the level, clientMax is the max client count
//---------------------------------------------------------------------------------
void CEmptyServerPlugin::ServerActivate( edict_t *pEdictList, int edictCount, int clientMax )
{
}

//---------------------------------------------------------------------------------
// Purpose: called once per server frame, do recurring work here (like checking for timeouts)
//---------------------------------------------------------------------------------
void CEmptyServerPlugin::GameFrame( bool simulating )
{
	//if ( simulating )
	//{
		g_pTSCallQueue->callQueueGameFrame();
	//	curtime = engine->Time();
	//	if ( curtime - oldtime >= 10.0 ){
	//		oldtime = curtime;
	//		g_pMessage->AllUserHudMsg( "HELLO WHALE\n" );
	//		g_pMessage->AllUserHudHintText( "jajajaja\n" );
	//	}
	//	Bot_RunAll();
	//}
		/*
	int *state = ((int *)((unsigned char*)g_pTFGameRulesProxy + 836));
	bool *bWaiting = ((bool *)((unsigned char*)g_pTFGameRulesProxy + 852));
	if (state && bWaiting)
	{
		static int oldstate = 0;
		static bool waiting = false;
		if (oldstate != *state)
		{
			//g_pMessage->AllUserHudHintText( UTIL_VarArgs( "state changed from %i to %i\n", oldstate, *state ) );
			Msg( "~~~~~~~~~\n" );
			Msg( "~~~~~~~~~\n" );
			Msg( "~~~~~~~~~\n" );
			Msg( "state changed from %i to %i\n", oldstate, *state );
			Msg( "~~~~~~~~~\n" );
			Msg( "~~~~~~~~~\n" );
			Msg( "~~~~~~~~~\n" );
			oldstate = *state;
		}
		if (waiting != *bWaiting)
		{
			//g_pMessage->AllUserHudHintText( UTIL_VarArgs( "state changed from %i to %i\n", oldstate, *state ) );
			Msg( "~~~~~~~~~\n" );
			Msg( "~~~~~~~~~\n" );
			Msg( "~~~~~~~~~\n" );
			if (!waiting)
				Msg( "waiting for players\n" );
			else
				Msg( "not waiting for players\n" );
			Msg( "~~~~~~~~~\n" );
			Msg( "~~~~~~~~~\n" );
			Msg( "~~~~~~~~~\n" );
			waiting = *bWaiting;
		}
	}*/
}

//---------------------------------------------------------------------------------
// Purpose: called on level end (as the server is shutting down or going to a new map)
//---------------------------------------------------------------------------------
void CEmptyServerPlugin::LevelShutdown( void ) // !!!!this can get called multiple times per map change
{
#ifdef COUNT_CYCLES
	Msg( "clock cycles: %i\n", m_CycleCount.GetCycles() );
	Msg( "milliseconds: %i\n", m_CycleCount.GetMilliseconds() );
	m_CycleCount.Init( (int64)0 );
#endif
	pEngine->LogPrint("LevelShutdown\n");
	m_SizzlingStats.SS_DeleteAllPlayerData();
}

//---------------------------------------------------------------------------------
// Purpose: called when a client spawns into a server (i.e as they begin to play)
//---------------------------------------------------------------------------------
void CEmptyServerPlugin::ClientActive( edict_t *pEdict )
{
	pEngine->LogPrint(UTIL_VarArgs( "ClientActive: %u, %u\n", (unsigned int)pEdict, pEngine->IndexOfEdict(pEdict) ));
	if( !pEdict || pEdict->IsFree() )		//TODO: decide on having the null and isfree check in the insert, or outside
		return;								// for now... BOTH!	heuheuuehuhuehuhe

	g_pUserIdTracker->ClientActive( pEdict );
	m_SizzlingStats.SS_InsertPlayer( pEdict );
}

//---------------------------------------------------------------------------------
// Purpose: called when a client leaves a server (or is timed out)
//---------------------------------------------------------------------------------
void CEmptyServerPlugin::ClientDisconnect( edict_t *pEdict )
{
	pEngine->LogPrint(UTIL_VarArgs( "ClientDisconnect: %u, %u\n", (unsigned int)pEdict, pEngine->IndexOfEdict(pEdict) ));
	if( !pEdict || pEdict->IsFree() )
		return;
	m_SizzlingStats.SS_DeletePlayer( pEdict );
	g_pUserIdTracker->ClientDisconnect( pEdict );

}

//---------------------------------------------------------------------------------
// Purpose: called on 
//---------------------------------------------------------------------------------
void CEmptyServerPlugin::ClientPutInServer( edict_t *pEntity, char const *playername )
{
	pEngine->LogPrint(UTIL_VarArgs( "ClientPutInServer: %s, %u, %u\n", playername, (unsigned int)pEntity, pEngine->IndexOfEdict(pEntity) ));
	if( !pEntity || pEntity->IsFree() )
		return;

	IPlayerInfo *pPlayerInfo = playerinfomanager->GetPlayerInfo( pEntity );
	if (pPlayerInfo)
		Msg("player: %s put in server\n", pPlayerInfo->GetName() );
	else
		Msg("ClientPutInServer error\n");

	//KeyValues *kv = new KeyValues( "msg" );
	//kv->SetString( "title", "Hello" );
	//kv->SetString( "msg", "Hello there" );
	//kv->SetColor( "color", Color( 255, 0, 0, 255 ));
	//kv->SetInt( "level", 5);
	//kv->SetInt( "time", 10);
	//helpers->CreateMessage( pEntity, DIALOG_MSG, kv, this );
	//kv->deleteThis();
}

//---------------------------------------------------------------------------------
// Purpose: called on level start
//---------------------------------------------------------------------------------
void CEmptyServerPlugin::SetCommandClient( int index )
{
	++index;
	m_iClientCommandIndex = index;
	m_SayHook.SetCommandClient(index);
	m_SayTeamHook.SetCommandClient(index);
}

//---------------------------------------------------------------------------------
// Purpose: called on level start
//---------------------------------------------------------------------------------
void CEmptyServerPlugin::ClientSettingsChanged( edict_t *pEdict )
{
	//if ( playerinfomanager )
	//{
	//	IPlayerInfo *playerinfo = playerinfomanager->GetPlayerInfo( pEdict );

	//	const char * name = engine->GetClientConVarValue( engine->IndexOfEdict(pEdict), "name" );

	//	if ( playerinfo && name && playerinfo->GetName() && 
	//		 Q_stricmp( name, playerinfo->GetName()) ) // playerinfo may be NULL if the MOD doesn't support access to player data 
	//												   // OR if you are accessing the player before they are fully connected
	//	{
	//		char msg[128];
	//		Q_snprintf( msg, sizeof(msg), "Your name changed to \"%s\" (from \"%s\"\n", name, playerinfo->GetName() ); 
	//		engine->ClientPrintf( pEdict, msg ); // this is the bad way to check this, the better option it to listen for the "player_changename" event in FireGameEvent()
	//											// this is here to give a real example of how to use the playerinfo interface
	//	}
	//}
}

//---------------------------------------------------------------------------------
// Purpose: called when a client joins a server
//---------------------------------------------------------------------------------
PLUGIN_RESULT CEmptyServerPlugin::ClientConnect( bool *bAllowConnect, edict_t *pEntity, const char *pszName, const char *pszAddress, char *reject, int maxrejectlen )
{
	pEngine->LogPrint(UTIL_VarArgs( "ClientConnect: %s, %u, %u\n", pszName, (unsigned int)pEntity, pEngine->IndexOfEdict(pEntity) ));
	IPlayerInfo *pPlayerInfo = playerinfomanager->GetPlayerInfo( pEntity );
	if (pPlayerInfo)
		Msg("player: %s connected\n", pPlayerInfo->GetName() );
	else
		Msg("ClientConnect error\n");

	return PLUGIN_CONTINUE;
}

//CON_COMMAND( DoAskConnect, "Server plugin example of using the ask connect dialog" )
//{
//	if ( args.ArgC() < 2 )
//	{
//		Warning ( "DoAskConnect <server IP>\n" );
//	}
//	else
//	{
//		const char *pServerIP = args.Arg( 1 );
//
//		KeyValues *kv = new KeyValues( "menu" );
//		kv->SetString( "title", pServerIP );	// The IP address of the server to connect to goes in the "title" field.
//		kv->SetInt( "time", 3 );
//
//		for ( int i=1; i < gpGlobals->maxClients; i++ )
//		{
//			edict_t *pEdict = engine->PEntityOfEntIndex( i );
//			if ( pEdict )
//			{
//				helpers->CreateMessage( pEdict, DIALOG_ASKCONNECT, kv, &g_EmtpyServerPlugin );
//			}
//		}
//
//		kv->deleteThis();
//	}
//}

//---------------------------------------------------------------------------------
// Purpose: called when a client types in a command (only a subset of commands however, not CON_COMMAND's)
//---------------------------------------------------------------------------------
PLUGIN_RESULT CEmptyServerPlugin::ClientCommand( edict_t *pEntity, const CCommand &args )
{
	const char *pcmd = args[0];

	if ( !pEntity || pEntity->IsFree() ) 
	{
		return PLUGIN_CONTINUE;
	}

	if ( SCHelpers::FStrEq( pcmd, "closed_htmlpage" ) )
	{
		Msg( "index: %i, closed_htmlpage\n", m_iClientCommandIndex );
		//g_pMessage->SingleUserEmptyVGUIMenu( m_iClientCommandIndex );
	}

	//if ( FStrEq( pcmd, "SA_changefov" ) )
	//{
	//	KeyValues *kv = new KeyValues( "SA_changefov" );

	//}

	//if ( FStrEq( pcmd, "menu" ) )
	//{
	//	KeyValues *kv = new KeyValues( "menu" );
	//	kv->SetString( "title", "You've got options, hit ESC" );
	//	kv->SetInt( "level", 1 );
	//	kv->SetColor( "color", Color( 255, 0, 0, 255 ));
	//	kv->SetInt( "time", 20 );
	//	kv->SetString( "msg", "Pick an option\nOr don't." );
	//	
	//	for( int i = 1; i < 9; i++ )
	//	{
	//		char num[10], msg[10], cmd[10];
	//		Q_snprintf( num, sizeof(num), "%i", i );
	//		Q_snprintf( msg, sizeof(msg), "Option %i", i );
	//		Q_snprintf( cmd, sizeof(cmd), "option%i", i );

	//		KeyValues *item1 = kv->FindKey( num, true );
	//		item1->SetString( "msg", msg );
	//		item1->SetString( "command", cmd );
	//	}

	//	helpers->CreateMessage( pEntity, DIALOG_MENU, kv, this );
	//	kv->deleteThis();
	//	return PLUGIN_STOP; // we handled this function
	//}
	//else if ( FStrEq( pcmd, "rich" ) )
	//{
	//	KeyValues *kv = new KeyValues( "menu" );
	//	kv->SetString( "title", "A rich message" );
	//	kv->SetInt( "level", 1 );
	//	kv->SetInt( "time", 20 );
	//	kv->SetString( "msg", "This is a long long long text string.\n\nIt also has line breaks." );
	//	
	//	helpers->CreateMessage( pEntity, DIALOG_TEXT, kv, this );
	//	kv->deleteThis();
	//	return PLUGIN_STOP; // we handled this function
	//}
	//else if ( FStrEq( pcmd, "msg" ) )
	//{
	//	KeyValues *kv = new KeyValues( "menu" );
	//	kv->SetString( "title", "Just a simple hello" );
	//	kv->SetInt( "level", 1 );
	//	kv->SetInt( "time", 20 );
	//	
	//	helpers->CreateMessage( pEntity, DIALOG_MSG, kv, this );
	//	kv->deleteThis();
	//	return PLUGIN_STOP; // we handled this function
	//}
	//else if ( FStrEq( pcmd, "entry" ) )
	//{
	//	KeyValues *kv = new KeyValues( "entry" );
	//	kv->SetString( "title", "Stuff" );
	//	kv->SetString( "msg", "Enter something" );
	//	kv->SetString( "command", "say" ); // anything they enter into the dialog turns into a say command
	//	kv->SetInt( "level", 1 );
	//	kv->SetInt( "time", 20 );
	//	
	//	helpers->CreateMessage( pEntity, DIALOG_ENTRY, kv, this );
	//	kv->deleteThis();
	//	return PLUGIN_STOP; // we handled this function		
	//}

	return PLUGIN_CONTINUE;
}

//---------------------------------------------------------------------------------
// Purpose: called when a client is authenticated
//---------------------------------------------------------------------------------
PLUGIN_RESULT CEmptyServerPlugin::NetworkIDValidated( const char *pszUserName, const char *pszNetworkID )
{
	pEngine->LogPrint(UTIL_VarArgs( "NetworkIDValidated: %s, %s\n", pszUserName, pszNetworkID ));
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
	if (!playerinfomanager){
		Warning( "Unable to load playerinfomanager, aborting load\n" );
		return false;
	}
	if (!pEngine){
		Warning( "Unable to load engine, aborting load\n" );
		return false;
	}

	if (!gameeventmanager){
		Warning( "Unable to load gameeventmanager, aborting load\n" );
		return false;
	}

	if (!helpers){
		Warning( "Unable to load helpers, aborting load\n" );
		return false;
	}

#ifndef SERVER_BUILD
	if (!pEngineClient){
		Warning( "Unable to load pEngineClient, aborting load\n" );
		return false;
	}

	if (!pBaseClientDLL){
		Warning( "Unable to load pBaseClientDLL, aborting load\n" );
		return false;
	}
#endif
	if (!pServerDLL){
		Warning( "Unable to load pServerDLL, aborting load\n" );
		return false;
	}

	if (!pServerEnts){
		Warning( "Unable to load pServerEnts, aborting load\n" );
		return false;
	}

	Msg( "All interfaces sucessfully loaded\n" );
	return true;
}

//void CEmptyServerPlugin::GetMessageInts()
//{
//	//Returns MessageIndex IDs from Msg String
//	char pName[64] = "";
//	int iSize = 0;
//	int i = 0;
//	bool bReturn = true;
//	CStringPool sPool;
//
//	while (bReturn)
//	{
//		bReturn =  pServerDLL->GetUserMessageInfo(++i, pName, 64, iSize);
//		if (bReturn)
//		{
//			const char *formattedname = sPool.Allocate(pName);
//			m_MessageNameToIDMap.Insert( formattedname, i );
//			Msg("name: %s, %i\n", formattedname, i);
//			Msg("name: %s, %i\n", formattedname, m_MessageNameToIDMap[ m_MessageNameToIDMap.Find(formattedname) ] );
//		}
//	}
//
//	Msg("name: %s, %i\n", "SayText", m_MessageNameToIDMap[ m_MessageNameToIDMap.Find( sPool.Find("SayText") ) ] );
//	Msg("name: %s, %i\n", "SayText", m_MessageNameToIDMap[ m_MessageNameToIDMap.Find( "SayText" )] );
//	return; 
//}

#ifndef SERVER_BUILD
void RecurseClientTable( RecvTable *pTable, int &spacing )
{
	RecvTable *pRecvTable = pTable;
	if (pRecvTable == NULL){
		spacing--;
		return;
	}
	
	char TableName[128];
	int size = sizeof(TableName);

	memset( TableName, 0, size );
	for (int i = 0; i < spacing; i++)
		V_strcat( TableName, "  |", size );
	V_strcat( TableName, pRecvTable->GetName(), size );
	Msg( "%s\n", TableName );

	spacing++;
	int num = pRecvTable->GetNumProps();
	for (int i = 0; i < num; i++)
	{
		RecvProp *pProp = pRecvTable->GetProp(i);

		memset( TableName, 0, sizeof(TableName) );
		for (int j = 0; j < spacing; j++)
			V_strcat( TableName, "  |", size );
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
#endif

#ifdef GetProp
#undef GetProp
#endif

void RecurseServerTable( SendTable *pTable, int &spacing )
{
	SendTable *pSendTable = pTable;
	if (pSendTable == NULL){
		spacing--;
		return;
	}
	
	char TableName[128];
	int size = sizeof(TableName);

	memset( TableName, 0, size );
	for (int i = 0; i < spacing; i++)
		V_strcat( TableName, "  |", size );
	V_strcat( TableName, pSendTable->GetName(), size );
	Msg( "%s\n", TableName );

	spacing++;
	int num = pSendTable->GetNumProps();
	for (int i = 0; i < num; i++)
	{
		SendProp *pProp = pSendTable->GetProp(i);

		memset( TableName, 0, sizeof(TableName) );
		for (int j = 0; j < spacing; j++)
			V_strcat( TableName, "  |", size );
		V_strcat( TableName, pProp->GetName(), size );
		Msg( "%s, Offset: %i\n", TableName, pProp->GetOffset() );

		RecurseServerTable( pProp->GetDataTable(), ++spacing );
	}
	spacing-=2;
}

CON_COMMAND ( printservertables, "prints the server tables ya" )
{
	ServerClass *pClass = pServerDLL->GetAllServerClasses();
	while ( pClass )
	{
		Msg("%s\n", pClass->m_pNetworkName );
		SendTable *pTable = pClass->m_pTable;
		int i = 1;
		RecurseServerTable( pTable, i );
		Msg("\n");
		pClass = pClass->m_pNext;
	}
}

//void CreateMenu(bf_write* pBuffer, const char* szMessage, int nOptions, int iSecondsToStayOpen)
//{
//	Assert(pBuffer);
// 
//	// Add option to bits
//	int optionBits = 0;
//	for(int i = 0; i < nOptions; i++)
//		optionBits |= (1<<i);
// 
//	pBuffer->WriteShort(optionBits); // Write options
//	pBuffer->WriteChar( iSecondsToStayOpen ); // Seconds to stay open
//	pBuffer->WriteByte(false); // We don't need to receive any more of this menu
//	pBuffer->WriteString(szMessage); // Write the menu message
//}

//void HUD_printf (char *string, edict_t *pPlayerEdict)
//{
//   // this function sends "string" over the network for display on pPlayerEdict's HUD
// 
//   bf_write *netmsg; // our network message object
//   SimpleRecipientFilter recipient_filter; // the corresponding recipient filter
// 
//   // BUG FIX: the Source engine "eats" the last character from HUD SayText messages :(
//   strcat (string, "\n"); // so add an arbitrary carriage return to the string :)
// 
//   // pPlayerEdict is a pointer to the edict of a player for which you want this HUD message
//   // to be displayed. There can be several recipients in the filter, don't forget.
//   recipient_filter.AddPlayer (engine->IndexOfEdict (pPlayerEdict));
// 
//   // HACK: Valve doesn't permit server plugins to know about net messages yet, we have
//   // to figure out ourselves the message numbers, which is not nice. Here "SayText" is 3.
//   // Hopefully this network message code being in game_shared, we can assume many game DLLs
//   // will keep the same message number. Until Valve gives us another interface...
//   netmsg = engine->UserMessageBegin (&recipient_filter, 3);
//   netmsg->WriteByte (0); // index of the player this message comes from. "0" means the server.
//   netmsg->WriteString (string); // the HUD message itself
//   netmsg->WriteByte (1); // I don't know yet the purpose of this byte, it can be 1 or 0
//   engine->MessageEnd ();
// 
//   return; // et voilà
//}

//---------------------------------------------------------------------------------
// Purpose: called when an event is fired
//---------------------------------------------------------------------------------
void CEmptyServerPlugin::FireGameEvent( IGameEvent *event )
{
#ifdef COUNT_CYCLES
	CTimeAdder Timer( &m_CycleCount );
#endif
	using namespace SCHelpers;

	const char * name = event->GetName();
	//Msg( "CEmptyServerPlugin::FireGameEvent: Got event \"%s\"\n", name );
/*	if ( FStrEq( name, "player_shoot" ) )
	{
		Msg( "userid %i fired weapon\n", event->GetInt( "userid" ) );
		int userid = event->GetInt( "userid" );
		m_SizzlingStats.m_UserIDToExtraDataMap.Element( m_SizzlingStats.m_UserIDToExtraDataMap.Find( userid ) ).shots_fired += 1;
	}
	else */if ( m_bShouldRecord && FStrEq( name, "player_healed" ) )
	{
		IGameEvent &gameevent = *event;
		int patient = gameevent.GetInt( "patient" );
		if ( patient == gameevent.GetInt( "healer" ) )
			return;

		//m_SizzlingStats.m_UserIDToExtraDataMap.Element( m_SizzlingStats.m_UserIDToExtraDataMap.Find( patient ) ).healsrecv += gameevent.GetInt( "amount" );
		int patientIndex = g_pUserIdTracker->GetEntIndex( patient );
		if (patientIndex > 0)
			m_SizzlingStats.getEntIndexToExtraData()[patientIndex]->healsrecv += gameevent.GetInt( "amount" );
	}
	//else if ( FStrEq( name, "player_spawn" ) )
	//{
	//	int nClass = event->GetInt( "class" );
	//	if ( nClass != 5 )	// 5 is medic
	//		return;

	//	int userid = event->GetInt( "userid" );
	//	m_SizzlingStats.m_UserIDToExtraDataMap.Element( m_SizzlingStats.m_UserIDToExtraDataMap.Find( userid ) ).currentubertime
	//}
	//else if ( FStrEq( name, "player_chargedeployed" ) )
	//{
	//	int medic = event->GetInt( "userid" );

	//}
	else if ( m_bShouldRecord && FStrEq( name, "medic_death" ) )
	{
		IGameEvent &gameevent = *event;
		int killerIndex = g_pUserIdTracker->GetEntIndex( gameevent.GetInt( "attacker" ) );	// med picks
		int victimIndex = g_pUserIdTracker->GetEntIndex( gameevent.GetInt( "userid" ) );
		bool charged = gameevent.GetBool( "charged" ); // med drops

		if ( killerIndex > 0 && killerIndex != victimIndex )
			m_SizzlingStats.getEntIndexToExtraData()[killerIndex]->medpicks += 1;

		if ( charged && victimIndex > 0 )
			m_SizzlingStats.getEntIndexToExtraData()[victimIndex]->ubersdropped += 1;
	}
	else if ( FStrEq( name, "teamplay_round_win" ) || FStrEq( name, "teamplay_round_stalemate" ) )
	{
		m_bShouldRecord = false;
		m_SizzlingStats.SS_AllUserChatMessage( "Stats Recording Stopped\n" );
		m_SizzlingStats.SS_EndOfRound();
	}
	else if ( FStrEq( name, "teamplay_round_active" ) || FStrEq( name, "arena_round_start" ) )
	{
		m_SizzlingStats.SS_ResetData();
		m_bShouldRecord = true;
		m_SizzlingStats.SS_AllUserChatMessage( "Stats Recording Started\n" );
		//GetEntityByClassname( "CTeam" );
	}
	else if ( FStrEq( name, "teamplay_win_panel" ) || FStrEq( name, "arena_win_panel" ) )
	{
		//pEngine->LogPrint(UTIL_VarArgs( "[SIZZLOG]: winreason '%i'\n", event->GetInt("winreason")));
		//pEngine->LogPrint(UTIL_VarArgs( "[SIZZLOG]: bluescore: '%i', redscore '%i'\n", event->GetInt("blue_score"), event->GetInt("red_score")));
		m_SizzlingStats.SetTeamScores(event->GetInt("red_score"), event->GetInt("blue_score"));


		//if ( event->GetInt("winreason") == 1 ) //Reason: 1=CP 3=Flag 4=defended 5=Stalemate/Death 7=flag maybe
		//{
		//	char cappers[32] = "";
		//	V_strncpy( cappers, event->GetString( "cappers" ), 32);
		//	int length = V_strlen( cappers );
		//	for (int i = 0; i < length; i++)
		//	{
		//		m_SizzlingStats.SS_CheckFixEndOfRoundCappers( cappers[i] );
		//	}
		//}
		
		//Msg( "time of win panel: %f\n", gpGlobals->curtime );
		float time = gpGlobals->curtime;
		//GetEventInt(event, "winreason");    //Reason: 1=CP 3=Flag 4=defended 5=Stalemate/Death like below.
		if ( time - m_SizzlingStats.SS_GetTimeOfLastCap() < 0.00001 )
		{
			char cappers[32] = "";
			V_strncpy( cappers, event->GetString( "cappers" ), 32);
			int length = V_strlen( cappers );
			for (int i = 0; i < length; ++i)
			{
				m_SizzlingStats.SS_CheckFixEndOfRoundCappers( cappers[i] );

				//int index = cappers[i];
				//Msg("%i captured a point\n", index );
				//m_SizzlingStats.SS_PrintIndex();
			}
		}
	}
	else if ( FStrEq( name, "teamplay_point_captured" ) )
	{
		//Msg( "time of cap: %f\n", gpGlobals->curtime );
		//Msg( "cp name: %s\n", event->GetString( "cpname" ) );
		m_SizzlingStats.SS_SetTimeOfLastCap( gpGlobals->curtime );

	}
	else if ( /*FStrEq( name, "teamplay_game_over" ) ||*/ FStrEq( name, "tf_game_over" ) )
	{
		g_pMessage->AllUserChatMessage( "\x03This server is running SizzlingStats v" PLUGIN_VERSION "\n" );
		g_pMessage->AllUserChatMessage( "\x03\x46or credits type \".ss_credits\"\n" ); // \x03F is recognised as '?'
		m_SizzlingStats.SS_GameOver();
	}
	/*else if ( FStrEq( name, "tournament_stateupdate" ) )
	{
		if (event->GetBool("namechange"))
		{
			int id = UserIDToEntIndex(event->GetInt("userid"));
			// ent index to get the team id to change the name of
			m_SizzlingStats.TeamNameChange( id, event->GetString("newname") );
		}
	}*/
	else if ( FStrEq( name, "player_say" ) )
	{
		const char *text = event->GetString( "text" );
		if ( FStrEq( text, ".ss_credits" ) )
		{
			int userid = event->GetInt( "userid" );
			int entindex = SCHelpers::UserIDToEntIndex( userid );
			if ( entindex != 0 )
				m_SizzlingStats.SS_Credits( entindex, PLUGIN_VERSION );
		}
#ifndef PUBLIC_RELEASE
		else if ( FStrEq( text, ".testshowstats" ) )
		{
			int userid = event->GetInt( "userid" );
			int entindex = SCHelpers::UserIDToEntIndex( userid );
			m_SizzlingStats.SS_ShowHtmlStats( entindex );
		}
		else if ( FStrEq( text, ".testupdatestats" ) )
		{
			m_SizzlingStats.SS_UploadStats();
		}
		else if ( FStrEq( text, ".testthreading" ) )
		{
			m_SizzlingStats.SS_TestThreading();
		}
		else if ( FStrEq( text, ".testpost" ) )
		{
			m_SizzlingStats.SS_TestPost();
		}
		else if ( FStrEq( text, ".menutest" ) )
		{
			KeyValues *kv = new KeyValues( "menu" );
			kv->SetString( "title", "You've got options, hit ESC" );
			kv->SetInt( "level", 1 );
			kv->SetColor( "color", Color( 255, 0, 0, 255 ));
			kv->SetInt( "time", 20 );
			kv->SetString( "msg", "Pick an option\nOr don't." );
				
			for( int i = 1; i < 9; i++ )
			{
				char num[10], msg[10], cmd[10];
				Q_snprintf( num, sizeof(num), "%i", i );
				Q_snprintf( msg, sizeof(msg), "Option %i", i );
				Q_snprintf( cmd, sizeof(cmd), "option%i", i );

				KeyValues *item1 = kv->FindKey( num, true );
				item1->SetString( "msg", msg );
				item1->SetString( "command", cmd );
			}

			int userid = event->GetInt( "userid" );
			int entindex = SCHelpers::UserIDToEntIndex( userid );
			if ( entindex != 0 )
				helpers->CreateMessage( pEngine->PEntityOfEntIndex(entindex), DIALOG_MENU, kv, this );
			kv->deleteThis();
		}
#endif
	}
}

//CBaseEntity *GetBaseFromID(int id) {
//   edict_t *pEntity = NULL;
//   for(int i = 1; i <= pEngineClient->GetMaxClients(); i++) { 
//      pEntity = engine->PEntityOfEntIndex(i); 
//      if(!pEntity || pEntity->IsFree()) 
//		  continue; 
//      if(engine->GetPlayerUserId(pEntity) == id) 
//		  return engine->PEntityOfEntIndex(i)->GetUnknown()->GetBaseEntity();
//   } 
//   return 0; 
//}

//void GetEntityByClassname(const char *szClassname) {
//	edict_t *pEntity = NULL;
//	//for(int i = gpGlobals->maxClients; i <= gpGlobals->maxEntities; i++) { 
//	//	pEntity = engine->PEntityOfEntIndex(i); 
//	//	if(!pEntity || pEntity->IsFree()) 
//	//		continue;
//	CBaseEntity *pBaseEntity = engine->PEntityOfEntIndex( gpGlobals->maxClients )->GetUnknown()->GetBaseEntity();
//	CBaseEntity *pClass = gEntList.FindEntityByClassname( pBaseEntity, szClassname );
//	Msg( "%s\n", pClass->GetClassname() );
//	//}
//}

//CON_COMMAND( isdedicated, "dedicated server?" )
//{
//	if (engine->IsDedicatedServer())
//		Msg("yep\n");
//	else
//		Msg("nope\n");
//}
//
//CON_COMMAND( isingame, "dedicated server?" )
//{
//	if (pEngineClient->IsInGame())
//		Msg("yep\n");
//	else
//		Msg("nope\n");
//}
//
//CON_COMMAND( isplayingdemo, "dedicated server?" )
//{
//	if (pEngineClient->IsPlayingDemo())
//		Msg("yep\n");
//	else
//		Msg("nope\n");
//}
//
//CON_COMMAND( ishltv, "dedicated server?" )
//{
//	if (pEngineClient->IsHLTV())
//		Msg("yep\n");
//	else
//		Msg("nope\n");
//}
//
//CON_COMMAND( getlocalplayer, "gets the local player" )
//{
//	IPlayerInfo *player = NULL;
//	int localplayer = pEngineClient->GetLocalPlayer();
//	Msg("localplayer: %i\n", localplayer );
//	player = playerinfomanager->GetPlayerInfo(engine->PEntityOfEntIndex( localplayer ) );
//	if (player)
//		Msg("player name: %s\n", player->GetName() );
//	else
//		Msg("player has not spawned\n");
//}

//void PrintInfo( void )
//{
//	edict_t *pEntity = engine->PEntityOfEntIndex( 1 );
//	if( !pEntity || pEntity->IsFree() )
//		return;
//
//	CBaseEntity *pPlayer = NULL;
//	pPlayer = pServerEnts->EdictToBaseEntity( pEntity );
//	if ( !pPlayer )
//		return;
//
//	unsigned int *fFlags = (unsigned int *)( ((unsigned char *)pPlayer) + oKills);
//	//Msg("flags offset: %i\n", oKills);
//	Msg( "flags: %i\n", *fFlags );
//	if ( *fFlags & FL_ONGROUND )
//		Msg("player is on teh ground\n");
//	if ( *fFlags & FL_DUCKING )
//		Msg("player is ducking\n");
//	if ( *fFlags & FL_CLIENT )
//		Msg("player is a client\n");
//	if ( *fFlags & FL_FAKECLIENT )
//		Msg("player is a fake client\n");
//	if ( *fFlags & FL_INWATER )
//		Msg("player is in water\n");
//}

//ConVar classtables_showvars( "classtables_showvars", "1", FCVAR_REPLICATED );

//CON_COMMAND( classtables_client, "Lists the client classes, and also their member variables if classtables_showvars is set" )
//{
//	int start = 0, end = INT_MAX;
//	if ( args.ArgC() > 2 )
//	{
//		start = atoi(args[1]);
//		if ( args.ArgC() > 1 )
//			end = atoi(args[2]);
//	}
//
//	int pos = 0;
//	ClientClass *pClass = pBaseClientDLL->GetAllClasses();
//	if (pClass)
//		Msg("CC name: %s\n", pClass->GetName());
//	while ( pClass )
//	{
//		if ( pos >= start && pos <= end )
//		{
//			Msg("%s (%i)\n",pClass->m_pNetworkName,pClass->m_ClassID);
//			if ( classtables_showvars.GetInt()  > 0 )
//			{
//				RecvTable *pTable = pClass->m_pRecvTable;
//				if ( pTable == NULL )
//					continue;
//				int num = pTable->GetNumProps();
//				for ( int i=0; i<num; i++ )
//				{
//					RecvProp *pProp = pTable->GetProp(i);
//					Msg("   %s\n",pProp->GetName());
//					int offset =  pProp->GetOffset();
//					Msg("		%i\n", offset);
//				}
//				Msg("\n");
//			}
//		}
//		pClass = pClass->m_pNext;
//		pos ++;
//	}
//}

//CON_COMMAND( classtables_server, "Lists the server classes, and also their member variables if classtables_showvars is set" )
//{
//	int start = 0, end = INT_MAX;
//	if ( args.ArgC() > 2 )
//	{
//		start = atoi(args[1]);
//		if ( args.ArgC() > 1 )
//			end = atoi(args[2]);
//	}
//
//	int pos = 0;
//	ServerClass *pClass = pServerDLL->GetAllServerClasses();
//	if (pClass)
//		Msg( "SC name: %s\n", pClass->GetName() );
//	while ( pClass )
//	{
//		if ( pos >= start && pos <= end )
//		{
//			Msg("%s (%i)\n",pClass->m_pNetworkName,pClass->m_ClassID);
//			if ( classtables_showvars.GetInt()  > 0 )
//			{
//				SendTable *pTable = pClass->m_pTable;
//				if ( pTable == NULL )
//					continue;
//				Msg(" %s\n", pTable->GetName() );
//				int num = pTable->GetNumProps();
//				for ( int i=0; i<num; i++ )
//				{
//					SendProp *pProp = pTable->GetProp(i);
//					Msg("   %i, %s\n",pProp->GetOffset(), pProp->GetName());
//					if ( FStrEq(pProp->GetName(), "m_Shared" ) || FStrEq(pProp->GetName(), "m_PlayerClass" )){
//						SendTable *pSSendTable = pProp->GetDataTable();
//						Msg("	%s\n", pSSendTable->GetName());
//						int num2 = pSSendTable->GetNumProps();
//						for (int j = 0; j < num2; j++)
//						{
//							SendProp *pPProp = pSSendTable->GetProp(j);
//							Msg("		%i, %s\n", pPProp->GetOffset(), pPProp->GetName() );
//							if ( FStrEq(pPProp->GetName(), "tfsharedlocaldata" ) ){
//								SendTable *pSSSendTable = pPProp->GetDataTable();
//								Msg("			%s\n", pSSSendTable->GetName());
//								int num3 = pSSSendTable->GetNumProps();
//								for (int k = 0; k < num3; k++)
//								{
//									SendProp *pPPProp = pSSSendTable->GetProp(k);
//									Msg("				%i, %s\n", pPPProp->GetOffset(), pPPProp->GetName() );
//									if ( FStrEq(pPPProp->GetName(), "m_ScoreData" ) || FStrEq(pPPProp->GetName(), "m_RoundScoreData" ) ){
//										SendTable *pSSSSendTable = pPPProp->GetDataTable();
//										Msg("					%s\n", pSSSSendTable->GetName());
//										int num4 = pSSSSendTable->GetNumProps();
//										for (int l = 0; l < num4; l++)
//										{
//											SendProp *pPPPProp = pSSSSendTable->GetProp(l);
//											Msg("						%i, %s\n", pPPPProp->GetOffset(), pPPPProp->GetName() );
//										}
//									}
//								}
//							}
//						}
//					}
//				}
//				Msg("\n");
//			}
//		}
//		pClass = pClass->m_pNext;
//		pos ++;
//	}
//}

//---------------------------------------------------------------------------------
// Purpose: an example of how to implement a new command
//---------------------------------------------------------------------------------
//CON_COMMAND( empty_version, "prints the version of the empty plugin" )
//{
//	Msg( "Version:1.0.0.0\n" );
//}

//CON_COMMAND( empty_log, "logs the version of the empty plugin" )
//{
//	engine->LogPrint( "Version:1.0.0.0\n" );
//}

//---------------------------------------------------------------------------------
// Purpose: an example cvar
//---------------------------------------------------------------------------------
//static ConVar empty_cvar("plugin_empty", "0", 0, "Example plugin cvar");

//void KickAll_SS ( const CCommand &args )
//{
//	//if ( args.ArgC() < 1 | args.Arg(1) == "" )
//	//{
//	//	Msg( "Usage: my_say <text>\n" );
//	//	return;
//	//}
//	
//	for ( int i = 1; i <= gpGlobals->maxClients; ++i )
//	{
//		edict_t *pEdict = engine->PEntityOfEntIndex( i );
//		if ( !pEdict )
//			return;
//		int id = engine->GetPlayerUserId( pEdict );
//		if ( id == -1 )
//			return;
//		char command[64] = "";
//		Q_snprintf( command, 64, "kickid %i %s\n", id, args.Arg(1) );
//		engine->ServerCommand( command );
//	}
//}
// 
//ConCommand my_say( "ss_kickall", KickAll_SS , "kicks everyone (including you) with a message of your choice", 0);
