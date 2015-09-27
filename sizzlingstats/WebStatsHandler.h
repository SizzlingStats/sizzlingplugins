
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

////////////////////////////////////////////////////////////////////////////////
// Filename: WebStatsHandler.h
////////////////////////////////////////////////////////////////////////////////
#ifndef WEB_STATS_HANDLER_H
#define WEB_STATS_HANDLER_H

#include "ThreadFunctorQueue.h"
#include "utlbuffer.h"
#include "SSPlayerData.h"
#include "sizzstring.h"
#include "PluginDefines.h"
#include <functional>

#ifdef USE_STAGING_URLS
#define STATS_UPDATE_URL "http://staging.sizzlingstats.com/api/stats/update"
#define GAME_START_URL "http://staging.sizzlingstats.com/api/stats/new"
#define GAMEOVER_URL "http://staging.sizzlingstats.com/api/stats/gameover"
#define S3UPLOADFINISHED_URL "http://staging.sizzlingstats.com/api/stats/stvuploadfinished"
#else
#define STATS_UPDATE_URL "http://sizzlingstats.com/api/stats/update"
#define GAME_START_URL "http://sizzlingstats.com/api/stats/new"
#define GAMEOVER_URL "http://sizzlingstats.com/api/stats/gameover"
#define S3UPLOADFINISHED_URL "http://sizzlingstats.com/api/stats/stvuploadfinished"
#endif
#define HEADER_SIZZSTATS_VERSION "sizzlingstats: v0.2"

typedef struct chatInfo_s
{
	chatInfo_s();
	chatInfo_s(uint64 timestamp,
			const char *steamid,
			const char *message,
			bool bTeamChat);
	
	double m_timestamp;
	char m_steamid[32];
	sizz::CString m_message;
	bool m_bTeamChat;
} chatInfo_t;

typedef struct hostInfo_s
{
	hostInfo_s();
	hostInfo_s( const hostInfo_s &other );
	hostInfo_s &operator=( const hostInfo_s &other );

	char m_hostname[64];
	char m_mapname[64];
	char m_bluname[32];
	char m_redname[32];
	char m_ip[32];
	double m_roundduration;
	int32 m_hostip;
	int16 m_hostport;
	unsigned char m_iFirstCapTeamIndex;
	unsigned char m_bluscore;
	unsigned char m_redscore;
} hostInfo_t;

// don't conflict with player_info_t from cdll_int.h
typedef struct playerInfo_s
{
	char m_name[32];
	//uint64 m_steamid;
	char m_steamid[32];
	char m_ip[32];
	uint16 m_mostPlayedClass;
	uint16 m_playedClasses;
	unsigned char m_teamid;
} playerInfo_t;

typedef struct playerWebStats_s
{
	playerInfo_t	m_playerInfo;
	ScoreData		m_scoreData;

	bool operator==( const playerWebStats_s& a ) const
	{
		return ( V_strcmp( m_playerInfo.m_steamid, a.m_playerInfo.m_steamid ) == 0 );
	}
} playerWebStats_t;

typedef struct responseInfo_s
{
	responseInfo_s();

	void SetSessionId( const char *id );
	void GetSessionId( char *str, int maxlen );
	void ResetSessionId();
	bool HasSessionId();
	bool HasMatchUrl();
	void SetMatchUrl( const char *url );
	void GetMatchUrl( char *str, int maxlen );
	void ResetMatchUrl();
	bool HasSTVUploadUrl();
	void SetSTVUploadUrl( const char *url );
	void GetSTVUploadUrl( char *str, int maxlen );
	void ResetSTVUploadUrl();

private:
	sizz::CThreadMutex	matchUrlMutex;
	sizz::CThreadMutex	sessionIdMutex;
	sizz::CThreadMutex	stvUploadUrlMutex;
	char			matchUrl[128];
	char			sessionId[64];
	char			stvUploadUrl[256];
} responseInfo_t;

class CWebStatsHandler
{
public:
	CWebStatsHandler();
	~CWebStatsHandler();

	void Initialize();
	void Shutdown();

	void ClearPlayerStats();
	void EnqueuePlayerStats(playerWebStats_t const &item);
	void EnqueuePlayerInfo(playerInfo_t const &info);

	void SetHostData(hostInfo_t const &info);

	void GetMatchUrl( char *str, int maxlen );
	bool HasMatchUrl();

	void GetSTVUploadUrl( char *str, int maxlen );
	bool HasSTVUploadUrl();

	void PlayerChatEvent( double timestamp, const char *szSteamId, const char *szText, bool bTeamChat );

	void SendStatsToWeb();
	void SendGameStartEvent();
	void SendGameOverEvent(double flMatchDuration);
	void SendS3UploadFinishedEvent(const sizz::CString &sessionid);

	void SetApiKey( const char *apikey );

	void SetReceiveSessionIdCallback( std::function<void(sizz::CString)> func );
	void SetReceiveMatchUrlCallback( std::function<void(sizz::CString)> func );
	void SetReceiveSTVUploadUrlCallback( std::function<void(sizz::CString)> func );

protected:
	void SendStatsToWebInternal();
	void SendGameStartEventInternal();
	void SendGameOverEventInternal(double flMatchDuration);
	void SendS3UploadFinishedEventInternal(const sizz::CString &sessionid);

private:
	void SetSessionId( const char *sessionid );
	void SetMatchUrl( const char *matchurl );
	void SetSTVUploadUrl( const char *stvuploadurl );

private:
	// used for sending the data
	static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *userdata);

	// used for grabbing the sessionid and matchurl
	static size_t header_read_callback(void *ptr, size_t size, size_t nmemb, void *userdata);
	
	// adds the stats and chat to the buff in json form
	static void producePostString(const hostInfo_t &host, const CUtlVector<playerWebStats_t> &data, const CUtlVector<chatInfo_t> &chatInfo, const char *sessionId, CUtlBuffer &buff);

	// adds the chat to the buff in json form
	static void addChatToBuff(const CUtlVector<chatInfo_t> &chatInfo, CUtlBuffer &buff);

	void createMatchPlayerInfo(CUtlBuffer &buff);

private:
	CSizzFuncQueueThread			m_queue;
	sizz::CThreadMutex					m_dataListAndChatMutex;
	sizz::CThreadMutex					m_hostInfoMutex;
	sizz::CThreadMutex					m_playerInfoMutex;
	CUtlVector<chatInfo_t>			m_chatLog;
	CUtlVector<playerWebStats_t>	m_webStats;
	CUtlVector<playerInfo_t>		m_playerInfo;
	hostInfo_t						m_hostInfo;
	responseInfo_t					m_responseInfo;
	std::function<void(sizz::CString)> m_RecvSessionIdCallback;
	std::function<void(sizz::CString)> m_RecvMatchUrlCallback;
	std::function<void(sizz::CString)> m_RecvSTVUploadUrlCallback;
	// this is protected with the hostInfo mutex, 
	// but isn't in the hostinfo since it's only 
	// sent once
	char	m_apikey[37]; // 36 char + null
};

class CNullWebStatsHandler
{
public:
	CNullWebStatsHandler() {}
	~CNullWebStatsHandler() {}

	void Initialize() {}
	void Shutdown() {}

	void ClearPlayerStats() {}
	void EnqueuePlayerStats(playerWebStats_t const &item) {}
	void EnqueuePlayerInfo(playerInfo_t const &info) {}

	void SetHostData(hostInfo_t const &info) {}

	void GetMatchUrl( char *str, int maxlen ) {}
	bool HasMatchUrl() { return false; }

	void GetSTVUploadUrl( char *str, int maxlen ) {}
	bool HasSTVUploadUrl() { return false; }

	void PlayerChatEvent( double timestamp, const char *szSteamId, const char *szText, bool bTeamChat ) {}

	void SendStatsToWeb() {}
	void SendGameStartEvent() {}
	void SendGameOverEvent(double flMatchDuration) {}
};

#pragma warning( push )
#pragma warning( disable : 4351 ) // arrays will be default initialized

inline chatInfo_s::chatInfo_s():
	m_timestamp(0),
	m_steamid(),
	m_message(""),
	m_bTeamChat(false)
{
}

inline chatInfo_s::chatInfo_s(uint64 timestamp,
		const char *steamid,
		const char *message,
		bool bTeamChat):
	m_timestamp(timestamp),
	m_message(message),
	m_bTeamChat(bTeamChat)
{
	V_strncpy(m_steamid, steamid, 32);
}

inline hostInfo_s::hostInfo_s():
	m_hostname(),
	m_mapname(),
	m_bluname(),
	m_redname(),
	m_ip(),
	m_roundduration(0.0),
	m_hostip(0),
	m_hostport(0),
	m_iFirstCapTeamIndex(0),
	m_bluscore(0),
	m_redscore(0)
{
}

inline hostInfo_s::hostInfo_s( const hostInfo_t &other ):
	m_roundduration(other.m_roundduration),
	m_hostip(other.m_hostip),
	m_hostport(other.m_hostport),
	m_iFirstCapTeamIndex(other.m_iFirstCapTeamIndex),
	m_bluscore(other.m_bluscore),
	m_redscore(other.m_redscore)
{
	V_strncpy(m_hostname, other.m_hostname, sizeof(m_hostname));
	V_strncpy(m_mapname, other.m_mapname, sizeof(m_mapname));
	V_strncpy(m_bluname, other.m_bluname, sizeof(m_bluname));
	V_strncpy(m_redname, other.m_redname, sizeof(m_redname));
	V_strncpy(m_ip, other.m_ip, sizeof(m_ip));
}

inline hostInfo_s &hostInfo_s::operator=( const hostInfo_s &other )
{
	return *::new(this) hostInfo_t(other);
}

inline responseInfo_s::responseInfo_s():
	matchUrl(),
	sessionId(),
	stvUploadUrl()
{
}

#pragma warning( pop )

inline void responseInfo_s::SetSessionId( const char *id )
{
	sessionIdMutex.Lock();
	V_strncpy(sessionId, id, sizeof(sessionId));
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

inline void responseInfo_s::SetMatchUrl( const char *url )
{
	matchUrlMutex.Lock();
	V_strncpy(matchUrl, url, sizeof(matchUrl));
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

inline bool responseInfo_s::HasSTVUploadUrl()
{
	stvUploadUrlMutex.Lock();
	bool result = stvUploadUrl[0] == '\0' ? false : true;
	stvUploadUrlMutex.Unlock();
	return result;
}

inline void responseInfo_s::SetSTVUploadUrl( const char *url )
{
	stvUploadUrlMutex.Lock();
	V_strncpy(stvUploadUrl, url, sizeof(stvUploadUrl));
	stvUploadUrlMutex.Unlock();
}

inline void responseInfo_s::GetSTVUploadUrl( char *str, int maxlen )
{
	if (str)
	{
		stvUploadUrlMutex.Lock();
		V_strncpy(str, stvUploadUrl, maxlen);
		stvUploadUrlMutex.Unlock();
	}
}

inline void responseInfo_s::ResetSTVUploadUrl()
{
	stvUploadUrlMutex.Lock();
	stvUploadUrl[0] = '\0';
	stvUploadUrlMutex.Unlock();
}

#pragma warning( push )
#pragma warning( disable : 4351 ) // arrays will be default initialized

inline CWebStatsHandler::CWebStatsHandler():
	m_apikey()
{
}

#pragma warning( pop )

inline CWebStatsHandler::~CWebStatsHandler()
{
}

inline void CWebStatsHandler::Initialize()
{
}

inline void CWebStatsHandler::Shutdown()
{
	m_queue.JoinQueue();
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
	int idx = m_webStats.Find(item);
	if (idx > -1)
	{
		m_webStats.Element(idx).m_scoreData += item.m_scoreData;
	}
	else
	{
		m_webStats.AddToTail(item);
	}	
	m_dataListAndChatMutex.Unlock();
}

inline void CWebStatsHandler::EnqueuePlayerInfo(playerInfo_t const &info)
{
	m_playerInfoMutex.Lock();
	m_playerInfo.AddToTail(info);
	m_playerInfoMutex.Unlock();
}

inline void CWebStatsHandler::GetMatchUrl( char *str, int maxlen )
{
	m_responseInfo.GetMatchUrl(str, maxlen);
}

inline bool CWebStatsHandler::HasMatchUrl()
{
	return m_responseInfo.HasMatchUrl();
}

inline void CWebStatsHandler::GetSTVUploadUrl( char *str, int maxlen )
{
	m_responseInfo.GetSTVUploadUrl(str, maxlen);
}

inline bool CWebStatsHandler::HasSTVUploadUrl()
{
	return m_responseInfo.HasSTVUploadUrl();
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
	m_queue.EnqueueFunctor(CreateFunctor(this, &CWebStatsHandler::SendStatsToWebInternal));
}

inline void CWebStatsHandler::SendGameStartEvent()
{
	m_queue.EnqueueFunctor(CreateFunctor(this, &CWebStatsHandler::SendGameStartEventInternal));
}

inline void CWebStatsHandler::SendGameOverEvent(double flMatchDuration)
{
	m_queue.EnqueueFunctor(CreateFunctor(this, &CWebStatsHandler::SendGameOverEventInternal, flMatchDuration));
}

inline void CWebStatsHandler::SendS3UploadFinishedEvent(const sizz::CString &sessionid)
{
	m_queue.EnqueueFunctor(CreateFunctor(this, &CWebStatsHandler::SendS3UploadFinishedEventInternal, sessionid));
}

inline void CWebStatsHandler::SetApiKey( const char *apikey )
{
	m_hostInfoMutex.Lock();
	V_strncpy(m_apikey, apikey, sizeof(m_apikey));
	m_hostInfoMutex.Unlock();
}

inline void CWebStatsHandler::SetReceiveSessionIdCallback( std::function<void(sizz::CString)> func )
{
	m_RecvSessionIdCallback = std::move(func);
}

inline void CWebStatsHandler::SetReceiveMatchUrlCallback( std::function<void(sizz::CString)> func )
{
	m_RecvMatchUrlCallback = std::move(func);
}

inline void CWebStatsHandler::SetReceiveSTVUploadUrlCallback( std::function<void(sizz::CString)> func )
{
	m_RecvSTVUploadUrlCallback = std::move(func);
}

inline void CWebStatsHandler::SetSessionId( const char *sessionid )
{
	m_responseInfo.SetSessionId(sessionid);
	if (m_RecvSessionIdCallback)
	{
		m_RecvSessionIdCallback(sizz::CString(sessionid));
	}
}

inline void CWebStatsHandler::SetMatchUrl( const char *matchurl )
{
	m_responseInfo.SetMatchUrl(matchurl);
	if (m_RecvMatchUrlCallback)
	{
		m_RecvMatchUrlCallback(sizz::CString(matchurl));
	}
}

inline void CWebStatsHandler::SetSTVUploadUrl( const char *stvuploadurl )
{
	m_responseInfo.SetSTVUploadUrl(stvuploadurl);
	if (m_RecvSTVUploadUrlCallback)
	{
		m_RecvSTVUploadUrlCallback(sizz::CString(stvuploadurl));
	}
}


#endif // WEB_STATS_HANDLER_H
