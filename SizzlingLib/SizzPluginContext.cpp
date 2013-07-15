
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

#include "SizzPluginContext.h"

#include "eiface.h"
#include "UserIdTracker.h"
#include "edict.h"
#include "steam\steamclientpublic.h"

CSizzPluginContext::CSizzPluginContext():
	m_pEngine(nullptr),
	m_pGlobals(nullptr),
	m_pUserIDTracker(new CUserIDTracker()),
	m_tickcount(0),
	m_flTime(0.0f)
{
}

CSizzPluginContext::~CSizzPluginContext()
{
	delete m_pUserIDTracker;
}

void CSizzPluginContext::Initialize( const plugin_context_init_t &init )
{
	m_pEngine = init.pEngine;
	m_pGlobals = init.pGlobals;
	m_pUserIDTracker->Reset();
}

int CSizzPluginContext::UserIDFromEntIndex( int ent_index )
{
	edict_t *pEdict = m_pEngine->PEntityOfEntIndex(ent_index);
	if (pEdict)
	{
		return m_pEngine->GetPlayerUserId(pEdict);
	}
	return -1;
}

int CSizzPluginContext::EntIndexFromUserID( int userid )
{
	int ent_index = m_pUserIDTracker->GetEntIndex(userid);
	return ent_index;
}

int CSizzPluginContext::SteamIDFromUserID( int userid )
{
	int ent_index = EntIndexFromUserID(userid);
	return SteamIDFromEntIndex(ent_index);
}

int CSizzPluginContext::SteamIDFromEntIndex( int ent_index )
{
	const CSteamID *pID = m_pEngine->GetClientSteamIDByPlayerIndex(ent_index);
	if (pID)
	{
		return pID->GetAccountID();
	}
	return 0;
}

int CSizzPluginContext::GetCurrentTick() const
{
	return m_tickcount;
}

float CSizzPluginContext::GetTime() const
{
	return m_flTime;
}

void CSizzPluginContext::ClientActive( const edict_t *pEdict )
{
	if (pEdict)
	{
		int ent_index = m_pEngine->IndexOfEdict(pEdict);
		if (ent_index != -1)
		{
			int userid = UserIDFromEntIndex(ent_index);
			if (userid != -1)
			{
				m_pUserIDTracker->ClientActive(userid, ent_index);
			}
		}
	}
}

void CSizzPluginContext::ClientDisconnect( const edict_t *pEdict )
{
	if (pEdict)
	{
		int ent_index = m_pEngine->IndexOfEdict(pEdict);
		if (ent_index != -1)
		{
			int userid = UserIDFromEntIndex(ent_index);
			if (userid != -1)
			{
				m_pUserIDTracker->ClientDisconnect(userid);
			}
		}
	}
}

void CSizzPluginContext::GameFrame( bool simulating )
{
	m_tickcount = m_pGlobals->tickcount;
	m_flTime = m_pGlobals->realtime;
}
