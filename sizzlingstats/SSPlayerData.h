
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

#ifndef SS_PLAYER_DATA
#define SS_PLAYER_DATA

#include "tier1/utlvector.h"
#include "PlayerClassTracker.h"
#include "strtools.h"

class CBaseEntity;

enum stats
{
	Captures = 0,
	Defenses,
	Kills,
	Deaths,
	Suicides,
	Dominations,
	Revenge,
	BuildingsBuilt,
	BuildingsDestroyed,
	Headshots,
	Backstabs,
	HealPoints,
	Invulns,
	Teleports,
	DamageDone,
	Crits,
	ResupplyPoints,
	KillAssists,
	BonusPoints,
	Points,
	HealsReceived,
	MedPicks,
	UbersDropped,
	OverkillDamage,
	ShotsFired,
	ShotsHit,
	NumOfStats
};

typedef struct extradata_s
{
	extradata_s():
        healsrecv(0), medpicks(0), ubersdropped(0), overkillDamage(0)
	{
	}

	extradata_s &operator = ( const int a )
	{
		healsrecv = a;
		medpicks = a;
		ubersdropped = a;
		overkillDamage = a;
		return *this;
	}

	int healsrecv;
	short medpicks;
	short ubersdropped;
	int overkillDamage;
} extradata_t;

struct ScoreData
{
#pragma warning( disable : 4351 )
	ScoreData(): data() {}
#pragma warning( default : 4351 )

	int data[NumOfStats];

	int getStat( stats Stat ) const
	{
		return data[Stat];
	}

	ScoreData &operator += ( const ScoreData &a )
	{
		for ( int i = 0; i < NumOfStats; ++i )
			data[i] += a.data[i];
		return *this;
	}

	void Reset()
	{
		V_memset(data, 0, sizeof(data));
	}
};

struct ScoreDataOffsets
{
	unsigned int offsets[20];
};

////////////////////////////////////////////////////////////////////////////////
// Class name: SS_PlayerData
////////////////////////////////////////////////////////////////////////////////
class SS_PlayerData
{
public:
	SS_PlayerData();

	void Reset( CBaseEntity *pEnt );
	CBaseEntity *GetBaseEntity() const { return m_base_entity; }

	void		UpdateRoundStatsData( const unsigned int pPropOffsets[] );
	void		ResetRoundStatsData();
	ScoreData	GetRoundStatsData();

	void		UpdateRoundExtraData( const extradata_t &dat );

	int			GetStat( int StatID );

	CPlayerClassTracker	*GetClassTracker();

	void TriggerCapFix()
	{
		m_bCapFix = true;
	}

private:
	int			GetDataFromOffset( int PropName, const unsigned int pPropOffsets[]);

private:
	CBaseEntity *m_base_entity;
	ScoreData m_RoundScoreData;
	CPlayerClassTracker m_classTracker;
	bool m_bCapFix;
};

#endif // SS_PLAYER_DATA

//DT_TFPlayerScoringDataExclusive
//	4, Captures				1
//	8, Defenses				2
//	12, Kills				3
//	16, Deaths				4
//	20, Suicides			5
//	24, Dominations			6
//	28, Revenge				7
//	32, BuildingsBuilt		8
//	36, BuildingsDestroyed	9
//	40, Headshots			10
//	44, Backstabs			11
//	48, HealPoints			12
//	52, Invulns				13
//	56, Teleports			14
//	60, DamageDone			15
//	64, Crits				16
//	68, ResupplyPoints		17
//	72, KillAssists			18
//	76, BonusPoints			19
//	80, Points				20
