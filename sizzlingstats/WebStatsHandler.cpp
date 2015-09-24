
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

#include "WebStatsHandler.h"

#include "curlconnection.h"
#include "JsonUtils.h"
#include "SC_helpers.h"

#include "ThreadCallQueue.h"
#include "eiface.h"

void CWebStatsHandler::SetHostData( hostInfo_t const &info )
{
	m_hostInfoMutex.Lock();
	::new(&m_hostInfo) hostInfo_t(info);
	m_hostInfoMutex.Unlock();
}

void CWebStatsHandler::SendStatsToWebInternal()
{
	CUtlBuffer postString;
	char sessionId[64] = {0};
	m_responseInfo.GetSessionId(sessionId, 64);
		
	m_hostInfoMutex.Lock();
	hostInfo_t tempInfo(m_hostInfo);
	m_hostInfoMutex.Unlock();

	m_dataListAndChatMutex.Lock();
	producePostString( tempInfo, m_webStats, m_chatLog, sessionId, postString );
	m_chatLog.RemoveAll();
	m_dataListAndChatMutex.Unlock();

	ClearPlayerStats();
	
	CCurlConnection connection;
	if (connection.Initialize())
	{
		connection.SetHttpSendType(CCurlConnection::POST);
		connection.AddHeader("Transfer-Encoding: chunked");
		connection.AddHeader("Content-type: application/json");
		connection.AddHeader("Expect:");
		connection.AddHeader("endofround: true");
		connection.AddHeader(HEADER_SIZZSTATS_VERSION);

		if (sessionId[0] != '\0')
		{
			char temp[128] = {};
			V_snprintf( temp, 128, "sessionid: %s", sessionId);
			connection.AddHeader(temp);
		}

		connection.SetUrl(STATS_UPDATE_URL);
		connection.SetBodyReadFunction(read_callback);
		connection.SetBodyReadUserdata(&postString);
		connection.SetHeaderReadFunction(header_read_callback);
		connection.SetHeaderReadUserdata(this);

		connection.Perform();
		connection.Close();
	}
}

void CWebStatsHandler::SendGameStartEventInternal()
{
	CCurlConnection connection;
	if (connection.Initialize())
	{
		connection.SetHttpSendType(CCurlConnection::POST);
		connection.AddHeader("Transfer-Encoding: chunked");
		connection.AddHeader("Content-type: application/json");
		connection.AddHeader("Expect:");
		connection.AddHeader(HEADER_SIZZSTATS_VERSION);

		connection.SetUrl(GAME_START_URL);

		connection.SetBodyReadFunction(read_callback);

		CUtlBuffer postString;
		createMatchPlayerInfo(postString);

		m_playerInfoMutex.Lock();
		m_playerInfo.RemoveAll();
		m_playerInfoMutex.Unlock();

		connection.SetBodyReadUserdata(&postString);
		connection.SetHeaderReadFunction(header_read_callback);
		connection.SetHeaderReadUserdata(this);

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
			connection.AddHeader("Transfer-Encoding: chunked");
			connection.AddHeader("Content-type: application/json");
			connection.AddHeader("Expect:");
			connection.AddHeader(HEADER_SIZZSTATS_VERSION);

			{
				char lengthHeader[64] = {};
				V_snprintf( lengthHeader, 64, "matchduration: %.0f", flMatchDuration+0.5 );
				connection.AddHeader(lengthHeader);
			}

			{
				char sessionId[64] = {};
				m_responseInfo.GetSessionId(sessionId, 64);
				char temp[128] = {};
				V_snprintf( temp, 128, "sessionid: %s", sessionId);

				connection.AddHeader(temp);
			}

			connection.SetUrl(GAMEOVER_URL);
			connection.SetBodyReadFunction(read_callback);

			CUtlBuffer postString;
			m_dataListAndChatMutex.Lock();
			addChatToBuff( m_chatLog, postString );
			m_chatLog.RemoveAll();
			m_dataListAndChatMutex.Unlock();

			connection.SetBodyReadUserdata(&postString);
			connection.SetHeaderReadFunction(header_read_callback);
			connection.SetHeaderReadUserdata(this);

			connection.Perform();
			connection.Close();
		}
		m_responseInfo.ResetSessionId();
	}
}

void CWebStatsHandler::SendS3UploadFinishedEventInternal(const sizz::CString &sessionid)
{
	CCurlConnection connection;
	if (connection.Initialize())
	{
		connection.SetHttpSendType(CCurlConnection::POST);
		connection.AddHeader("Transfer-Encoding: chunked");
		connection.AddHeader("Content-type: application/json");
		connection.AddHeader("Expect:");
		connection.AddHeader(HEADER_SIZZSTATS_VERSION);

		{
			char temp[128] = {};
			V_snprintf( temp, 128, "sessionid: %s", sessionid.ToCString());

			connection.AddHeader(temp);
		}

		connection.SetUrl(S3UPLOADFINISHED_URL);

		// a put has to have a body.
		// curl just loops on a read if it doesn't have one.
		// can't even specify it with 'Content-Length: 0'
		CUtlBuffer postString;
		connection.SetBodyReadUserdata(&postString);
		connection.SetBodyReadFunction(read_callback);

		connection.Perform();
		connection.Close();
	}
}

size_t CWebStatsHandler::read_callback(void *ptr, size_t size, size_t nmemb, void *userdata)
{
	CUtlBuffer *pBuffer = static_cast<CUtlBuffer*>(userdata);
	const size_t maxSize = size*nmemb;
	const size_t bytesRemaining = pBuffer->GetBytesRemaining();
	if ( bytesRemaining >= maxSize )
	{
		pBuffer->Get( ptr, maxSize );
		return maxSize;
	}
	else
	{
		pBuffer->Get( ptr, bytesRemaining );
		return bytesRemaining;
	}
}

size_t CWebStatsHandler::header_read_callback(void *ptr, size_t size, size_t nmemb, void *userdata)
{
	const size_t maxSize = size*nmemb;
	char *data = (char*)ptr;

	if ( V_strstr( data, "sessionid: " ) )
	{
		
		const char *pStart = V_strstr(data, " ") + 1;
		int length = V_strlen(pStart) - 2; // -2 bytes for '\n' and '\r'
		
		// keep the 2 bytes stripped when 
		// passing to V_strncpy
		char sessionid[64];
		length = length > sizeof(sessionid)-1 ? sizeof(sessionid)-1 : length;
		V_strncpy(sessionid, pStart, length+1);

		CWebStatsHandler *pWebStats = static_cast<CWebStatsHandler*>(userdata);
		pWebStats->SetSessionId(sessionid);
	}
	else if ( V_strstr( data, "matchurl: " ) )
	{
		const char *pStart = V_strstr(data, " ") + 1;
		int length = V_strlen(pStart) - 2; // -2 bytes for '\n' and '\r'

		// keep the 2 bytes stripped when 
		// passing to V_strncpy
		char matchurl[128];
		length = length > sizeof(matchurl)-1 ? sizeof(matchurl)-1 : length;
		V_strncpy(matchurl, pStart, length+1);

		CWebStatsHandler *pWebStats = static_cast<CWebStatsHandler*>(userdata);
		pWebStats->SetMatchUrl(matchurl);
	}
	else if ( V_strstr( data, "stvuploadurl: " ) )
	{
		const char *pStart = V_strstr(data, " ") + 1;
		int length = V_strlen(pStart) - 2; // -2 bytes for '\n' and '\r'

		// keep the 2 bytes stripped when 
		// passing to V_strncpy
		char stvuploadurl[256];
		length = length > sizeof(stvuploadurl)-1 ? sizeof(stvuploadurl)-1 : length;
		V_strncpy(stvuploadurl, pStart, length+1);

		CWebStatsHandler *pWebStats = static_cast<CWebStatsHandler*>(userdata);
		pWebStats->SetSTVUploadUrl(stvuploadurl);
	}
		
	return maxSize;
}

void CWebStatsHandler::producePostString(const hostInfo_t &host, const CUtlVector<playerWebStats_t> &data, const CUtlVector<chatInfo_t> &chatInfo, const char *sessionId, CUtlBuffer &buff)
{
	json::JsonWriter jsonWriter;
	jsonWriter.StartObject();
	jsonWriter.StartObject("stats");
	{
		jsonWriter.InsertKV("teamfirstcap", host.m_iFirstCapTeamIndex);
		jsonWriter.InsertKV("bluscore", host.m_bluscore);
		jsonWriter.InsertKV("redscore", host.m_redscore);
		jsonWriter.InsertKV("roundduration", SCHelpers::RoundDBL(host.m_roundduration));

		jsonWriter.StartArray("players");
		{
			const size_t numPlayers = data.Count();
			for (size_t i = 0; i < numPlayers; ++i)
			{
				jsonWriter.StartObject();
				const playerInfo_t *pInfo = &data[i].m_playerInfo;
				jsonWriter.InsertKV("name", pInfo->m_name);
				jsonWriter.InsertKV("steamid", pInfo->m_steamid);
				jsonWriter.InsertKV("ip", pInfo->m_ip);
				jsonWriter.InsertKV("mostplayedclass", pInfo->m_mostPlayedClass);
				jsonWriter.InsertKV("playedclasses", pInfo->m_playedClasses);
				jsonWriter.InsertKV("team", pInfo->m_teamid);

				const ScoreData *pScores = &data[i].m_scoreData;
				jsonWriter.InsertKV("kills", pScores->getStat(Kills));
				jsonWriter.InsertKV("killassists", pScores->getStat(KillAssists));
				jsonWriter.InsertKV("deaths", pScores->getStat(Deaths));
				jsonWriter.InsertKV("captures", pScores->getStat(Captures));
				jsonWriter.InsertKV("defenses", pScores->getStat(Defenses));
				jsonWriter.InsertKV("suicides", pScores->getStat(Suicides));
				jsonWriter.InsertKV("dominations", pScores->getStat(Dominations));
				jsonWriter.InsertKV("revenge", pScores->getStat(Revenge));
				jsonWriter.InsertKV("buildingsbuilt", pScores->getStat(BuildingsBuilt));
				jsonWriter.InsertKV("buildingsdestroyed", pScores->getStat(BuildingsDestroyed));
				jsonWriter.InsertKV("headshots", pScores->getStat(Headshots));
				jsonWriter.InsertKV("backstabs", pScores->getStat(Backstabs));
				jsonWriter.InsertKV("healpoints", pScores->getStat(HealPoints));
				jsonWriter.InsertKV("invulns", pScores->getStat(Invulns));
				jsonWriter.InsertKV("teleports", pScores->getStat(Teleports));
				jsonWriter.InsertKV("damagedone", pScores->getStat(DamageDone));
				jsonWriter.InsertKV("crits", pScores->getStat(Crits));
				jsonWriter.InsertKV("resupplypoints", pScores->getStat(ResupplyPoints));
				jsonWriter.InsertKV("bonuspoints", pScores->getStat(BonusPoints));
				jsonWriter.InsertKV("points", pScores->getStat(Points));
				jsonWriter.InsertKV("healsreceived", pScores->getStat(HealsReceived));
				jsonWriter.InsertKV("medpicks", pScores->getStat(MedPicks));
				jsonWriter.InsertKV("ubersdropped", pScores->getStat(UbersDropped));
				jsonWriter.InsertKV("overkillDamage", pScores->getStat(OverkillDamage));
				jsonWriter.EndObject();
			}
		}
		jsonWriter.EndArray();
		jsonWriter.StartArray("chats");
		{
			const size_t numChats = chatInfo.Count();
			for (size_t i = 0; i < numChats; ++i)
			{
				jsonWriter.StartObject();
				const chatInfo_t *pInfo = &chatInfo[i];
				jsonWriter.InsertKV("steamid", pInfo->m_steamid);
				jsonWriter.InsertKV("isTeam", pInfo->m_bTeamChat);
				jsonWriter.InsertKV("time", SCHelpers::RoundDBL(pInfo->m_timestamp));
				const char *pMessage = pInfo->m_message.ToCString();
				jsonWriter.InsertKV("message", pMessage);
				jsonWriter.EndObject();
			}
		}
		jsonWriter.EndArray();
	}
	jsonWriter.EndObject();
	jsonWriter.EndObject();

	buff.SetBufferType(true, true);
	buff.PutString(jsonWriter.GetJsonString());
}

void CWebStatsHandler::addChatToBuff(const CUtlVector<chatInfo_t> &chatInfo, CUtlBuffer &buff)
{
	json::JsonWriter jsonWriter;
	jsonWriter.StartObject();
	jsonWriter.StartArray("chats");
	{
		const size_t numChats = chatInfo.Count();
		for (size_t i = 0; i < numChats; ++i)
		{
			jsonWriter.StartObject();
			const chatInfo_t *pInfo = &chatInfo[i];
			jsonWriter.InsertKV("steamid", pInfo->m_steamid);
			jsonWriter.InsertKV("isTeam", pInfo->m_bTeamChat);
			jsonWriter.InsertKV("time", SCHelpers::RoundDBL(pInfo->m_timestamp));
			const char *pMessage = pInfo->m_message.ToCString();
			jsonWriter.InsertKV("message", pMessage);
			jsonWriter.EndObject();
		}
	}
	jsonWriter.EndArray();
	jsonWriter.EndObject();

	buff.SetBufferType(true, true);
	buff.PutString(jsonWriter.GetJsonString());
}

void CWebStatsHandler::createMatchPlayerInfo(CUtlBuffer &buff)
{
	json::JsonWriter jsonWriter;
	jsonWriter.StartObject();
	jsonWriter.StartObject("stats");
	{
		m_hostInfoMutex.Lock();
		hostInfo_t hostInfo(m_hostInfo);
		m_hostInfoMutex.Unlock();
		
		jsonWriter.InsertKV("hostname", hostInfo.m_hostname);
		jsonWriter.InsertKV("apikey", m_apikey);
		jsonWriter.InsertKV("map", hostInfo.m_mapname);
		jsonWriter.InsertKV("bluname", hostInfo.m_bluname);
		jsonWriter.InsertKV("redname", hostInfo.m_redname);
		{
			char buff[16];
			SCHelpers::IntIPToString(hostInfo.m_hostip, buff, sizeof(buff));
			jsonWriter.InsertKV("hostip", buff);
		}
		jsonWriter.InsertKV("ip", hostInfo.m_ip);
		jsonWriter.InsertKV("hostport", hostInfo.m_hostport);

		jsonWriter.StartArray("players");
		{
			auto& playerInfoMutex = m_playerInfoMutex;

			playerInfoMutex.Lock();
			const size_t numPlayerInfo = m_playerInfo.Count();
			playerInfoMutex.Unlock();

			for (size_t i = 0; i < numPlayerInfo; ++i)
			{
				jsonWriter.StartObject();

				playerInfoMutex.Lock();
				const playerInfo_t playerInfo(m_playerInfo[i]);
				playerInfoMutex.Unlock();

				jsonWriter.InsertKV("name", playerInfo.m_name);
				jsonWriter.InsertKV("steamid", playerInfo.m_steamid);
				jsonWriter.InsertKV("ip", playerInfo.m_ip);
				jsonWriter.InsertKV("mostplayedclass", playerInfo.m_mostPlayedClass);
				jsonWriter.InsertKV("team", playerInfo.m_teamid);
				jsonWriter.EndObject();
			}
			
		}
		jsonWriter.EndArray();
	}
	jsonWriter.EndObject();
	jsonWriter.EndObject();

	buff.SetBufferType(true, true);
	buff.PutString(jsonWriter.GetJsonString());
}
