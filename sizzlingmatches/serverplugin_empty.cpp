
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/
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

#include "SC_helpers.h"
#include "dt_send.h"
#include "server_class.h"
#include "const.h"
#include "playerdata.h"
#include "SizzlingMatches.h"
#include "timedeventmgr.h"
//#include "queuethread.h"
//#include "cbase.h"

#ifndef SERVER_BUILD
	#include "cdll_int.h"
	#include "dt_recv.h"
	#include "client_class.h"
#endif


// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Interfaces from the engine
IVEngineServer			*pEngine = NULL; // helper functions (messaging clients, loading content, making entities, running commands, etc)
IGameEventManager2		*gameeventmanager = NULL; // game events interface
IPlayerInfoManager		*playerinfomanager = NULL; // game dll interface to interact with players
//IBotManager			*botmanager = NULL; // game dll interface to interact with bots
IServerPluginHelpers	*helpers = NULL; // special 3rd party plugin helpers from the engine
IUniformRandomStream	*randomStr = NULL;
IEngineTrace			*enginetrace = NULL;

#ifndef SERVER_BUILD
	IBaseClientDLL			*pBaseClientDLL = NULL;
	IVEngineClient			*pEngineClient = NULL;
#endif
IServerGameDLL			*pServerDLL = NULL;
IServerGameEnts			*pServerEnts = NULL;

CGlobalVars				*gpGlobals = NULL;

//===========================================================================//

// function to initialize any cvars/command in this plugin
//void Bot_RunAll( void ); 

template <typename T>
T *GetInterface( const char	*pchModuleName, const char *pszInterfaceVersion );

SendTable		*GetDataTable( const char *pTableName, SendTable *pTable );
unsigned int	GetPropOffsetFromTable(const char *pTableName, const char *pPropName, bool &bErr);
//void			GetEntityByClassname(const char *szClassname);
//void			PrintInfo( void );
void			CreateMenu(bf_write* pBuffer, const char* szMessage, int nOptions=10, int iSecondsToStayOpen=-1);

CBaseEntity *GetBaseFromID(int id);

void ShowMenu( edict_t *pEntity, int buttons, int time, const char *pText) // STOLE IT FROM TONY AND MODIFIED IT LOL
{
	SRecipientFilter filter( pEngine->IndexOfEdict( pEntity ) );

	int umsg = 9;	// showmenu is 9
	if(umsg != -1)
	{
		char text[2048];
		char buf[251];
		char *p = text;
		//int limit = strlen(pText);

		strncpy(text, pText, sizeof(text));
		text[sizeof(text)-1] = '\0';

		// write messages with more option enabled while there is enough data
		while(strlen(p) > sizeof(buf)-1)
		{
			strncpy(buf, p, sizeof(buf));
			buf[sizeof(buf)-1] = '\0';

			bf_write *pBuffer = pEngine->UserMessageBegin(&filter, umsg);
			pBuffer->WriteShort(buttons);       // Sets how many options the menu has
			pBuffer->WriteChar(time);           // Sets how long the menu stays open -1 for stay until option selected
			pBuffer->WriteByte(true);           // more?
			pBuffer->WriteString(buf);          // The text shown on the menu
			pEngine->MessageEnd();

			p += sizeof(buf) - 1;
		}
		// then send last bit
		bf_write *pBuffer = pEngine->UserMessageBegin(&filter, umsg);
		pBuffer->WriteShort(buttons);       // Sets how many options the menu has
		pBuffer->WriteChar(time);           // Sets how long the menu stays open -1 for stay until option selected
		pBuffer->WriteByte(false);          // more?
		pBuffer->WriteString(p);            // The text shown on the menu
		pEngine->MessageEnd();
	}
}



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

template <typename T>
T *GetInterface( const char	*pchModuleName, const char *pszInterfaceVersion )
{
	int nReturnCode = 0;
	CDllDemandLoader module = CDllDemandLoader( pchModuleName );
	T *pInterface = (T*)module.GetFactory()(pszInterfaceVersion, &nReturnCode);
	module.Unload();
	//if (nReturnCode == IFACE_FAILED)
	//	return NULL;
	//else
	//	return pInterface;

	return (nReturnCode == IFACE_FAILED) ? NULL : pInterface;
}

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

	// Additions
	virtual bool			ConfirmInterfaces( void );
	//virtual bool			GetPropOffset( const char *pClassName, const char *pPropName, unsigned int &offset, bool bServerSide );
	//virtual void			GetPropsFromTable( const char *pTableName );
	//virtual void			GetMessageInts( void );

	// IGameEventListener Interface
	virtual void FireGameEvent( IGameEvent *event );

	virtual int GetCommandIndex() { return m_iClientCommandIndex; }

private:
	int m_iClientCommandIndex;
	SizzlingMatches m_SizzlingMatches;
	float m_fNextEventTime;
	//CUtlMap<char *, int> m_MessageNameToIDMap;
	//const char *m_pPropNames[20];
};

// 
// The plugin is a static singleton that is exported as an interface
//
CEmptyServerPlugin g_ServerPlugin;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CEmptyServerPlugin, IServerPluginCallbacks, INTERFACEVERSION_ISERVERPLUGINCALLBACKS, g_ServerPlugin );

//---------------------------------------------------------------------------------
// Purpose: constructor/destructor
//---------------------------------------------------------------------------------
CEmptyServerPlugin::CEmptyServerPlugin()
{
	m_iClientCommandIndex = 0;
	m_fNextEventTime = 0.0f;
}

CEmptyServerPlugin::~CEmptyServerPlugin()
{
}

//---------------------------------------------------------------------------------
// Purpose: called when the plugin is loaded, load the interface we need from the engine
//---------------------------------------------------------------------------------
bool CEmptyServerPlugin::Load(	CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory )
{
	//bool bSucess = true;
	ConnectTier1Libraries( &interfaceFactory, 1 );
	ConnectTier2Libraries( &interfaceFactory, 1 );

	//g_pFullFileSystem = GetInterface<IFileSystem>( "bin/filesystem_steam.dll", FILESYSTEM_INTERFACE_VERSION );
	g_pFullFileSystem = (IFileSystem *)interfaceFactory(FILESYSTEM_INTERFACE_VERSION, NULL);
//#ifdef WIN32
//	pServerDLL = GetInterface<IServerGameDLL>( "tf/bin/server.dll", INTERFACEVERSION_SERVERGAMEDLL );
//#else
//	pServerDLL = GetInterface<IServerGameDLL>( "tf/bin/server.so", INTERFACEVERSION_SERVERGAMEDLL );
//#endif

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
	//if ( !playerinfomanager )
	//{
	//	Warning( "Unable to load playerinfomanager, ignoring\n" ); // this isn't fatal, we just won't be able to access specific player data
	//}

	//botmanager = (IBotManager *)gameServerFactory(INTERFACEVERSION_PLAYERBOTMANAGER, NULL);
	//if ( !botmanager )
	//{
	//	Warning( "Unable to load botcontroller, ignoring\n" ); // this isn't fatal, we just won't be able to access specific bot functions
	//}

	pEngine = (IVEngineServer*)interfaceFactory(INTERFACEVERSION_VENGINESERVER, NULL);
	gameeventmanager = (IGameEventManager2*)interfaceFactory(INTERFACEVERSION_GAMEEVENTSMANAGER2, NULL);
	helpers = (IServerPluginHelpers*)interfaceFactory(INTERFACEVERSION_ISERVERPLUGINHELPERS, NULL);
	enginetrace = (IEngineTrace *)interfaceFactory(INTERFACEVERSION_ENGINETRACE_SERVER,NULL);
	randomStr = (IUniformRandomStream *)interfaceFactory(VENGINE_SERVER_RANDOM_INTERFACE_VERSION, NULL);

	pServerEnts = (IServerGameEnts *)gameServerFactory(INTERFACEVERSION_SERVERGAMEENTS, NULL);
	pServerDLL = (IServerGameDLL *)gameServerFactory(INTERFACEVERSION_SERVERGAMEDLL, NULL);
#ifndef SERVER_BUILD
	pEngineClient = (IVEngineClient *)interfaceFactory(VENGINE_CLIENT_INTERFACE_VERSION, NULL);
	//pEngineClient = UTIL_LoadInterface<IVEngineClient>(interfaceFactory, VENGINE_CLIENT_INTERFACE_VERSION, bSucess);
	//pBaseClientDLL = UTIL_LoadInterface<IBaseClientDLL>(pfnClientFactory, CLIENT_DLL_INTERFACE_VERSION, bSucess);

	pBaseClientDLL = GetInterface<IBaseClientDLL>( "tf/bin/client.dll", CLIENT_DLL_INTERFACE_VERSION );
#endif

	// get the interfaces we want to use
	//if(	! ( pEngine && gameeventmanager && g_pFullFileSystem && helpers && enginetrace && randomStr ) )
	//{
	//	Warning( "Unable to load one of these: engine && gameeventmanager && g_pFullFileSystem && helpers && enginetrace && randomStr\n" );
	//	return false; // we require all these interface to function
	//}

	if ( !ConfirmInterfaces() ){
		return false;
	}

	//if ( playerinfomanager ){
	gpGlobals = playerinfomanager->GetGlobalVars();
	//}
	
	//GetMessageInts();

	gameeventmanager->AddListener( this, "teamplay_round_stalemate", true );
	gameeventmanager->AddListener( this, "teamplay_round_active", true );		//9:54
	gameeventmanager->AddListener( this, "teamplay_round_win", true );			//end round
	gameeventmanager->AddListener( this, "teamplay_point_captured", true );
	gameeventmanager->AddListener( this, "arena_win_panel", true );
	gameeventmanager->AddListener( this, "teamplay_win_panel", true );			//fix for incorrect round points
	gameeventmanager->AddListener( this, "player_say", true );
	gameeventmanager->AddListener( this, "teamplay_game_over", true );
	gameeventmanager->AddListener( this, "player_changename", true );
	gameeventmanager->AddListener( this, "player_team", true );
	gameeventmanager->AddListener( this, "tf_game_over", true );
	//gameeventmanager->AddListener( this, "teamplay_broadcast_audio", true );

	m_SizzlingMatches.SM_GetPropOffsets();
	m_SizzlingMatches.SM_SetEventUpdateInterval( 3.0f );
	m_SizzlingMatches.SM_LoadCurrentPlayers();

	//pEngine->ServerCommand( "con_logfile sizzmatches.txt\n" );
	//Name: 	player_changename
	//Structure: 	
	//short 	userid 	user ID on server
	//string 	oldname 	players old (current) name
	//string 	newname 	players new name 

	//Name: 	player_chat						// doesn't work
	//Structure: 	
	//bool 	teamonly 	true if team only chat
	//short 	userid 	chatting player
	//string 	text 	chat text 

	//Name: 	player_say
	//Structure: 	
	//short 	userid 	chatting player
	//string 	text 	chat text 

	MathLib_Init( 2.2f, 2.2f, 0.0f, 2 );
	ConVar_Register( 0 );
	return true;
}

//---------------------------------------------------------------------------------
// Purpose: called when the plugin is unloaded (turned off)
//---------------------------------------------------------------------------------
void CEmptyServerPlugin::Unload( void )
{
	m_SizzlingMatches.SM_Msg( "plugin unloading\n" );
	gameeventmanager->RemoveListener( this ); // make sure we are unloaded from the event system

	m_SizzlingMatches.SM_DeleteAllPlayerData();

	ConVar_Unregister( );
	DisconnectTier2Libraries( );
	DisconnectTier1Libraries( );
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
	return "SizzlingMatches v0.1.0.0, SizzlingCalamari";
}

//---------------------------------------------------------------------------------
// Purpose: called on level start
//---------------------------------------------------------------------------------
void CEmptyServerPlugin::LevelInit( char const *pMapName )
{
	//Msg( "Level \"%s\" has been loaded\n", pMapName );
	//gameeventmanager->AddListener( this, "player_death", true );
	m_SizzlingMatches.LevelInit( pMapName );
	m_fNextEventTime = 0.0f;
}

//---------------------------------------------------------------------------------
// Purpose: called on level start, when the server is ready to accept client connections
//		edictCount is the number of entities in the level, clientMax is the max client count
//---------------------------------------------------------------------------------
void CEmptyServerPlugin::ServerActivate( edict_t *pEdictList, int edictCount, int clientMax )
{
	//m_SizzlingMatches.SM_GetEntities();
}

//---------------------------------------------------------------------------------
// Purpose: called once per server frame, do recurring work here (like checking for timeouts)
//---------------------------------------------------------------------------------
void CEmptyServerPlugin::GameFrame( bool simulating )
{
	if ( simulating )
	{
		m_SizzlingMatches.GameFrame();
	}
	//if ( simulating )
	//{
	//	//float curtime = gpGlobals->curtime;
	//	//if ( curtime >= m_fNextEventTime ){
	//	//	m_fNextEventTime = curtime + 3.0f;
	//	//	FireEvent();
	//	//	//g_pMessage->AllUserChatMessage(  );
	//	//	//g_pMessage->AllUserHudMsg( "HELLO WHALE\n" );
	//	//	//g_pMessage->AllUserHudHintText( "jajajaja\n" );
	//	//}
	//	//Bot_RunAll();
	//}
}

//---------------------------------------------------------------------------------
// Purpose: called on level end (as the server is shutting down or going to a new map)
//---------------------------------------------------------------------------------
void CEmptyServerPlugin::LevelShutdown( void ) // !!!!this can get called multiple times per map change
{
	//gameeventmanager->RemoveListener( this );
	m_SizzlingMatches.LevelShutdown();
}


//---------------------------------------------------------------------------------
// Purpose: called when a client spawns into a server (i.e as they begin to play)
//---------------------------------------------------------------------------------
void CEmptyServerPlugin::ClientActive( edict_t *pEntity )
{
	if( !pEntity || pEntity->IsFree() )		//TODO: decide on having the null and isfree check in the insert, or outside
		return;								// for now... BOTH!	heuheuuehuhuehuhe

	m_SizzlingMatches.SM_InsertPlayer( pEntity );
}

//---------------------------------------------------------------------------------
// Purpose: called when a client leaves a server (or is timed out)
//---------------------------------------------------------------------------------
void CEmptyServerPlugin::ClientDisconnect( edict_t *pEntity )
{
	if( !pEntity || pEntity->IsFree() )
		return;

	m_SizzlingMatches.PlayerDisconnect( pEntity );
}

//---------------------------------------------------------------------------------
// Purpose: called on 
//---------------------------------------------------------------------------------
void CEmptyServerPlugin::ClientPutInServer( edict_t *pEntity, char const *playername )
{
	if( !pEntity || pEntity->IsFree() )
		return;

	IPlayerInfo *pPlayerInfo = playerinfomanager->GetPlayerInfo( pEntity );
	if (pPlayerInfo)
		Msg("player: %s put in server\n", pPlayerInfo->GetName() );
	else
		Msg("ClientPutInServer error\n");
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
	IPlayerInfo *pPlayerInfo = playerinfomanager->GetPlayerInfo( pEntity );
	if (pPlayerInfo)
		Msg("player: %s connected\n", pPlayerInfo->GetName() );
	else
		Msg("ClientConnect error\n");

	return PLUGIN_CONTINUE;
}

//---------------------------------------------------------------------------------
// Purpose: called when a client types in a command (only a subset of commands however, not CON_COMMAND's)
//---------------------------------------------------------------------------------
PLUGIN_RESULT CEmptyServerPlugin::ClientCommand( edict_t *pEntity, const CCommand &args )
{
	const char *cmd = args.Arg(0);
    const char *cmd1 = args.Arg(1);

    if (strcmp(cmd, "achievements")==0)
    {
        //int client = engine->IndexOfEdict(pEntity);
        // Create a filter and add this client to it
        //SRecipientFilter filter;
		
        //filter.AddRecipient(engine->IndexOfEdict(pEntity));

        //// Start the usermessage and get a bf_write
        //bf_write* pBuffer = engine->UserMessageBegin(&filter, 9);

        //// Send the menu
        //CreateMenu(pBuffer, "Menu Title\n---------------\n->1.Option1\n->2.Option2\n->3.Option3\n->4.Option4\n->5.Option5\n->6.Option6\n->7.Option7\n->8.Option8", 8, 10);

        //engine->MessageEnd();

		ShowMenu( pEntity, 0xffff, -1, "Menu Title\n---------------\n->1.Option1\n->2.Option2\n->3.Option3\n->4.Option4\n->5.Option5\n->6.Option6\n->7.Option7\n->8.Option8" );

        return PLUGIN_CONTINUE;
    }

    if (strcmp(cmd, "menuselect")==0)
    {
        switch(atoi(cmd1))
        {
        case 1:
            Msg("Selected Option1");
            break;
        case 2:
            Msg("Selected Option2");
            break;
        case 3:
            Msg("Selected Option3");
            break;
        case 4:
            Msg("Selected Option4");
            break;
        case 5:
            Msg("Selected Option5");
            break;
        case 6:
            Msg("Selected Option6");
            break;
        case 7:
            Msg("Selected Option7");
            break;
        case 8:
            Msg("Selected Option8");
            break;
        }
 
        return PLUGIN_CONTINUE;
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
		Warning( "Unable to load pEngine, aborting load\n" );
		return false;
	}

	if (!gameeventmanager){
		Warning( "Unable to load gameeventmanager, aborting load\n" );
		return false;
	}

	if (!g_pFullFileSystem){
		Warning( "Unable to load g_pFullFileSystem, aborting load\n" );
		return false;
	}

	if (!helpers){
		Warning( "Unable to load helpers, aborting load\n" );
		return false;
	}

	if (!enginetrace){
		Warning( "Unable to load enginetrace, aborting load\n" );
		return false;
	}

	if (!randomStr){
		Warning( "Unable to load randomStr, aborting load\n" );
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
		Msg( "%s, offset: %i\n", TableName, pProp->GetOffset() );

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

//---------------------------------------------------------------------------------
// Purpose: used by the GetPropOffsetFromTable func to get a specific table
//---------------------------------------------------------------------------------
SendTable *GetDataTable( const char *pTableName, SendTable *pTable )
{
	SendTable *pSendTable = pTable;
	if (pSendTable == NULL)
		return NULL;
	if ( SCHelpers::FStrEq( pTableName, pSendTable->GetName() ) )
		return pSendTable;
	int num = pSendTable->GetNumProps();
	for (int i = 0; i < num; i++){
		SendProp *pProp = pSendTable->GetProp(i);
		SendTable *pSubTable = GetDataTable( pTableName, pProp->GetDataTable() );
		if (pSubTable == NULL)
			continue;
		if ( SCHelpers::FStrEq(pSubTable->GetName(), pTableName) )
			return pSubTable;
	}
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
			if ( SCHelpers::FStrEq( pPropName, pProp->GetName() ) )
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

void CreateMenu(bf_write* pBuffer, const char* szMessage, int nOptions, int iSecondsToStayOpen)
{
	Assert(pBuffer);
 
	// Add option to bits
	int optionBits = 0;
	for(int i = 0; i < nOptions; i++)
		optionBits |= (1<<i);
 
	pBuffer->WriteShort(optionBits); // Write options
	pBuffer->WriteChar( iSecondsToStayOpen ); // Seconds to stay open
	pBuffer->WriteByte(false); // We don't need to receive any more of this menu
	pBuffer->WriteString(szMessage); // Write the menu message
}

//---------------------------------------------------------------------------------
// Purpose: called when an event is fired
//---------------------------------------------------------------------------------
void CEmptyServerPlugin::FireGameEvent( IGameEvent *event )
{
	using namespace SCHelpers;

	const char * name = event->GetName();
	Msg( "CEmptyServerPlugin::FireGameEvent: Got event \"%s\"\n", name );

	if ( FStrEq( name, "teamplay_round_win" ) || FStrEq( name, "teamplay_round_stalemate" ) )
	{
		//m_SizzlingMatches.SM_AllUserChatMessage( "Stats Recording Stopped\n" );
		m_SizzlingMatches.SM_EndOfRound();
	}
	else if ( FStrEq( name, "teamplay_round_active" ) || FStrEq( name, "arena_round_start" ) )
	{
		m_SizzlingMatches.SM_StartOfRound();
		//GetEntityByClassname( "CTeam" );
	}
	else if ( FStrEq( name, "teamplay_win_panel" ) || FStrEq( name, "arena_win_panel" ) )
	{
	}
	else if ( FStrEq( name, "teamplay_game_over" ) || FStrEq( name, "tf_game_over" ) )
	{
		m_SizzlingMatches.SM_GameOver();
	}
	else if ( FStrEq( name, "player_say" ) )
	{
		const char *text = event->GetString( "text" );

		if ( !m_SizzlingMatches.SM_IsStarted() )
		{
			if ( FStrEq( text, ".ready" ) || FStrEq( text, ".gaben" ) )
			{
				m_SizzlingMatches.SM_PlayerChangeReadyState( event->GetInt( "userid" ), true );
			}
			else if ( FStrEq( text, ".notready" ) )
			{
				m_SizzlingMatches.SM_PlayerChangeReadyState( event->GetInt( "userid" ), false );
			}
			else if ( FStrEq( text, ".forcestart" ) )
			{
				if ( UserIDToSteamID( event->GetInt( "userid" ) ) == 28707326 ) //SizzlingCalamari's Acc ID 28707326
				{
					m_SizzlingMatches.SM_AllUserChatMessage( "Forcing Match Start\n" );
					m_SizzlingMatches.StartGame();
				}
			}
		}
		else if ( FStrEq( text, ".forcequit" ) )
		{
			if ( UserIDToSteamID( event->GetInt( "userid" ) ) == 28707326 ) //SizzlingCalamari's Acc ID 28707326
			{
				IGameEvent *event = gameeventmanager->CreateEvent("tf_game_over", true);
				event->SetString( "reason", "SizzMatch" );
				gameeventmanager->FireEvent(event);
				m_SizzlingMatches.SM_AllUserChatMessage( "Forcing Match End\n" );
				m_SizzlingMatches.SM_GameOver();
			}
		}
	}
	else if ( FStrEq( name, "player_team" ) ) // this gets called for some stupid reason after the player leaves and data is deleted
	{
		char team = event->GetInt( "team" );
		if ( team == 0 )
			return;
		char oldteam = event->GetInt( "oldteam" );
		m_SizzlingMatches.SM_Msg( "old team: %i, new team: %i\n", oldteam, team );
		m_SizzlingMatches.SM_PlayerChangeTeam( event->GetInt( "userid" ), team , oldteam );
	}
	else if ( FStrEq( name, "player_changename" ) )
	{
		const char *newname = event->GetString( "newname" );
		m_SizzlingMatches.SM_Msg( "old name: %s, new name: %s\n", event->GetString( "oldname" ), newname );
		m_SizzlingMatches.SM_PlayerChangeName( event->GetInt( "userid" ), newname );
	}
	else if ( FStrEq( name, "teamplay_broadcast_audio" ) )
	{
		Msg( "teamplay broadcast audio: %s\n", event->GetString( "sound" ) );
	}
}
