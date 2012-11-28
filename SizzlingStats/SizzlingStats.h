////////////////////////////////////////////////////////////////////////////////
// Filename: SizzlingStats.h
////////////////////////////////////////////////////////////////////////////////
#ifndef SIZZLING_STATS_H
#define SIZZLING_STATS_H

#include "tier1/utlmap.h"
#include "tier1/utlvector.h"
#include "tier1/utlhash.h"
#include "mempool.h"

#include "WebStatsHandler.h"

#include "convar.h"

#include "PluginDefines.h"

#include "PlayerDataManager.h"

class CFuncQueueThread;

class SizzlingStats
{
public:
	SizzlingStats(void);
	~SizzlingStats(void);

	// called when the plugin is loaded
	void	Load();

	// called when the plugin is unloaded
	void	Unload();

	// called on level start
	void	LevelInit(const char *pMapName);

	void	GameFrame();
	
	void	PlayerHealed( int entindex, int amount );
	void	MedPick( int entindex );
	void	UberDropped( int entindex );
	
	// this needs to be called whenever the player switches classes or goes to spec
	// set player_class to k_ePlayerClassUndefined if switching to spec
	void	PlayerChangedClass( int entindex, EPlayerClass player_class );
	
	void	ChatEvent( int entindex, const char *pText, bool bTeamChat );

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

	void	SS_TournamentMatchStarted();

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

	void	SS_ShowHtmlStats( int entindex );

	//	gets the prop offest
	void	GetPropOffsets();

private:
	unsigned int	m_aPropOffsets[20];
	unsigned int	m_PlayerFlagsOffset;
	unsigned int	m_TeamRoundsWonOffset;
	unsigned int	m_PlayerClassOffset;
	uint32			m_iWeaponsOffset;
	uint32			m_iChargeLevelOffset;
	uint32			m_iOriginOffset;
	uint32			m_iChargeReleaseOffset;

	int				m_nCurrentRound;
	CUtlVector<char> m_vecMedics; //ent index of medics
	CUtlHashFast<playerAndExtra_t>	m_playerDataArchive;
	// the vector is for freeing all of the mempool memory in the archive when we destruct
	//CUtlVector<CUtlReference<playerAndExtra_t>>		m_playerDataArchiveVec;
private:
	CPlayerDataManager m_PlayerDataManager;
#ifdef PUBLIC_RELEASE
	CNullWebStatsHandler *m_pWebStatsHandler;
#else
	CWebStatsHandler *m_pWebStatsHandler;
#endif
	ConVarRef m_refHostname;
	ConVarRef m_refBlueTeamName;
	ConVarRef m_refRedTeamName;
	hostInfo_t m_hostInfo;
	double m_flRoundDuration;
	double m_flMatchDuration;
	bool m_bTournamentMatchRunning;
};

#endif // SIZZLING_STATS_H
