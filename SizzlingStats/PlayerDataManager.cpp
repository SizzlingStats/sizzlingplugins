
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

#include "PlayerDataManager.h"

#include "game/server/iplayerinfo.h" // for IPlayerInfo
#include "SSPlayerData.h"
#include "strtools.h"
#include "const.h"
#include "SC_helpers.h"
#include "SizzPluginContext.h"
#include "TFPlayerWrapper.h"

#pragma warning( push )
#pragma warning( disable : 4351 )
CPlayerDataManager::CPlayerDataManager():
	m_pPlayerData(),
	m_pEntIndexToExtraData(),
	m_PlayerDataMemPool(MAX_PLAYERS),
	m_ExtraDataMemPool(MAX_PLAYERS),
	m_nPlayers(0)
{
	// sets the buckets to 64
	// HAS TO BE A POWER OF 2
	m_playerDataArchive.Init(64);
}
#pragma warning( pop )

CPlayerDataManager::~CPlayerDataManager()
{
	m_PlayerDataMemPool.Clear();
	m_ExtraDataMemPool.Clear();
}

bool CPlayerDataManager::InsertPlayer( int ent_index, CBaseEntity *pEnt )
{
	// normalize with a -1 for array indexing
	--ent_index;

	//uint32 account_id = pSteamID->GetAccountID();
	UtlHashFastHandle_t hHash = m_playerDataArchive.InvalidHandle(); //m_playerDataArchive.Find(account_id);

	// if the player is not in the hash map, ie. not archived
	if (hHash == m_playerDataArchive.InvalidHandle())
	{
		// allocate new memory for the player
		m_pPlayerData[ent_index] = m_PlayerDataMemPool.Alloc();
		m_pEntIndexToExtraData[ent_index] = m_ExtraDataMemPool.Alloc();
	}/*
	else // we have previous data of the player, load it
	{
		// reuse the old memory and 
		// remove the player from the hash map
		playerAndExtra_t &data = m_playerDataArchive.Element(hHash);
		m_pPlayerData[ent_index] = data.m_pPlayerData;
		m_pEntIndexToExtraData[ent_index] = data.m_pExtraData;
		m_playerDataArchive.Remove(hHash);
	}*/

	// reset the entity pointers
	m_pPlayerData[ent_index]->SetBaseEntity(pEnt);
	m_nPlayers += 1;
	
	PD_Msg( "current players: %i\n", m_nPlayers );
	return true;
}

void CPlayerDataManager::RemovePlayer( int ent_index, IPlayerInfo *pPlayerInfo, unsigned int account_id )
{
	// normalize with -1 for array indexing
	--ent_index;

	// we don't want to archive these types of players
	/*
	if (!pPlayerInfo->IsFakeClient() && !pPlayerInfo->IsHLTV() && !pPlayerInfo->IsReplay())
	{
		// archives the data for players who 
		// might rejoin before the round/match is over
		playerAndExtra_t data = {m_pPlayerData[ent_index], m_pEntIndexToExtraData[ent_index]};
		// fastinsert doesn't check for duplicates
		m_playerDataArchive.FastInsert(account_id, data);
	}*/

	// clear the array pointers
	m_pPlayerData[ent_index] = NULL;
	m_pEntIndexToExtraData[ent_index] = NULL;

	PD_Msg( "deleted data index #%i\n", ent_index );
	PD_Msg( "size before delete: %i\n", m_nPlayers );
	m_nPlayers -= 1;
	PD_Msg( "size after delete: %i\n", m_nPlayers );
}

void CPlayerDataManager::RemoveArchivedPlayers()
{
	/*
	// clean up the archived memory saved in the hash table
	FOR_EACH_LL( m_playerDataArchive.m_aDataPool, it )
	{
		playerAndExtra_t &data = m_playerDataArchive.m_aDataPool.Element(it).m_Data;
		m_PlayerDataMemPool.Free(data.m_pPlayerData);
		m_ExtraDataMemPool.Free(data.m_pExtraData);
	}
	m_playerDataArchive.RemoveAll();
	*/
}

void CPlayerDataManager::RemoveAllPlayers()
{
	// clean up the memory allocated in the arrays
	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		if (m_pPlayerData[i])
		{
			m_PlayerDataMemPool.Free(m_pPlayerData[i]);
			m_pPlayerData[i] = NULL;
			m_ExtraDataMemPool.Free(m_pEntIndexToExtraData[i]);
			m_pEntIndexToExtraData[i] = NULL;
		
			PD_Msg( "deleted data index #%i\n", i );
			PD_Msg( "size before delete: %i\n", m_nPlayers );
			m_nPlayers -= 1;
			PD_Msg( "size after delete: %i\n", m_nPlayers );
		}
	}

	// remove the rest of the player data
	RemoveArchivedPlayers();
}

playerAndExtra_t CPlayerDataManager::GetPlayerData( int entindex )
{
	// normalize with -1 for array indexing
	entindex--;
	if (entindex >= 0 && entindex < MAX_PLAYERS)
	{
		playerAndExtra_t temp = {m_pPlayerData[entindex], m_pEntIndexToExtraData[entindex]};
		return temp;
	}
	return playerAndExtra_t();
}

int CPlayerDataManager::GetNumPlayers() const
{
	return m_nPlayers;
}

void CPlayerDataManager::SetCapFix( int entindex )
{
	playerAndExtra_t data = GetPlayerData(entindex);
	if (data.m_pPlayerData)
	{
		data.m_pPlayerData->TriggerCapFix();
	}
}

void CPlayerDataManager::PD_Msg( const char *pMsg, ... )
{
	va_list argList;
	va_start( argList, pMsg );
	char message[96] = {};

	V_vsnprintf( message, 96, pMsg, argList );
	Msg( "[SS]: %s", message );

	va_end( argList );
}

void CPlayerDataManager::ResetAndStartClassTracking(uint64 curtime)
{
	CTFPlayerWrapper player;
	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		if (m_pPlayerData[i])
		{
			SS_PlayerData *pData = m_pPlayerData[i];
			player.SetPlayer(pData->GetBaseEntity());
			CPlayerClassTracker *pTracker = pData->GetClassTracker();
			EPlayerClass player_class = static_cast<EPlayerClass>(player.GetClass());
			pTracker->StartRecording(player_class, curtime);
		}
	}
}

void CPlayerDataManager::StopClassTracking( uint64 curtime )
{
	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		if (m_pPlayerData[i])
		{
			SS_PlayerData *pData = m_pPlayerData[i];
			CPlayerClassTracker *pTracker = pData->GetClassTracker();
			pTracker->StopRecording(curtime);
		}
	}
}

void CPlayerDataManager::PlayerChangedClass( int entindex, EPlayerClass player_class, uint64 curtime )
{
	SS_PlayerData *pData = GetPlayerData(entindex).m_pPlayerData;
	if (pData)
	{
		CPlayerClassTracker *pTracker = pData->GetClassTracker();
		pTracker->PlayerChangedClass(player_class, curtime);
	}
}

