
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

#ifndef PLAYER_CLASS_TRACKER_H
#define PLAYER_CLASS_TRACKER_H

#include "strtools.h"

enum EPlayerClass
{
	k_ePlayerClassUnknown = 0,
	k_ePlayerClassScout,
	k_ePlayerClassSniper,
	k_ePlayerClassSoldier,
	k_ePlayerClassDemoman,
	k_ePlayerClassMedic,
	k_ePlayerClassHeavy,
	k_ePlayerClassPyro,
	k_ePlayerClassSpy,
	k_ePlayerClassEngineer,
	k_eNumPlayerClasses
};

#define PLAYED_SCOUT	( 1 << 0 )
#define PLAYED_SNIPER	( 1 << 1 )
#define PLAYED_SOLDIER	( 1 << 2 )
#define PLAYED_DEMOMAN	( 1 << 3 )
#define PLAYED_MEDIC	( 1 << 4 )
#define PLAYED_HEAVY	( 1 << 5 )
#define PLAYED_PYRO		( 1 << 6 )
#define PLAYED_SPY		( 1 << 7 )
#define PLAYED_ENGINEER	( 1 << 8 )

class CPlayerClassTracker
{
public:
	CPlayerClassTracker();
	~CPlayerClassTracker();

	void Reset( EPlayerClass player_class = k_ePlayerClassUnknown, uint64 curtime = 0 );

	void StartRecording( EPlayerClass player_class, uint64 curtime );
	void StopRecording( uint64 curtime );

	void PlayerChangedClass( EPlayerClass player_class, uint64 curtime );

	EPlayerClass GetMostPlayedClass();
	uint16 GetPlayedClasses();
	/*
	bool PlayedScout() const	{ return (m_classflags & PLAYED_SCOUT); }
	bool PlayedSoldier() const	{ return (m_classflags & PLAYED_SOLDIER); }
	bool PlayedPyro() const		{ return (m_classflags & PLAYED_PYRO); }
	bool PlayedDemoman() const	{ return (m_classflags & PLAYED_DEMOMAN); }
	bool PlayedHeavy() const	{ return (m_classflags & PLAYED_HEAVY); }
	bool PlayedEngineer() const { return (m_classflags & PLAYED_ENGINEER); }
	bool PlayedMedic() const	{ return (m_classflags & PLAYED_MEDIC); }
	bool PlayedSniper() const	{ return (m_classflags & PLAYED_SNIPER); }
	bool PlayedSpy() const		{ return (m_classflags & PLAYED_SPY); }
	*/
private:
	
	void UpdateTimes( uint64 curtime );
	void UpdateMostPlayedClass( uint64 curtime );

	void FlagClassAsPlayed( uint16 player_class );
	void ResetFlags( uint16 player_class );

	static bool IsTFClass( uint16 player_class );

private:
	// index is EPlayerClasses-1
	// undefined class isn't counted
	uint64 m_timeplayed[9];
	uint64 m_lastUpdate;
	uint16 m_classflags;
	uint16 m_currentClass;
	uint16 m_mostPlayedClass;
};

#pragma warning( push )
#pragma warning( disable : 4351 )
inline CPlayerClassTracker::CPlayerClassTracker():
	m_timeplayed(),
	m_lastUpdate(0),
	m_classflags(0),
	m_currentClass(0),
	m_mostPlayedClass(0)
{
}
#pragma warning( pop )

inline CPlayerClassTracker::~CPlayerClassTracker()
{
}

inline void CPlayerClassTracker::StopRecording( uint64 curtime )
{
	UpdateTimes(curtime);
}

inline EPlayerClass CPlayerClassTracker::GetMostPlayedClass()
{
	return static_cast<EPlayerClass>(m_mostPlayedClass);
}

inline uint16 CPlayerClassTracker::GetPlayedClasses()
{
	return m_classflags;
}

inline void CPlayerClassTracker::ResetFlags( uint16 player_class )
{
	m_classflags = 0;
	FlagClassAsPlayed(player_class);
}

inline bool CPlayerClassTracker::IsTFClass( uint16 player_class )
{
	return ((player_class > k_ePlayerClassUnknown) && (player_class < k_eNumPlayerClasses));
}

#endif // PLAYER_CLASS_TRACKER_H
