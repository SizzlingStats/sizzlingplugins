
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

#include "PlayerClassTracker.h"

class SS_PlayerData;
struct extradata_s;
typedef struct extradata_s extradata_t;
class CBaseEntity;

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
	
	// don't even try to pass in an invalid index.
	void InsertPlayer( int ent_index, CBaseEntity *pEnt );
	void RemovePlayer( int ent_index );
	void RemoveAllPlayers();
	
	playerAndExtra_t GetPlayerData( int ent_index );
	bool IsValidPlayer( int ent_index );
	
	void ResetAndStartClassTracking( uint64 curtime );
	void StopClassTracking( uint64 curtime );
	void PlayerChangedClass( int ent_index, EPlayerClass player_class, uint64 curtime );

	void SetCapFix( int ent_index );

private:
	SS_PlayerData *m_pPlayerData;
	extradata_t *m_pEntIndexToExtraData;
};

#endif //PLAYER_DATA_MANAGER_H

