
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

#include "PlayerDataManager.h"

struct edict_t;
class CGlobalVars;
#include "game/server/iplayerinfo.h" // for IPlayerInfo
#include "SSPlayerData.h"
#include "strtools.h"
#include "const.h"
#include "SC_helpers.h"
#include "SizzPluginContext.h"
#include "TFPlayerWrapper.h"
#include "shareddefs.h"

CPlayerDataManager::CPlayerDataManager()
{
	m_pPlayerData = new SS_PlayerData[MAX_PLAYERS];
	m_pEntIndexToExtraData = new extradata_t[MAX_PLAYERS];
}

CPlayerDataManager::~CPlayerDataManager()
{
	delete [] m_pEntIndexToExtraData;
	delete [] m_pPlayerData;
}

void CPlayerDataManager::InsertPlayer( int ent_index, CBaseEntity *pEnt )
{
	// normalize with a -1 for array indexing
	--ent_index;

	// reset the entity pointers
	m_pPlayerData[ent_index].Reset(pEnt);
}

void CPlayerDataManager::RemovePlayer( int ent_index )
{
	// normalize with -1 for array indexing
	--ent_index;

	m_pPlayerData[ent_index].Reset(nullptr);
}

void CPlayerDataManager::RemoveAllPlayers()
{
	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		m_pPlayerData[i].Reset(nullptr);
	}
}

playerAndExtra_t CPlayerDataManager::GetPlayerData( int ent_index )
{
	// normalize with -1 for array indexing
	--ent_index;

	playerAndExtra_t temp = {&m_pPlayerData[ent_index], &m_pEntIndexToExtraData[ent_index]};
	return temp;
}

bool CPlayerDataManager::IsValidPlayer( int ent_index )
{
	// normalize with -1 for array indexing
	--ent_index;

	return m_pPlayerData[ent_index].GetBaseEntity() != nullptr;
}

void CPlayerDataManager::SetCapFix( int ent_index )
{
	// normalize with a -1 for array indexing
	--ent_index;

	if (m_pPlayerData[ent_index].GetBaseEntity())
	{
		m_pPlayerData[ent_index].TriggerCapFix();
	}
}

void CPlayerDataManager::ResetAndStartClassTracking( uint64 curtime )
{
	CTFPlayerWrapper player;
	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		CBaseEntity *pEnt = m_pPlayerData[i].GetBaseEntity();
		if (pEnt)
		{
			player.SetPlayer(pEnt);
			CPlayerClassTracker *pTracker = m_pPlayerData[i].GetClassTracker();
			EPlayerClass player_class = static_cast<EPlayerClass>(player.GetClass());
			pTracker->StartRecording(player_class, curtime);
		}
	}
}

void CPlayerDataManager::StopClassTracking( uint64 curtime )
{
	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		if (m_pPlayerData[i].GetBaseEntity())
		{
			CPlayerClassTracker *pTracker = m_pPlayerData[i].GetClassTracker();
			pTracker->StopRecording(curtime);
		}
	}
}

void CPlayerDataManager::PlayerChangedClass( int ent_index, EPlayerClass player_class, uint64 curtime )
{
	// normalize with a -1 for array indexing
	--ent_index;

	if (m_pPlayerData[ent_index].GetBaseEntity())
	{
		CPlayerClassTracker *pTracker = m_pPlayerData[ent_index].GetClassTracker();
		pTracker->PlayerChangedClass(player_class, curtime);
	}
}
