
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
#include "steam/steamclientpublic.h"
#include "ServerPluginHandler.h"
#include "game/server/iplayerinfo.h"
#include "igameevents.h"
#include "ThreadCallQueue.h"

CSizzPluginContext::CSizzPluginContext():
	m_pEngine(nullptr),
	m_pPlayerInfoManager(nullptr),
	m_pPluginManager(nullptr),
	m_pGameEventManager(nullptr),
	m_pServerGameDLL(nullptr),
	m_pGlobals(nullptr),
	m_pUserIDTracker(new CUserIDTracker()),
	m_pCallQueue(new CTSCallQueue()),
	m_tickcount(0),
	m_flTime(0.0f),
	m_max_clients(0)
{
}

CSizzPluginContext::~CSizzPluginContext()
{
	delete m_pCallQueue;
	delete m_pUserIDTracker;
}

void CSizzPluginContext::Initialize( const plugin_context_init_t &init )
{
	m_pEngine = init.pEngine;
	m_pPlayerInfoManager = init.pPlayerInfoManager;
	m_pPluginManager = reinterpret_cast<CServerPlugin*>(init.pHelpers);
	m_pGameEventManager = init.pGameEventManager;
	m_pServerGameDLL = init.pServerGameDLL;
	m_pGlobals = m_pPlayerInfoManager->GetGlobalVars();
	m_pUserIDTracker->Reset();
}

IVEngineServer *CSizzPluginContext::GetEngine() const
{
	return m_pEngine;
}

IGameEventManager2 *CSizzPluginContext::GetGameEventManager() const
{
	return m_pGameEventManager;
}

IServerGameDLL *CSizzPluginContext::GetServerGameDLL() const
{
	return m_pServerGameDLL;
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

int CSizzPluginContext::EntIndexFromUserID( int userid ) const
{
	int ent_index = m_pUserIDTracker->GetEntIndex(userid);
	return ent_index;
}

unsigned int CSizzPluginContext::SteamIDFromUserID( int userid )
{
	int ent_index = EntIndexFromUserID(userid);
	return SteamIDFromEntIndex(ent_index);
}

unsigned int CSizzPluginContext::SteamIDFromEntIndex( int ent_index )
{
	const CSteamID *pID = m_pEngine->GetClientSteamIDByPlayerIndex(ent_index);
	if (pID)
	{
		return pID->GetAccountID();
	}
	return static_cast<unsigned int>(-1);
}

void CSizzPluginContext::GetSteamIDString( int userid, char *dest, int buff_size )
{
	if (dest && (buff_size > 0))
	{
		int ent_index = EntIndexFromUserID(userid);
		if (ent_index != -1)
		{
			IPlayerInfo *pInfo = GetPlayerInfo(ent_index);
			if (pInfo)
			{
				const char *src = pInfo->GetNetworkIDString();
				if (src)
				{
					V_strncpy(dest, src, buff_size);
				}
			}
		}
	}
}

int CSizzPluginContext::GetCurrentTick() const
{
	return m_tickcount;
}

float CSizzPluginContext::GetTime() const
{
	return m_flTime;
}

int CSizzPluginContext::GetMaxClients() const
{
	return m_max_clients;
}

const char *CSizzPluginContext::GetMapName() const
{
	return m_pGlobals->mapname.ToCStr();
}

int CSizzPluginContext::GetPluginIndex( const IServerPluginCallbacks *pPlugin ) const
{
	if (pPlugin)
	{
		int num_plugins = m_pPluginManager->m_plugins.Count();
		for ( int i = 0; i < num_plugins; ++i )
		{
			CPlugin *plugin = m_pPluginManager->m_plugins[i];
			if (plugin && (plugin->m_pPlugin == pPlugin))
			{
				return i;
			}
		}
	}
	return -1;
}

int CSizzPluginContext::GetPluginIndex( const char *pszDescriptionPart ) const
{
	if (pszDescriptionPart)
	{
		int num_plugins = m_pPluginManager->m_plugins.Count();
		for ( int i = 0; i < num_plugins; ++i )
		{
			CPlugin *pPlugin = m_pPluginManager->m_plugins[i];
			if (pPlugin && V_strstr(pPlugin->m_szName, pszDescriptionPart))
			{
				return i;
			}
		}
	}
	return -1;
}

void CSizzPluginContext::CreateMessage( int ent_index, DIALOG_TYPE type, KeyValues *data, IServerPluginCallbacks *plugin )
{
	if (data && plugin)
	{
		edict_t *pEnt = m_pEngine->PEntityOfEntIndex(ent_index);
		if (pEnt)
		{
			m_pPluginManager->CreateMessage(pEnt, type, data, plugin);
		}
	}
}

void CSizzPluginContext::ServerCommand( const char *command )
{
	m_pEngine->ServerCommand(command);
}

void CSizzPluginContext::ServerExecute()
{
	m_pEngine->ServerExecute();
}

void CSizzPluginContext::LogPrint( const char *msg )
{
	m_pEngine->LogPrint(msg);
}

bool CSizzPluginContext::IsPaused()
{
	return m_pEngine->IsPaused();
}

bool CSizzPluginContext::AddListener( IGameEventListener2 *listener, const char *name, bool bServerSide )
{
	return m_pGameEventManager->AddListener(listener, name, bServerSide);
}

void CSizzPluginContext::RemoveListener( IGameEventListener2 *listener )
{
	m_pGameEventManager->RemoveListener(listener);
}

IPlayerInfo *CSizzPluginContext::GetPlayerInfo( int ent_index )
{
	if ((1 <= ent_index) && (ent_index < GetMaxClients()))
	{
		edict_t *pEdict = m_pEngine->PEntityOfEntIndex(ent_index);
		if (pEdict && !pEdict->IsFree())
		{
			return m_pPlayerInfoManager->GetPlayerInfo(pEdict);
		}
	}
	return nullptr;
}

void CSizzPluginContext::EnqueueGameFrameFunctor( CFunctor *pFunctor )
{
	if (pFunctor)
	{
		m_pCallQueue->EnqueueFunctor(pFunctor);
	}
}

ServerClass *CSizzPluginContext::GetAllServerClasses()
{
	return m_pServerGameDLL->GetAllServerClasses();
}

void CSizzPluginContext::LevelShutdown()
{
	m_pUserIDTracker->Reset();
}

int CSizzPluginContext::ClientActive( const edict_t *pEdict )
{
	if (pEdict)
	{
		int userid = m_pEngine->GetPlayerUserId(pEdict);
		if (userid != -1)
		{
			int ent_index = m_pEngine->IndexOfEdict(pEdict);
			if (ent_index != -1)
			{
				m_pUserIDTracker->ClientActive(userid, ent_index);
				return ent_index;
			}
		}
	}
	return -1;
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
	m_max_clients = m_pGlobals->maxClients;

	m_pCallQueue->callQueueGameFrame();
}

int CSizzPluginContext::EntIndexFromEdict( const edict_t *pEdict )
{
	if (pEdict)
	{
		return m_pEngine->IndexOfEdict(pEdict);
	}
	return -1;
}
