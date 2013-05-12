
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
	KeyValues *kv = *SCHelpers::ByteOffsetFromPointer<KeyValues*>(pEvent, 8);
	if (kv)
	{
		const char *name = kv->GetName();
		if (SCHelpers::FStrCmp(pEvent->GetName(), name))
		{
			std::shared_ptr<SizzEvent::SizzEvent> pSizzEvent(new SizzEvent::SizzEvent());

			pSizzEvent->set_message_version(1);
			pSizzEvent->set_timestamp(server_tick);
			pSizzEvent->set_name(name);
		
			FOR_EACH_SUBKEY(kv, kvSubKey)
			{
				SizzEvent::SizzEvent_EventData *pData = pSizzEvent->add_event_data();
				pData->set_data_key(kvSubKey->GetName());

				KeyValues::types_t type = kvSubKey->GetDataType();
				switch (type)
				{
				case KeyValues::TYPE_STRING:
					{
						pData->set_data_value(kvSubKey->GetString());
					}
					break;
				case KeyValues::TYPE_INT:
					{
						int num = kvSubKey->GetInt();
						pData->set_data_value(&num, sizeof(int));
					}
					break;
				case KeyValues::TYPE_FLOAT:
					{
						float num = kvSubKey->GetFloat();
						pData->set_data_value(&num, sizeof(float));
					}
					break;
				default:
					break;
				}
			}

			m_send_queue.EnqueueFunctor(CreateFunctor(this, &CEventSender::SendEventInternal, pSizzEvent));
		}
	}
}

void CEventSender::SendEventInternal( std::shared_ptr<SizzEvent::SizzEvent> pEvent )
{
	AUTO_LOCK(m_connection_lock);
	if (m_connection.IsConnected())
	{
		m_send_buff.EnsureCapacity(pEvent->ByteSize());
		int size = m_send_buff.NumAllocated();
		pEvent->SerializeToArray(m_send_buff.Base(), size);

		m_connection.Send(m_send_buff.Base(), size, 10);
		std::string asdf(m_send_buff.Base(), size);
		Msg("%s\n", asdf.c_str()); 
	}
	else
	{
		m_send_queue.EnqueueFunctor(CreateFunctor(this, &CEventSender::SendEventInternal, pEvent));
	}
}
