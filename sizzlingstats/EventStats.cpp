
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

#include "SizzEvent.pb.h"
#include "EventStats.h"

#include "SizzPluginContext.h"
#include "game/server/iplayerinfo.h"
#include "igameevents.h"
#include "SC_helpers.h"
#include "KeyValues.h"
#include "TFPlayerWrapper.h"

#define FOR_EACH_SUBKEY( kvRoot, kvSubKey ) \
for ( KeyValues * kvSubKey = kvRoot->GetFirstSubKey(); kvSubKey != NULL; kvSubKey = kvSubKey->GetNextKey() )

enum EVENT_DATA_TYPES
{
	EVENT_TYPE_NONE = 0,
	EVENT_TYPE_STRING,	// null terminated string
	EVENT_TYPE_FLOAT,	// 4 bytes
	EVENT_TYPE_LONG,	// 4 bytes
	EVENT_TYPE_SHORT,	// 2 bytes
	EVENT_TYPE_BYTE,	// 1 byte
	EVENT_TYPE_BOOL,	// 1 byte for simplicity
	NUM_EVENT_TYPES
};

bool CEventStats::Initialize()
{
	return m_event_sender.BeginConnection( "dy-dx.com:8007" );
}

void CEventStats::Shutdown()
{
	m_event_sender.EndConnection();
}

void CEventStats::OnTournamentMatchStart( CSizzPluginContext *pPluginContext, unsigned int server_tick )
{
	{
		CSizzEvent event(m_event_sender.AllocEvent(), server_tick);
		event.SetName("ss_tournament_match_start");
		event.SetString("hostname", pPluginContext->GetHostName());
		event.SetString("mapname", pPluginContext->GetMapName());
		event.SetString("bluname", pPluginContext->GetBluTeamName());
		event.SetString("redname", pPluginContext->GetRedTeamName());
		//event.SetString("ip", ipppppppppp);
		//event.SetShort("port", poooooort);
		m_event_sender.SendEvent(&event);
	}

	int max_clients = pPluginContext->GetMaxClients();
	for (int i = 1; i <= max_clients; ++i)
	{
		IPlayerInfo *pInfo = pPluginContext->GetPlayerInfo(i);
		CTFPlayerWrapper player(pPluginContext->BaseEntityFromEntIndex(i));
		if (pInfo && pInfo->IsConnected())
		{
			CSizzEvent event(m_event_sender.AllocEvent(), server_tick);
			event.SetName("ss_player_info");
			event.SetString("name", pInfo->GetName());
			event.SetShort("userid", pPluginContext->UserIDFromEntIndex(i));
			event.SetByte("entindex", i);
			event.SetString("steamid", pInfo->GetNetworkIDString());
			event.SetByte("teamid", pInfo->GetTeamIndex());
			event.SetString("netaddr", pPluginContext->GetPlayerIPPortString(i));
			event.SetByte("class", player.GetClass());
			event.SetBool("isstv", pInfo->IsHLTV());
			event.SetBool("isbot", pInfo->IsFakeClient());
			event.SetBool("isreplay", pInfo->IsReplay());

			m_event_sender.SendEvent(&event);
		}
	}
}

void CEventStats::OnTournamentMatchEnd( CSizzPluginContext *pPluginContext, unsigned int server_tick )
{
	SendNamedEvent("ss_tournament_match_end", server_tick);
}

void CEventStats::OnFireGameEvent( IGameEvent *pEvent, unsigned int server_tick )
{
	CSizzEvent event(m_event_sender.AllocEvent(), server_tick);
	if (ConvertEvent(pEvent, &event))
	{
		m_event_sender.SendEvent(&event);
	}
}

bool CEventStats::ConvertEvent( IGameEvent *pEvent, CSizzEvent *event )
{
	// these are the event data keyvalues
	//static const int KV_VALUES_OFFSET = 8;
	//KeyValues *kv = *SCHelpers::ByteOffsetFromPointer<KeyValues**>(pEvent, KV_VALUES_OFFSET);

	static const int EVENT_DESC_OFFSET = 4;
	static const int KV_TYPES_OFFSET = 36;

	// these are the event data TYPE keyvalues
	char *event_desc = *SCHelpers::ByteOffsetFromPointer<char**>(pEvent, EVENT_DESC_OFFSET);
	KeyValues *kv_types = *SCHelpers::ByteOffsetFromPointer<KeyValues**>(event_desc, KV_TYPES_OFFSET);
	
	if (kv_types && SCHelpers::FStrCmp("descriptor", kv_types->GetName()))
	{
		SizzEvent::SizzEvent *pSizzEvent = event->GetEvent();
		pSizzEvent->set_event_name(pEvent->GetName());
			
		FOR_EACH_SUBKEY(kv_types, kvSubKey)
		{
			SizzEvent::SizzEvent_EventData *pData = pSizzEvent->add_event_data();

			int type = kvSubKey->GetInt();
			pData->set_value_type(static_cast<SizzEvent::SizzEvent_EventData_DATA_TYPE>(type));
			const char *key_name = kvSubKey->GetName();
			pData->set_key_name(key_name);
			switch (type)
			{
			case EVENT_TYPE_STRING:
				{
					const char *val = pEvent->GetString(key_name);
					pData->set_value_string(val);
				}
				break;
			case EVENT_TYPE_FLOAT:
				{
					float val =  pEvent->GetFloat(key_name);
					pData->set_value_float(val);
				}
				break;
			case EVENT_TYPE_LONG:
				{
					int val = pEvent->GetInt(key_name);
					pData->set_value_long(val);
				}
				break;
			case EVENT_TYPE_SHORT:
				{
					short val = pEvent->GetInt(key_name);
					pData->set_value_short(val);
				}
				break;
			case EVENT_TYPE_BYTE:
				{
					char val = pEvent->GetInt(key_name);
					pData->set_value_byte(val);
				}
				break;
			case EVENT_TYPE_BOOL:
				{
					bool val = pEvent->GetBool(key_name);
					pData->set_value_bool(val);
				}
				break;
			default:
				break;
			}
		}
	}
	return (event->GetEvent() != nullptr);
}

void CEventStats::SendNamedEvent( const char *event_name, unsigned int server_tick )
{
	CSizzEvent event(m_event_sender.AllocEvent(), server_tick);
	event.SetName(event_name);

	m_event_sender.SendEvent(&event);
}
