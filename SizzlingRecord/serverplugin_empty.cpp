//===== Copyright � 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose: 
//
// $NoKeywords: $
//
//===========================================================================//

#include "PluginDefines.h"

#include <stdio.h>
#include <time.h>

#include "PluginContext.h"

#include "interface.h"
#include "filesystem.h"
#include "eiface.h"
#include "engine/iserverplugin.h"
#include "game/server/iplayerinfo.h"
#include "engine/IEngineSound.h"
#include "convar.h"
#include "igameevents.h"

#include "NetPropUtils.h"
#include "PlayerMessage.h"
#include "Helpers.h"
#include "SC_helpers.h"
#include "ConCommandHook.h"

#include "tier1/utllinkedlist.h"
#include "utlbuffer.h"

#include "cdll_int.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CTeamplayRoundBasedRules;

static ConVar enabled("sizz_record_enable", "1", FCVAR_NONE, "If nonzero, enables tournament match demo recording.");

//===========================================================================//

class CClientDemoRecorder
{
public:
	typedef struct DemoRecording_s
	{
		char m_szDemoName[64];
		uint32 m_nParts;
	} DemoRecording_t;

	// 32 bytes
	typedef struct bookmarkInfo_s
	{
		// the optional comment for the bookmark
		char comment[20];
		// a time value in time_t
		uint64 time;
		// tick number since the start of recording
		int32 tick;
	} bookmarkInfo_t;

public:
	CClientDemoRecorder();
	~CClientDemoRecorder();

	void Load();
	void ReloadConfig();

	void TournamentMatchStarted( PluginContext_t *pContext );
	void TournamentMatchEnded( PluginContext_t *pContext );

	void FixInvisiblePlayers( PluginContext_t *pContext );
	void StartRecordingFromCommand( PluginContext_t *pContext );

	void DeleteLatestDemo( PluginContext_t *pContext );

	void Bookmark( PluginContext_t *pContext, const char *comment );

	void StopRecordingEvent( PluginContext_t *pContext );

private:
	void WriteOutBookmarks( IFileSystem *pFileSystem );
	void LoadConfig();
	void ConstructDemoName( IVEngineClient *pEngineClient );

private:
	static void GetDateAndTime( struct tm &ltime );
	static void ConvertTimeToLocalTime( const time_t &t, struct tm &ltime );
	static void GetMapName( IVEngineClient *pEngineClient, char *out, int length );

	static bool CanRecordDemo( const IBaseClientDLL *pBaseClientDLL );

	static void StartRecording( IVEngineClient *pEngineClient, const DemoRecording_t &demoInfo, IEngineSound *pEngineSound, bool bEmitSound = true );
	static void StopRecording( IVEngineClient *pEngineClient );

private:
	DemoRecording_t m_LatestDemo;
	bool m_bRecording;
	bool m_bIStopped;

	ConVarRef m_bluTeamName;
	ConVarRef m_redTeamName;
	DemoRecording_t *m_pLastDemo;

	CUtlLinkedList<bookmarkInfo_t> m_bookmarks;
	CUtlBuffer m_bookmarkBuff;
};

CClientDemoRecorder::CClientDemoRecorder():
	m_LatestDemo(),
	m_bRecording(false),
	m_bIStopped(false),
	m_bluTeamName((IConVar*)NULL),
	m_redTeamName((IConVar*)NULL),
	m_pLastDemo(NULL)
{
	m_bookmarkBuff.SetBufferType(true, true);
}

CClientDemoRecorder::~CClientDemoRecorder()
{
	delete m_pLastDemo;
}

void CClientDemoRecorder::Load()
{
	m_bluTeamName.Init("mp_tournament_blueteamname", false);
	m_redTeamName.Init("mp_tournament_redteamname", false);
	LoadConfig();
}

void CClientDemoRecorder::ReloadConfig()
{
	LoadConfig();
}

void CClientDemoRecorder::TournamentMatchStarted( PluginContext_t *pContext )
{
	if (enabled.GetBool() && !m_bRecording && CanRecordDemo(pContext->m_pBaseClientDLL))
	{
		ConstructDemoName(pContext->m_pEngineClient);
		m_LatestDemo.m_nParts = 1;
		m_bRecording = true;
		m_bIStopped = true;

		m_bookmarks.RemoveAll();

		StartRecording(pContext->m_pEngineClient, m_LatestDemo, pContext->m_pEngineSound);
	}
}

void CClientDemoRecorder::TournamentMatchEnded( PluginContext_t *pContext )
{
	if (enabled.GetBool() && m_bRecording)
	{
		// let the 'stop' hook worry about
		// setting m_bRecording to false
		StopRecording(pContext->m_pEngineClient);
	}
}

void CClientDemoRecorder::FixInvisiblePlayers( PluginContext_t *pContext )
{
	if (m_bRecording)
	{
		m_bIStopped = true;
		m_LatestDemo.m_nParts += 1;
		StartRecording(pContext->m_pEngineClient, m_LatestDemo, pContext->m_pEngineSound, false);
	}
	else
	{
		pContext->m_pEngineClient->ClientCmd_Unrestricted( "record fix; stop\n" );
	}
}

void CClientDemoRecorder::StartRecordingFromCommand( PluginContext_t *pContext )
{
	if (!m_bRecording && CanRecordDemo(pContext->m_pBaseClientDLL))
	{
		ConstructDemoName(pContext->m_pEngineClient);
		m_LatestDemo.m_nParts = 1;
		m_bRecording = true;
		m_bIStopped = true;

		m_bookmarks.RemoveAll();

		StartRecording(pContext->m_pEngineClient, m_LatestDemo, pContext->m_pEngineSound);
	}
	else
	{
		Warning( "[SizzlingRecord] Error: Already recording.\n" );
	}
}

void CClientDemoRecorder::DeleteLatestDemo( PluginContext_t *pContext )
{
	if (m_pLastDemo)
	{
		char temp[128] = {};
		V_snprintf(temp, 128, "%s.dem", m_pLastDemo->m_szDemoName);

		IFileSystem *pFileSystem = pContext->m_pFullFileSystem;
		// remove the main demo file
		pFileSystem->RemoveFile(temp);

		// remove the parts if it has any
		for (uint32 i = 2; i <= m_pLastDemo->m_nParts; ++i)
		{
			V_snprintf(temp, 128, "%s_part%d.dem", m_pLastDemo->m_szDemoName, i);
			pFileSystem->RemoveFile(temp);
		}

		delete m_pLastDemo;
		m_pLastDemo = NULL;
	}
	else
	{
		Warning( "[SizzlingRecord] Error: No previous demo to delete.\n" );
	}
}

void CClientDemoRecorder::Bookmark( PluginContext_t *pContext, const char *comment )
{
	if (m_bRecording)
	{
		bookmarkInfo_t info;
		V_strncpy(info.comment, comment, sizeof(info.comment));
		info.tick = pContext->m_pEngineClient->GetDemoRecordingTick();
		info.time = time(NULL);

		m_bookmarks.AddToTail(info);
	}
}

void CClientDemoRecorder::StopRecordingEvent( PluginContext_t *pContext )
{
	if (!m_bIStopped && m_bRecording)
	{
		// stop recording
		m_bRecording = false;

		WriteOutBookmarks(pContext->m_pFullFileSystem);
		
		// update the most recent completed demo
		delete m_pLastDemo;
		m_pLastDemo = new DemoRecording_t;
		m_pLastDemo->m_nParts = m_LatestDemo.m_nParts;
		V_strcpy(m_pLastDemo->m_szDemoName, m_LatestDemo.m_szDemoName);
	}
	
	m_bIStopped = false;
}

void CClientDemoRecorder::WriteOutBookmarks( IFileSystem *pFileSystem )
{
	if (m_bookmarks.Count() > 0)
	{
		m_bookmarkBuff.Clear();

		for (int i = 0; i < m_bookmarks.Count(); ++i)
		{
			bookmarkInfo_t &info = m_bookmarks.Element(i);

			struct tm ltime;
			ConvertTimeToLocalTime(info.time, ltime);

			m_bookmarkBuff.Printf( "[%04i/%02i/%02i %02i:%02i] Player bookmark (\"%s\" at %i)\t\t// %s\r\n", 
				ltime.tm_year, ltime.tm_mon, ltime.tm_mday, ltime.tm_hour, ltime.tm_min,
				m_LatestDemo.m_szDemoName, info.tick, info.comment );
		}

		pFileSystem->AsyncAppend("Killstreaks.txt", m_bookmarkBuff.Base(), m_bookmarkBuff.TellPut(), false);

		m_bookmarks.RemoveAll();
	}
}

void CClientDemoRecorder::LoadConfig()
{
}

void CClientDemoRecorder::ConstructDemoName( IVEngineClient *pEngineClient )
{
	// get map name
	char szMapName[32] = {};
	GetMapName(pEngineClient, szMapName, 32);

	// get date and time
	struct tm ltime;
	GetDateAndTime(ltime);

	// construct the demo name
	V_snprintf(m_LatestDemo.m_szDemoName, 64, "%.4d%.2d%.2d_%.2d%.2d_%s_%s_%s", ltime.tm_year, ltime.tm_mon, ltime.tm_mday, ltime.tm_hour, ltime.tm_min, szMapName, m_bluTeamName.GetString(), m_redTeamName.GetString());
}

void CClientDemoRecorder::GetDateAndTime( struct tm &ltime )
{
	// get the time as an int64
	time_t t = time(NULL);

	ConvertTimeToLocalTime(t, ltime);
}

void CClientDemoRecorder::ConvertTimeToLocalTime( const time_t &t, struct tm &ltime )
{
	// convert it to a struct of time values
	ltime = *localtime(&t);

	// normalize the year and month
	ltime.tm_year = ltime.tm_year + 1900;
	ltime.tm_mon = ltime.tm_mon + 1;
}

void CClientDemoRecorder::GetMapName( IVEngineClient *pEngineClient, char *out, int length )
{
	const char *map_name = V_UnqualifiedFileName( pEngineClient->GetLevelName() );
	V_StripExtension(map_name, out, length);
	out[length-1] = '\0';
}

bool CClientDemoRecorder::CanRecordDemo( const IBaseClientDLL *pBaseClientDLL )
{
	return pBaseClientDLL->CanRecordDemo(NULL, 0);
}

void CClientDemoRecorder::StartRecording( IVEngineClient *pEngineClient, const DemoRecording_t &demoInfo, IEngineSound *pEngineSound, bool bEmitSound /*= true*/ )
{
	char command[256] = {};
	if (demoInfo.m_nParts == 1)
	{
		V_snprintf( command, 256, "stop; record %s\n", demoInfo.m_szDemoName );
	}
	else
	{
		V_snprintf( command, 256, "stop; record %s_part%d\n", demoInfo.m_szDemoName, demoInfo.m_nParts );
	}

	if (bEmitSound)
	{
		pEngineSound->EmitAmbientSound( "buttons/button17.wav", DEFAULT_SOUND_PACKET_VOLUME );
	}
	pEngineClient->ClientCmd_Unrestricted( command );
}

void CClientDemoRecorder::StopRecording( IVEngineClient *pEngineClient )
{
	pEngineClient->ClientCmd_Unrestricted( "stop\n" );
}

//===========================================================================//

static void VersionChangeCallback( IConVar *var, const char *pOldValue, float flOldValue );
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
class CEmptyServerPlugin: public IServerPluginCallbacks, public IGameEventListener2, public IRecvPropHookCallback, public ICommandCallback, public ICommandHookCallback
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
	
	// IRecvPropHookCallback Interface
	virtual bool RecvPropHookCallback( const CRecvProxyData *pData, void *pStruct, void *pOut );
	
	// ICommandCallback Interface
	virtual void CommandCallback( const CCommand &command );
	
	// ICommandHookCallback interface
	virtual bool CommandPreExecute( const CCommand &args );
	virtual void CommandPostExecute( const CCommand &args, bool bWasCommandExecuted );

	// Additions
	bool WaitingForPlayersChangeCallback( const CRecvProxyData *pData, void *pStruct, void *pOut );
	
	int GetCommandIndex() { return m_iClientCommandIndex; }
	
	void HookProps();
	void UnhookProps();

private:
	static bool ConfirmInterfaces( PluginContext_t *pContext );

private:
	CClientDemoRecorder m_DemoRecorder;

	CDllDemandLoader m_BaseClientDLL;
	CRecvPropHook m_bInWaitingForPlayersHook;
	ConVarRef m_refTournamentMode;
	CConCommandHook m_hookStop;
	// Interfaces from the engine in this struct
	PluginContext_t m_PluginContext;
	
	int	*m_iRoundState;
	bool *m_bInWaitingForPlayers;
	int m_iClientCommandIndex;
	bool m_bTournamentMatchStarted;
};

// 
// The plugin is a static singleton that is exported as an interface
//
static CEmptyServerPlugin s_EmptyServerPlugin;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CEmptyServerPlugin, IServerPluginCallbacks, INTERFACEVERSION_ISERVERPLUGINCALLBACKS, s_EmptyServerPlugin);

static ConCommand invis_players("sizz_record_fix_invisible_players", &s_EmptyServerPlugin, "Fixes invisible players by restarting the demo recording.");
static ConCommand start_recording("sizz_record_start_recording", &s_EmptyServerPlugin, "Starts recording a demo with SizzlingRecord.");
static ConCommand delete_last("sizz_record_delete_last_demo", &s_EmptyServerPlugin, "Deletes the most recent completed demo in this game session.");

static ConCommand reload_config("sizz_record_reload_config", &s_EmptyServerPlugin, "Reloads the configuration file for SizzlingRecord.");
static ConCommand bookmark("sizz_record_bookmark", &s_EmptyServerPlugin, "If recording, bookmarks the current tick and appends it to bookmarks.txt.\nAn optional comment is allowed as quoted or unquoted text after the command.");

//---------------------------------------------------------------------------------
// Purpose: constructor/destructor
//---------------------------------------------------------------------------------
CEmptyServerPlugin::CEmptyServerPlugin():
	m_BaseClientDLL("../tf/bin/client" PLUGIN_EXTENSION),
	m_refTournamentMode((IConVar*)NULL),
	m_PluginContext(),
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

	m_PluginContext.m_pEngineSound = (IEngineSound*)interfaceFactory(IENGINESOUND_CLIENT_INTERFACE_VERSION, NULL);
	m_PluginContext.m_pGameEventManager = (IGameEventManager2*)interfaceFactory(INTERFACEVERSION_GAMEEVENTSMANAGER2, NULL);
	m_PluginContext.m_pEngineClient = (IVEngineClient *)interfaceFactory(VENGINE_CLIENT_INTERFACE_VERSION, NULL);
	m_PluginContext.m_pBaseClientDLL = (IBaseClientDLL*)m_BaseClientDLL.GetFactory()(CLIENT_DLL_INTERFACE_VERSION, NULL);
	m_PluginContext.m_pFullFileSystem = (IFileSystem *)interfaceFactory(FILESYSTEM_INTERFACE_VERSION, NULL);
	
	IPlayerInfoManager *pPlayerInfoManager = (IPlayerInfoManager *)gameServerFactory(INTERFACEVERSION_PLAYERINFOMANAGER, NULL);
	if (pPlayerInfoManager)
	{
		m_PluginContext.m_pGlobals = pPlayerInfoManager->GetGlobalVars();
	}

	if ( !ConfirmInterfaces(&m_PluginContext) )
	{
		return false;
	}
	
	HookProps();
	
	// hook this event so we can get the new gamerules pointer each time
	//m_PluginContext.m_pGameEventManager->AddListener( this, "game_newmap", false );
	
	m_refTournamentMode.Init( "mp_tournament", false );
	m_hookStop.Hook(this, cvar, "stop");

	m_DemoRecorder.Load();

	MathLib_Init( 2.2f, 2.2f, 0.0f, 2 );
	ConVar_Register( 0 );
	return true;
}

//---------------------------------------------------------------------------------
// Purpose: called when the plugin is unloaded (turned off)
//---------------------------------------------------------------------------------
void CEmptyServerPlugin::Unload( void )
{
	if (m_PluginContext.m_pGameEventManager)
	{
		m_PluginContext.m_pGameEventManager->RemoveListener( this );
	}
	
	UnhookProps();
	
	if (cvar)
	{
		ConVar_Unregister( );
	}
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
	// this function is called on the client
	if (m_bTournamentMatchStarted)
	{
		m_DemoRecorder.TournamentMatchEnded(&m_PluginContext);
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
bool CEmptyServerPlugin::ConfirmInterfaces( PluginContext_t *pContext )
{
	bool bGood = true;

	if (!pContext->m_pEngineSound)
	{
		Warning( "Unable to load pEngineSound, aborting load\n" );
		bGood = false;
	}

	if (!pContext->m_pGameEventManager)
	{
		Warning( "Unable to load pGameEventManager, aborting load\n" );
		bGood = false;
	}
	
	if (!pContext->m_pEngineClient)
	{
		Warning( "Unable to load pEngineClient, aborting load\n" );
		bGood = false;
	}

	if (!pContext->m_pBaseClientDLL)
	{
		Warning( "Unable to load pBaseClientDLL, aborting load\n" );
		bGood = false;
	}
	
	if (!pContext->m_pFullFileSystem)
	{
		Warning( "Unable to load pFullFileSystem, aborting load\n" );
		bGood = false;
	}
	
	if (!pContext->m_pGlobals)
	{
		Warning( "Unable to load pGlobals, aborting load\n" );
		bGood = false;
	}

	return bGood;
}

void CEmptyServerPlugin::FireGameEvent( IGameEvent *event )
{
	/*const char *name = event->GetName();
	
	if (SCHelpers::FStrEq(name, "game_newmap"))
	{
		// get the new gamerules pointer in here
		// do i really need it for anything though?
	}*/
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

void CEmptyServerPlugin::CommandCallback( const CCommand &command )
{
	using namespace SCHelpers;

	const char *name = command.Arg(0);

	if ( FStrEq(name, "sizz_record_bookmark") )
	{
		m_DemoRecorder.Bookmark(&m_PluginContext, command.ArgS());
	}
	else if ( FStrEq(name, "sizz_record_fix_invisible_players") )
	{
		m_DemoRecorder.FixInvisiblePlayers(&m_PluginContext);
	}
	else if ( FStrEq(name, "sizz_record_start_recording") )
	{
		m_DemoRecorder.StartRecordingFromCommand(&m_PluginContext);
	}
	else if ( FStrEq(name, "sizz_record_delete_last_demo") )
	{
		m_DemoRecorder.DeleteLatestDemo(&m_PluginContext);
	}
	else if ( FStrEq(name, "sizz_record_reload_config") )
	{
		m_DemoRecorder.ReloadConfig();
	}
}

bool CEmptyServerPlugin::CommandPreExecute( const CCommand &args )
{
	m_DemoRecorder.StopRecordingEvent(&m_PluginContext);
	return true;
}

void CEmptyServerPlugin::CommandPostExecute( const CCommand &args, bool bWasCommandExecuted )
{
}

bool CEmptyServerPlugin::WaitingForPlayersChangeCallback( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	bool bWaitingForPlayers = !!(pData->m_Value.m_Int);

	// if the plugin is loaded right before the match starts,
	// this being true will trigger a recording started when the
	// match starts. that is what we want.
	static bool oldWaitingForPlayers = true;

	if (oldWaitingForPlayers != bWaitingForPlayers)
	{
		//using namespace Teamplay_GameRule_States;

		//Msg( UTIL_VarArgs( "round state is %s\n", GetStateName((gamerules_roundstate_t)roundstate) ) );
		bool bTournamentMode = m_refTournamentMode.GetInt() == 1;

		if (bWaitingForPlayers == true)
		{
			if (m_bTournamentMatchStarted)
			{
				m_DemoRecorder.TournamentMatchEnded(&m_PluginContext);
				m_bTournamentMatchStarted = false;
			}
		}
		else
		{
			// i don't think i need to check the round state for client stuff
			//int roundstate = *m_iRoundState;
			
			if (bTournamentMode && !m_bTournamentMatchStarted/* && (roundstate != GR_STATE_PREGAME)*/)
			{
				m_DemoRecorder.TournamentMatchStarted(&m_PluginContext);
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
	
	RecvProp *pbInWaitingForPlayers = Helpers::GetPropFromClassAndTable( m_PluginContext.m_pBaseClientDLL, "CTeamplayRoundBasedRulesProxy", "DT_TeamplayRoundBasedRules", "m_bInWaitingForPlayers" );
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

