
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

#pragma once

#include "EventSender.h"

class CSizzPluginContext;
class IGameEvent;

class CEventStats
{
public:
	bool Initialize();
	void Shutdown();

	void OnTournamentMatchStart( CSizzPluginContext *pPluginContext, unsigned int server_tick );
	void OnTournamentMatchEnd( CSizzPluginContext *pPluginContext, unsigned int server_tick );

	void OnFireGameEvent( IGameEvent *pEvent, unsigned int server_tick );

	void SendNamedEvent( const char *event_name, unsigned int server_tick );

private:
	bool ConvertEvent( IGameEvent *pEvent, CSizzEvent *event );

private:
	CEventSender m_event_sender;
};
