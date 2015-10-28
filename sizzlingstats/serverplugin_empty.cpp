/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

#include "dbgflag.h"

#include <stdio.h>

#include "SizzlingStats.h"
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

#include "SizzFileSystem.h"
#include "ThreadCallQueue.h"

#include "SC_helpers.h"

#include "PluginDefines.h"
#include "autoupdate.h"
#include "UserIdTracker.h"
#include "ServerPluginHandler.h"
#include "LogStats.h"

#include "curl/curl.h"

#include "ConCommandHook.h"
#include "teamplay_gamerule_states.h"

#include "SizzPluginContext.h"
#include "UserMessageHelpers.h"
#include "MRecipientFilter.h"

#include "TFPlayerWrapper.h"
#include "TFTeamWrapper.h"

#ifdef PROTO_STATS
#include "EventStats.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Interfaces from the engine
IEngineTrace			*enginetrace = NULL;
IServerGameDLL			*pServerDLL = NULL;
IFileSystem			*g_pFullFileSystem = NULL;

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
	virtual void			OnEdictAllocated( edict_t *edict );
	virtual void			OnEdictFreed( const edict_t *edict );

	// Additions
	bool	ConfirmInterfaces( void );
	void	LoadCurrentPlayers();

	virtual bool CommandPreExecute( const CCommand &args );
	virtual void CommandPostExecute( const CCommand &args, bool bWasCommandExecuted );

	// IGameEventListener Interface
	virtual void FireGameEvent( IGameEvent *event );

	virtual int GetCommandIndex() { return m_iClientCommandIndex; }

private:
	void LoadUpdatedPlugin();
	void OnAutoUpdateReturn( bool bLoadUpdate );
	void GetGameRules();
	void GetPropOffsets();

	void TournamentMatchStarted();
	void TournamentMatchEnded();

private:
	CSizzPluginContext m_plugin_context;
#ifdef PROTO_STATS
	CEventStats m_EventStats;
#endif
#ifdef LOG_STATS
	CLogStats m_logstats;
#else
	CNullLogStats m_logstats;
#endif
	SizzlingStats m_SizzlingStats;
	CConCommandHook m_SayHook;
	CConCommandHook m_SayTeamHook;
	CConCommandHook m_SwitchTeamsHook;
	CConCommandHook m_PauseHook;
	CConCommandHook m_UnpauseHook;
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

	CON_COMMAND_MEMBER_F(CEmptyServerPlugin, "printservertables", PrintServerTables, "prints the server tables ya", 0);
};

// 
// The plugin is a static singleton that is exported as an interface
//
static CEmptyServerPlugin g_EmptyServerPlugin;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CEmptyServerPlugin, IServerPluginCallbacks, INTERFACEVERSION_ISERVERPLUGINCALLBACKS_VERSION_1, g_EmptyServerPlugin );

//---------------------------------------------------------------------------------
// Purpose: constructor/destructor
//---------------------------------------------------------------------------------
CEmptyServerPlugin::CEmptyServerPlugin():
	m_logstats(),
	m_SizzlingStats(),
	m_SayHook(),
	m_SayTeamHook(),
	m_SwitchTeamsHook(),
	m_PauseHook(),
	m_UnpauseHook(),
	m_refTournamentMode((IConVar*)NULL),
	m_pAutoUpdater(NULL),
	m_pTeamplayRoundBasedRules(NULL),
	m_iRoundState(NULL),
	m_bInWaitingForPlayers(NULL),
	m_iClientCommandIndex(0),
	m_iLastCapTick(0),
	m_bShouldRecord(false),
	m_bTournamentMatchStarted(false),
	m_bAlreadyLevelShutdown(false)
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

	autoUpdateInfo_t a = { FULL_PLUGIN_PATH, URL_TO_UPDATED, URL_TO_META, PLUGIN_PATH, 0, PLUGIN_VERSION };
	m_pAutoUpdater = new CAutoUpdateThread(a, s_pluginInfo);

	using namespace std::placeholders;
	m_pAutoUpdater->SetOnFinishedUpdateCallback(std::bind(&CEmptyServerPlugin::OnAutoUpdateReturn, this, _1));
	m_pAutoUpdater->StartThread();

	g_pFullFileSystem = (IFileSystem *)interfaceFactory(FILESYSTEM_INTERFACE_VERSION, NULL);
	if (!g_pFullFileSystem)
	{
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

	enginetrace = (IEngineTrace*)interfaceFactory(INTERFACEVERSION_ENGINETRACE_SERVER, NULL);
	pServerDLL = (IServerGameDLL *)gameServerFactory(INTERFACEVERSION_SERVERGAMEDLL, NULL);

	if ( !ConfirmInterfaces() )
	{
		return false;
	}

	plugin_context_init_t init;
	init.pEngine = (IVEngineServer*)interfaceFactory(INTERFACEVERSION_VENGINESERVER, NULL);
	init.pPlayerInfoManager = (IPlayerInfoManager *)gameServerFactory(INTERFACEVERSION_PLAYERINFOMANAGER,NULL);
	init.pHelpers = (IServerPluginHelpers*)interfaceFactory(INTERFACEVERSION_ISERVERPLUGINHELPERS, NULL);
	init.pGameEventManager = (IGameEventManager2*)interfaceFactory(INTERFACEVERSION_GAMEEVENTSMANAGER2, NULL);
	init.pServerGameDLL = (IServerGameDLL *)gameServerFactory(INTERFACEVERSION_SERVERGAMEDLL, NULL);

	if (!m_plugin_context.Initialize(init))
	{
		return false;
	}

	CTFPlayerWrapper::InitializeOffsets();
	CTFTeamWrapper::InitializeOffsets();

	m_logstats.Load(m_plugin_context);

	//GetGameRules();
	GetPropOffsets();
	//HookProps();

	//m_plugin_context.AddListener( this, "teamplay_round_stalemate", true );
	//m_plugin_context.AddListener( this, "teamplay_round_active", true );		// 9:54
	//m_plugin_context.AddListener( this, "arena_round_start", true );
	//m_plugin_context.AddListener( this, "teamplay_round_win", true );			// end round
	m_plugin_context.AddListener( this, "teamplay_point_captured", true );		// point captured
	
	// for team scores
	m_plugin_context.AddListener( this, "arena_win_panel", true );
	m_plugin_context.AddListener( this, "teamplay_win_panel", true );
	
	// player changes name
	//m_plugin_context.AddListener( this, "player_changename", true );
	
	// player healed (not incl buffs)
	m_plugin_context.AddListener( this, "player_healed", true );
	
	// happens when mp_winlimit or mp_timelimit is met or something i don't know, i forget
	m_plugin_context.AddListener( this, "teamplay_game_over", true );
	m_plugin_context.AddListener( this, "tf_game_over", true );
	
	// when a medic dies
	m_plugin_context.AddListener( this, "medic_death", true );
	
	// when a player types in chat (doesn't include data to differentiate say and say_team)
	m_plugin_context.AddListener( this, "player_say", true );

	// to track times for classes
	m_plugin_context.AddListener( this, "player_changeclass", true );
	m_plugin_context.AddListener( this, "player_team", true );
	
	m_plugin_context.AddListener( this, "player_death", true );
	//m_plugin_context.AddListener( this, "tournament_stateupdate", true ); // for getting team names
	//m_plugin_context.AddListener( this, "player_shoot", true );		// for accuracy stats

	//m_plugin_context.AddListener( this, "player_chargedeployed", true );	// when a medic deploys uber/kritz
	//m_plugin_context.AddListener( this, "player_spawn", true );	// when a player spawns...

	//m_plugin_context.AddListener( this, "teamplay_suddendeath_end", true );
	//m_plugin_context.AddListener( this, "teamplay_overtime_end", true );
#ifdef PROTO_STATS
	m_EventStats.Initialize();
	m_plugin_context.AddListenerAll(this, true);
#endif
	m_SizzlingStats.Load(&m_plugin_context);

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
	m_PauseHook.Hook(this, cvar, "pause");
	m_UnpauseHook.Hook(this, cvar, "unpause");

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
	if (m_bTournamentMatchStarted)
	{
		TournamentMatchEnded();
	}

	m_SayHook.Unhook();
	m_SayTeamHook.Unhook();
	m_SwitchTeamsHook.Unhook();
	m_PauseHook.Unhook();
	m_UnpauseHook.Unhook();
	if (m_plugin_context.GetEngine())
	{
		m_plugin_context.LogPrint("Unload\n");
	}
	m_SizzlingStats.SS_Msg( "plugin unloading\n" );
	if (m_plugin_context.GetGameEventManager())
	{
		m_plugin_context.RemoveListener( this ); // make sure we are unloaded from the event system
	}

	m_SizzlingStats.SS_DeleteAllPlayerData();
	m_SizzlingStats.Unload(&m_plugin_context);
	
#ifdef PROTO_STATS
	m_EventStats.Shutdown();
#endif
	m_logstats.Unload();
	
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

	// MSVC++ 12.0 _MSC_VER == 1800 (Visual Studio 2013)
	// MSVC++ 11.0 _MSC_VER == 1700 (Visual Studio 2012)
#if defined(_WIN32) && !defined(_DLL) && (_MSC_VER >= 1700)
	// MS Fix
	//
	// http://connect.microsoft.com/VisualStudio/feedback/details/781665/stl-using-std-threading-objects-adds-extra-load-count-for-hosted-dll
	//

		// get the current module handle
		MEMORY_BASIC_INFORMATION mbi;
		static int address;
		VirtualQuery(&address, &mbi, sizeof(mbi));

		// decrement the reference count
		FreeLibrary(reinterpret_cast<HMODULE>(mbi.AllocationBase));

		// vs2013 requires two FreeLibrary calls
#if (_MSC_VER == 1800)
		FreeLibrary(reinterpret_cast<HMODULE>(mbi.AllocationBase));
#endif
	//
	//
#endif
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
	m_plugin_context.LogPrint( "[SizzlingStats]: Attempting update.\n" );
	m_pAutoUpdater->StartThread();
	m_plugin_context.LogPrint( "[SizzlingStats]: Update attempt complete.\n" );
	
	m_SizzlingStats.LevelInit(&m_plugin_context, pMapName);
	m_logstats.LevelInit(pMapName);
}

//---------------------------------------------------------------------------------
// Purpose: called on level start, when the server is ready to accept client connections
//		edictCount is the number of entities in the level, clientMax is the max client count
//---------------------------------------------------------------------------------
void CEmptyServerPlugin::ServerActivate( edict_t *pEdictList, int edictCount, int clientMax )
{
	m_bAlreadyLevelShutdown = false;
	m_plugin_context.ServerActivate(pEdictList, edictCount, clientMax);
	//GetGameRules();
	GetPropOffsets();
	m_SizzlingStats.ServerActivate(&m_plugin_context);
	//HookProps();
}

//---------------------------------------------------------------------------------
// Purpose: called once per server frame, do recurring work here (like checking for timeouts)
//---------------------------------------------------------------------------------
void CEmptyServerPlugin::GameFrame( bool simulating )
{
	m_plugin_context.GameFrame(simulating);

	m_SizzlingStats.GameFrame();

	if (m_iRoundState && m_bInWaitingForPlayers)
	{
		bool bWaitingForPlayers = *m_bInWaitingForPlayers;
		static bool oldWaitingForPlayers = true;

		int roundstate = *m_iRoundState;
		static int oldRoundState = roundstate;
		if (oldWaitingForPlayers != bWaitingForPlayers)
		{
			using namespace Teamplay_GameRule_States;

			Msg( UTIL_VarArgs( "round state is %s\n", GetStateName((gamerules_roundstate_t)roundstate) ) );
			bool bTournamentMode = m_refTournamentMode.GetInt() == 1;

			if (bWaitingForPlayers == true)
			{
				if (m_bTournamentMatchStarted)
				{
					TournamentMatchEnded();
				}
			}
			else
			{
				if (bTournamentMode && !m_bTournamentMatchStarted && (roundstate != GR_STATE_PREGAME))
				{
					TournamentMatchStarted();
				}
			}

			oldWaitingForPlayers = bWaitingForPlayers;
		}
		if (oldRoundState != roundstate)
		{
			using namespace Teamplay_GameRule_States;

			//Msg( UTIL_VarArgs( "round state is now %s\n", GetStateName(state) ) );
			switch (roundstate)
			{
			case GR_STATE_PREROUND:
				{
					m_bShouldRecord = true; // start extra stats recording
					m_logstats.PreRoundFreezeStarted(m_bTournamentMatchStarted);
					m_SizzlingStats.SS_PreRoundFreeze(&m_plugin_context);
				}
				break;
			case GR_STATE_RND_RUNNING:
				{
					m_SizzlingStats.SS_RoundStarted(&m_plugin_context);
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
		if (m_bTournamentMatchStarted)
		{
			TournamentMatchEnded();
		}
		m_pTeamplayRoundBasedRules = NULL;
		m_SizzlingStats.SS_DeleteAllPlayerData();
		m_plugin_context.LevelShutdown();
	}
}

//---------------------------------------------------------------------------------
// Purpose: called when a client spawns into a server (i.e as they begin to play)
//---------------------------------------------------------------------------------
void CEmptyServerPlugin::ClientActive( edict_t *pEdict )
{
	if (pEdict && !pEdict->IsFree())
	{
		int ent_index = m_plugin_context.ClientActive(pEdict);
		m_SizzlingStats.SS_PlayerConnect(&m_plugin_context, pEdict);
		m_logstats.ClientActive(ent_index);
	}
}

//---------------------------------------------------------------------------------
// Purpose: called when a client leaves a server (or is timed out)
//---------------------------------------------------------------------------------
void CEmptyServerPlugin::ClientDisconnect( edict_t *pEdict )
{
	if (pEdict && !pEdict->IsFree())
	{
		// ClientDisconnect isn't called for bots 
		// on LevelShutdown, so I just call 
		// ClientDisconnect myself in LevelShutdown 
		// for all players.
		// This check makes sure that ClientDisconnect 
		// isn't called twice for the same player.
		if (!m_bAlreadyLevelShutdown)
		{
			m_SizzlingStats.SS_PlayerDisconnect(&m_plugin_context, pEdict);
			m_logstats.ClientDisconnect(SCHelpers::EntIndexFromEdict(pEdict));
			m_plugin_context.ClientDisconnect(pEdict);
		}
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
	m_iClientCommandIndex = ++index;
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
	using namespace SCHelpers;
	const char *pcmd = args[0];

	if ( !pEntity || pEntity->IsFree() ) 
	{
		return PLUGIN_CONTINUE;
	}

	int entindex = SCHelpers::EntIndexFromEdict(pEntity);
	if (entindex > 0)
	{
		if ( FStrEq(pcmd, "sizz_show_stats") )
		{
			m_SizzlingStats.SS_ShowHtmlStats(&m_plugin_context, entindex, false);
		}
		else if ( FStrEq(pcmd, "sizz_hide_stats") )
		{
			m_SizzlingStats.SS_HideHtmlStats(&m_plugin_context, entindex);
		}
#ifdef DEV_COMMANDS_ON
		else if ( FStrEq( pcmd, "gibuber" ) )
		{
			if ( entindex > 0 )
			{
				m_SizzlingStats.GiveUber( &m_plugin_context, entindex );
			}
		}
		else if ( FStrEq( pcmd, "tryend" ) )
		{
			m_SizzlingStats.SS_EndOfRound(&m_plugin_context);
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
				m_plugin_context.CreateMessage(entindex, DIALOG_MENU, kv, this);
			}
			kv->deleteThis();
		}
#endif
	}
	return PLUGIN_CONTINUE;
}

//---------------------------------------------------------------------------------
// Purpose: called when a client is authenticated
//---------------------------------------------------------------------------------
PLUGIN_RESULT CEmptyServerPlugin::NetworkIDValidated( const char *pszUserName, const char *pszNetworkID )
{
	return PLUGIN_CONTINUE;
}

//---------------------------------------------------------------------------------
// Purpose: called when a cvar value query is finished
//---------------------------------------------------------------------------------
void CEmptyServerPlugin::OnQueryCvarValueFinished( QueryCvarCookie_t iCookie, edict_t *pPlayerEntity, EQueryCvarValueStatus eStatus, const char *pCvarName, const char *pCvarValue )
{
	//Msg( "Cvar query (cookie: %d, status: %d) - name: %s, value: %s\n", iCookie, eStatus, pCvarName, pCvarValue );
}

void CEmptyServerPlugin::OnEdictAllocated( edict_t *edict )
{
}

void CEmptyServerPlugin::OnEdictFreed( const edict_t *edict )
{
}

//---------------------------------------------------------------------------------
// Purpose: confirm the validity of the interface pointers
//---------------------------------------------------------------------------------
bool CEmptyServerPlugin::ConfirmInterfaces( void )
{
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

	Msg( "All interfaces sucessfully loaded\n" );
	return true;
}

//---------------------------------------------------------------------------------
// Purpose: called once on plugin load incase the plugin is loaded dynamically
//---------------------------------------------------------------------------------
void CEmptyServerPlugin::LoadCurrentPlayers()
{
	int max_clients = m_plugin_context.GetMaxClients();
	for ( int i = 1; i <= max_clients; ++i )
	{
		edict_t *pEdict = m_plugin_context.EdictFromEntIndex(i);
		if (pEdict && !pEdict->IsFree())
		{
			// might not need this second check
			if (SCHelpers::EdictToBaseEntity(pEdict))
			{
				int ent_index = m_plugin_context.ClientActive(pEdict);
				m_SizzlingStats.SS_PlayerConnect(&m_plugin_context, pEdict);
				m_logstats.ClientActive(ent_index);
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
				m_SizzlingStats.ChatEvent( &m_plugin_context, m_iClientCommandIndex, args.ArgS(), false );
			}
			else if ( FStrEq( szCommand, "say_team" ) )
			{
				m_SizzlingStats.ChatEvent( &m_plugin_context, m_iClientCommandIndex, args.ArgS(), true );
			}
		}

		if ( FStrEq( szCommand, "mp_switchteams" ) )
		{
			//Msg( "teams switched\n" );
		}
#ifdef PROTO_STATS
		else if ( FStrEq( szCommand, "pause" ) )
		{
			bool paused = m_plugin_context.IsPaused();
			if (!paused)
			{
				m_EventStats.SendNamedEvent("ss_pause", m_plugin_context.GetCurrentTick());
			}
		}
		else if ( FStrEq( szCommand, "unpause" ) )
		{
			bool paused = m_plugin_context.IsPaused();
			if (paused)
			{
				m_EventStats.SendNamedEvent("ss_unpause", m_plugin_context.GetCurrentTick());
			}
		}
#endif
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
#ifdef PROTO_STATS
	if (m_bTournamentMatchStarted)
	{
		m_EventStats.OnFireGameEvent(event, m_plugin_context.GetCurrentTick());
	}
#endif
	using namespace SCHelpers;

	const char *RESTRICT name = event->GetName();

	if ( m_bShouldRecord && FStrEq( name, "player_healed" ) )
	{
		int patient = event->GetInt( "patient" );
		if ( patient != event->GetInt( "healer" ) )
		{
			int patientIndex = m_plugin_context.EntIndexFromUserID(patient);
			if (patientIndex > 0)
			{
				m_SizzlingStats.PlayerHealed( patientIndex, event->GetInt("amount") );
			}
		}
	}
	else if ( m_bShouldRecord && FStrEq( name, "player_death" ) )
	{
		const int victim = event->GetInt("victim_entindex");
		const int inflictor = event->GetInt("inflictor_entindex");
		m_SizzlingStats.OnPlayerDeath(inflictor, victim);
		//m_SizzlingStats.CheckPlayerDropped( &m_plugin_context, victim );
	}
	else if ( m_bShouldRecord && FStrEq( name, "medic_death" ) )
	{
		int killerIndex = m_plugin_context.EntIndexFromUserID(event->GetInt( "attacker" ));	// med picks
		int victimIndex = m_plugin_context.EntIndexFromUserID(event->GetInt( "userid" ));
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
		int entindex = m_plugin_context.EntIndexFromUserID(userid);
		EPlayerClass player_class = static_cast<EPlayerClass>(event->GetInt("class"));
		m_SizzlingStats.PlayerChangedClass( entindex, player_class );
	}
	else if ( FStrEq( name, "teamplay_point_captured" ) )
	{
		m_iLastCapTick = m_plugin_context.GetCurrentTick();
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
				int entindex = m_plugin_context.EntIndexFromUserID(userid);
				m_SizzlingStats.PlayerChangedClass(entindex, k_ePlayerClassUnknown);
			}
		}
	}
	else if ( FStrEq( name, "teamplay_win_panel" ) || FStrEq( name, "arena_win_panel" ) )
	{
		m_SizzlingStats.SetTeamScores(event->GetInt("red_score"), event->GetInt("blue_score"));

		if ( m_iLastCapTick == m_plugin_context.GetCurrentTick() )
		{
			if ( event->GetInt("winreason") == Teamplay_GameRule_States::WINREASON_ALL_POINTS_CAPTURED )
			{
				const char *RESTRICT cappers = event->GetString("cappers");
				int length = V_strlen(cappers);
				m_SizzlingStats.CapFix( cappers, length );
			}
		}

		m_bShouldRecord = false; // stop extra stats recording
		m_SizzlingStats.SS_RoundEnded(&m_plugin_context);
	}
	else if ( FStrEq( name, "teamplay_game_over" ) || FStrEq( name, "tf_game_over" ) )
	{
		CUserMessageHelpers h(&m_plugin_context);
		h.AllUserChatMessage("\x03This server is running SizzlingStats v" PLUGIN_VERSION "\n");
		h.AllUserChatMessage("\x03\x46or credits type \".ss_credits\"\n"); // \x03F is recognised as '?'
		if (m_bTournamentMatchStarted)
		{
			h.AllUserChatMessage("\x03To view the match stats, type \".sizzlingstats\" or \".ss\"\n");
		}
	}
	else if ( FStrEq( name, "player_say" ) )
	{
		const char *RESTRICT text = event->GetString( "text" );

		if ( FStrEq( text, ".ss_credits" ) )
		{
			int userid = event->GetInt( "userid" );
			int entindex = m_plugin_context.EntIndexFromUserID(userid);
			// don't try to send messages to worldspawn
			if ( entindex != 0 )
			{
				m_SizzlingStats.SS_Credits( &m_plugin_context, entindex, PLUGIN_VERSION );
			}
		}
		else if ( FStrEq( text, ".ss" ) ||
					FStrEq( text, ".stats" ) || 
					FStrEq( text, ".showstats" ) || 
					FStrEq( text, ".sizzlingstats" ) || 
					FStrEq( text, ".ss_showstats" ) || 
					FStrEq( text, ".doritos" ) ||
					FStrEq( text, ".gg" ) )
		{
			int userid = event->GetInt( "userid" );
			int entindex = m_plugin_context.EntIndexFromUserID(userid);
			m_SizzlingStats.SS_ShowHtmlStats( &m_plugin_context, entindex, true );
		}
	}
}

void CEmptyServerPlugin::LoadUpdatedPlugin()
{
	int plugin_index = m_plugin_context.GetPluginIndex(this);
	if (plugin_index >= 0)
	{
		char temp[576];
		// unload the old plugin, load the new plugin
		V_snprintf(temp, sizeof(temp), "plugin_unload %i; plugin_load %s\n", plugin_index, FULL_PLUGIN_PATH);
		
		m_plugin_context.ServerCommand(temp);
		// the plugin will be unloaded when tf2 executes the command,
		// which then also loads the new version of the plugin.
		// the new version runs the updater which checks for plugin_old
		// and deletes it.
	}
}

void CEmptyServerPlugin::OnAutoUpdateReturn( bool bLoadUpdate )
{
	if (bLoadUpdate)
	{
		m_plugin_context.EnqueueGameFrameFunctor(CreateFunctor(this, &CEmptyServerPlugin::LoadUpdatedPlugin));
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
	
	unsigned int gamerulesoffset = GetPropOffsetFromTable( "DT_TFGameRulesProxy", "baseclass" ) +
		GetPropOffsetFromTable( "DT_TeamplayRoundBasedRulesProxy", "teamplayroundbased_gamerules_data" );
		
	int roundstateoffset = gamerulesoffset + GetPropOffsetFromTable( "DT_TeamplayRoundBasedRules", "m_iRoundState" );
	int waitingoffset = gamerulesoffset + GetPropOffsetFromTable( "DT_TeamplayRoundBasedRules", "m_bInWaitingForPlayers" );

	m_iRoundState = ByteOffsetFromPointer<int*>(m_pTeamplayRoundBasedRules, roundstateoffset);
	m_bInWaitingForPlayers = ByteOffsetFromPointer<bool*>(m_pTeamplayRoundBasedRules, waitingoffset);
}

void CEmptyServerPlugin::TournamentMatchStarted()
{
	const char *RESTRICT hostname = m_plugin_context.GetHostName();
	const char *RESTRICT mapname = m_plugin_context.GetMapName();
	const char *RESTRICT bluname = m_plugin_context.GetBluTeamName();
	const char *RESTRICT redname = m_plugin_context.GetRedTeamName();

	m_logstats.TournamentMatchStarted(hostname, mapname, bluname, redname);
	m_SizzlingStats.SS_TournamentMatchStarted(&m_plugin_context);
#ifdef PROTO_STATS
	int tick = m_plugin_context.GetCurrentTick();
	m_EventStats.OnTournamentMatchStart(&m_plugin_context, tick);
#endif
	m_bTournamentMatchStarted = true;
}

void CEmptyServerPlugin::TournamentMatchEnded()
{
	m_logstats.TournamentMatchEnded();
	m_SizzlingStats.SS_TournamentMatchEnded(&m_plugin_context);

#ifdef PROTO_STATS
	m_EventStats.OnTournamentMatchEnd(&m_plugin_context, m_plugin_context.GetCurrentTick());
#endif
	m_bTournamentMatchStarted = false;
}

#ifdef GetProp
#undef GetProp
#endif

static void RecurseServerTable( SendTable *pTable, int &spacing )
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

void CEmptyServerPlugin::PrintServerTables( const CCommand &args )
{
	ServerClass *pClass = m_plugin_context.GetAllServerClasses();
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

