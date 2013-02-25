
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

#include "PlayerDataManager.h"

#include "eiface.h" // for IVEngineServer
#include "game/server/iplayerinfo.h" // for IPlayerInfoManager
#include "playerdata.h"
#include "strtools.h"
#include "steam/steamclientpublic.h" // for a log using accountid

#pragma warning( push )
#pragma warning( disable : 4351 )
CPlayerDataManager::CPlayerDataManager():
	m_pPlayerData(),
	m_pEntIndexToExtraData(),
	m_PlayerDataMemPool(MAX_PLAYERS),
	m_ExtraDataMemPool(MAX_PLAYERS),
	m_nPlayers(0)
{
}
#pragma warning( pop )

CPlayerDataManager::~CPlayerDataManager()
{
	m_PlayerDataMemPool.Clear();
	m_ExtraDataMemPool.Clear();
}

bool CPlayerDataManager::InsertPlayer( engineContext_t &context, edict_t *pEdict )
{
	IPlayerInfo *pPlayerInfo = context.pPlayerInfoManager->GetPlayerInfo(pEdict);
	if (pPlayerInfo)
	{
		if (!(pPlayerInfo->IsConnected()))
		{
			PD_Msg("error: player not yet connected, aborting insert\n");
		}
	}
	else
	{
		PD_Msg("error: could not get player info, aborting insert\n");
		return false;
	}
	
	const CSteamID *pSteamID = context.pEngine->GetClientSteamID(pEdict);
	if (!pSteamID)
	{
		PD_Msg("error: client %s not authenticated with steam, aborting insert\n", pPlayerInfo->GetName());
		return false;
	}
	
	int entIndex = context.pEngine->IndexOfEdict(pEdict);
	m_pPlayerData[entIndex] = m_PlayerDataMemPool.Alloc();
	m_pPlayerData[entIndex]->SetBaseData(pEdict, pPlayerInfo);
	m_pEntIndexToExtraData[entIndex] = m_ExtraDataMemPool.Alloc();
	m_nPlayers += 1;
	
	PD_Msg( "current players: %i\n", m_nPlayers );
	PD_Msg( "Stats for player #%i: '%s' will be tracked\n", pSteamID->GetAccountID(), pPlayerInfo->GetName() );
	
	return true;
}

void CPlayerDataManager::RemovePlayer( engineContext_t &context, edict_t *pEdict )
{
	const CSteamID *pSteamID = context.pEngine->GetClientSteamID(pEdict);
	if (!pSteamID)
	{
		// TODO: verify that this can happen
		PD_Msg("error: client not authenticated with steam, aborting delete\n");
		return;
	}
	
	int entIndex = context.pEngine->IndexOfEdict(pEdict);
	
	m_PlayerDataMemPool.Free(m_pPlayerData[entIndex]);
	m_pPlayerData[entIndex] = NULL;
	m_ExtraDataMemPool.Free(m_pEntIndexToExtraData[entIndex]);
	m_pEntIndexToExtraData[entIndex] = NULL;

	PD_Msg( "deleted data index #%i\n", entIndex );
	PD_Msg( "size before delete: %i\n", m_nPlayers );
	m_nPlayers -= 1;
	PD_Msg( "size after delete: %i\n", m_nPlayers );
}

void CPlayerDataManager::RemoveAllPlayers( engineContext_t &context )
{
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
}

playerAndExtra_t CPlayerDataManager::GetPlayerData( int entindex )
{
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

void CPlayerDataManager::ResetAndStartClassTracking(unsigned int playerClassOffset, uint64 curtime)
{
	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		if (m_pPlayerData[i])
		{
			SS_PlayerData *pData = m_pPlayerData[i];
			CPlayerClassTracker *pTracker = pData->GetClassTracker();
			EPlayerClass player_class = static_cast<EPlayerClass>(pData->GetClass(playerClassOffset));
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

