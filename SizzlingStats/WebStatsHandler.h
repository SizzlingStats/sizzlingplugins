////////////////////////////////////////////////////////////////////////////////
// Filename: WebStatsHandler.h
////////////////////////////////////////////////////////////////////////////////
#ifndef WEB_STATS_HANDLER_H
#define WEB_STATS_HANDLER_H

#include "queuethread.h"
#include "utlbuffer.h"
#include "funcqueuethread.h"
#include "playerdata.h"

#define STATS_UPDATE_URL "sizzlingstats.com/api/stats/update"
#define GAME_START_URL "sizzlingstats.com/api/stats/new"
#define GAMEOVER_URL "sizzlingstats.com/api/stats/gameover"
#define HEADER_SIZZSTATS_VERSION "sizzlingstats: v0.1"

typedef struct chatInfo_s
{
	chatInfo_s();
	chatInfo_s(uint64 timestamp,
			const char *steamid,
			const char *message,
			bool bTeamChat);
	
	double m_timestamp;
	char m_steamid[32];
	CUtlBuffer m_message;
	bool m_bTeamChat;
} chatInfo_t;

typedef struct hostInfo_s
{
	hostInfo_s();
	hostInfo_s(const char *hostname,
			const char *mapname,
			const char *bluname,
			const char *redname,
			double roundduration,
			unsigned int bluscore,
			unsigned int redscore);

	char m_hostname[64];
	char m_mapname[64];
	char m_bluname[32];
	char m_redname[32];
	double m_roundduration;
	unsigned char m_bluscore;
	unsigned char m_redscore;
} hostInfo_t;

// don't conflict with player_info_t from cdll_int.h
typedef struct playerInfo_s
{
	char m_name[32];
	//uint64 m_steamid;
	char m_steamid[32];
	uint16 m_mostPlayedClass;
	uint16 m_playedClasses;
	unsigned char m_teamid;
} playerInfo_t;

typedef struct playerWebStats_s
{
	playerInfo_t	m_playerInfo;
	ScoreData		m_scoreData;
} playerWebStats_t;

typedef struct responseInfo_s
{
	responseInfo_s();

	void SetSessionId( const char *id, int lengthToCopy );
	void GetSessionId( char *str, int maxlen );
	void ResetSessionId();
	bool HasSessionId();
	bool HasMatchUrl();
	void SetMatchUrl( const char *url, int lengthToCopy );
	void GetMatchUrl( char *str, int maxlen );
	void ResetMatchUrl();

private:
	CThreadMutexPthread		matchUrlMutex;
	CThreadMutexPthread		sessionIdMutex;
	char					matchUrl[128];
	char					sessionId[64];
} responseInfo_t;

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

	void PlayerChatEvent( double timestamp, const char *szSteamId, const char *szText, bool bTeamChat );

	void SendStatsToWeb();
	void SendGameStartEvent();
	void SendGameOverEvent(double flMatchDuration);

protected:
	void SendStatsToWebInternal();
	void SendGameStartEventInternal();
	void SendGameOverEventInternal(double flMatchDuration);

private:
	// used for sending the data
	static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *userdata);

	// used for grabbing the sessionid and matchurl
	static size_t header_read_callback(void *ptr, size_t size, size_t nmemb, void *userdata);
	
	// adds the stats and chat to the buff in json form
	static void producePostString(const hostInfo_t &host, const CUtlVector<playerWebStats_t> &data, const CUtlVector<chatInfo_t> &chatInfo, const char *sessionId, CUtlBuffer &buff);

	// adds the chat to the buff in json form
	static void addChatToBuff(const CUtlVector<chatInfo_t> &chatInfo, CUtlBuffer &buff);

private:
	CFuncQueueThread m_queue;
	CThreadMutexPthread				m_dataListAndChatMutex;
	CThreadMutexPthread				m_hostInfoMutex;
	CUtlVector<chatInfo_t>			m_chatLog;
	CUtlVector<playerWebStats_t>	m_webStats;
	hostInfo_t						m_hostInfo;
	responseInfo_t					m_responseInfo;
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
	void SendGameOverEvent(double flMatchDuration) {}
};

#pragma warning( push )
#pragma warning( disable : 4351 ) // arrays will be default initialized

inline chatInfo_s::chatInfo_s():
	m_timestamp(0),
	m_steamid(),
	m_message(0, 0, CUtlBuffer::TEXT_BUFFER|CUtlBuffer::CONTAINS_CRLF),
	m_bTeamChat(false)
{
}

inline chatInfo_s::chatInfo_s(uint64 timestamp,
		const char *steamid,
		const char *message,
		bool bTeamChat):
	m_timestamp(timestamp),
	m_message(0, 0, CUtlBuffer::TEXT_BUFFER|CUtlBuffer::CONTAINS_CRLF),
	m_bTeamChat(bTeamChat)
{
	V_strncpy(m_steamid, steamid, 32);
	m_message.PutString(message);
}

inline hostInfo_s::hostInfo_s():
	m_hostname(),
	m_mapname(),
	m_bluname(),
	m_redname(),
	m_roundduration(0.0),
	m_bluscore(0),
	m_redscore(0)
{
}

inline hostInfo_s::hostInfo_s(const char *hostname,
		const char *mapname,
		const char *bluname,
		const char *redname,
		double roundduration,
		unsigned int bluscore,
		unsigned int redscore):
	m_roundduration(roundduration),
	m_bluscore(bluscore),
	m_redscore(redscore)
{
	V_strncpy(m_hostname, hostname, 64);
	V_strncpy(m_mapname, mapname, 64);
	V_strncpy(m_bluname, bluname, 32);
	V_strncpy(m_redname, redname, 32);
}

inline responseInfo_s::responseInfo_s():
	matchUrl(),
	sessionId()
{
	matchUrlMutex.Unlock();
	sessionIdMutex.Unlock();
}

#pragma warning( pop )

inline void responseInfo_s::SetSessionId( const char *id, int lengthToCopy )
{
	int size = lengthToCopy > 64 ? 64 : lengthToCopy;
	sessionIdMutex.Lock();
	V_strncpy(sessionId, id, size);
	sessionIdMutex.Unlock();
}

inline void responseInfo_s::GetSessionId( char *str, int maxlen )
{
	if (str)
	{
		sessionIdMutex.Lock();
		V_strncpy(str, sessionId, maxlen);
		sessionIdMutex.Unlock();
	}
}

inline void responseInfo_s::ResetSessionId()
{
	sessionIdMutex.Lock();
	sessionId[0] = '\0';
	sessionIdMutex.Unlock();
}

inline bool responseInfo_s::HasSessionId()
{
	sessionIdMutex.Lock();
	bool result = sessionId[0] == '\0' ? false : true;
	sessionIdMutex.Unlock();
	return result;
}

inline bool responseInfo_s::HasMatchUrl()
{
	matchUrlMutex.Lock();
	bool result = matchUrl[0] == '\0' ? false : true;
	matchUrlMutex.Unlock();
	return result;
}

inline void responseInfo_s::SetMatchUrl( const char *url, int lengthToCopy )
{
	int size = lengthToCopy > 128 ? 128 : lengthToCopy;
	matchUrlMutex.Lock();
	V_strncpy(matchUrl, url, size);
	matchUrlMutex.Unlock();
}

inline void responseInfo_s::GetMatchUrl( char *str, int maxlen )
{
	if (str)
	{
		matchUrlMutex.Lock();
		V_strncpy(str, matchUrl, maxlen);
		matchUrlMutex.Unlock();
	}
}

inline void responseInfo_s::ResetMatchUrl()
{
	matchUrlMutex.Lock();
	matchUrl[0] = '\0';
	matchUrlMutex.Unlock();
}

inline CWebStatsHandler::CWebStatsHandler()
{
	m_dataListAndChatMutex.Unlock();
	m_hostInfoMutex.Unlock();
}

inline CWebStatsHandler::~CWebStatsHandler()
{
}

inline void CWebStatsHandler::ClearPlayerStats()
{
	m_dataListAndChatMutex.Lock();
	m_webStats.RemoveAll();
	m_dataListAndChatMutex.Unlock();
}

inline void CWebStatsHandler::EnqueuePlayerStats(playerWebStats_t const &item)
{
	m_dataListAndChatMutex.Lock();
	m_webStats.AddToTail(item);
	m_dataListAndChatMutex.Unlock();
}

inline void CWebStatsHandler::GetMatchUrl( char *str, int maxlen )
{
	m_responseInfo.GetMatchUrl(str, maxlen);
}

inline bool CWebStatsHandler::HasMatchUrl()
{
	return m_responseInfo.HasMatchUrl();
}

inline void CWebStatsHandler::PlayerChatEvent( double timestamp, const char *szSteamId, const char *szText, bool bTeamChat )
{
	m_dataListAndChatMutex.Lock();
	int elem = m_chatLog.AddToTail();
	::new( &m_chatLog.Element(elem) ) chatInfo_t(timestamp, szSteamId, szText, bTeamChat);
	m_dataListAndChatMutex.Unlock();
}

inline void CWebStatsHandler::SendStatsToWeb()
{
	m_queue.EnqueueItem(CreateFunctor(this, &CWebStatsHandler::SendStatsToWebInternal));
}

inline void CWebStatsHandler::SendGameStartEvent()
{
	m_queue.EnqueueItem(CreateFunctor(this, &CWebStatsHandler::SendGameStartEventInternal));
}

inline void CWebStatsHandler::SendGameOverEvent(double flMatchDuration)
{
	m_queue.EnqueueItem(CreateFunctor(this, &CWebStatsHandler::SendGameOverEventInternal, flMatchDuration));
}

#endif // WEB_STATS_HANDLER_H

