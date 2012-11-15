
#include "PlayerClassTracker.h"

#pragma warning( push )
#pragma warning( disable : 4351 )
CPlayerClassTracker::CPlayerClassTracker():
	m_timeplayed(),
	m_lastUpdate(0),
	m_classflags(0),
	m_currentClass(0),
	m_mostPlayedClass(0)
{
}
#pragma warning( pop )

CPlayerClassTracker::~CPlayerClassTracker()
{
}

void CPlayerClassTracker::StartRecording( EPlayerClass player_class, uint64 curtime )
{
	ResetInfo( player_class, curtime );
}

void CPlayerClassTracker::StopRecording( uint64 curtime )
{
	UpdateTimes(curtime);
}

void CPlayerClassTracker::PlayerChangedClass( EPlayerClass player_class, uint64 curtime )
{
	uint16 iPlayerClass = static_cast<uint16>(player_class);
	if (m_currentClass != iPlayerClass)
	{
		UpdateTimes(curtime);
		if (IsTFClass(player_class))
		{
			m_currentClass = player_class;
			m_classflags |= ( 1 << (player_class-1) );
		}
	}
}

EPlayerClass CPlayerClassTracker::GetMostPlayedClass()
{
	return static_cast<EPlayerClass>(m_mostPlayedClass);
}

uint16 CPlayerClassTracker::GetPlayedClasses()
{
	return m_classflags;
}

void CPlayerClassTracker::UpdateTimes( uint64 curtime )
{
	if (IsTFClass(m_currentClass))
	{
		m_timeplayed[m_currentClass] += (curtime - m_lastUpdate);
		m_lastUpdate = curtime;
		UpdateMostPlayedClass(curtime);
	}
}

void CPlayerClassTracker::UpdateMostPlayedClass( uint64 curtime )
{
	if (IsTFClass(m_mostPlayedClass))
	{
		if (m_timeplayed[m_currentClass] >= m_timeplayed[m_mostPlayedClass])
		{
			m_mostPlayedClass = m_currentClass;
		}
	}
	else
	{
		m_mostPlayedClass = m_currentClass;
	}
}

void CPlayerClassTracker::FlagClassAsPlayed( EPlayerClass player_class )
{
	if (IsTFClass(player_class))
	{
		m_classflags |= ( 1 << (player_class-1) );
	}
}

void CPlayerClassTracker::ResetFlags( EPlayerClass player_class )
{
	m_classflags = 0;
	FlagClassAsPlayed(player_class);
}

void CPlayerClassTracker::ResetInfo( EPlayerClass current_class, uint64 curtime )
{
	// reset all the data
	m_timeplayed[0] = 0;
	m_timeplayed[1] = 0;
	m_timeplayed[2] = 0;
	m_timeplayed[3] = 0;
	m_timeplayed[4] = 0;
	m_timeplayed[5] = 0;
	m_timeplayed[6] = 0;
	m_timeplayed[7] = 0;
	m_timeplayed[8] = 0;
	m_timeplayed[9] = 0;
	m_lastUpdate = curtime;
	m_currentClass = current_class;
	m_mostPlayedClass = 0;
	ResetFlags(current_class);
}

bool CPlayerClassTracker::IsTFClass( uint16 player_class )
{
	if (player_class > k_ePlayerClassUnknown && player_class < k_eNumPlayerClasses)
	{
		return true;
	}
	else
	{
		return false;
	}
}
