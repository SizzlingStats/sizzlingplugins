////////////////////////////////////////////////////////////////////////////////
// Filename: WebStatsHandler.h
////////////////////////////////////////////////////////////////////////////////
#ifndef WEB_STATS_HANDLER_H
#define WEB_STATS_HANDLER_H

#include "queuethread.h"
#include "utlbuffer.h"
#include "curl/curl.h"

#include "ThreadCallQueue.h"
#include "playerdata.h"
#include "eiface.h"

#define WEB_SERVER_IP "206.253.166.149"
#define WEB_LISTEN_IP "206.253.166.149:1337"

extern CTSCallQueue		*g_pTSCallQueue;
extern IVEngineServer	*pEngine;

#define USE_CHUNKED 1

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
	#pragma warning( default : 4351 )

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

#define END_STATIC_CHAR_CONVERSION( _name, _delimiter, _escapeChar ) \
	}; \
	static CUtlCharConversion _name( _escapeChar, _delimiter, sizeof( s_pConversionArray ## _name ) / sizeof( CUtlCharConversion::ConversionArray_t ), s_pConversionArray ## _name );

BEGIN_CHAR_CONVERSION( s_conv, "\"", '\\' )
	{ '\\', "\\" },
	{ '\"', "\"" }
END_STATIC_CHAR_CONVERSION( s_conv, "\"", '\\' );

// can be a named or unnamed object
class CJsonObject
{
public:
	CJsonObject(CUtlBuffer &buff, const char *name = NULL):
		m_buff(buff),
		m_bNeedsComma(false)
	{
		if (!m_buff.IsText())
			m_buff.SetBufferType(true,true);

		if (name)
		{
			m_buff.PutDelimitedString(&s_conv, name);
			m_buff.PutString(":");
		}
		
		m_buff.PutString("{");
	}

	~CJsonObject()
	{
		m_buff.PutString("}");
	}

	void InsertKV( const char *key, const char *value )
	{
		InsertKey(key);
		m_buff.PutDelimitedString(&s_conv, value);
	}

	void InsertKV( const char *key, int value )
	{
		InsertKey(key);
		m_buff.PutInt(value);
	}

	void InsertKV( const char *key, uint64 value )
	{
		InsertKey(key);
		m_buff.Printf( "%llu", value );//.Put(&value, sizeof(uint64));
	}

private:
	void InsertKey( const char *key )
	{
		if (m_bNeedsComma)
		{
			m_buff.PutString(",");
		}
		else
		{
			m_bNeedsComma = true;
		}
		m_buff.PutDelimitedString(&s_conv, key);
		m_buff.PutString(":");
	}

private:
	CUtlBuffer &m_buff;
	bool m_bNeedsComma;
};

class CJsonArray
{
public:
	CJsonArray(CUtlBuffer &buff, const char *name):
		m_buff(buff)
	{
		if (!m_buff.IsText())
			m_buff.SetBufferType(true,true);

		m_buff.PutDelimitedString(&s_conv, name);
		m_buff.PutString(":[");
	}

	~CJsonArray()
	{
		m_buff.PutString("]");
	}

private:
	CUtlBuffer &m_buff;
};

static void producePostString(const hostInfo_t &host, const CUtlVector<playerWebStats_t> &data, const char *sessionId, CUtlBuffer &buff)
{
	buff.SetBufferType(true, true);
	
	// need to rewrite the json stuff recursively
	{
		CJsonObject outer(buff);
		{
			CJsonObject temp(buff, "stats");
			temp.InsertKV("sessionid", sessionId);
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

typedef size_t (*FnCurlCallback)(void*, size_t, size_t, void*);

enum HttpSendType
{
	POST = CURLOPT_POST,
	PUT = CURLOPT_UPLOAD,
	GET = CURLOPT_HTTPGET
};

class CHttpSend
{
public:
	CHttpSend( HttpSendType type = GET ):
		m_sendType(type)
	{
	}

	~CHttpSend()
	{
	}

	void Perform( const char *Url, curl_slist *pHeaderList, FnCurlCallback read_function, void *read_data, FnCurlCallback header_read_callback, void *header_read_data )
	{
		CURLcode res;

		CURL *curl = curl_easy_init();
		if(curl) {
			/* First set the URL that is about to receive our POST. This URL can
			just as well be a https:// URL if that is what should receive the
			data. */ 
			if (Url)
				curl_easy_setopt(curl, CURLOPT_URL, Url);

			curl_easy_setopt(curl, (CURLoption)m_sendType, 1L);

			if (read_function)
				curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_function);
			
			if (read_data)
				curl_easy_setopt(curl, CURLOPT_READDATA, read_data);

			if (header_read_callback)
				curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_read_callback);

			if (header_read_data)
				curl_easy_setopt(curl, CURLOPT_HEADERDATA, header_read_data);

			if (pHeaderList)
				res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, pHeaderList);

			/* Perform the request, res will get the return code */ 
			//try
			//{
				res = curl_easy_perform(curl);
			//} catch (...)
			//{
			//	g_pTSCallQueue->EnqueueFunctor( CreateFunctor(pEngine, &IVEngineServer::LogPrint, (const char *)"curl threw... gross\n") );
			//}

			/* always cleanup */ 
			curl_easy_cleanup(curl);
		}
	}

private:
	HttpSendType m_sendType;
};

struct responseInfo
{
	#pragma warning( disable : 4351 )
	responseInfo(): matchUrl(), sessionId()
	{
		matchUrlMutex.Unlock();
		sessionIdMutex.Unlock();
	}
	#pragma warning( default : 4351 )

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
	#pragma warning( disable : 4351 )
	CWebStatsHandlerThread():
		m_responseInfo()
	{
		m_dataListMutex.Unlock();
		m_hostInfoMutex.Unlock();
	}
	#pragma warning( default : 4351 )

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
			//int len = V_strlen(pStart);
			//char temp[64] = {0};
			//V_strncpy( temp, pStart, len-1 );
			
			pInfo->SetSessionId(pStart, V_strlen(pStart)-1);
			
		}
		else if ( V_strstr( data, "matchurl: " ) )
		{
			responseInfo *pInfo = static_cast<responseInfo*>(userdata);
			const char *pStart = V_strstr(data, " ") + 1;
			//int len = V_strlen(pStart);
			//char temp[64] = {0};
			//V_strncpy( temp, pStart, len-1 );
			
			pInfo->SetMatchUrl(pStart, V_strlen(pStart)-1);
		}
		
		//Warning(temp);
		return maxSize;
	}

	virtual int Run()
	{
		g_pTSCallQueue->EnqueueFunctor( CreateFunctor(pEngine, &IVEngineServer::LogPrint, (const char *)"testing thread log\n") );
		if (!m_responseInfo.HasSessionId())
		{
			CHttpSend a(GET);

			struct curl_slist *chunk = NULL;
			//chunk = curl_slist_append(chunk, "Transfer-Encoding: chunked");
			chunk = curl_slist_append(chunk, "sizzlingstats: v0.1");

			a.Perform(WEB_LISTEN_IP, chunk, NULL, NULL, header_read_callback, &m_responseInfo);
			curl_slist_free_all(chunk);
		}

		CUtlBuffer postString;
		char sessionId[64] = {0};
		m_responseInfo.GetSessionId(sessionId, 64);
		
		m_hostInfoMutex.Lock();
		hostInfo_t tempInfo(m_hostInfo);
		m_hostInfoMutex.Unlock();

		m_dataListMutex.Lock();
		producePostString( tempInfo, m_webStats, sessionId, postString );
		m_dataListMutex.Unlock();

		Yield();

		m_dataListMutex.Lock();
		m_webStats.RemoveAll();
		m_dataListMutex.Unlock();

		CHttpSend a(POST);

		struct curl_slist *chunk = NULL;
		chunk = curl_slist_append(chunk, "Transfer-Encoding: chunked");
		chunk = curl_slist_append(chunk, "Content-type: application/json");

		a.Perform(WEB_LISTEN_IP, chunk, read_callback, &postString, NULL, NULL);
		curl_slist_free_all(chunk);

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
