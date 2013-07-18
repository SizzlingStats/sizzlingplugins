
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

////////////////////////////////////////////////////////////////////////////////
// Filename: PlayerDataManager.h
////////////////////////////////////////////////////////////////////////////////
#ifndef PLAYER_DATA_MANAGER_H
#define PLAYER_DATA_MANAGER_H

#include "mempool.h"
#include "PlayerClassTracker.h"
#include "utlhash.h"
#include "shareddefs.h"

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
class IPlayerInfo;

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
	
	bool InsertPlayer( int ent_index, CBaseEntity *pEnt );
	void RemovePlayer( int ent_index, IPlayerInfo *pPlayerInfo, unsigned int account_id );
	void RemoveArchivedPlayers();
	void RemoveAllPlayers();
	
	playerAndExtra_t GetPlayerData( int entindex );
	int GetNumPlayers() const;
	
	void ResetAndStartClassTracking( unsigned int playerClassOffset, uint64 curtime );
	void StopClassTracking( uint64 curtime );
	void PlayerChangedClass( int entindex, EPlayerClass player_class, uint64 curtime );

	void SetCapFix( int entindex );
	
private:
	void PD_Msg( const char *pMsg, ... );

private:
	SS_PlayerData *m_pPlayerData[MAX_PLAYERS];
	extradata_t *m_pEntIndexToExtraData[MAX_PLAYERS];
	CClassMemoryPool<SS_PlayerData> m_PlayerDataMemPool;
	CClassMemoryPool<extradata_t> m_ExtraDataMemPool;
	CUtlHashFast<playerAndExtra_t>	m_playerDataArchive;
	int m_nPlayers;
};

#endif //PLAYER_DATA_MANAGER_H

