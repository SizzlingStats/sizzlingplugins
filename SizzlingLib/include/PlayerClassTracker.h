
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
#define PLAYED_SOLDIER	( 1 << 1 )
#define PLAYED_PYRO		( 1 << 2 )
#define PLAYED_DEMOMAN	( 1 << 3 )
#define PLAYED_HEAVY	( 1 << 4 )
#define PLAYED_ENGINEER ( 1 << 5 )
#define PLAYED_MEDIC	( 1 << 6 )
#define PLAYED_SNIPER	( 1 << 7 )
#define PLAYED_SPY		( 1 << 8 )

class CPlayerClassTracker
{
public:
	CPlayerClassTracker();
	~CPlayerClassTracker();

	void StartRecording( EPlayerClass player_class, double curtime );
	void StopRecording( double curtime );

	void PlayerChangedClass( EPlayerClass player_class, double curtime );

	EPlayerClass GetMostPlayedClass();

	bool PlayedScout() const	{ return (m_classflags & PLAYED_SCOUT); }
	bool PlayedSoldier() const	{ return (m_classflags & PLAYED_SOLDIER); }
	bool PlayedPyro() const		{ return (m_classflags & PLAYED_PYRO); }
	bool PlayedDemoman() const	{ return (m_classflags & PLAYED_DEMOMAN); }
	bool PlayedHeavy() const	{ return (m_classflags & PLAYED_HEAVY); }
	bool PlayedEngineer() const { return (m_classflags & PLAYED_ENGINEER); }
	bool PlayedMedic() const	{ return (m_classflags & PLAYED_MEDIC); }
	bool PlayedSniper() const	{ return (m_classflags & PLAYED_SNIPER); }
	bool PlayedSpy() const		{ return (m_classflags & PLAYED_SPY); }

private:
	void ResetInfo( double curtime );

private:
	double m_timeplayed[9]; // index is EPlayerClasses
	uint16 m_classflags;
	uint16 m_currentClass;
	uint16 m_mostPlayedClass;
};

#endif // PLAYER_CLASS_TRACKER_H