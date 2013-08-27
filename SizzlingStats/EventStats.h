
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

#pragma once

#include "EventSender.h"

class IGameEvent;

class CEventStats
{
public:
	bool Initialize();
	void Shutdown();

	void OnTournamentMatchStart( const char *RESTRICT hostname, const char *RESTRICT mapname, 
								const char *RESTRICT bluname, const char *RESTRICT redname, 
								unsigned int server_tick );

	void OnTournamentMatchEnd( unsigned int server_tick );

	void OnFireGameEvent( IGameEvent *pEvent, unsigned int server_tick );

	void SendNamedEvent( const char *event_name, unsigned int server_tick );

private:
	bool CEventStats::ConvertEvent( IGameEvent *pEvent, CSizzEvent *event );

private:
	CEventSender m_event_sender;
};
