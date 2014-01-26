
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

#pragma once

#ifndef SIZZ_PLUGIN_CONTEXT_H
#define SIZZ_PLUGIN_CONTEXT_H

#include "engine/iserverplugin.h"
#include "Color.h"
#include "convar.h"

class IVEngineServer;
class CGlobalVars;
class CUserIDTracker;
struct edict_t;
class CServerPlugin;
class IServerPluginHelpers;
class IGameEventManager2;
class IPlayerInfoManager;
class IServerPluginCallbacks;
class IGameEventListener2;
class IPlayerInfo;
class CTSCallQueue;
class CFunctor;
class ServerClass;
class IServerGameDLL;
class KeyValues;
class IRecipientFilter;
class CGameEventManager;

typedef struct plugin_context_init_s
{
	IVEngineServer *pEngine;
	IPlayerInfoManager *pPlayerInfoManager;
	IServerPluginHelpers *pHelpers;
	IGameEventManager2 *pGameEventManager;
	IServerGameDLL *pServerGameDLL;
} plugin_context_init_t;

typedef struct hud_msg_cfg_s
{
	hud_msg_cfg_s():
		rgba(Color(0,0,0,255)), screentime(20.0f),
		x(-1), y(-1), channel(1)
	{
	}

	Color rgba;
	float screentime;
	float x;
	float y;
	int channel;
} hud_msg_cfg_t;

enum MOTDPANE_TYPE
{
	// Treat msg as plain text
	MOTDPANEL_TYPE_TEXT	= 0,

	//Msg is auto determined by the engine
	MOTDPANEL_TYPE_INDEX = 1,

	//Treat msg as an URL link
	MOTDPANEL_TYPE_URL = 2,

	// Treat msg as a filename to be openned (on the client)
	MOTDPANEL_TYPE_FILE = 3
};

typedef struct motd_msg_cfg_s
{
	motd_msg_cfg_s(): type(MOTDPANEL_TYPE_INDEX),
		visible(true), large_window(true)
	{
	}

	MOTDPANE_TYPE type;
	bool visible;
	bool large_window;
} motd_msg_cfg_t;

class CSizzPluginContext
{
	friend class CEmptyServerPlugin;
public:
	CSizzPluginContext();
	~CSizzPluginContext();

	bool Initialize( const plugin_context_init_t &init );

	// get a pointer to the engine interface
	IVEngineServer *GetEngine() const;

	// get a pointer to the game events manager interface
	IGameEventManager2 *GetGameEventManager() const;

	// get a pointer to the server game dll interface
	IServerGameDLL *GetServerGameDLL() const;

	// returns -1 if ent index is invalid
	int UserIDFromEntIndex( int ent_index );

	// returns -1 if userid is invalid
	int EntIndexFromUserID( int userid ) const;

	// returns max int (-1) on error
	unsigned int SteamIDFromUserID( int userid );

	// returns max int (-1) on error
	unsigned int SteamIDFromEntIndex( int ent_index );

	// writes the steam id string to the buffer at dest
	// for the passed in userid
	void GetSteamIDString( int userid, char *dest, int buff_size );

	// the current server tick
	int GetCurrentTick() const;

	// the current time (per frame incremented)
	float GetTime() const;

	// the max number of clients
	int GetMaxClients() const;

	// returns the name of the server
	// from the hostname cvar
	const char *GetHostName() const;

	// returns the name of the current running map
	const char *GetMapName() const;

	// returns the team name strings
	const char *GetRedTeamName() const;
	const char *GetBluTeamName() const;

	// returns the index of the plugin attached 
	// to the passed in plugin callbacks
	int GetPluginIndex( const IServerPluginCallbacks *pPlugin ) const;

	// returns the index of the first plugin whos 
	// description string contains the passed 
	// in description string
	int GetPluginIndex( const char *pszDescriptionPart ) const;

	// creates and sends a message of type 
	// DIALOG_TYPE to the specified client
	void CreateMessage( int ent_index, DIALOG_TYPE type, KeyValues *data, IServerPluginCallbacks *plugin );

	// Issue a command to the command parser as if it was typed at the server console.
	void ServerCommand( const char *command );

	// Execute any commands currently in the command parser immediately (instead of once per frame)
	void ServerExecute();

	// Print a message to the server log file
	void LogPrint( const char *msg );

	// Is the game paused?
	bool IsPaused();

	// adds a listener for a particular event
	bool AddListener( IGameEventListener2 *listener, const char *name, bool bServerSide );

	// adds a listener for all events
	bool AddListenerAll( IGameEventListener2 *listener, bool bServerSide );
	
	// removes a listener
	void RemoveListener( IGameEventListener2 *listener );

	// returns the player info pointer for the 
	// given player ent index
	IPlayerInfo *GetPlayerInfo( int ent_index );

	// adds a functor to the call queue to be executed 
	// on a future game frame (thread safe with the game logic)
	void EnqueueGameFrameFunctor( CFunctor *pFunctor );

	// returns a pointer to the server class list
	ServerClass *GetAllServerClasses();

	// send a chat message to the recipients in the filter
	void ChatMessage( IRecipientFilter *pFilter, const char *format, ... );
	void ChatMessageArg( IRecipientFilter *pFilter, const char *format, va_list args );

	// sends a hud reset message to the recipients in the filter
	// used for clearing hud messages
	void HudResetMessage( IRecipientFilter *pFilter );

	// sends a hud message to the recipients in the filter
	void HudMessage( IRecipientFilter *pFilter, const hud_msg_cfg_t &cfg, const char *format, ... );
	void HudMessageArg( IRecipientFilter *pFilter, const hud_msg_cfg_t &cfg, const char *format, va_list args );

	// sends a hud hint message to the recipients in the filter
	void HudHintMessage( IRecipientFilter *pFilter, const char *format, ... );
	void HudHintMessageArg( IRecipientFilter *pFilter, const char *format, va_list args );

	// sends a motd pane message to the recipients in the filter
	void MOTDPanelMessage( IRecipientFilter *pFilter, const char *msg, const motd_msg_cfg_t &cfg );

	// returns the base entity of the base handle
	// returns null on error
	CBaseEntity *BaseEntityFromBaseHandle( const CBaseHandle *pHandle );

	// returns the base entity of the ent index
	// returns null on error
	CBaseEntity *BaseEntityFromEntIndex( int ent_index );

	// returns the IHandleEntity pointer from the given ent index
	IHandleEntity *HandleEntityFromEntIndex( int ent_index );

	// finds the entities with the class name that matches 'name' and 
	// puts them in the 'out' param to a maximum of max_out.
	// returns the number of entities written to 'out'
	int GetEntityByClassName( const char *name, edict_t *out[], int max_out );

	// returns the first entity that matches the passed in name.
	// the starting entity to search from can be specified with start_ent.
	// returns null on error
	edict_t *GetEntityByClassName( const char *name, int start_ent = 0 );

	// returns the IP address and port in string form, 
	// of the player with the passed in ent index.
	const char *GetPlayerIPPortString( int ent_index );

	// returns true if the server is set up and ready 
	// to record a demo.
	bool CanRecordDemo();

protected:
	void LevelShutdown();

	// returns the ent index of the client
	// returns -1 on error
	int ClientActive( const edict_t *pEdict );

	void ClientDisconnect( const edict_t *pEdict );

	void ServerActivate( edict_t *pEdictList, int edictCount, int clientMax );

	void GameFrame( bool simulating );
	
	// returns the corresponding edict_t pointer 
	// to the passed in ent index
	// returns nullptr on error
	edict_t *EdictFromEntIndex( int ent_index );

private:
	IVEngineServer *m_pEngine;
	IPlayerInfoManager *m_pPlayerInfoManager;
	CServerPlugin *m_pPluginManager;
	CGameEventManager *m_pGameEventManager;
	IServerGameDLL *m_pServerGameDLL;
	CGlobalVars *m_pGlobals;

	CUserIDTracker *m_pUserIDTracker;
	CTSCallQueue *m_pCallQueue;
	int m_tickcount;
	float m_flTime;
	int m_max_clients;

	edict_t *m_edict_list;

	ConVarRef m_refHostname;
	ConVarRef m_refBlueTeamName;
	ConVarRef m_refRedTeamName;
};

#endif // SIZZ_PLUGIN_CONTEXT_H
