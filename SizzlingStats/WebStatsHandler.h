////////////////////////////////////////////////////////////////////////////////
// Filename: WebStatsHandler.h
////////////////////////////////////////////////////////////////////////////////
#ifndef WEB_STATS_HANDLER_H
#define WEB_STATS_HANDLER_H

#include "queuethread.h"
#include "JsonUtils.h"
#include "utlbuffer.h"

#include "ThreadCallQueue.h"
#include "playerdata.h"
#include "eiface.h"

#include "curlconnection.h"

#define WEB_SERVER_IP "206.253.166.149/api/stats"

extern CTSCallQueue		*g_pTSCallQueue;
extern IVEngineServer	*pEngine;

// don't conflict with player_info_t from cdll_int.h
typedef struct playerInfo_s
{
	char m_name[32];
	//uint64 m_steamid;
	char m_steamid[32];
	unsigned char m_teamid;
} playerInfo_t;

typedef struct hostInfo_s
{
	#pragma warning( push )
	#pragma warning( disable : 4351 )
	hostInfo_s():
		m_hostname(),
		m_mapname(),
		m_bluname(),
		m_redname(),
		m_bluscore(0),
		m_redscore(0)
	{
	}
	#pragma warning( pop )

	hostInfo_s(const char *hostname,
			const char *mapname,
			const char *bluname,
			const char *redname,
			unsigned int bluscore,
			unsigned int redscore):
		m_bluscore(bluscore),
		m_redscore(redscore)
	{
		V_strncpy(m_hostname, hostname, 64);
		V_strncpy(m_mapname, mapname, 64);
		V_strncpy(m_bluname, bluname, 32);
		V_strncpy(m_redname, redname, 32);
	}

	char m_hostname[64];
	char m_mapname[64];
	char m_bluname[32];
	char m_redname[32];
	unsigned char m_bluscore;
	unsigned char m_redscore;
} hostInfo_t;

typedef struct playerWebStats_s
{
	playerInfo_t	m_playerInfo;
	ScoreData		m_scoreData;
} playerWebStats_t;

static void producePostString(const hostInfo_t &host, const CUtlVector<playerWebStats_t> &data, const char *sessionId, CUtlBuffer &buff)
{
	buff.SetBufferType(true, true);
	
	// need to rewrite the json stuff recursively
	{
		CJsonObject outer(buff);
		{
			CJsonObject temp(buff, "stats");
			//temp.InsertKV("sessionid", sessionId);
			temp.InsertKV("map", host.m_mapname);
			temp.InsertKV("hostname", host.m_hostname);
			temp.InsertKV("bluname", host.m_bluname);
			temp.InsertKV("redname", host.m_redname);
			temp.InsertKV("bluscore", host.m_bluscore);
			temp.InsertKV("redscore", host.m_redscore);
			buff.PutString(",");
			{
				CJsonArray temp2(buff, "players");
				for (int i = 0; i < data.Count(); ++i)
				{
					if (i > 0)
					{
						buff.PutString(",");
					}
					CJsonObject temp3(buff);
					temp3.InsertKV("steamid", data[i].m_playerInfo.m_steamid);
					temp3.InsertKV("team", data[i].m_playerInfo.m_teamid);
					temp3.InsertKV("name", data[i].m_playerInfo.m_name);

					temp3.InsertKV("kills", data[i].m_scoreData.getStat(Kills));
					temp3.InsertKV("deaths", data[i].m_scoreData.getStat(Deaths));
					temp3.InsertKV("damage", data[i].m_scoreData.getStat(DamageDone));
					temp3.InsertKV("heals", data[i].m_scoreData.getStat(HealsReceived));
					temp3.InsertKV("medkills", data[i].m_scoreData.getStat(MedPicks));
					temp3.InsertKV("assists", data[i].m_scoreData.getStat(KillAssists));
				}
			}
		}
	}
}

struct responseInfo
{
	#pragma warning( push )
	#pragma warning( disable : 4351 )
	responseInfo(): matchUrl(), sessionId()
	{
		matchUrlMutex.Unlock();
		sessionIdMutex.Unlock();
	}
	#pragma warning( pop )

	void SetSessionId( const char *id, int lengthToCopy )
	{
		int size = lengthToCopy > 64 ? 64 : lengthToCopy;
		sessionIdMutex.Lock();
		V_strncpy(sessionId, id, size);
		sessionIdMutex.Unlock();
	}

	void GetSessionId( char *str, int maxlen )
	{
		if (str)
		{
			sessionIdMutex.Lock();
			V_strncpy(str, sessionId, maxlen);
			sessionIdMutex.Unlock();
		}
	}

	void ResetSessionId()
	{
		sessionIdMutex.Lock();
		sessionId[0] = '\0';
		sessionIdMutex.Unlock();
	}

	bool HasSessionId()
	{
		sessionIdMutex.Lock();
		bool result = sessionId[0] == '\0' ? false : true;
		sessionIdMutex.Unlock();
		return result;
	}

	bool HasMatchUrl()
	{
		matchUrlMutex.Lock();
		bool result = matchUrl[0] == '\0' ? false : true;
		matchUrlMutex.Unlock();
		return result;
	}

	void SetMatchUrl( const char *url, int lengthToCopy )
	{
		int size = lengthToCopy > 128 ? 128 : lengthToCopy;
		matchUrlMutex.Lock();
		V_strncpy(matchUrl, url, size);
		matchUrlMutex.Unlock();
	}

	void GetMatchUrl( char *str, int maxlen )
	{
		if (str)
		{
			matchUrlMutex.Lock();
			V_strncpy(str, matchUrl, maxlen);
			matchUrlMutex.Unlock();
		}
	}

	void ResetMatchUrl()
	{
		matchUrlMutex.Lock();
		matchUrl[0] = '\0';
		matchUrlMutex.Unlock();
	}

private:
	CThreadMutexPthread		matchUrlMutex;
	CThreadMutexPthread		sessionIdMutex;
	char					matchUrl[128];
	char					sessionId[64];
};

class CWebStatsHandlerThread: public CThread
{
public:
	#pragma warning( push )
	#pragma warning( disable : 4351 )
	CWebStatsHandlerThread():
		m_responseInfo()
	{
		m_dataListMutex.Unlock();
		m_hostInfoMutex.Unlock();
	}
	#pragma warning( pop )

	virtual ~CWebStatsHandlerThread()
	{
	}

	void SetHostData(hostInfo_t const &info)
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

	void ClearQueue()
	{
		m_dataListMutex.Lock();
		m_webStats.RemoveAll();
		m_dataListMutex.Unlock();
	}

	void EnqueueItem(playerWebStats_t const &item)
	{
		m_dataListMutex.Lock();
		m_webStats.AddToTail( item );
		m_dataListMutex.Unlock();
	}

	void StartThread()
	{
		if (IsAlive())
		{
			Join();
		}
		Start();
	}

	// used for sending the stats data
	static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *userdata)
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

	// used for grabbing the sessionid and matchurl
	static size_t header_read_callback(void *ptr, size_t size, size_t nmemb, void *userdata)
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

	virtual int Run()
	{
		g_pTSCallQueue->EnqueueFunctor( CreateFunctor(pEngine, &IVEngineServer::LogPrint, (const char *)"testing thread log\n") );

		CUtlBuffer postString;
		char sessionId[64] = {0};
		m_responseInfo.GetSessionId(sessionId, 64);
		
		m_hostInfoMutex.Lock();
		hostInfo_t tempInfo(m_hostInfo);
		m_hostInfoMutex.Unlock();

		m_dataListMutex.Lock();
		producePostString( tempInfo, m_webStats, sessionId, postString );
		m_dataListMutex.Unlock();

		m_dataListMutex.Lock();
		m_webStats.RemoveAll();
		m_dataListMutex.Unlock();

		CCurlConnection connection;
		if (connection.Initialize())
		{
			connection.SetHttpSendType(CCurlConnection::POST);
			connection.AddHeader("Transfer-Encoding: chunked");
			connection.AddHeader("Content-type: application/json");
			connection.AddHeader("Expect:");
			connection.AddHeader("sizzlingstats: v0.1");

			if (sessionId)
			{
				char temp[128] = {};
				V_snprintf( temp, 128, "sessionid: %s", sessionId);
				connection.AddHeader(temp);
			}

			connection.SetUrl(WEB_SERVER_IP);
			connection.SetBodyReadFunction(read_callback);
			connection.SetBodyUserdata(&postString);
			connection.SetHeaderReadFunction(header_read_callback);
			connection.SetHeaderUserdata(&m_responseInfo);

			connection.Perform();
			connection.Close();
		}

		return 0;
	}

	void ResetSession()
	{
		//Join(); // make sure we have the sessionid in an active thread before we try to reset it
		m_responseInfo.ResetSessionId();
		//m_responseInfo.ResetMatchUrl();
	}

	void GetMatchUrl( char *str, int maxlen )
	{
		m_responseInfo.GetMatchUrl(str, maxlen);
	}

	bool HasMatchUrl()
	{
		return m_responseInfo.HasMatchUrl();
	}

	/*virtual void OnExit()
	{
	}*/

private:
	
	CUtlVector<playerWebStats_t>	m_webStats;
	hostInfo_t						m_hostInfo;
	CThreadMutexPthread				m_dataListMutex;
	CThreadMutexPthread				m_hostInfoMutex;
	responseInfo					m_responseInfo;
};

#endif // WEB_STATS_HANDLER_H
