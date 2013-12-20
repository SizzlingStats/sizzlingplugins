
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
#include "irecipientfilter.h"
#include "SC_helpers.h"
#include "server_class.h"
#include "GameEventManager.h"
#include "inetchannelinfo.h"

#define USERMSG_MAX_LENGTH 192

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
	m_max_clients(0),
	m_edict_list(nullptr),
	m_refHostname((IConVar*)nullptr),
	m_refBlueTeamName((IConVar*)nullptr),
	m_refRedTeamName((IConVar*)nullptr)
{
}

CSizzPluginContext::~CSizzPluginContext()
{
	delete m_pCallQueue;
	delete m_pUserIDTracker;
}

bool CSizzPluginContext::Initialize( const plugin_context_init_t &init )
{
	m_pEngine = init.pEngine;
	m_pPlayerInfoManager = init.pPlayerInfoManager;
	m_pPluginManager = reinterpret_cast<CServerPlugin*>(init.pHelpers);
	m_pGameEventManager = reinterpret_cast<CGameEventManager*>(init.pGameEventManager);
	m_pServerGameDLL = init.pServerGameDLL;

	bool ret = true;
	if (!m_pEngine)
	{
		Warning( "Unable to load engine, aborting load\n" );
		ret = false;
	}
	if (!m_pPlayerInfoManager)
	{
		Warning( "Unable to load playerinfomanager, aborting load\n" );
		ret = false;
	}
	if (!m_pPluginManager)
	{
		Warning( "Unable to load helpers, aborting load\n" );
		ret = false;
	}
	if (!m_pGameEventManager)
	{
		Warning( "Unable to load gameeventmanager, aborting load\n" );
		ret = false;
	}
	if (!m_pServerGameDLL)
	{
		Warning( "Unable to load pServerDLL, aborting load\n" );
		ret = false;
	}

	m_pGlobals = m_pPlayerInfoManager->GetGlobalVars();
	if (!m_pGlobals)
	{
		Warning( "Unable to load globals, aborting load\n" );
		ret = false;
	}

	m_pUserIDTracker->Reset();

	m_refHostname.Init("hostname", false);
	m_refBlueTeamName.Init("mp_tournament_blueteamname", false);
	m_refRedTeamName.Init("mp_tournament_redteamname", false);

	return ret;
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
	edict_t *pEdict = EdictFromEntIndex(ent_index);
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
		// bots have an account type of k_EAccountTypeAnonGameServer
		// maybe check that and return a special value
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

const char *CSizzPluginContext::GetHostName() const
{
	return m_refHostname.GetString();
}

const char *CSizzPluginContext::GetMapName() const
{
	return m_pGlobals->mapname.ToCStr();
}

const char *CSizzPluginContext::GetRedTeamName() const
{
	return m_refRedTeamName.GetString();
}

const char *CSizzPluginContext::GetBluTeamName() const
{
	return m_refBlueTeamName.GetString();
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
		edict_t *pEnt = EdictFromEntIndex(ent_index);
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

bool CSizzPluginContext::AddListenerAll( IGameEventListener2 *listener, bool bServerSide )
{
	bool res = false;
	if (listener)
	{
		m_pGameEventManager->RemoveListener(listener);
		auto pEvents = &m_pGameEventManager->m_events;
		int num_events = pEvents->Count();
		res = true;
		for (int i = 0; ((i < num_events) && res); ++i)
		{
			//Msg("registering for event %i: '%s'\n", i, event_name);
			res = m_pGameEventManager->AddListener(listener, pEvents->Element(i).m_name, bServerSide);
		}
	}
	return res;
}

void CSizzPluginContext::RemoveListener( IGameEventListener2 *listener )
{
	m_pGameEventManager->RemoveListener(listener);
}

IPlayerInfo *CSizzPluginContext::GetPlayerInfo( int ent_index )
{
	if ((1 <= ent_index) && (ent_index <= GetMaxClients()))
	{
		edict_t *pEdict = EdictFromEntIndex(ent_index);
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

//	COLOURS
//[CSS]         Print       SayText2
//-----------------------------------
//    \x01      Yellow      Yellow
//    \x02      Yellow      Yellow
//    \x03      LightGreen  Red,Blue,Grey
//    \x04      Green       Green
//    
//[TF]
//-----------------------------------
//    \x01      White       White
//    \x02      White       White
//    \x03      LightGreen  Red,Blue,Grey
//    \x04      Green       Green
//    \x05      Olive       Olive
//    
//[DOD]
//-----------------------------------
//    \x01      Grey          -
//    \x02      Grey          -
//    \x03      Grey          -
//    \x04      Green         -
//    \x05      Olive         -

void CSizzPluginContext::ChatMessage( IRecipientFilter *pFilter, const char *format, ... )
{
	if (pFilter && format)
	{
		va_list args;
		va_start(args, format);
		ChatMessageArg(pFilter, format, args);
		va_end(args);
	}
}

void CSizzPluginContext::ChatMessageArg( IRecipientFilter *pFilter, const char *format, va_list args )
{
	if (pFilter && format && args)
	{
		// Start the usermessage and get a bf_write
		// SayText: 3
		bf_write *pBuffer = m_pEngine->UserMessageBegin(pFilter, 3);
		if (pBuffer)
		{
			char msg[USERMSG_MAX_LENGTH];
			V_vsnprintf(msg, USERMSG_MAX_LENGTH, format, args);

			// Send the message
			pBuffer->WriteByte(0);
			pBuffer->WriteString(msg);
			pBuffer->WriteByte(0);

			// End the message
			m_pEngine->MessageEnd();
		}
	}
}

void CSizzPluginContext::HudResetMessage( IRecipientFilter *pFilter )
{
	if (pFilter)
	{
		// ResetHUD: 6
		bf_write *pBuffer = m_pEngine->UserMessageBegin(pFilter, 6);
		if (pBuffer)
		{
			pBuffer->WriteByte(0);
			m_pEngine->MessageEnd();
		}
	}
}

void CSizzPluginContext::HudMessage( IRecipientFilter *pFilter, const hud_msg_cfg_t &cfg, const char *format, ... )
{
	if (pFilter && format)
	{
		va_list args;
		va_start(args, format);
		HudMessageArg(pFilter, cfg, format, args);
		va_end(args);
	}
}

void CSizzPluginContext::HudMessageArg( IRecipientFilter *pFilter, const hud_msg_cfg_t &cfg, const char *format, va_list args )
{
	if (pFilter && format && args)
	{
		// HudMsg: 21
		bf_write *pBuffer = m_pEngine->UserMessageBegin(pFilter, 21);
		if (pBuffer)
		{
			// channel byte
			pBuffer->WriteByte(cfg.channel & 0xFF);

			// x, y (-1 = center)
			pBuffer->WriteFloat(cfg.x);
			pBuffer->WriteFloat(cfg.y);

			char r = cfg.rgba.r();
			char g = cfg.rgba.g();
			char b = cfg.rgba.b();
			char a = cfg.rgba.a();

			// second colour
			pBuffer->WriteByte(r);
			pBuffer->WriteByte(g);
			pBuffer->WriteByte(b);
			pBuffer->WriteByte(a);

			// init colour
			pBuffer->WriteByte(r);
			pBuffer->WriteByte(g);
			pBuffer->WriteByte(b);
			pBuffer->WriteByte(a);

			// effect (0 = fade in/out, 1 = flickery credits, 2 = write out)
			pBuffer->WriteByte(0);

			// fade in time (per char in effect 2)
			pBuffer->WriteFloat(0);

			// fade out time
			pBuffer->WriteFloat(0);

			// hold time
			pBuffer->WriteFloat(cfg.screentime);

			// fx time (for effect type 2)
			pBuffer->WriteFloat(0);

			char msg[USERMSG_MAX_LENGTH];
			V_vsnprintf(msg, USERMSG_MAX_LENGTH, format, args);

			// message
			pBuffer->WriteString(msg);

			m_pEngine->MessageEnd();
		}
	}
}

void CSizzPluginContext::HudHintMessage( IRecipientFilter *pFilter, const char *format, ... )
{
	if (pFilter && format)
	{
		va_list args;
		va_start(args, format);
		HudHintMessageArg(pFilter, format, args);
		va_end(args);
	}
}

void CSizzPluginContext::HudHintMessageArg( IRecipientFilter *pFilter, const char *format, va_list args )
{
	if (pFilter && format && args)
	{
		// KeyHintText: 20
		bf_write *pBuffer = m_pEngine->UserMessageBegin(pFilter, 20);
		if (pBuffer)
		{
			// number of messages to write
			pBuffer->WriteByte( 1 );

			char msg[USERMSG_MAX_LENGTH];
			V_vsnprintf(msg, USERMSG_MAX_LENGTH, format, args);

			// message
			pBuffer->WriteString(msg);

			m_pEngine->MessageEnd();
		}
	}
}

void CSizzPluginContext::MOTDPanelMessage( IRecipientFilter *pFilter, const char *msg, const motd_msg_cfg_t &cfg )
{
	if (pFilter && msg)
	{
		// VGUIMenu: 12
		bf_write *pBuffer = m_pEngine->UserMessageBegin(pFilter, 12);
		if (pBuffer)
		{
			// the string type of the panel
			// some more are defined in viewport_panel_names.h
			pBuffer->WriteString("info");

			// will the panel be visible? 1 is yes, 0 is no
			pBuffer->WriteByte(cfg.visible ? 1 : 0);

			// number of entries in the following'table
			pBuffer->WriteByte(5);
			{
				// Title of the panel (printed on the top border of the window).
				pBuffer->WriteString("title");
				pBuffer->WriteString("");

				const char *int_to_str[] = 
				{
					"0", "1", "2", "3"
				};

				// Determines the way to treat the message body of the panel.
				// the types are defined above
				pBuffer->WriteString("type");
				pBuffer->WriteString(int_to_str[cfg.type]);

				// Contents of the panel, it can be treated as an url, filename or plain text
				// depending on the type parameter (WARNING: msg has to be 192 bytes maximum!)
				// urls have to be prefixed with http:// or https:// !!!
				pBuffer->WriteString("msg");							
				pBuffer->WriteString(msg);

				// 0 means use a small vgui window, 1 means a large (tf2 only)
				pBuffer->WriteString("customsvr");
				pBuffer->WriteString(int_to_str[cfg.large_window]);

				// what to execute after the window is closed
				//  0 for no command
				//  1 for joingame
				//  2 for changeteam
				//  3 for impulse 101
				//  4 for mapinfo
				//  5 for closed_htmlpage
				//  6 for chooseteam
				pBuffer->WriteString("cmd");
				pBuffer->WriteString("5"); 
			}
			m_pEngine->MessageEnd();
		}
	}
}

CBaseEntity *CSizzPluginContext::BaseEntityFromBaseHandle( const CBaseHandle *pHandle )
{
	if (pHandle && pHandle->IsValid())
	{
		int entindex = pHandle->GetEntryIndex();
		return BaseEntityFromEntIndex(entindex);
	}
	return nullptr;
}

CBaseEntity *CSizzPluginContext::BaseEntityFromEntIndex( int ent_index )
{
	edict_t *pEdict = EdictFromEntIndex(ent_index);
	return SCHelpers::EdictToBaseEntity(pEdict);
}

IHandleEntity *CSizzPluginContext::HandleEntityFromEntIndex( int ent_index )
{
	edict_t *pEdict = EdictFromEntIndex(ent_index);
	if (pEdict)
	{
		IServerEntity *pServerEnt = pEdict->GetIServerEntity();
		if (pServerEnt)
		{
			return static_cast<IHandleEntity*>(pServerEnt);
		}
	}
	return nullptr;
}

int CSizzPluginContext::GetEntityByClassName( const char *name, edict_t *out[], int max_out )
{
	int cur_ents = 0;
	if (name && out)
	{
		int max_ents = m_pGlobals->maxEntities;
		for (int i = 0; ((i < max_ents) && (cur_ents < max_out)); ++i)
		{
			edict_t *pEdict = EdictFromEntIndex(i);
			if (pEdict && !pEdict->IsFree())
			{
				IServerNetworkable *pNetworkable = pEdict->GetNetworkable();
				if (pNetworkable)
				{
					ServerClass *pServerClass = pNetworkable->GetServerClass();
					if (pServerClass && SCHelpers::FStrEq(pServerClass->GetName(), name))
					{
						out[cur_ents++] = pEdict;
					}
				}
			}
		}
	}
	return cur_ents;
}

edict_t *CSizzPluginContext::GetEntityByClassName( const char *name, int start_ent /*= 0*/ )
{
	if (name)
	{
		int max_ents = m_pGlobals->maxEntities;
		for (int i = start_ent; i < max_ents; ++i)
		{
			edict_t *pEdict = EdictFromEntIndex(i);
			if (pEdict && !pEdict->IsFree())
			{
				IServerNetworkable *pNetworkable = pEdict->GetNetworkable();
				if (pNetworkable)
				{
					ServerClass *pServerClass = pNetworkable->GetServerClass();
					if (pServerClass && SCHelpers::FStrEq(pServerClass->GetName(), name))
					{
						return pEdict;
					}
				}
			}
		}
	}
	return nullptr;
}

const char *CSizzPluginContext::GetPlayerIPPortString( int ent_index )
{
	INetChannelInfo *pNetInfo = m_pEngine->GetPlayerNetInfo(ent_index);
	if (pNetInfo)
	{
		return pNetInfo->GetAddress();
	}
	return "";
}

bool CSizzPluginContext::CanRecordDemo()
{
	for (int i = 1; i <= GetMaxClients(); ++i)
	{
		edict_t *pEdict = EdictFromEntIndex(i);
		IPlayerInfo *pInfo = m_pPlayerInfoManager->GetPlayerInfo(pEdict);
		if (pInfo && pInfo->IsHLTV())
		{
			return true;
		}
	}
	return false;
}

void CSizzPluginContext::LevelShutdown()
{
	m_edict_list = nullptr;

	m_pUserIDTracker->Reset();
}

int CSizzPluginContext::ClientActive( const edict_t *pEdict )
{
	if (pEdict)
	{
		int userid = m_pEngine->GetPlayerUserId(pEdict);
		if (userid != -1)
		{
			int ent_index = SCHelpers::EntIndexFromEdict(pEdict);
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
		int ent_index = SCHelpers::EntIndexFromEdict(pEdict);
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

void CSizzPluginContext::ServerActivate( edict_t *pEdictList, int edictCount, int clientMax )
{
	m_edict_list = pEdictList;
}

void CSizzPluginContext::GameFrame( bool simulating )
{
	m_tickcount = m_pGlobals->tickcount;
	m_flTime = m_pGlobals->realtime;
	m_max_clients = m_pGlobals->maxClients;

	m_pCallQueue->callQueueGameFrame();
}

edict_t *CSizzPluginContext::EdictFromEntIndex( int ent_index )
{
	edict_t *pEdict = nullptr;
	// first try using the list
	if (m_edict_list)
	{
		if ((0 <= ent_index) && (ent_index < MAX_EDICTS))
		{
			pEdict = (m_edict_list + ent_index);
			if (pEdict->IsFree())
			{
				pEdict = nullptr;
			}
		}
	}
	else
	{
		// try to get the list
		m_edict_list = m_pEngine->PEntityOfEntIndex(0);
		if (m_edict_list)
		{
			pEdict = EdictFromEntIndex(ent_index);
		}
		else // no reason why this should happen
		{
			// fallback engine call
			pEdict = m_pEngine->PEntityOfEntIndex(ent_index);
		}
	}
	return pEdict;
}
