
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

#pragma once

#include "ThreadFunctorQueue.h"
#include "curlconnection.h"
#include "utlmemory.h"
#include "strtools.h"

class CTCPSocket
{
public:
	CTCPSocket():
		m_pCurl(nullptr),
		m_socket(static_cast<unsigned int>(-1))
	{
	}

	~CTCPSocket()
	{
		Disconnect();
	}

	bool Connect( const char *url )
	{
		if (url && !m_pCurl && ((m_pCurl = curl_easy_init()) != nullptr))
		{
			curl_easy_setopt(m_pCurl, CURLOPT_URL, url);
			curl_easy_setopt(m_pCurl, CURLOPT_CONNECT_ONLY, 1L);
			curl_easy_setopt(m_pCurl, CURLOPT_CONNECTTIMEOUT, 60L);
			curl_easy_setopt(m_pCurl, CURLOPT_TIMEOUT, 60L);
			curl_easy_setopt(m_pCurl, CURLOPT_NOSIGNAL, 1L);

			curl_easy_perform(m_pCurl);
			curl_easy_getinfo(m_pCurl, CURLINFO_LASTSOCKET, &m_socket);
			
			bool success = (m_socket != -1);
			if (!success)
			{
				Disconnect();
			}
			return success;
		}
		return false;
	}

	void Disconnect()
	{
		if (m_pCurl)
		{
			curl_easy_cleanup(m_pCurl);
			m_pCurl = nullptr;
			m_socket = static_cast<unsigned int>(-1);
		}
	}

	bool IsConnected() const
	{
		return (m_pCurl != nullptr);
	}

	unsigned int Send( const void *data, unsigned int num_bytes, int timeout )
	{
		if (m_pCurl && data)
		{
			socket_wait(m_socket, false, timeout);
			size_t bytes_sent = 0;
			CURLcode c = curl_easy_send(m_pCurl, data, num_bytes, &bytes_sent);
			return (c == CURLE_OK) ? bytes_sent : 0;
		}
		return 0;
	}

	unsigned int Recv( void *out, unsigned int max_bytes, int timeout )
	{
		if (m_pCurl && out)
		{
			socket_wait(m_socket, true, timeout);
			size_t bytes_recv = 0;
			CURLcode c = curl_easy_recv(m_pCurl, out, max_bytes, &bytes_recv);
			return (c == CURLE_OK) ? bytes_recv : 0;
		}
		return 0;
	}

private:
	static int socket_wait( curl_socket_t socket, bool on_recv, long timeout_s )
	{
		struct timeval tv;

		tv.tv_sec = timeout_s;
		tv.tv_usec = 0;

		fd_set in, out, err;
		FD_ZERO(&in);
		FD_ZERO(&out);
		FD_ZERO(&err);

		FD_SET(socket, &err);
		if (on_recv)
		{
			FD_SET(socket, &in);
		}
		else
		{
			FD_SET(socket, &out);
		}

		return select(socket + 1, &in, &out, &err, &tv);
	}

private:
	CURL *m_pCurl;
	curl_socket_t m_socket;
};

class IGameEvent;
class CEventSender;
namespace SizzEvent { class SizzEvent; }

class CSizzEvent
{
public:
	CSizzEvent( SizzEvent::SizzEvent *pEvent, unsigned int server_tick );

	void SetName( const char *name );

	void SetString( const char *name, const char *value );
	void SetFloat( const char *name, float value );
	void SetInt( const char *name, int value );
	void SetShort( const char *name, int value );
	void SetByte( const char *name, int value );
	void SetBool( const char *name, bool value );

	SizzEvent::SizzEvent *GetEvent() const
	{
		return m_pEvent;
	}

private:
	SizzEvent::SizzEvent *m_pEvent;
};

class CEventSender
{
public:
	bool BeginConnection( const char *url );
	void EndConnection();

	static SizzEvent::SizzEvent *AllocEvent();

	// sends and frees the event
	void SendEvent( CSizzEvent *pEvent );

	void SendNamedEvent( const char *event_name, unsigned int server_tick );

private:
	void SendEventInternal( const SizzEvent::SizzEvent *pEvent );

private:
	CSizzFuncQueueThread m_send_queue;
	CTCPSocket m_connection;
	sizz::CThreadMutex m_connection_lock;
	CUtlMemory<char> m_send_buff;
};
