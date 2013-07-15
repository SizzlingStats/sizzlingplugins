
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

typedef struct plugin_context_init_s
{
	IVEngineServer *pEngine;
	IPlayerInfoManager *pPlayerInfoManager;
	IServerPluginHelpers *pHelpers;
	IGameEventManager2 *pGameEventManager;
	IServerGameDLL *pServerGameDLL;
} plugin_context_init_t;

class CSizzPluginContext
{
	friend class CEmptyServerPlugin;
public:
	CSizzPluginContext();
	~CSizzPluginContext();

	void Initialize( const plugin_context_init_t &init );

	// returns -1 if ent index is invalid
	int UserIDFromEntIndex( int ent_index );

	// returns -1 if userid is invalid
	int EntIndexFromUserID( int userid ) const;

	// returns -1 on error
	int SteamIDFromUserID( int userid );

	// returns -1 on error
	int SteamIDFromEntIndex( int ent_index );

	// the current server tick
	int GetCurrentTick() const;

	// the current time (per frame incremented)
	float GetTime() const;

	// returns the index of the plugin attached 
	// to the passed in plugin callbacks
	int GetPluginIndex( const IServerPluginCallbacks *pPlugin ) const;

	// returns the index of the first plugin whos 
	// description string contains the passed 
	// in description string
	int GetPluginIndex( const char *pszDescriptionPart ) const;

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

protected:
	void ClientActive( const edict_t *pEdict );
	void ClientDisconnect( const edict_t *pEdict );
	void GameFrame( bool simulating );

private:
	IVEngineServer *m_pEngine;
	IPlayerInfoManager *m_pPlayerInfoManager;
	CServerPlugin *m_pPluginManager;
	IGameEventManager2 *m_pGameEventManager;
	IServerGameDLL *m_pServerGameDLL;
	CGlobalVars *m_pGlobals;

	CUserIDTracker *m_pUserIDTracker;
	CTSCallQueue *m_pCallQueue;
	int m_tickcount;
	float m_flTime;
};

#endif // SIZZ_PLUGIN_CONTEXT_H
