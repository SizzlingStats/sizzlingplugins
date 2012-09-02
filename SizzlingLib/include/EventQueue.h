#ifndef EVENTQUEUE_H
#define EVENTQUEUE_H

#include "igameevents.h"
#include "queuethread.h"

class CQueuedEvents: public CQueueThread<IGameEvent>
{
public:
	virtual bool ServiceItem(IGameEvent &item)
	{
		const char * name = item.GetName();
		Msg( "CEmptyServerPlugin::FireGameEvent: Got event \"%s\"\n", name );

		if ( m_pbShouldRecord && FStrEq( name, "player_healed" ) )
		{
			int patient = item.GetInt( "patient" );
			if ( patient == item.GetInt( "healer" ) )
				return;

			// all in one to save some cpu cycles
			// TODO: make a m_CUtlVector, then on player join, after the player is added to the map,
			//		 do a search through the whole map, updating positions of the elements in order
			//		 avoid using pMap->Find()
			//CUtlMap <int, unsigned int> *pMap = &m_SizzlingStats.m_UserIDToHealingDataMap;
			m_pSizzlingStats->m_UserIDToHealingDataMap.Element( m_pSizzlingStats->m_UserIDToHealingDataMap.Find( patient ) ) += item.GetInt( "amount" );
		}
		else if ( FStrEq( name, "teamplay_round_win" ) || FStrEq( name, "teamplay_round_stalemate" ) )
		{
			*m_pbShouldRecord = false;
			m_pSizzlingStats->SS_AllUserChatMessage( "Stats Recording Stopped\n" );
			m_pSizzlingStats->SS_EndOfRound();
		}
		else if ( FStrEq( name, "teamplay_round_active" ) || FStrEq( name, "arena_round_start" ) )
		{
			*m_pbShouldRecord = true;
			m_pSizzlingStats->SS_AllUserChatMessage( "Stats Recording Started\n" );
			//GetEntityByClassname( "CTeam" );
		}
		else if ( FStrEq( name, "teamplay_win_panel" ) || FStrEq( name, "arena_win_panel" ) )
		{
			Msg( "time of win panel: %f\n", gpGlobals->curtime );
			float time = gpGlobals->curtime;
			if ( time - m_pSizzlingStats->SS_GetTimeOfLastCap() < 0.000001 )
			{
				char cappers[32] = "";
				V_strncpy( cappers, item.GetString( "cappers" ), 32);
				int length = V_strlen( cappers );
				for (int i = 0; i < length; i++)
				{
					m_pSizzlingStats->SS_CheckFixEndOfRoundCappers( cappers[i] );

					//int index = cappers[i];
					//Msg("%i captured a point\n", index );
					//m_SizzlingStats.SS_PrintIndex();
				}
			}
		}
		else if ( FStrEq( name, "teamplay_point_captured" ) )
		{
			Msg( "time of cap: %f\n", gpGlobals->curtime );
			m_pSizzlingStats->SS_SetTimeOfLastCap( gpGlobals->curtime );

		}
		else if ( FStrEq( name, "teamplay_game_over" ) || FStrEq( name, "tf_game_over" ) )
		{
			m_pSizzlingStats->SS_GameOver();
		}
	}

	virtual void ThreadInit()
	{
		m_pSizzlingStats = g_EmptyServerPlugin.GetSizzlingStatsPtr();
		m_pbShouldRecord = g_EmptyServerPlugin.GetShouldRecordFlag();
	}

private:
	SizzlingStats *m_pSizzlingStats;
	bool *m_pbShouldRecord;
};

#endif // EVENTQUEUE_H
