////////////////////////////////////////////////////////////////////////////////
// Filename: WebStatsHandler.h
////////////////////////////////////////////////////////////////////////////////
#ifndef WEB_STATS_HANDLER_H
#define WEB_STATS_HANDLER_H

#include "queuethread.h"
#include "JsonUtils.h"
#include "utlbuffer.h"

//#include "ThreadCallQueue.h"
#include "playerdata.h"
//#include "eiface.h"

#include "curlconnection.h"

#include "funcqueuethread.h"

#define WEB_SERVER_IP "sizzlingstats.com/api/stats"
#define GAMEOVER_URL "sizzlingstats.com/api/stats/gameover"
#define HEADER_SIZZSTATS_VERSION "sizzlingstats: v0.1"

//extern CTSCallQueue		*g_pTSCallQueue;
//extern IVEngineServer	*pEngine;

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

					const ScoreData *pScores = &data[i].m_scoreData;
					temp3.InsertKV("kills", pScores->getStat(Kills));
					temp3.InsertKV("killassists", pScores->getStat(KillAssists));
					temp3.InsertKV("deaths", pScores->getStat(Deaths));
					temp3.InsertKV("captures", pScores->getStat(Captures));
					temp3.InsertKV("defenses", pScores->getStat(Defenses));
					temp3.InsertKV("suicides", pScores->getStat(Suicides));
					temp3.InsertKV("dominations", pScores->getStat(Dominations));
					temp3.InsertKV("revenge", pScores->getStat(Revenge));
					temp3.InsertKV("buildingsbuilt", pScores->getStat(BuildingsBuilt));
					temp3.InsertKV("buildingsdestroyed", pScores->getStat(BuildingsDestroyed));
					temp3.InsertKV("headshots", pScores->getStat(Headshots));
					temp3.InsertKV("backstabs", pScores->getStat(Backstabs));
					temp3.InsertKV("healpoints", pScores->getStat(HealPoints));
					temp3.InsertKV("invulns", pScores->getStat(Invulns));
					temp3.InsertKV("teleports", pScores->getStat(Teleports));
					temp3.InsertKV("damagedone", pScores->getStat(DamageDone));
					temp3.InsertKV("crits", pScores->getStat(Crits));
					temp3.InsertKV("resupplypoints", pScores->getStat(ResupplyPoints));
					temp3.InsertKV("bonuspoints", pScores->getStat(BonusPoints));
					temp3.InsertKV("points", pScores->getStat(Points));
					temp3.InsertKV("healsreceived", pScores->getStat(HealsReceived));
					temp3.InsertKV("ubersdropped", pScores->getStat(UbersDropped));
					temp3.InsertKV("medpicks", pScores->getStat(MedPicks));
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

class CWebStatsHandler
{
public:
	CWebStatsHandler();
	~CWebStatsHandler();

	void ClearPlayerStats();
	void EnqueuePlayerStats(playerWebStats_t const &item);

	void SetHostData(hostInfo_t const &info);

	void GetMatchUrl( char *str, int maxlen );
	bool HasMatchUrl();

	void SendStatsToWeb();
	void SendGameOverEvent();

protected:
	void SendStatsToWebInternal();
	void SendGameOverEventInternal();

private:
	// used for sending the data
	static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *userdata);

	// used for grabbing the sessionid and matchurl
	static size_t header_read_callback(void *ptr, size_t size, size_t nmemb, void *userdata);

private:
	CFuncQueueThread m_queue;
	CThreadMutexPthread				m_dataListMutex;
	CThreadMutexPthread				m_hostInfoMutex;
	CUtlVector<playerWebStats_t>	m_webStats;
	hostInfo_t						m_hostInfo;
	responseInfo					m_responseInfo;
};

class CNullWebStatsHandler
{
public:
	CNullWebStatsHandler() {}
	~CNullWebStatsHandler() {}

	void ClearPlayerStats() {}
	void EnqueuePlayerStats(playerWebStats_t const &item) {}

	void SetHostData(hostInfo_t const &info) {}

	void GetMatchUrl( char *str, int maxlen ) {}
	bool HasMatchUrl() { return false; }

	void SendStatsToWeb() {}
	void SendGameOverEvent() {}
};

#endif // WEB_STATS_HANDLER_H
