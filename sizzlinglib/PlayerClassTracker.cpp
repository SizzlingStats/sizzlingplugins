
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

#include "PlayerClassTracker.h"

#define IS_SET(flag, bit)    ((flag) & (bit))
#define SET_BIT(var, bit)    ((var) |= (bit))
#define REMOVE_BIT(var, bit)    ((var) &= ~(bit))

void CPlayerClassTracker::Reset( 
	EPlayerClass player_class /*= k_ePlayerClassUnknown*/,
	uint64 curtime /*= 0*/ )
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
	m_lastUpdate = curtime;
	m_currentClass = player_class;
	m_mostPlayedClass = player_class;
	ResetFlags(player_class);
}

void CPlayerClassTracker::StartRecording( EPlayerClass player_class, uint64 curtime )
{
	Reset(player_class, curtime);
	REMOVE_BIT(m_classflags, PLAYER_DO_NOT_TRACK);
}

void CPlayerClassTracker::PlayerSpawned( EPlayerClass player_class, uint64 curtime )
{
	REMOVE_BIT(m_classflags, PLAYER_DO_NOT_TRACK);
	m_lastUpdate = curtime;
}

/*
 * Called when:
 * 	1) Player has died
 * 	2) Round has ended
 * 	3) Player has disconnected
 */
void CPlayerClassTracker::PlayerNoTrack( uint64 curtime )
{
	UpdateTimes(curtime);
	SET_BIT(m_classflags, PLAYER_DO_NOT_TRACK);
}


void CPlayerClassTracker::PlayerChangedClass( EPlayerClass player_class, uint64 curtime )
{
	uint16 iPlayerClass = static_cast<uint16>(player_class);
	// if we are not getting double messages
	if (m_currentClass != iPlayerClass)
	{
		UpdateTimes(curtime);
		m_currentClass = player_class;
		FlagClassAsPlayed(player_class);
	}
}

void CPlayerClassTracker::UpdateTimes( uint64 curtime )
{
	if (IsTFClass(m_currentClass) && !IS_SET(m_classflags,PLAYER_DO_NOT_TRACK))
	{
		m_timeplayed[m_currentClass-1] += (curtime - m_lastUpdate);
		UpdateMostPlayedClass(curtime);
	}
	m_lastUpdate = curtime;
}

void CPlayerClassTracker::UpdateMostPlayedClass( uint64 curtime )
{
	if (m_mostPlayedClass != m_currentClass)
	{
		if (IsTFClass(m_mostPlayedClass))
		{
			if (m_timeplayed[m_currentClass-1] >= m_timeplayed[m_mostPlayedClass-1])
			{
				m_mostPlayedClass = m_currentClass;
			}
		}
		else
		{
			m_mostPlayedClass = m_currentClass;
		}
	}
}

uint64 CPlayerClassTracker::GetPlayedTimeAs(uint16 player_class) const
{
	if(!IsTFClass(player_class))
		return 0;

	return m_timeplayed[player_class-1];
}


void CPlayerClassTracker::FlagClassAsPlayed( uint16 player_class )
{
	if (IsTFClass(player_class))
	{
		m_classflags |= ( 1 << (player_class-1) );
	}
}

