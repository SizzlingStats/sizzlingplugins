
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

////////////////////////////////////////////////////////////////////////////////
// Filename: SizzlingMatches.h
////////////////////////////////////////////////////////////////////////////////
#ifndef SIZZLING_MATCHES_H
#define SIZZLING_MATCHES_H


#include "engine/iserverplugin.h"
#include "game/server/iplayerinfo.h"
#include "eiface.h"
#include "igameevents.h"
#include "vstdlib/random.h"
#include "engine/IEngineTrace.h"

#include "tier1/utlmap.h"

#include "playerdata.h"
#include "PlayerMessage.h"
#include "SC_helpers.h"

#include "steam/steamclientpublic.h"

#include "timedeventmgr.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CSizzPluginContext;

// Interfaces from the engine
extern IVEngineServer			*pEngine; // helper functions (messaging clients, loading content, making entities, running commands, etc)
extern IGameEventManager2		*gameeventmanager; // game events interface
extern IPlayerInfoManager		*playerinfomanager; // game dll interface to interact with players

extern IServerPluginHelpers		*helpers; // special 3rd party plugin helpers from the engine
extern IUniformRandomStream		*randomStr;
extern IEngineTrace				*enginetrace;

extern IServerGameDLL			*pServerDLL;
extern IServerGameEnts			*pServerEnts;
extern CGlobalVars				*gpGlobals;

//---------------------------------------------------------------------------------
// Purpose: if non-zero, overrides the number of players ready to be able to start the game
//---------------------------------------------------------------------------------
static ConVar sizzmatch_start_override( "sizzmatch_start_override", "0", 0, "If non-zero, overrides the number of players needed to be ready to be able to start the game.", true, 0, true, 11);

class SizzlingMatches: public IEventRegisterCallback
{
public:
	SizzlingMatches(void);
	~SizzlingMatches(void);

	void	Load( CSizzPluginContext &context );

	//	call this after load to create data for current players if none exists
	void	SM_LoadCurrentPlayers();

	//	called on server plugin load to set the event timer
	void	SM_SetEventUpdateInterval( float interval );

	//	Sets all players to the not ready state
	void	SM_SetAllPlayersNotReady();

	//	resets the players ready and restarts the event timer for 3 seconds ( used after a match has finished )
	void	SM_ResetGame();

	//	insert and player and add them to the map
	bool	SM_InsertPlayer( edict_t *pEntity );

	//	delete a player and remove them from the map
	bool	SM_DeletePlayer( edict_t *pEntity );

	//	deletes all player data and empties the map
	bool	SM_DeleteAllPlayerData();

	//	console message with "[SM]: " prefix
	void	SM_Msg( const char *pMsg, ... );

	//	chat message to be sent to one user
	void	SM_SingleUserChatMessage( edict_t *pEntity, const char *pFormat, ... );

	//	chat message to be sent to all users
	void	SM_AllUserChatMessage( const char *pFormat, ... );

	//	called when the round starts
	void	SM_StartOfRound();

	//	called when the round ends
	void	SM_EndOfRound();

	//	gets the prop offest
	void	SM_GetPropOffsets();

	//	gets the entities used by this class
	void	SM_GetEntities();

	//	for use when the player_team event is caught
	void	SM_PlayerChangeTeam( int userid, int newteamid, int oldteamid );

	//	for use when the player_changename event is caught
	void	SM_PlayerChangeName( int userid, const char *szNewName );

	//	for use to ready and unready the player
	void	SM_PlayerChangeReadyState( int userid, bool state );

	//	gets the names lists and returns them as parameters for display
	void	SM_GetNamesLists( char RedTeamPlayers[], int RedArraySize, char BluTeamPlayers[], int BluArraySize );

	//	used by the timed event manager FireEvent to display the names every 3 seconds
	void	SM_DisplayNames( const char *RedTeamPlayers, const char *BluTeamPlayers );

	//	checks the player ready numbers and completes appropriate action
	void	UpdateMatchStartStatus();

	//	returns true if the match has started, false if the match has not
	bool	SM_IsStarted();

	//	called when the game ends
	void	SM_GameOver();

	// IEventRegister Callback
	virtual void	FireEvent();

	// Level Init
	void	LevelInit( char const *pMapName );

	// GameFrame
	void	GameFrame();

	// Level Shutdown
	void	LevelShutdown();

	//	Player disconnect
	void	PlayerDisconnect( edict_t *pEntity );

	void	StartGame();

private:
	CSizzPluginContext *m_plugin_context;
	CTimedEventMgr	m_TimedEventMgr;
	CEventRegister	m_EventRegister;

	unsigned int	m_aPropOffsets[20];
	unsigned int	m_PlayerFlagsOffset;
	unsigned int	m_TeamRoundsWonOffset;
	unsigned int	m_TeamIsReadyOffset;
	unsigned int	m_AwaitingReadyRestartOffset;

	bool			*m_bTeamReady;		// pointer to an array of bool
	bool			*m_bAwaitingReadyRestart;

	CBaseEntity		*m_pTeamplayRoundBasedRulesProxy;

	int				m_nPlayersReady;
	int				m_nPlayersOnTeams;

	bool			m_bMatchStarted;
	bool			m_bTimer5;
	int				m_nCountdown;
	int				m_n12sectimer;
	int				m_nCurrentPlayers;
	int				m_nCurrentRound;

	CUtlMap<unsigned int, SM_PlayerData *> m_SteamIDToPlayerDataMap;
};

#endif // SIZZLING_MATCHES_H
