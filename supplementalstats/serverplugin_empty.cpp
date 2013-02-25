
#include "PluginDefines.h"

#include "eiface.h"
//#include "igameevents.h"
//#include "tier0\icommandline.h"
#include "game/server/iplayerinfo.h"
#include "igameevents.h"
#include "steam/steamclientpublic.h"
#include "fasttimer.h"
#include "tier2/tier2.h"

//#include "ServerPluginHandler.h"
#include "UserIdTracker.h"
//#include "autoupdate.h"
//#include "ThreadCallQueue.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define INTERFACEVERSION_ISERVERPLUGINHELPERS			"ISERVERPLUGINHELPERS001"

// Interfaces from the engine
IVEngineServer				*pEngine = NULL;
IServerGameDLL				*pServerDLL;
IServerGameEnts				*pServerEnts;
IGameEventManager2			*gameeventmanager = NULL; // game events interface
IPlayerInfoManager			*playerinfomanager = NULL; // game dll interface to interact with players
CGlobalVars					*gpGlobals = NULL;
//extern s_ServerPlugin		*g_pServerPluginHandler;
extern UserIdTracker 		*g_pUserIdTracker;

struct playerInfo
{
	// which ones do i really need to reset? (instead of doing all of them)
	void reset() {
		memset( steamid, 0, 64 );
		memset( name, 0, 32 );
		teamid = 0;
		classid = 0;
		pEdict = NULL;
		pPlayerInfo = NULL;
		pSteamId = NULL;
	}
	char steamid[64];
	char name[32];
	int teamid;
	int classid;
	edict_t *pEdict;
	IPlayerInfo *pPlayerInfo;
	const CSteamID *pSteamId;
};

static const char *teamNames[] =
{
	"undefined",
	"undefined",
	"Red",
	"Blue"
};

static const char *classNames[] = 
{
	"undefined",
	"scout",
	"sniper",
	"soldier",
	"demoman",
	"medic",
	"heavyweapons",
	"pyro",
	"spy",
	"engineer"
};

inline bool FStrEq(const char *sz1, const char *sz2)
{
	return(Q_stricmp(sz1, sz2) == 0);
}

//const char *RenderSteamID( const CSteamID *pSteamID, char *dest )			// renders this steam ID to string
//{
//	switch( pSteamID->GetEAccountType() )
//	{
//	case k_EAccountTypeInvalid:
//	case k_EAccountTypeIndividual:
//		Q_snprintf(dest, sizeof(dest), "STEAM_0:%u:%u", (pSteamID->GetAccountID() % 2) ? 1 : 0, (int32)pSteamID->GetAccountID()/2);
//		break;
//	default:
//		Q_snprintf(dest, sizeof(dest), "%llu", pSteamID->ConvertToUint64());
//	}
//	return dest;
//}

//---------------------------------------------------------------------------------
// Purpose: a sample 3rd party plugin class
//---------------------------------------------------------------------------------
class CEmptyServerPlugin: public IServerPluginCallbacks, public IGameEventListener2
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

	//void					AutoUpdate();
	void					LoadCurrentPlayers();
	friend void				testDownloader();

	// IGameEventListener Interface
	virtual void			FireGameEvent( IGameEvent *event );

	virtual int				GetCommandIndex() { return m_iClientCommandIndex; }

	//void EnqueueFunctor( CFunctor *pFunctor ) { m_CallQueue.EnqueueFunctor( pFunctor ); }

private:
	playerInfo			m_entIndexToPlayerInfo[33];
	//CTSCallQueue		m_CallQueue;
	int					m_iClientCommandIndex;
};

// 
// The plugin is a static singleton that is exported as an interface
//
CEmptyServerPlugin g_EmptyServerPlugin;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CEmptyServerPlugin, IServerPluginCallbacks, INTERFACEVERSION_ISERVERPLUGINCALLBACKS, g_EmptyServerPlugin );

#pragma warning( push )
#pragma warning( disable: 4351 ) // new behavior: elements of array
//---------------------------------------------------------------------------------
// Purpose: constructor/destructor
//---------------------------------------------------------------------------------
CEmptyServerPlugin::CEmptyServerPlugin() : m_entIndexToPlayerInfo(), /*m_CallQueue(),*/ m_iClientCommandIndex(0)
{
}
#pragma warning( pop )

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

	if ( !cvar )
	{
	    Warning( "cvar is null, i hope we can get this\n" );
	    cvar = g_pCVar = (ICvar*)interfaceFactory(CVAR_INTERFACE_VERSION, NULL);
	    if ( !cvar )
	    {
	        Warning( "couldn't get cvar, aborting load\n" );
	        return false;
	    }
	}

	if ( !g_pCVar )
	{
	    Warning( "linking g_pCVar to cvar\n" );
	    g_pCVar = cvar;
	}

	g_pUserIdTracker->Load();

	pEngine = (IVEngineServer*)interfaceFactory(INTERFACEVERSION_VENGINESERVER, NULL);

	if (!pEngine){
		Warning( "Unable to load IVEngineServer, aborting load\n" );
		return false;
	}
	/*
	g_pServerPluginHandler = (s_ServerPlugin*)interfaceFactory(INTERFACEVERSION_ISERVERPLUGINHELPERS, NULL);
	
	if (!g_pServerPluginHandler)
	{
		Warning( "Unable to load IServerPluginHelpers, aborting load\n" );
		return false;
	}*/

	// hopefully nothing bad will happen from this being inbetween the interface loading
	// it needs at least these two interfaces and IFileSystem *g_pFullFileSystem
	// not sure if the filesystem is loaded here yet
	//AutoUpdate();

	gameeventmanager = (IGameEventManager2*)interfaceFactory(INTERFACEVERSION_GAMEEVENTSMANAGER2, NULL);
	if (!gameeventmanager)
	{
		Warning( "Unable to load IGameEventManager2, aborting load\n" );
		return false;
	}

	playerinfomanager = (IPlayerInfoManager *)gameServerFactory(INTERFACEVERSION_PLAYERINFOMANAGER,NULL);
	if (!playerinfomanager)
	{
		Warning( "Unable to load IPlayerInfoManager, aborting load\n" );
		return false;
	}

	gpGlobals = playerinfomanager->GetGlobalVars();

	gameeventmanager->AddListener( this, "item_pickup", true );
	gameeventmanager->AddListener( this, "player_hurt", true );
	gameeventmanager->AddListener( this, "player_healed", true );
	gameeventmanager->AddListener( this, "player_spawn", true );
	gameeventmanager->AddListener( this, "player_team", true );
	gameeventmanager->AddListener( this, "player_changename", true );
	gameeventmanager->AddListener( this, "player_changeclass", true );

	LoadCurrentPlayers();

	MathLib_Init( 2.2f, 2.2f, 0.0f, 2 );
	ConVar_Register( 0 );

	return true;
}

//---------------------------------------------------------------------------------
// Purpose: called when the plugin is unloaded (turned off)
//---------------------------------------------------------------------------------
void CEmptyServerPlugin::Unload( void )
{
	gameeventmanager->RemoveListener( this ); // make sure we are unloaded from the event system
	
	ConVar_Unregister( );
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
/*
void CEmptyServerPlugin::AutoUpdate()
{
    // need to change this to make it threaded
	autoUpdateInfo_s a = { PLUGIN_PATH PLUGIN_NAME, URL_TO_UPDATED, URL_TO_META, PLUGIN_PATH, 0, PLUGIN_VERSION };
	CAutoUpdater autoUpdater(a);
	//autoUpdater.OfflineTest();
	autoUpdater.PerformUpdateIfAvailable( s_pluginInfo );
	//autoUpdater.testDownloadMeta();
}*/

//---------------------------------------------------------------------------------
// Purpose: the name of this plugin, returned in "plugin_print" command
//---------------------------------------------------------------------------------
const char *CEmptyServerPlugin::GetPluginDescription( void )
{
	return "Supplemental Stats v" PLUGIN_VERSION ", Jean-Denis Caron, VSP port by SizzlingCalamari, " __DATE__;
}

//---------------------------------------------------------------------------------
// Purpose: called on level start
//---------------------------------------------------------------------------------
void CEmptyServerPlugin::LevelInit( char const *pMapName )
{
	char maplog[64];
	Q_snprintf( maplog, 64, "Loading map \"%s\"\n", pMapName );
	pEngine->LogPrint( maplog );
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
    // this is used to call engine functions from separate threads
    // since valve's code is probably not thread safe
   // m_CallQueue.callQueueGameFrame();
}

//---------------------------------------------------------------------------------
// Purpose: called on level end (as the server is shutting down or going to a new map)
//---------------------------------------------------------------------------------
void CEmptyServerPlugin::LevelShutdown( void ) // !!!!this can get called multiple times per map change
{
}

//---------------------------------------------------------------------------------
// Purpose: called when a client spawns into a server (i.e as they begin to play)
//---------------------------------------------------------------------------------
void CEmptyServerPlugin::ClientActive( edict_t *pEdict )
{
	if ( !pEdict || pEdict->IsFree() )
		return;
	IPlayerInfo *pPlayerInfo = playerinfomanager->GetPlayerInfo( pEdict );
	if ( !pPlayerInfo || !pPlayerInfo->IsConnected() )
	{
		return;
	}
	int index = g_pUserIdTracker->ClientActive( pEdict );
	playerInfo &pInfo = m_entIndexToPlayerInfo[index];
	pInfo.pEdict = pEdict;
	pInfo.pPlayerInfo = pPlayerInfo;
	Q_snprintf( pInfo.name, 32, "%s", pPlayerInfo->GetName() );
	pInfo.teamid = pPlayerInfo->GetTeamIndex();
	pInfo.pSteamId = pEngine->GetClientSteamID( pEdict );
	if ( !pInfo.pSteamId )
	{
		return;
	}
	Q_snprintf( pInfo.steamid, 64, "%s", pInfo.pPlayerInfo->GetNetworkIDString() );
}

//---------------------------------------------------------------------------------
// Purpose: called when a client leaves a server (or is timed out)
//---------------------------------------------------------------------------------
void CEmptyServerPlugin::ClientDisconnect( edict_t *pEdict )
{
	g_pUserIdTracker->ClientDisconnect( pEdict );
	int index = pEngine->IndexOfEdict( pEdict );
	// do i really need to reset here?
	// what uses the struct after the player leaves?
	m_entIndexToPlayerInfo[index].reset();
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
}

void CEmptyServerPlugin::LoadCurrentPlayers()
{
	for ( int i = 1; i <= gpGlobals->maxClients; ++i )
	{
		edict_t *pEdict = pEngine->PEntityOfEntIndex(i);
		if ( !pEdict || pEdict->IsFree() )
			continue;
		IPlayerInfo *pPlayerInfo = playerinfomanager->GetPlayerInfo( pEdict );
		if ( !pPlayerInfo || !pPlayerInfo->IsConnected() )
		{
			continue;
		}
		int index = g_pUserIdTracker->ClientActive( pEdict );
		playerInfo &pInfo = m_entIndexToPlayerInfo[index];
		pInfo.pEdict = pEdict;
		pInfo.pPlayerInfo = pPlayerInfo;
		Q_snprintf( pInfo.name, 32, "%s", pPlayerInfo->GetName() );
		pInfo.teamid = pPlayerInfo->GetTeamIndex();
		pInfo.pSteamId = pEngine->GetClientSteamID( pEdict );
		if ( !pInfo.pSteamId )
		{
			continue;
		}
		Q_snprintf( pInfo.steamid, 64, "%s", pInfo.pPlayerInfo->GetNetworkIDString() );
	}
}

//---------------------------------------------------------------------------------
// Purpose: called when an event is fired
//---------------------------------------------------------------------------------
void CEmptyServerPlugin::FireGameEvent( IGameEvent *event )
{
	const char * name = event->GetName();

	if ( FStrEq( name, "player_hurt" ) )
	{
#ifndef RELEASE_VERSION
		PROFILE_SCOPE( player_hurt );
#endif
		int damageamount = event->GetInt( "damageamount" );
		if ( damageamount == 0 )
			return;
		int attackerid = event->GetInt( "attacker" );
		if ( ( event->GetInt( "userid" ) != attackerid ) && attackerid != 0 )
		{
			int index = g_pUserIdTracker->GetEntIndex(attackerid);
			playerInfo &pInfo = m_entIndexToPlayerInfo[index];

			if ( !pInfo.pSteamId )
			{
				pInfo.pSteamId = pEngine->GetClientSteamID( pInfo.pEdict );
				if ( !pInfo.pSteamId )
					return;
				Q_snprintf( pInfo.steamid, 64, "%s", pInfo.pPlayerInfo->GetNetworkIDString() );
			}

			char log[128] = { '\0' };
			Q_snprintf( log, 128, "\"%s<%d><%s><%s>\" triggered \"damage\" (damage \"%d\")\n",
				pInfo.name,
				attackerid,
				pInfo.steamid,
				teamNames[pInfo.teamid],
				damageamount );
			pEngine->LogPrint( log );
			//L 03/21/2011 - 02:32:11: "rline<326><STEAM_0:1:796515><Blue>" triggered "damage" (damage "37")
		}
	}
	else if ( FStrEq( name, "player_healed" ) )
	{
#ifndef RELEASE_VERSION
		PROFILE_SCOPE( player_healed );
#endif
		int healer = event->GetInt( "healer" );
		int index1 = g_pUserIdTracker->GetEntIndex( healer );
		playerInfo &pInfo1 = m_entIndexToPlayerInfo[index1];

		int patient = event->GetInt( "patient" );
		int index2 = g_pUserIdTracker->GetEntIndex( patient );
		playerInfo &pInfo2 = m_entIndexToPlayerInfo[index2];

		if ( !pInfo1.pSteamId )
		{
			pInfo1.pSteamId = pEngine->GetClientSteamID( pInfo1.pEdict );
			if ( !pInfo1.pSteamId )
				return;
			Q_snprintf( pInfo1.steamid, 64, "%s", pInfo1.pPlayerInfo->GetNetworkIDString() );
		}

		if ( !pInfo2.pSteamId )
		{
			pInfo2.pSteamId = pEngine->GetClientSteamID( pInfo2.pEdict );
			if ( !pInfo2.pSteamId )
				return;
			Q_snprintf( pInfo2.steamid, 64, "%s", pInfo2.pPlayerInfo->GetNetworkIDString() );
		}
					
		char log[196] = { '\0' };
		Q_snprintf( log, 196, "\"%s<%d><%s><%s>\" triggered \"healed\" against \"%s<%d><%s><%s>\" (healing \"%d\")\n", 
			pInfo1.name,
			healer,
			pInfo1.steamid,
			teamNames[pInfo1.teamid],
			pInfo2.name,
			patient,
			pInfo2.steamid,
			teamNames[pInfo2.teamid],
			event->GetInt( "amount" ) );
		pEngine->LogPrint( log );
		//L 03/21/2011 - 02:35:55: "HackLimit2. MixMixMixMix<333><STEAM_0:1:15579670><Blue>" triggered "healed" against "AI kaytee� fbs!!<331><STEAM_0:0:9786107><Blue>" (healing "73")
	}
	else if ( FStrEq( name, "item_pickup" ) )
	{
#ifndef RELEASE_VERSION
		PROFILE_SCOPE( item_pickup );
#endif
		int userid = event->GetInt( "userid" );
		int index = g_pUserIdTracker->GetEntIndex( userid );

		playerInfo &pInfo = m_entIndexToPlayerInfo[index];
		if ( !pInfo.pSteamId )
		{
			pInfo.pSteamId = pEngine->GetClientSteamID( pInfo.pEdict );
			if ( !pInfo.pSteamId )
				return;
			Q_snprintf( pInfo.steamid, 64, "%s", pInfo.pPlayerInfo->GetNetworkIDString() );
		}

		char log[128] = { '\0' };
		Q_snprintf( log, 128, "\"%s<%d><%s><%s>\" picked up item \"%s\"\n", 
			pInfo.name,
			userid,
			pInfo.steamid,
			teamNames[pInfo.teamid],
			event->GetString( "item" ) );
		pEngine->LogPrint( log );
		//L 03/21/2011 - 02:35:56: "GooB<330><STEAM_0:1:23384772><Blue>" picked up item "tf_ammo_pack"
	}
	else if ( FStrEq( name, "player_spawn" ) )
	{
#ifndef RELEASE_VERSION
		PROFILE_SCOPE( player_spawn );
#endif
		int userid = event->GetInt( "userid" );
		int index = g_pUserIdTracker->GetEntIndex( userid );
		playerInfo &pInfo = m_entIndexToPlayerInfo[index];

		if ( !pInfo.pSteamId )
		{
			pInfo.pSteamId = pEngine->GetClientSteamID( pInfo.pEdict );
			if ( !pInfo.pSteamId )
				return;
			Q_snprintf( pInfo.steamid, 64, "%s", pInfo.pPlayerInfo->GetNetworkIDString() );
		}

		char log[128] = { '\0' };
		Q_snprintf( log, 128, "\"%s<%d><%s><%s>\" spawned as \"%s\"\n", 
			pInfo.name,
			userid,
			pInfo.steamid,
			teamNames[ pInfo.teamid ],
			classNames[ pInfo.classid ] );
		pEngine->LogPrint( log );

		        //LogToGame("\"%s<%d><%s><%s>\" spawned as \"%s\"",
          //        clientname,
          //        user,
          //        steamid,
          //        team,
          //        classes[clss]);
	}
	else if ( FStrEq( name, "player_changeclass" ) )
	{
		int userid = event->GetInt( "userid" );
		int index = g_pUserIdTracker->GetEntIndex( userid );
		m_entIndexToPlayerInfo[index].classid = event->GetInt( "class" );
	}
	else if ( FStrEq( name, "player_team" ) )
	{
		int userid = event->GetInt( "userid" );
		int index = g_pUserIdTracker->GetEntIndex( userid );
		m_entIndexToPlayerInfo[index].teamid = event->GetInt( "team" );
	}
	else if ( FStrEq( name, "player_changename" ) )
	{
		int userid = event->GetInt( "userid" );
		int index = g_pUserIdTracker->GetEntIndex( userid );
		Q_snprintf( m_entIndexToPlayerInfo[index].name, 32, "%s", event->GetString( "newname" ) );
	}
}
