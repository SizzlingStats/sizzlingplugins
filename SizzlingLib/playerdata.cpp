
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

////////////////////////////////////////////////////////////////////////////////
// Filename: playerdata.cpp
////////////////////////////////////////////////////////////////////////////////
#include "playerdata.h"
#include "eiface.h"
#include "SC_helpers.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Interfaces from the engine
extern IVEngineServer			*pEngine; // helper functions (messaging clients, loading content, making entities, running commands, etc)
extern IServerGameEnts			*pServerEnts;

// ---------------------------------------------------------------------- //
// BasePlayerData
// ---------------------------------------------------------------------- //
BasePlayerData::BasePlayerData():
	m_pPlayerEnt(NULL),
	m_pPlayerInfo(NULL),
	m_pEdict(NULL),
	m_iEntIndex(-1)
{
}

BasePlayerData::BasePlayerData( edict_t *pEdict, IPlayerInfo *pInfo ):
	m_pPlayerEnt( pServerEnts->EdictToBaseEntity( pEdict ) ),
	m_pPlayerInfo( pInfo ),
	m_pEdict( pEdict ),
	m_iEntIndex( pEngine->IndexOfEdict( pEdict ) )
{
}

BasePlayerData::~BasePlayerData()
{
}

void BasePlayerData::SetBaseData( edict_t *pEdict, IPlayerInfo *pInfo )
{
	m_pPlayerEnt = pServerEnts->EdictToBaseEntity(pEdict);
	m_pPlayerInfo = pInfo;
	m_pEdict = pEdict;
	m_iEntIndex = pEngine->IndexOfEdict(pEdict);
}

// ---------------------------------------------------------------------- //
// SS_PlayerData
// ---------------------------------------------------------------------- //
SS_PlayerData::SS_PlayerData():
	m_base_entity(nullptr),
	m_bCapFix(false)
{
}

void SS_PlayerData::UpdateRoundStatsData( const unsigned int pPropOffsets[] )
{
	for (int i = 0; i < HealsReceived; ++i)
	{
		m_RoundScoreData.data[i] = GetDataFromOffset(i, pPropOffsets);
	}

	if (m_bCapFix)
	{
		m_RoundScoreData.data[Captures] += 1;
		m_RoundScoreData.data[Points] += 2;
		m_bCapFix = false;
	}
}

void SS_PlayerData::ResetRoundStatsData()
{
	m_RoundScoreData.Reset();
}

ScoreData SS_PlayerData::GetRoundStatsData()
{
	return m_RoundScoreData;
}

void SS_PlayerData::UpdateRoundExtraData( extradata_t &dat )
{
	m_RoundScoreData.data[HealsReceived] = dat.healsrecv;
	m_RoundScoreData.data[MedPicks] = dat.medpicks;
	m_RoundScoreData.data[UbersDropped] = dat.ubersdropped;
}

int SS_PlayerData::GetStat( int StatID )
{
	return m_RoundScoreData.data[StatID];
}

CPlayerClassTracker *SS_PlayerData::GetClassTracker()
{
	return &m_classTracker;
}

int	SS_PlayerData::GetClass(unsigned int playerClassOffset)
{
	return *SCHelpers::ByteOffsetFromPointer<int>(m_base_entity, playerClassOffset);
}

int	SS_PlayerData::GetDataFromOffset( int PropName, const unsigned int pPropOffsets[] )
{
	return *SCHelpers::ByteOffsetFromPointer<int>(m_base_entity, pPropOffsets[PropName]);
}

// ---------------------------------------------------------------------- //
// SM_PlayerData
// ---------------------------------------------------------------------- //
SM_PlayerData::SM_PlayerData(): BasePlayerData()
{
}

SM_PlayerData::SM_PlayerData( edict_t *pEdict, IPlayerInfo *pInfo ): BasePlayerData( pEdict, pInfo ), m_bIsReady(false)
{
}

SM_PlayerData::~SM_PlayerData()
{
}

void SM_PlayerData::UpdateName( const char *szNewName )
{
	strcpy( m_szName, szNewName );
}

void SM_PlayerData::SetReadyState( bool state )
{
	m_bIsReady = state;
}

void SM_PlayerData::SetTeam( int teamindex )
{
	m_iTeam = teamindex;
}

int SM_PlayerData::GetTeam()
{
	return m_iTeam;
}

const char *SM_PlayerData::GetName()
{
	return m_szName;
}

bool SM_PlayerData::GetReadyState()
{
	return m_bIsReady;
}
