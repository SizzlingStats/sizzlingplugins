
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

#include "dbgflag.h"

#include <stdio.h>

#include "interface.h"
#include "filesystem.h"
#include "engine/iserverplugin.h"
#include "game/server/iplayerinfo.h"
#include "eiface.h"
#include "igameevents.h"
#include "convar.h"

#include "engine/IEngineTrace.h"

#include "dt_send.h"
#include "server_class.h"

#include "PlayerMessage.h"
#include "SizzlingStats.h"
#include "ThreadCallQueue.h"

#include "SC_helpers.h"

#ifdef COUNT_CYCLES
	#include "fasttimer.h"
#endif

#include "PluginDefines.h"
#include "autoupdate.h"
#include "UserIdTracker.h"
#include "ServerPluginHandler.h"

#include "curl/curl.h"

#include "ConCommandHook.h"
#include "teamplay_gamerule_states.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Interfaces from the engine
IVEngineServer			*pEngine = NULL; // helper functions (messaging clients, loading content, making entities, running commands, etc)
IGameEventManager2		*gameeventmanager = NULL; // game events interface
IPlayerInfoManager		*playerinfomanager = NULL; // game dll interface to interact with players
//IBotManager			*botmanager = NULL; // game dll interface to interact with bots
IServerPluginHelpers	*helpers = NULL; // special 3rd party plugin helpers from the engine
IEngineTrace			*enginetrace = NULL;
//s_ServerPlugin			*g_pServerPluginHandler = NULL;
//extern UserIdTracker 	*g_pUserIdTracker;
extern CTSCallQueue		*g_pTSCallQueue;

IServerGameDLL			*pServerDLL = NULL;
IServerGameEnts			*pServerEnts = NULL;
IFileSystem			*g_pFullFileSystem = NULL;

CGlobalVars				*gpGlobals = NULL;

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

//---------------------------------------------------------------------------------
// Purpose: a sample 3rd party plugin class
//---------------------------------------------------------------------------------
class CEmptyServerPlugin: public IServerPluginCallbacks, public IGameEventListener2, public ICommandHookCallback
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
	bool	ConfirmInterfaces( void );
	void	LoadCurrentPlayers();

	virtual bool CommandPreExecute( const CCommand &args );
	virtual void CommandPostExecute( const CCommand &args, bool bWasCommandExecuted );

	// IGameEventListener Interface
	virtual void FireGameEvent( IGameEvent *event );

	virtual int GetCommandIndex() { return m_iClientCommandIndex; }

private:
	void GetGameRules();
	void GetPropOffsets();

private:
	SizzlingStats m_SizzlingStats;
	CConCommandHook m_SayHook;
	CConCommandHook m_SayTeamHook;
	CConCommandHook m_SwitchTeamsHook;
	ConVarRef m_refTournamentMode;
	CAutoUpdateThread	*m_pAutoUpdater;
	CTeamplayRoundBasedRules *m_pTeamplayRoundBasedRules;
	int	*m_iRoundState;
	bool *m_bInWaitingForPlayers;
	int m_iClientCommandIndex;
	int m_iLastCapTick;
	bool m_bShouldRecord;
	bool m_bTournamentMatchStarted;

	// this var makes sure that LevelShutdown is 
	// only called once for every LevelInit
	bool m_bAlreadyLevelShutdown;
#ifdef COUNT_CYCLES
		CCycleCount m_CycleCount;
#endif
};

// 
// The plugin is a static singleton that is exported as an interface
//
static CEmptyServerPlugin g_EmptyServerPlugin;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CEmptyServerPlugin, IServerPluginCallbacks, INTERFACEVERSION_ISERVERPLUGINCALLBACKS, g_EmptyServerPlugin );

//---------------------------------------------------------------------------------
// Purpose: constructor/destructor
//---------------------------------------------------------------------------------
CEmptyServerPlugin::CEmptyServerPlugin():
	m_SizzlingStats(),
	m_SayHook(),
	m_SayTeamHook(),
	m_SwitchTeamsHook(),
	m_refTournamentMode((IConVar*)NULL),
	m_pAutoUpdater(NULL),
	m_pTeamplayRoundBasedRules(NULL),
	m_iRoundState(NULL),
	m_bInWaitingForPlayers(NULL),
	m_iClientCommandIndex(0),
	m_iLastCapTick(0),
	m_bShouldRecord(false),
	m_bTournamentMatchStarted(false),
	m_bAlreadyLevelShutdown(true)
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
	//ConnectTier1Libraries( &interfaceFactory, 1 );
	//ConnectTier2Libraries( &interfaceFactory, 1 );

	curl_global_init(CURL_GLOBAL_ALL);

	// needs to be before the auto updater since it's used to unload and reload the plugins
	g_pServerPluginHandler = (s_ServerPlugin*)interfaceFactory(INTERFACEVERSION_ISERVERPLUGINHELPERS, NULL);

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

	playerinfomanager = (IPlayerInfoManager *)gameServerFactory(INTERFACEVERSION_PLAYERINFOMANAGER,NULL);
	pEngine = (IVEngineServer*)interfaceFactory(INTERFACEVERSION_VENGINESERVER, NULL);
	gameeventmanager = (IGameEventManager2*)interfaceFactory(INTERFACEVERSION_GAMEEVENTSMANAGER2, NULL);
	helpers = (IServerPluginHelpers*)interfaceFactory(INTERFACEVERSION_ISERVERPLUGINHELPERS, NULL);
	enginetrace = (IEngineTrace*)interfaceFactory(INTERFACEVERSION_ENGINETRACE_SERVER, NULL);

	pServerEnts = (IServerGameEnts *)gameServerFactory(INTERFACEVERSION_SERVERGAMEENTS, NULL);
	pServerDLL = (IServerGameDLL *)gameServerFactory(INTERFACEVERSION_SERVERGAMEDLL, NULL);

	if ( !ConfirmInterfaces() ){
		return false;
	}

	gpGlobals = playerinfomanager->GetGlobalVars();

	//GetGameRules();
	GetPropOffsets();
	//HookProps();

	//gameeventmanager->AddListener( this, "teamplay_round_stalemate", true );
	//gameeventmanager->AddListener( this, "teamplay_round_active", true );		// 9:54
	//gameeventmanager->AddListener( this, "arena_round_start", true );
	//gameeventmanager->AddListener( this, "teamplay_round_win", true );			// end round
	gameeventmanager->AddListener( this, "teamplay_point_captured", true );		// point captured
	
	// for team scores
	gameeventmanager->AddListener( this, "arena_win_panel", true );
	gameeventmanager->AddListener( this, "teamplay_win_panel", true );
	
	// player changes name
	//gameeventmanager->AddListener( this, "player_changename", true );
	
	// player healed (not incl buffs)
	gameeventmanager->AddListener( this, "player_healed", true );
	
	// happens when mp_winlimit or mp_timelimit is met or something i don't know, i forget
	gameeventmanager->AddListener( this, "teamplay_game_over", true );
	gameeventmanager->AddListener( this, "tf_game_over", true );
	
	// when a medic dies
	gameeventmanager->AddListener( this, "medic_death", true );
	
	// when a player types in chat (doesn't include data to differentiate say and say_team)
	gameeventmanager->AddListener( this, "player_say", true );
#ifndef PUBLIC_RELEASE
	// to track times for classes
	gameeventmanager->AddListener( this, "player_changeclass", true );
	gameeventmanager->AddListener( this, "player_team", true );
	
	//gameeventmanager->AddListener( this, "player_death", true );
#endif
	//gameeventmanager->AddListener( this, "tournament_stateupdate", true ); // for getting team names
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

	m_SayHook.Hook(this, cvar, "say");
	m_SayTeamHook.Hook(this, cvar, "say_team");
	m_SwitchTeamsHook.Hook(this, cvar, "mp_switchteams");

	m_refTournamentMode.Init("mp_tournament", false);

	MathLib_Init( 2.2f, 2.2f, 0.0f, 2 );
	ConVar_Register( 0 );
	return true;
}

//---------------------------------------------------------------------------------
// Purpose: called when the plugin is unloaded (turned off)
//---------------------------------------------------------------------------------
void CEmptyServerPlugin::Unload( void )
{
	m_SayHook.Unhook();
	m_SayTeamHook.Unhook();
	m_SwitchTeamsHook.Unhook();
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
	
	//UnhookProps();

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
	m_bAlreadyLevelShutdown = false;

	//pEngine->LogPrint(UTIL_VarArgs( "LevelInit: %s\n", pMapName ));
	pEngine->LogPrint( "[SizzlingStats]: Attempting update.\n" );
	m_pAutoUpdater->StartThread();
	pEngine->LogPrint( "[SizzlingStats]: Update attempt complete.\n" );
	
	m_SizzlingStats.LevelInit(pMapName);
#ifndef PUBLIC_RELEASE
	pEngine->ServerCommand( "log on\n" );
	pEngine->ServerCommand( "logaddress_add sizzlingstats.com:8006\n" );
#endif
}

//---------------------------------------------------------------------------------
// Purpose: called on level start, when the server is ready to accept client connections
//		edictCount is the number of entities in the level, clientMax is the max client count
//---------------------------------------------------------------------------------
void CEmptyServerPlugin::ServerActivate( edict_t *pEdictList, int edictCount, int clientMax )
{
	//GetGameRules();
	GetPropOffsets();
	m_SizzlingStats.ServerActivate();
	//HookProps();
}

//---------------------------------------------------------------------------------
// Purpose: called once per server frame, do recurring work here (like checking for timeouts)
//---------------------------------------------------------------------------------
void CEmptyServerPlugin::GameFrame( bool simulating )
{
	g_pTSCallQueue->callQueueGameFrame();

	m_SizzlingStats.GameFrame();

	if (m_iRoundState && m_bInWaitingForPlayers)
	{
		bool bWaitingForPlayers = *m_bInWaitingForPlayers;
		static bool oldWaitingForPlayers = true;

		int roundstate = *m_iRoundState;
		static int oldRoundState = roundstate;
#ifndef PUBLIC_RELEASE
		if (oldWaitingForPlayers != bWaitingForPlayers)
		{
			using namespace Teamplay_GameRule_States;

			Msg( UTIL_VarArgs( "round state is %s\n", GetStateName((gamerules_roundstate_t)roundstate) ) );
			bool bTournamentMode = m_refTournamentMode.GetInt() == 1;

			if (bWaitingForPlayers == true)
			{
				if (m_bTournamentMatchStarted)
				{
					m_SizzlingStats.SS_TournamentMatchEnded();
					m_bTournamentMatchStarted = false;
				}
			}
			else
			{
				if (bTournamentMode && !m_bTournamentMatchStarted && (roundstate != GR_STATE_PREGAME))
				{
					m_SizzlingStats.SS_TournamentMatchStarted();
					m_bTournamentMatchStarted = true;
				}
			}

			oldWaitingForPlayers = bWaitingForPlayers;
		}
#endif
		if (oldRoundState != roundstate)
		{
			using namespace Teamplay_GameRule_States;

			//Msg( UTIL_VarArgs( "round state is now %s\n", GetStateName(state) ) );
			switch (roundstate)
			{
			case GR_STATE_PREROUND:
				{
					m_bShouldRecord = true; // start extra stats recording
					m_SizzlingStats.SS_PreRoundFreeze();
				}
				break;
			case GR_STATE_RND_RUNNING:
				{
					m_SizzlingStats.SS_RoundStarted();
				}
				break;
			/*case GR_STATE_TEAM_WIN:
			case GR_STATE_RESTART:
			case GR_STATE_STALEMATE:
				{
					m_bShouldRecord = false; // stop extra stats recording
					m_SizzlingStats.SS_RoundEnded();
				}
				break;*/
			default:
				break;
			}
			oldRoundState = roundstate;
		}
	}
}

//---------------------------------------------------------------------------------
// Purpose: called on level end (as the server is shutting down or going to a new map)
//---------------------------------------------------------------------------------
void CEmptyServerPlugin::LevelShutdown( void ) // !!!!this can get called multiple times per map change
{
	if (!m_bAlreadyLevelShutdown)
	{
		m_bAlreadyLevelShutdown = true;
	#ifdef COUNT_CYCLES
		Msg( "clock cycles: %i\n", m_CycleCount.GetCycles() );
		Msg( "milliseconds: %i\n", m_CycleCount.GetMilliseconds() );
		m_CycleCount.Init( (int64)0 );
	#endif
		//pEngine->LogPrint("LevelShutdown\n");
		if (m_bTournamentMatchStarted)
		{
			m_SizzlingStats.SS_TournamentMatchEnded();
			m_bTournamentMatchStarted = false;
		}
		m_pTeamplayRoundBasedRules = NULL;
		m_SizzlingStats.SS_DeleteAllPlayerData();
		g_pUserIdTracker->Reset();
	}
}

//---------------------------------------------------------------------------------
// Purpose: called when a client spawns into a server (i.e as they begin to play)
//---------------------------------------------------------------------------------
void CEmptyServerPlugin::ClientActive( edict_t *pEdict )
{
	//pEngine->LogPrint(UTIL_VarArgs( "ClientActive: %u, %u\n", (unsigned int)pEdict, pEngine->IndexOfEdict(pEdict) ));
	if( !pEdict || pEdict->IsFree() )
		return;

	g_pUserIdTracker->ClientActive( pEdict );
	m_SizzlingStats.SS_InsertPlayer( pEdict );
}

//---------------------------------------------------------------------------------
// Purpose: called when a client leaves a server (or is timed out)
//---------------------------------------------------------------------------------
void CEmptyServerPlugin::ClientDisconnect( edict_t *pEdict )
{
	// ClientDisconnect isn't called for bots 
	// on LevelShutdown, so I just call 
	// ClientDisconnect myself in LevelShutdown 
	// for all players.
	// This check makes sure that ClientDisconnect 
	// isn't called twice for the same player.
	if (!m_bAlreadyLevelShutdown)
	{
		//pEngine->LogPrint(UTIL_VarArgs( "ClientDisconnect: %u, %u\n", (unsigned int)pEdict, pEngine->IndexOfEdict(pEdict) ));
		if( !pEdict || pEdict->IsFree() )
			return;
		m_SizzlingStats.SS_DeletePlayer( pEdict );
		g_pUserIdTracker->ClientDisconnect( pEdict );
	}
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
	++index;
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
#ifdef DEV_COMMANDS_ON
	using namespace SCHelpers;
	const char *pcmd = args[0];

	if ( !pEntity || pEntity->IsFree() ) 
	{
		return PLUGIN_CONTINUE;
	}

	int entindex = pEngine->IndexOfEdict(pEntity);

	if ( FStrEq( pcmd, "gibuber" ) )
	{
		if ( entindex > 0 )
		{
			m_SizzlingStats.GiveUber( entindex );
		}
	}
	else if ( FStrEq( pcmd, "tryend" ) )
	{
		m_SizzlingStats.SS_EndOfRound();
	}
	else if ( FStrEq( pcmd, "testupdatestats" ) )
	{
		m_SizzlingStats.SS_UploadStats();
	}
	else if ( FStrEq( pcmd, "testthreading" ) )
	{
		m_SizzlingStats.SS_TestThreading();
	}
	else if ( FStrEq( pcmd, "menutest" ) )
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

		if ( entindex > 0 )
		{
			helpers->CreateMessage( pEngine->PEntityOfEntIndex(entindex), DIALOG_MENU, kv, this );
		}
		kv->deleteThis();
	}
#endif
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
	if (!playerinfomanager)
	{
		Warning( "Unable to load playerinfomanager, aborting load\n" );
		return false;
	}
	if (!pEngine)
	{
		Warning( "Unable to load engine, aborting load\n" );
		return false;
	}

	if (!gameeventmanager)
	{
		Warning( "Unable to load gameeventmanager, aborting load\n" );
		return false;
	}

	if (!helpers)
	{
		Warning( "Unable to load helpers, aborting load\n" );
		return false;
	}

	if (!enginetrace)
	{
		Warning( "Unable to load enginetrace, aborting load\n" );
		return false;
	}

	if (!pServerDLL)
	{
		Warning( "Unable to load pServerDLL, aborting load\n" );
		return false;
	}

	if (!pServerEnts)
	{
		Warning( "Unable to load pServerEnts, aborting load\n" );
		return false;
	}

	Msg( "All interfaces sucessfully loaded\n" );
	return true;
}

//---------------------------------------------------------------------------------
// Purpose: called once on plugin load incase the plugin is loaded dynamically
//---------------------------------------------------------------------------------
void CEmptyServerPlugin::LoadCurrentPlayers()
{
	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		edict_t *pEdict = pEngine->PEntityOfEntIndex(i);
		if (pEdict && !pEdict->IsFree())
		{
			if (pServerEnts->EdictToBaseEntity(pEdict))
			{
				g_pUserIdTracker->ClientActive( pEdict );
				m_SizzlingStats.SS_InsertPlayer( pEdict );
			}
		}
	}
}

bool CEmptyServerPlugin::CommandPreExecute( const CCommand &args )
{
	using namespace SCHelpers;
	const char *szCommand = args[0];

	if (m_bTournamentMatchStarted)
	{
		if (m_iClientCommandIndex > 0)
		{
			if ( FStrEq( szCommand, "say" ) )
			{
				m_SizzlingStats.ChatEvent( m_iClientCommandIndex, args.ArgS(), false );
			}
			else if ( FStrEq( szCommand, "say_team" ) )
			{
				m_SizzlingStats.ChatEvent( m_iClientCommandIndex, args.ArgS(), true );
			}
		}
		if ( FStrEq( szCommand, "mp_switchteams" ) )
		{
			//Msg( "teams switched\n" );
		}
	}

	// dispatch the command
	return true;
}

void CEmptyServerPlugin::CommandPostExecute( const CCommand &args, bool bWasCommandExecuted )
{
}

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

	if ( m_bShouldRecord && FStrEq( name, "player_healed" ) )
	{
		int patient = event->GetInt( "patient" );
		if ( patient != event->GetInt( "healer" ) )
		{
			int patientIndex = g_pUserIdTracker->GetEntIndex( patient );
			if (patientIndex > 0)
			{
				m_SizzlingStats.PlayerHealed( patientIndex, event->GetInt("amount") );
			}
		}
	}
	else if ( m_bShouldRecord && FStrEq( name, "player_death" ) )
	{
		int victim = event->GetInt( "victim_entindex" );
		m_SizzlingStats.CheckPlayerDropped( victim );
	}
	else if ( m_bShouldRecord && FStrEq( name, "medic_death" ) )
	{
		int killerIndex = g_pUserIdTracker->GetEntIndex( event->GetInt( "attacker" ) );	// med picks
		int victimIndex = g_pUserIdTracker->GetEntIndex( event->GetInt( "userid" ) );
		bool charged = event->GetBool( "charged" ); // med drops

		if ( killerIndex > 0 && killerIndex != victimIndex )
		{
			m_SizzlingStats.MedPick( killerIndex );
		}

		if ( charged && victimIndex > 0 )
		{
			m_SizzlingStats.UberDropped( victimIndex );
		}
	}
	else if ( FStrEq( name, "player_changeclass" ) )
	{
		int userid = event->GetInt( "userid" );
		int entindex = g_pUserIdTracker->GetEntIndex( userid );
		EPlayerClass player_class = static_cast<EPlayerClass>(event->GetInt("class"));
		m_SizzlingStats.PlayerChangedClass( entindex, player_class );
	}
	else if ( FStrEq( name, "teamplay_point_captured" ) )
	{
		m_iLastCapTick = gpGlobals->tickcount;
		m_SizzlingStats.TeamCapped( event->GetInt("team") );
	}
	else if ( FStrEq( name, "player_team" ) )
	{
		bool bDisconnect = event->GetBool("disconnect");
		if (!bDisconnect)
		{
			int teamid = event->GetInt("team");
			// if they are switching to spec
			if (teamid == 1) //TODO: verify that 1 is spec
			{
				int userid = event->GetInt( "userid" );
				int entindex = g_pUserIdTracker->GetEntIndex( userid );
				m_SizzlingStats.PlayerChangedClass(entindex, k_ePlayerClassUnknown);
			}
		}
	}
	else if ( FStrEq( name, "teamplay_win_panel" ) || FStrEq( name, "arena_win_panel" ) )
	{
		m_SizzlingStats.SetTeamScores(event->GetInt("red_score"), event->GetInt("blue_score"));

		if ( m_iLastCapTick == gpGlobals->tickcount )
		{
			if ( event->GetInt("winreason") == Teamplay_GameRule_States::WINREASON_ALL_POINTS_CAPTURED )
			{
				const char *cappers = event->GetString("cappers");
				int length = V_strlen(cappers);
				m_SizzlingStats.CapFix( cappers, length );
			}
		}

		m_bShouldRecord = false; // stop extra stats recording
		m_SizzlingStats.SS_RoundEnded();
	}
	else if ( FStrEq( name, "teamplay_game_over" ) || FStrEq( name, "tf_game_over" ) )
	{
		CPlayerMessage::AllUserChatMessage( "\x03This server is running SizzlingStats v" PLUGIN_VERSION "\n" );
		CPlayerMessage::AllUserChatMessage( "\x03\x46or credits type \".ss_credits\"\n" ); // \x03F is recognised as '?'
		if (m_bTournamentMatchStarted)
		{
			CPlayerMessage::AllUserChatMessage( "\x03To view the match stats, type \".sizzlingstats\" or \".ss\"\n" );
		}
	}
	else if ( FStrEq( name, "player_say" ) )
	{
		const char *text = event->GetString( "text" );

		if ( FStrEq( text, ".ss_credits" ) )
		{
			int userid = event->GetInt( "userid" );
			int entindex = SCHelpers::UserIDToEntIndex( userid );
			// don't try to send messages to worldspawn
			if ( entindex != 0 )
			{
				m_SizzlingStats.SS_Credits( entindex, PLUGIN_VERSION );
			}
		}
#ifndef PUBLIC_RELEASE
		else if ( FStrEq( text, ".ss" ) ||
					FStrEq( text, ".stats" ) || 
					FStrEq( text, ".showstats" ) || 
					FStrEq( text, ".sizzlingstats" ) || 
					FStrEq( text, ".ss_showstats" ) || 
					FStrEq( text, ".doritos" ) )
		{
			int userid = event->GetInt( "userid" );
			int entindex = SCHelpers::UserIDToEntIndex( userid );
			m_SizzlingStats.SS_ShowHtmlStats( entindex );
		}
#endif
	}
}

void CEmptyServerPlugin::GetGameRules()
{
	m_pTeamplayRoundBasedRules = SCHelpers::GetTeamplayRoundBasedGameRulesPointer();
}

void CEmptyServerPlugin::GetPropOffsets()
{
	using namespace SCHelpers;

	if (!m_pTeamplayRoundBasedRules)
	{
		GetGameRules();
	}
	
	bool bError = false;
	unsigned int gamerulesoffset = GetPropOffsetFromTable( "DT_TFGameRulesProxy", "baseclass", bError ) +
		GetPropOffsetFromTable( "DT_TeamplayRoundBasedRulesProxy", "teamplayroundbased_gamerules_data", bError );
		
	int roundstateoffset = gamerulesoffset + GetPropOffsetFromTable( "DT_TeamplayRoundBasedRules", "m_iRoundState", bError );
	int waitingoffset = gamerulesoffset + GetPropOffsetFromTable( "DT_TeamplayRoundBasedRules", "m_bInWaitingForPlayers", bError );

	m_iRoundState = ByteOffsetFromPointer<int>(m_pTeamplayRoundBasedRules, roundstateoffset);
	m_bInWaitingForPlayers = ByteOffsetFromPointer<bool>(m_pTeamplayRoundBasedRules, waitingoffset);
}

#ifdef GetProp
#undef GetProp
#endif

void RecurseServerTable( SendTable *pTable, int &spacing )
{
	SendTable *pSendTable = pTable;
	if (pSendTable == NULL)
	{
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
		SendPropType PropType = pProp->m_Type;
		char type[10];
		switch( PropType )
		{
			case 0: 
				Q_strncpy( type, "int", 10 );
				break;
			case 1: 
				Q_strncpy( type, "float", 10 );
				break;
			case 2: 
				Q_strncpy( type, "vector", 10 );
				break;
			case 3: 
				Q_strncpy( type, "vectorxy", 10 );
				break;
			case 4: 
				Q_strncpy( type, "string", 10 );
				break;
			case 5: 
				Q_strncpy( type, "array", 10 );
				break;
			case 6: 
				Q_strncpy( type, "datatable", 10 );
				break;
			default:
				break;
		}

		memset( TableName, 0, sizeof(TableName) );
		for (int j = 0; j < spacing; j++)
			V_strcat( TableName, "  |", size );
		V_strcat( TableName, pProp->GetName(), size );
		Msg( "%s, Offset: %i ( type: %s, size: %i bits )\n", TableName, pProp->GetOffset(), type, pProp->m_nBits );

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