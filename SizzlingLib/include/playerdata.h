////////////////////////////////////////////////////////////////////////////////
// Filename: playerdata.h
////////////////////////////////////////////////////////////////////////////////
#ifndef PLAYERDATA_H
#define PLAYERDATA_H

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

//////////////
// INCLUDES //
//////////////
#include "tier1/utlvector.h"
#include "PlayerClassTracker.h"

class CBaseEntity;
class IPlayerInfo;
struct edict_t;

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
// Class name: BasePlayerData
////////////////////////////////////////////////////////////////////////////////
class BasePlayerData
{
public:
	BasePlayerData();
	BasePlayerData( edict_t *pEdict, IPlayerInfo *pInfo );

	void SetBaseData( edict_t *pEdict, IPlayerInfo *pInfo );
	virtual ~BasePlayerData();

	CBaseEntity *GetBaseEntity() const
	{
		return m_pPlayerEnt;
	}

	IPlayerInfo *GetPlayerInfo() const
	{
		return m_pPlayerInfo;
	}

	edict_t	*GetEdict() const
	{
		return m_pEdict;
	}

	int	GetEntIndex() const
	{
		return m_iEntIndex;
	}

private:
	CBaseEntity *m_pPlayerEnt;
	IPlayerInfo *m_pPlayerInfo;
	edict_t	*m_pEdict;
	int	m_iEntIndex;
};

////////////////////////////////////////////////////////////////////////////////
// Class name: SS_PlayerData
////////////////////////////////////////////////////////////////////////////////
class SS_PlayerData
{
public:
	SS_PlayerData();
	SS_PlayerData( edict_t *pEdict, IPlayerInfo *pInfo );
	virtual ~SS_PlayerData();

	void SetBaseData( edict_t *pEdict, IPlayerInfo *pInfo )
	{
		m_BasePlayerData.SetBaseData(pEdict, pInfo);
	}

	CBaseEntity *GetBaseEntity() const
	{
		return m_BasePlayerData.GetBaseEntity();
	}

	IPlayerInfo *GetPlayerInfo() const
	{
		return m_BasePlayerData.GetPlayerInfo();
	}

	edict_t	*GetEdict() const
	{
		return m_BasePlayerData.GetEdict();
	}

	int	GetEntIndex() const
	{
		return m_BasePlayerData.GetEntIndex();
	}

	void		UpdateRoundData( int CurrentRound, const unsigned int pPropOffsets[] );
	void		UpdateTotalData( int CurrentRound );
	void		ResetTotalData();
	ScoreData	GetTotalData();

	void		UpdateExtraData( int CurrentRound, extradata_t &dat );

	int			GetStat( int RoundNumber, int StatID );	//if RoundNumber is -1, returns the totalscore stats

	ScoreData	GetScoreData( int RoundNumber ) const;

	void		ResetExtraData( int CurrentRound );

	CPlayerClassTracker	*GetClassTracker();
	int			GetClass(unsigned int playerClassOffset);

private:
	int			GetDataFromOffset( int PropName, const unsigned int pPropOffsets[]);

private:
	BasePlayerData	m_BasePlayerData;

	CUtlVector<ScoreData> m_aRoundScoreData;	//[Round Number][Data Number]
	ScoreData m_aTotalScoreData;		//[data number]
	CPlayerClassTracker m_classTracker;
};

////////////////////////////////////////////////////////////////////////////////
// Class name: SM_PlayerData
////////////////////////////////////////////////////////////////////////////////
class SM_PlayerData: public BasePlayerData
{
public:
	SM_PlayerData();
	SM_PlayerData( edict_t *pEdict, IPlayerInfo *pInfo );
	virtual ~SM_PlayerData();

	void		UpdateName( const char *szNewName );
	void		SetReadyState( bool state );
	void		SetTeam( int teamindex );

	int			GetTeam();
	const char *GetName();
	bool		GetReadyState();
	
private:
	char m_szName[32];
	int m_iTeam;
	bool m_bIsReady;
};

#endif //PLAYERDATA_H

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
