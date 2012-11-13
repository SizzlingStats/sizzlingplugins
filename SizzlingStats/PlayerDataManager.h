////////////////////////////////////////////////////////////////////////////////
// Filename: PlayerDataManager.h
////////////////////////////////////////////////////////////////////////////////
#ifndef PLAYER_DATA_MANAGER_H
#define PLAYER_DATA_MANAGER_H

#include "mempool.h"
#include "PlayerClassTracker.h"

#define MAX_PLAYERS 34

class IPlayerInfoManager;
class IVEngineServer;

typedef struct engineContext_s
{
	IPlayerInfoManager	*pPlayerInfoManager;
	IVEngineServer		*pEngine;
} engineContext_t;

class SS_PlayerData;
struct edict_t;
struct extradata_s;
typedef struct extradata_s extradata_t;

typedef struct playerAndExtra_s
{
	SS_PlayerData	*m_pPlayerData;
	extradata_t		*m_pExtraData;
} playerAndExtra_t;

class CPlayerDataManager
{
public:
	CPlayerDataManager();
	~CPlayerDataManager();
	
	bool InsertPlayer( engineContext_t &context, edict_t *pEdict );
	void RemovePlayer( engineContext_t &context, edict_t *pEdict );
	void RemoveAllPlayers( engineContext_t &context );
	
	playerAndExtra_t GetPlayerData( int entindex );
	int GetNumPlayers() const;
	
	void ResetAndStartClassTracking( unsigned int playerClassOffset, double curtime );
	void StopClassTracking( double curtime );
	void PlayerChangedClass( int entindex, EPlayerClass player_class, double curtime );
	
private:
	void PD_Msg( const char *pMsg, ... );

private:
	SS_PlayerData *m_pPlayerData[MAX_PLAYERS];
	extradata_t *m_pEntIndexToExtraData[MAX_PLAYERS];
	CClassMemoryPool<SS_PlayerData> m_PlayerDataMemPool;
	CClassMemoryPool<extradata_t> m_ExtraDataMemPool;
	int m_nPlayers;
};

#endif //PLAYER_DATA_MANAGER_H

