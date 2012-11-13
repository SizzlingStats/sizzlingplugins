
#include "PlayerClassTracker.h"

CPlayerClassTracker::CPlayerClassTracker():
	m_timeplayed(),
	m_classflags(0)
{
}

CPlayerClassTracker::~CPlayerClassTracker()
{
}

void CPlayerClassTracker::StartRecording( EPlayerClass player_class, double curtime )
{
	ResetInfo(curtime);
	PlayerChangedClass(player_class, curtime);
}

void CPlayerClassTracker::StopRecording( double curtime )
{
	PlayerChangedClass(k_ePlayerClassUnknown, curtime);
}

void CPlayerClassTracker::PlayerChangedClass( EPlayerClass player_class, double curtime )
{
	uint16 iPlayerClass = static_cast<uint16>(player_class);
	
	// if it isn't an invalid class, and if we aren't receiving double messages
	if (m_currentClass > 0 && m_currentClass != iPlayerClass)
	{
		// update the time played for the previous class
		m_timeplayed[m_currentClass-1] += (curtime - m_timeplayed[m_currentClass-1]);

		// if we've played the previous class more than our current most played, update it
		if (m_timeplayed[m_currentClass-1] > m_timeplayed[m_mostPlayedClass-1])
		{
			m_mostPlayedClass = m_currentClass;
		}
	}

	// set the flag to true for the class to say we played it
	m_classflags |= iPlayerClass;

	// set the current class
	m_currentClass = iPlayerClass;
}

EPlayerClass CPlayerClassTracker::GetMostPlayedClass()
{
	return static_cast<EPlayerClass>(m_mostPlayedClass);
}

void CPlayerClassTracker::ResetInfo( double curtime )
{
	// reset all the data
	m_timeplayed[0] = curtime;
	m_timeplayed[1] = curtime;
	m_timeplayed[2] = curtime;
	m_timeplayed[3] = curtime;
	m_timeplayed[4] = curtime;
	m_timeplayed[5] = curtime;
	m_timeplayed[6] = curtime;
	m_timeplayed[7] = curtime;
	m_timeplayed[8] = curtime;
	m_classflags = 0;
	m_currentClass = 0;
	m_mostPlayedClass = 0;
}
