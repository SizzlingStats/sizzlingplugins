
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

// reversed from valve binaries

#ifndef GAME_EVENT_MANAGER_H
#define GAME_EVENT_MANAGER_H

#include "utlvector.h"
#include "utlsymbol.h"

#include "igameevents.h"
#include "KeyValues.h"

class CGameEventDescriptor
{
public:
	char m_name[32];
	char m_other[32];
};

class CGameEventCallback
{
public:
	IGameEventListener2 *m_listener;

	// in the disasm: m_something = (int)(unsigned __int8)(bServerSide ^ 1);
	int m_something;
};

class CGameEvent: public IGameEvent
{
public:
	CGameEventDescriptor *m_event_descriptor;
	KeyValues *m_event_data;
};

class CGameEventManager: public IGameEventManager2
{
public:
	CUtlVector<CGameEventDescriptor> m_events;
	CUtlVector<CGameEventCallback*> m_callbacks;
	CUtlSymbolTable m_member3;
	CUtlVector<int> m_member4;
	bool m_bClientListenersSomething;
};

#endif // GAME_EVENT_MANAGER_H
