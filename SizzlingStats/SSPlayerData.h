
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
	UbersDropped,
	MedPicks,
	ShotsFired,
	ShotsHit,
	//SumTimeCharging,
	NumOfStats
};

typedef struct extradata_s
{
	extradata_s():healsrecv(0), medpicks(0), ubersdropped(0)//, shots_fired(0), shots_hit(0), currentubertime(0.0), sumofubertimes(0.0)
	{
	}

	//extradata_s( int ): healsrecv(0), medpicks(0), ubersdropped(0)//, shots_fired(0), shots_hit(0), currentubertime(0.0), sumofubertimes(0.0)
	//{
	//}

	extradata_s &operator = ( const int a )
	{
		healsrecv = a;
		medpicks = a;
		ubersdropped = a;
		//shots_fired = a;
		//shots_hit = a;
		return *this;
	}

	int healsrecv;
	short medpicks;
	short ubersdropped;
	//unsigned int shots_fired;
	// shots_hit will always be <= shots_fired
	//unsigned int shots_hit;
	//float currentubertime;
	//float sumofubertimes;
} extradata_t;

struct ScoreData
{
#pragma warning( disable : 4351 )
	ScoreData(): data()
	{
		/*for ( int i = 20; i < NumOfStats; ++i )
		{
			data[i] = 0;
		}*/
	}
#pragma warning( default : 4351 )
	int data[NumOfStats];

	int getStat( stats Stat ) const
	{
		return data[Stat];
	}
	ScoreData &operator += ( const ScoreData &a )
	{
		//V_memcpy(data, &a, sizeof(a));
		for ( int i = 0; i < NumOfStats; ++i )
			data[i] += a.data[i];
		return *this;
	}

	//ScoreData &operator = ( ScoreData &a )	// just examples from the ibm site CAUSE I SUCK
	//{
	//	return a;
	//}

	void Reset()
	{
		for ( int i = 0; i < NumOfStats; ++i )
			data[i] = 0;
	}

	//ScoreData &operator = ( int a )
	//{
	//	V_memcpy(data, &a, sizeof(a));
	//	return *this;
	//}
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

	void SetBaseEntity( CBaseEntity *pEnt ) { m_base_entity = pEnt; }
	CBaseEntity *GetBaseEntity() const { return m_base_entity; }

	void		UpdateRoundStatsData( const unsigned int pPropOffsets[] );
	void		ResetRoundStatsData();
	ScoreData	GetRoundStatsData();

	void		UpdateRoundExtraData(  extradata_t &dat );

	int			GetStat( int StatID );

	CPlayerClassTracker	*GetClassTracker();
	int			GetClass(unsigned int playerClassOffset);

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
