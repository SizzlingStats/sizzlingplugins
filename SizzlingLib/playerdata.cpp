////////////////////////////////////////////////////////////////////////////////
// Filename: playerdata.cpp
////////////////////////////////////////////////////////////////////////////////
#include "playerdata.h"
#include "eiface.h"

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
	m_BasePlayerData()
{
}

SS_PlayerData::SS_PlayerData( edict_t *pEdict, IPlayerInfo *pInfo ):
	m_BasePlayerData( pEdict, pInfo )
{
}

SS_PlayerData::~SS_PlayerData()
{
}

void SS_PlayerData::UpdateRoundData( int CurrentRound, const unsigned int pPropOffsets[] )
{
	ScoreData scoreData;
	for (int i = 0; i < 20; i++)
	{
		scoreData.data[i] = GetDataFromOffset(i, pPropOffsets);
	}

	if ( !m_aRoundScoreData.IsValidIndex( CurrentRound ) )
	{
		m_aRoundScoreData.AddToTail( scoreData );
	}
	else
	{
		m_aRoundScoreData.Element( CurrentRound ) = scoreData;
	}
}

void SS_PlayerData::UpdateTotalData( int CurrentRound )
{
	m_aTotalScoreData += m_aRoundScoreData[CurrentRound];
	//for (int i = 0; i < 20; i++ )
	//	m_aTotalScoreData += m_aRoundScoreData.Element(CurrentRound);
}

void SS_PlayerData::ResetTotalData()
{
	m_aTotalScoreData.Reset();
}

ScoreData SS_PlayerData::GetTotalData()
{
	return m_aTotalScoreData;
}

void SS_PlayerData::UpdateExtraData( int CurrentRound, extradata_t &dat )
{
	int *data = m_aRoundScoreData.Element( CurrentRound ).data;
	data[HealsReceived] = dat.healsrecv;
	data[MedPicks] = dat.medpicks;
	data[UbersDropped] = dat.ubersdropped;
	//data[SumTimeCharging] = dat.sumofubertimes;
}

int SS_PlayerData::GetStat( int RoundNumber, int StatID )
{
	if ( StatID >= NumOfStats )
		return -1;
	return (RoundNumber != -1) ? ( m_aRoundScoreData.Element(RoundNumber).data[StatID] ) : ( m_aTotalScoreData.data[StatID] );
}

ScoreData SS_PlayerData::GetScoreData( int RoundNumber ) const
{
	return m_aRoundScoreData.Element(RoundNumber);
}

void SS_PlayerData::ResetExtraData( int CurrentRound )
{
	if ( !m_aRoundScoreData.IsValidIndex( CurrentRound ) ) 
	{
		ScoreData data;
		m_aRoundScoreData.AddToTail( data );
	}
	int *data = m_aRoundScoreData.Element( CurrentRound ).data;
	*(data+HealsReceived) = 0;
	*(data+MedPicks) = 0;
	*(data+UbersDropped) = 0;
	//*(data+SumTimeCharging) = 0.0;
}

CPlayerClassTracker *SS_PlayerData::GetClassTracker()
{
	return &m_classTracker;
}

int	SS_PlayerData::GetClass(unsigned int playerClassOffset)
{
	return *((unsigned int *)(((unsigned char *)m_BasePlayerData.GetBaseEntity()) + playerClassOffset));
}

int	SS_PlayerData::GetDataFromOffset( int PropName, const unsigned int pPropOffsets[] )
{
	return *((unsigned int *)(((unsigned char *)m_BasePlayerData.GetBaseEntity()) + pPropOffsets[PropName]));
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
