
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

#include "EventSender.h"
#include "SizzEvent.pb.h"

#include "igameevents.h"
#include "SC_helpers.h"
#include "KeyValues.h"

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

bool CEventSender::BeginConnection( const char *url )
{
	AUTO_LOCK(m_connection_lock);
	return m_connection.Connect(url);
}

void CEventSender::EndConnection()
{
	AUTO_LOCK(m_connection_lock);
	m_connection.Disconnect();
}

void CEventSender::SendEvent( IGameEvent *pEvent, unsigned int server_tick )
{
	// these are the event data keyvalues
	//static const int KV_VALUES_OFFSET = 8;
	//KeyValues *kv = *SCHelpers::ByteOffsetFromPointer<KeyValues*>(pEvent, KV_VALUES_OFFSET);

	static const int EVENT_DESC_OFFSET = 4;
	static const int KV_TYPES_OFFSET = 36;

	// these are the event data TYPE keyvalues
	char *event_desc = *SCHelpers::ByteOffsetFromPointer<char*>(pEvent, EVENT_DESC_OFFSET);
	KeyValues *kv_types = *SCHelpers::ByteOffsetFromPointer<KeyValues*>(event_desc, KV_TYPES_OFFSET);
	
	if (kv_types && SCHelpers::FStrCmp("descriptor", kv_types->GetName()))
	{
		std::shared_ptr<SizzEvent::SizzEvent> pSizzEvent(new SizzEvent::SizzEvent());
		pSizzEvent->set_event_id(1);
			
		FOR_EACH_SUBKEY(kv_types, kvSubKey)
		{
			SizzEvent::SizzEvent_EventData *pData = pSizzEvent->add_event_data();

			int type = kvSubKey->GetInt();
			const char *value_name = kvSubKey->GetName();
			switch (type)
			{
			case EVENT_TYPE_STRING:
				{
					const char *val = pEvent->GetString(value_name);
					pData->set_value_string(val);
				}
				break;
			case EVENT_TYPE_FLOAT:
				{
					float val =  pEvent->GetFloat(value_name);
					pData->set_value_float(val);
				}
				break;
			case EVENT_TYPE_LONG:
				{
					int val = pEvent->GetInt(value_name);
					pData->set_value_float(val);
				}
				break;
			case EVENT_TYPE_SHORT:
				{
					short val = pEvent->GetInt(value_name);
					pData->set_value_short(val);
				}
				break;
			case EVENT_TYPE_BYTE:
				{
					char val = pEvent->GetInt(value_name);
					pData->set_value_byte(val);
				}
				break;
			case EVENT_TYPE_BOOL:
				{
					bool val = pEvent->GetBool(value_name);
					pData->set_value_bool(val);
				}
				break;
			default:
				break;
			}
		}
			
		m_send_queue.EnqueueFunctor(CreateFunctor(this, &CEventSender::SendEventInternal, pSizzEvent));
	}
}

void CEventSender::SendEventInternal( std::shared_ptr<SizzEvent::SizzEvent> pEvent )
{
	AUTO_LOCK(m_connection_lock);
	if (m_connection.IsConnected())
	{
		int size = pEvent->ByteSize();
		m_send_buff.EnsureCapacity(size);
		pEvent->SerializeToArray(m_send_buff.Base(), size);
		m_connection.Send(m_send_buff.Base(), size, 10);
	}
	else
	{
		m_send_queue.EnqueueFunctor(CreateFunctor(this, &CEventSender::SendEventInternal, pEvent));
	}
}