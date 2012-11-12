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

#define MAX_PLAYERS 34

class CFuncQueueThread;
class SS_PlayerData;
struct extradata_s;
struct edict_t;

typedef extradata_s extradata_t;

struct playerAndExtra
{
public:
	SS_PlayerData	*m_pPlayerData;
	extradata_t		*m_pExtraData;
};

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

	// insert and player and add them to the map
	bool	SS_InsertPlayer( edict_t *pEdict );

	// delete a player and remove them from the map
	bool	SS_DeletePlayer( edict_t *pEdict );

	// deletes all player data and empties the map
	bool	SS_DeleteAllPlayerData();

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

	void	TeamNameChange( int entindex, const char *teamname );

	void	SS_TestThreading();

	void	SS_UploadStats();

	void	SS_ShowHtmlStats( int entindex );

	//	gets the prop offest
	void	GetPropOffsets();

	extradata_t **getEntIndexToExtraData()
	{
		return m_pEntIndexToExtraData;
	}

private:
	unsigned int	m_aPropOffsets[20];
	unsigned int	m_PlayerFlagsOffset;
	unsigned int	m_TeamRoundsWonOffset;
	unsigned int	m_PlayerClassOffset;

	int				m_nCurrentPlayers;
	int				m_nCurrentRound;

	CUtlHashFast<playerAndExtra>	m_playerDataArchive;
	// the vector is for freeing all of the mempool memory in the archive when we destruct
	//CUtlVector<CUtlReference<playerAndExtra>>		m_playerDataArchiveVec;
private:
	CClassMemoryPool<SS_PlayerData> m_PlayerDataMemPool;
	CClassMemoryPool<extradata_t>	m_ExtraDataMemPool;
	SS_PlayerData	*m_pPlayerData[MAX_PLAYERS];
	// this is temp storage for faster access than going through the playerdata map to store everything
	extradata_t		*m_pEntIndexToExtraData[MAX_PLAYERS]; // 33 slot servers will break if this is only set to 33 indicies
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
	bool m_bShowStats;
	bool m_bTournamentMatchRunning;
};

#endif // SIZZLING_STATS_H
