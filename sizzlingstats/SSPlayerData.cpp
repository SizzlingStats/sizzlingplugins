
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

#include "SSPlayerData.h"

#include "SC_helpers.h"

// ---------------------------------------------------------------------- //
// SS_PlayerData
// ---------------------------------------------------------------------- //
SS_PlayerData::SS_PlayerData():
	m_base_entity(nullptr),
	m_bCapFix(false)
{
}

void SS_PlayerData::Reset( CBaseEntity *pEnt )
{
	m_base_entity = pEnt;
	ResetRoundStatsData();
	m_classTracker.Reset();
	m_bCapFix = false;
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

void SS_PlayerData::UpdateRoundExtraData( const extradata_t &dat )
{
	m_RoundScoreData.data[HealsReceived] = dat.healsrecv;
	m_RoundScoreData.data[MedPicks] = dat.medpicks;
	m_RoundScoreData.data[UbersDropped] = dat.ubersdropped;
	m_RoundScoreData.data[OverkillDamage] = dat.overkillDamage;
}

int SS_PlayerData::GetStat( int StatID )
{
	return m_RoundScoreData.data[StatID];
}

CPlayerClassTracker *SS_PlayerData::GetClassTracker()
{
	return &m_classTracker;
}

int	SS_PlayerData::GetDataFromOffset( int PropName, const unsigned int pPropOffsets[] )
{
	return *SCHelpers::ByteOffsetFromPointer<int*>(m_base_entity, pPropOffsets[PropName]);
}
