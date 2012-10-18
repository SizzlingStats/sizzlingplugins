////////////////////////////////////////////////////////////////////////////////
// Filename: SizzlingStats.h
////////////////////////////////////////////////////////////////////////////////
#ifndef SIZZLING_STATS_H
#define SIZZLING_STATS_H

#include "tier1/utlmap.h"
#include "tier1/utlvector.h"
#include "tier1/utlhash.h"
#include "mempool.h"
#include "utlobjectreference.h"

#include "WebStatsHandler.h"

#include "PluginDefines.h"

#include "NetPropUtils.h"

#define MAX_PLAYERS 34

//#include "PlayerDataManager.h"

class CWebStatsHandlerThread;
class SS_PlayerData;
struct extradata_s;
struct edict_t;

typedef extradata_s extradata_t;

struct playerAndExtra
{
public:
	SS_PlayerData	*m_pPlayerData;
	extradata_t		*m_pExtraData;
	
//private:
//	DECLARE_REFERENCED_CLASS(playerAndExtra);
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

	void	SetTeamplayRoundBasedGameRules( void *pGameRules )
	{
		m_pTeamplayRoundBasedGameRules = pGameRules;
	}

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

	void	SS_RoundStarted();

	void	SS_RoundEnded();

	//	displays the stats for use at the end of a round
	void	SS_DisplayStats( SS_PlayerData &PlayerData );

	//	print the player index
	void	SS_PrintIndex();

	//	fix for the cappers' score and caps from the win panel event
	void	SS_CheckFixEndOfRoundCappers( int capper );

	//	called when the round ends
	void	SS_EndOfRound();

	// resets the player data
	void	SS_ResetData();

	//	called when a user types ".ss_credits"
	void	SS_Credits( int entindex, const char *pszVersion );

	void	SS_TestPost();

	void	SetTeamScores( int redscore, int bluscore );

	void	TeamNameChange( int entindex, const char *teamname );

	friend void	SS_SendHttpPostData();

	void	SS_TestThreading();

	void	SS_UploadStats();

	void	SS_ShowHtmlStats( int entindex );

	void	SS_GameOver();

	//	gets the prop offest
	void	GetPropOffsets();

	float	SS_GetTimeOfLastCap() const
	{
		return m_flTimeOfLastCap;
	}

	void	SS_SetTimeOfLastCap( float time )
	{
		m_flTimeOfLastCap = time;
	}

	extradata_t **getEntIndexToExtraData()
	{
		return m_pEntIndexToExtraData;
	}

private:
	unsigned int	m_aPropOffsets[20];
	unsigned int	m_PlayerFlagsOffset;
	unsigned int	m_TeamRoundsWonOffset;
	unsigned int	m_PlayerClassOffset;
	unsigned int	m_iRoundStateOffset;
	unsigned int	m_bInWaitingForPlayersOffset;
	float			m_flTimeOfLastCap;

	int				m_nCurrentPlayers;
	int				m_nCurrentRound;

	CSendPropHook	m_iRoundStateHook;
	CSendPropHook	m_bInWaitingForPlayersHook;

	CUtlHashFast<playerAndExtra>	m_playerDataArchive;
	// the vector is for freeing all of the mempool memory in the archive when we destruct
	//CUtlVector<CUtlReference<playerAndExtra>>		m_playerDataArchiveVec;
private:
	CClassMemoryPool<SS_PlayerData> m_PlayerDataMemPool;
	CClassMemoryPool<extradata_t>	m_ExtraDataMemPool;
	SS_PlayerData	*m_pPlayerData[MAX_PLAYERS];
	// this is temp storage for faster access than going through the playerdata map to store everything
	extradata_t		*m_pEntIndexToExtraData[MAX_PLAYERS]; // 33 slot servers will break if this is only set to 33 indicies
	void *m_pTeamplayRoundBasedGameRules;
#ifndef PUBLIC_RELEASE
	CWebStatsHandlerThread	*m_pWebStatsThread;
#endif
	hostInfo_t m_hostInfo;
};

#endif // SIZZLING_STATS_H
