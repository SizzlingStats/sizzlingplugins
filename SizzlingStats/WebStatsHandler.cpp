
#include "WebStatsHandler.h"

CWebStatsHandler::CWebStatsHandler()
{
	m_dataListMutex.Unlock();
	m_hostInfoMutex.Unlock();
}

CWebStatsHandler::~CWebStatsHandler()
{
}

void CWebStatsHandler::ClearPlayerStats()
{
	m_dataListMutex.Lock();
	m_webStats.RemoveAll();
	m_dataListMutex.Unlock();
}

void CWebStatsHandler::EnqueuePlayerStats(playerWebStats_t const &item)
{
	m_dataListMutex.Lock();
	m_webStats.AddToTail(item);
	m_dataListMutex.Unlock();
}

void CWebStatsHandler::SetHostData(hostInfo_t const &info)
{
	m_hostInfoMutex.Lock();

	V_strncpy(m_hostInfo.m_hostname, info.m_hostname, 64);
	V_strncpy(m_hostInfo.m_mapname, info.m_mapname, 64);
	V_strncpy(m_hostInfo.m_bluname, info.m_bluname, 32);
	V_strncpy(m_hostInfo.m_redname, info.m_redname, 32);
	m_hostInfo.m_bluscore = info.m_bluscore;
	m_hostInfo.m_redscore = info.m_redscore;

	m_hostInfoMutex.Unlock();
}

void CWebStatsHandler::GetMatchUrl( char *str, int maxlen )
{
	m_responseInfo.GetMatchUrl(str, maxlen);
}

bool CWebStatsHandler::HasMatchUrl()
{
	return m_responseInfo.HasMatchUrl();
}

void CWebStatsHandler::SendStatsToWeb()
{
	m_queue.EnqueueItem(CreateFunctor(this, &CWebStatsHandler::SendStatsToWebInternal));
}

void CWebStatsHandler::SendGameOverEvent(double flMatchDuration)
{
	m_queue.EnqueueItem(CreateFunctor(this, &CWebStatsHandler::SendGameOverEventInternal, flMatchDuration));
}

size_t CWebStatsHandler::read_callback(void *ptr, size_t size, size_t nmemb, void *userdata)
{
	CUtlBuffer *pBuffer = static_cast<CUtlBuffer*>(userdata);
	const int maxSize = size*nmemb;
	if ( pBuffer->GetBytesRemaining() >= maxSize )
	{
		pBuffer->Get( ptr, maxSize );
		return maxSize;
	}
	else
	{
		const int bytesRemaining = pBuffer->GetBytesRemaining();
		pBuffer->Get( ptr, bytesRemaining );
		return bytesRemaining;
	}
}

size_t CWebStatsHandler::header_read_callback(void *ptr, size_t size, size_t nmemb, void *userdata)
{
	const int maxSize = size*nmemb;
	char *data = (char*)ptr;

	if ( V_strstr( data, "sessionid: " ) )
	{
		responseInfo *pInfo = static_cast<responseInfo*>(userdata);
		const char *pStart = V_strstr(data, " ") + 1;
		pInfo->SetSessionId(pStart, V_strlen(pStart)-1);
	}
	else if ( V_strstr( data, "matchurl: " ) )
	{
		responseInfo *pInfo = static_cast<responseInfo*>(userdata);
		const char *pStart = V_strstr(data, " ") + 1;
		pInfo->SetMatchUrl(pStart, V_strlen(pStart)-1);
	}
		
	return maxSize;
}

void CWebStatsHandler::SendStatsToWebInternal()
{
	CUtlBuffer postString;
	char sessionId[64] = {0};
	m_responseInfo.GetSessionId(sessionId, 64);
		
	m_hostInfoMutex.Lock();
	hostInfo_t tempInfo(m_hostInfo);
	m_hostInfoMutex.Unlock();

	m_dataListMutex.Lock();
	producePostString( tempInfo, m_webStats, sessionId, postString );
	m_dataListMutex.Unlock();

	ClearPlayerStats();

	CCurlConnection connection;
	if (connection.Initialize())
	{
		connection.SetHttpSendType(CCurlConnection::POST);
		connection.AddHeader("Transfer-Encoding: chunked");
		connection.AddHeader("Content-type: application/json");
		connection.AddHeader("Expect:");
		connection.AddHeader(HEADER_SIZZSTATS_VERSION);

		if (sessionId[0] != '\0')
		{
			char temp[128] = {};
			V_snprintf( temp, 128, "sessionid: %s", sessionId);
			connection.AddHeader(temp);
		}

		connection.SetUrl(WEB_SERVER_IP);
		connection.SetBodyReadFunction(read_callback);
		connection.SetBodyReadUserdata(&postString);
		connection.SetHeaderReadFunction(header_read_callback);
		connection.SetHeaderReadUserdata(&m_responseInfo);

		connection.Perform();
		connection.Close();
	}
}

void CWebStatsHandler::SendGameOverEventInternal(double flMatchDuration)
{
	if (m_responseInfo.HasSessionId())
	{
		CCurlConnection connection;
		if (connection.Initialize())
		{
			connection.SetHttpSendType(CCurlConnection::POST);
			connection.AddHeader(HEADER_SIZZSTATS_VERSION);

			{
				char lengthHeader[64] = {};
				V_snprintf( lengthHeader, 64, "matchduration: %.0f", flMatchDuration );
				connection.AddHeader(lengthHeader);
			}

			{
				char sessionId[64] = {};
				m_responseInfo.GetSessionId(sessionId, 64);
				char temp[128] = {};
				V_snprintf( temp, 128, "sessionid: %s", sessionId);

				connection.AddHeader(temp);
			}

			connection.SetOption(CURLOPT_POSTFIELDS, const_cast<char*>(""));
			connection.SetUrl(GAMEOVER_URL);

			CURLcode res = connection.Perform();
			connection.Close();
		}
		m_responseInfo.ResetSessionId();
	}
}
