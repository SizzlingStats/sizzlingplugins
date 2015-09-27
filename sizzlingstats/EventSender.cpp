
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

#include "SizzEvent.pb.h"
#include "EventSender.h"

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

CSizzEvent::CSizzEvent( SizzEvent::SizzEvent *pEvent, unsigned int server_tick ):
	m_pEvent(pEvent)
{
	Assert(pEvent);
	m_pEvent->set_event_timestamp(server_tick);
}

void CSizzEvent::SetName( const char *name )
{
	m_pEvent->set_event_name(name);
}

void CSizzEvent::SetString( const char *name, const char *value )
{
	if (name && value)
	{
		using namespace SizzEvent;
		SizzEvent_EventData *pData = m_pEvent->add_event_data();
		pData->set_value_type(static_cast<SizzEvent_EventData_DATA_TYPE>(EVENT_TYPE_STRING));
		pData->set_key_name(name);
		pData->set_value_string(value);
	}
}

void CSizzEvent::SetFloat( const char *name, float value )
{
	if (name)
	{
		using namespace SizzEvent;
		SizzEvent_EventData *pData = m_pEvent->add_event_data();
		pData->set_value_type(static_cast<SizzEvent_EventData_DATA_TYPE>(EVENT_TYPE_FLOAT));
		pData->set_key_name(name);
		pData->set_value_float(value);
	}
}

void CSizzEvent::SetInt( const char *name, int value )
{
	if (name)
	{
		using namespace SizzEvent;
		SizzEvent_EventData *pData = m_pEvent->add_event_data();
		pData->set_value_type(static_cast<SizzEvent_EventData_DATA_TYPE>(EVENT_TYPE_LONG));
		pData->set_key_name(name);
		pData->set_value_long(value);
	}
}

void CSizzEvent::SetShort( const char *name, int value )
{
	if (name)
	{
		using namespace SizzEvent;
		SizzEvent_EventData *pData = m_pEvent->add_event_data();
		pData->set_value_type(static_cast<SizzEvent_EventData_DATA_TYPE>(EVENT_TYPE_SHORT));
		pData->set_key_name(name);
		pData->set_value_short(value);
	}
}

void CSizzEvent::SetByte( const char *name, int value )
{
	if (name)
	{
		using namespace SizzEvent;
		SizzEvent_EventData *pData = m_pEvent->add_event_data();
		pData->set_value_type(static_cast<SizzEvent_EventData_DATA_TYPE>(EVENT_TYPE_BYTE));
		pData->set_key_name(name);
		pData->set_value_byte(value);
	}
}

void CSizzEvent::SetBool( const char *name, bool value )
{
	if (name)
	{
		using namespace SizzEvent;
		SizzEvent_EventData *pData = m_pEvent->add_event_data();
		pData->set_value_type(static_cast<SizzEvent_EventData_DATA_TYPE>(EVENT_TYPE_BOOL));
		pData->set_key_name(name);
		pData->set_value_bool(value);
	}
}

bool CEventSender::BeginConnection( const char *url )
{
	sizz::CAutoLock lk(m_connection_lock);
	return m_connection.Connect(url);
}

void CEventSender::EndConnection()
{
	sizz::CAutoLock lk(m_connection_lock);
	m_connection.Disconnect();
}

SizzEvent::SizzEvent *CEventSender::AllocEvent()
{
	return new SizzEvent::SizzEvent();
}

void CEventSender::SendEvent( CSizzEvent *pEvent )
{
	m_send_queue.EnqueueFunctor(CreateFunctor(this, &CEventSender::SendEventInternal, pEvent->GetEvent()));
}

void CEventSender::SendNamedEvent( const char *event_name, unsigned int server_tick )
{
	CSizzEvent event(AllocEvent(), server_tick);
	event.SetName(event_name);

	SendEvent(&event);
}

void CEventSender::SendEventInternal( const SizzEvent::SizzEvent *pEvent )
{
	sizz::CAutoLock lk(m_connection_lock);
	if (m_connection.IsConnected())
	{
		int size = pEvent->ByteSize();
		int total_size = size + sizeof(short);
		m_send_buff.EnsureCapacity(total_size);
		*reinterpret_cast<short*>(m_send_buff.Base()) = BigShort(static_cast<short>(size));
		pEvent->SerializeToArray(m_send_buff.Base() + sizeof(short), size);
		m_connection.Send(m_send_buff.Base(), total_size, 10);
	}/* TODO: replace this with retry try count
	else
	{
		m_send_queue.EnqueueFunctor(CreateFunctor(this, &CEventSender::SendEventInternal, pEvent));
	}*/
	delete pEvent;
}
