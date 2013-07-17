
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

////////////////////////////////////////////////////////////////////////////////
// Filename: SizzlingStats.h
////////////////////////////////////////////////////////////////////////////////
#ifndef SIZZLING_STATS_H
#define SIZZLING_STATS_H

#include "WebStatsHandler.h"
#include "convar.h"
#include "PluginDefines.h"
#include "PlayerDataManager.h"
#include "sizzstring.h"

#include "tier1/utlvector.h"

class CFuncQueueThread;
class CSizzPluginContext;

class SizzlingStats
{
public:
	SizzlingStats(void);
	~SizzlingStats(void);

	// called when the plugin is loaded
	void	Load( CSizzPluginContext *context );

	// called when the plugin is unloaded
	void	Unload();

	// called on level start
	void	LevelInit(const char *pMapName);

	void	ServerActivate();

	void	GameFrame();

	void	LoadConfig();
	
	void	PlayerHealed( int entindex, int amount );
	void	MedPick( int entindex );
	void	UberDropped( int entindex );
	
	// this needs to be called whenever the player switches classes or goes to spec
	// set player_class to k_ePlayerClassUndefined if switching to spec
	void	PlayerChangedClass( int entindex, EPlayerClass player_class );
	
	void	ChatEvent( int entindex, const char *pText, bool bTeamChat );

	void	TeamCapped( int team_index );

	void	GiveUber( int entindex );

	// checks to see if any of the medics with uber on the same team 
	// as the player that died dropped him
	void	CheckPlayerDropped( int victimIndex );

	void	CapFix( const char *cappers, int length );

	// insert and player and add them to the map
	bool	SS_InsertPlayer( edict_t *pEdict );

	// delete a player and remove them from the map
	void	SS_DeletePlayer( edict_t *pEdict );

	// deletes all player data and empties the map
	void	SS_DeleteAllPlayerData();

	//	console message with "[SS]: " prefix
	void	SS_Msg( const char *pMsg, ... );

	//	chat message to be sent to one user
	void	SS_SingleUserChatMessage( edict_t *pEntity, const char *szMessage );

	//	chat message to be sent to all users
	void	SS_AllUserChatMessage( const char *szMessage );

	void	SS_TournamentMatchStarted( const char *RESTRICT hostname, 
										const char *RESTRICT mapname, 
										const char *RESTRICT bluname, 
										const char *RESTRICT redname );

	void	SS_TournamentMatchEnded();

	void	SS_PreRoundFreeze();

	void	SS_RoundStarted();

	void	SS_RoundEnded();

	//	displays the stats for use at the end of a round
	void	SS_DisplayStats( SS_PlayerData &PlayerData );

	//	called when the round ends
	void	SS_EndOfRound();

	// resets the player data
	void	SS_ResetData();

	//	called when a user types ".ss_credits"
	void	SS_Credits( int entindex, const char *pszVersion );

	void	SetTeamScores( int redscore, int bluscore );

	void	SS_TestThreading();

	void	SS_UploadStats();

	void	SS_ShowHtmlStats( int entindex, bool reload_page );

	void	SS_HideHtmlStats( int endindex );

private:
	void	OnSessionIdReceived( sizz::CString sessionid );
	void	LogSessionId( const sizz::CString &str );

	void	OnMatchUrlReceived( sizz::CString matchurl );
	void	CacheSiteOnPlayer( const sizz::CString &match_url );

	//	gets the prop offests
	void	GetPropOffsets();

	void	GetEntities();

private:
	CSizzPluginContext *m_plugin_context;
	unsigned int	m_aPropOffsets[20];
	unsigned int	m_PlayerFlagsOffset;
	unsigned int	m_TeamRoundsWonOffset;
	unsigned int	m_PlayerClassOffset;
	uint32			m_iWeaponsOffset;
	uint32			m_iChargeLevelOffset;
	uint32			m_iOriginOffset;
	uint32			m_iChargeReleaseOffset;

	CBaseEntity		*m_pRedTeam;
	CBaseEntity		*m_pBluTeam;
	uint32			m_iTeamScoreOffset;
	uint32			m_iTeamNumOffset;
	uint16			m_iOldRedScore;
	uint16			m_iOldBluScore;

	int				m_nCurrentRound;
	CUtlVector<char> m_vecMedics; //ent index of medics
private:
	CPlayerDataManager m_PlayerDataManager;
	CWebStatsHandler *m_pWebStatsHandler;
	ConVarRef m_refHostIP;
	ConVarRef m_refIP;
	ConVarRef m_refHostPort;
	hostInfo_t m_hostInfo;
	double m_flRoundDuration;
	double m_flMatchDuration;
	bool m_bTournamentMatchRunning;
	bool m_bFirstCapOfRound;
};

#endif // SIZZLING_STATS_H
