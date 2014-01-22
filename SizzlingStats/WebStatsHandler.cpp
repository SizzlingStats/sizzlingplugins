
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

void CWebStatsHandler::SendS3UploadFinishedEventInternal(const sizz::CString sessionid)
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
	buff.SetBufferType(true, true);
	
	// need to rewrite the json stuff recursively
	{
		CJsonObject outer(buff);
		{
			CJsonObject temp(buff, "stats");
			temp.InsertKV("teamfirstcap", host.m_iFirstCapTeamIndex);
			temp.InsertKV("bluscore", host.m_bluscore);
			temp.InsertKV("redscore", host.m_redscore);
			temp.InsertKV("roundduration", SCHelpers::RoundDBL(host.m_roundduration));
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
					const playerInfo_t *pInfo = &data[i].m_playerInfo;
					temp3.InsertKV("name", pInfo->m_name);
					temp3.InsertKV("steamid", pInfo->m_steamid);
					temp3.InsertKV("ip", pInfo->m_ip);
					temp3.InsertKV("mostplayedclass", pInfo->m_mostPlayedClass);
					temp3.InsertKV("playedclasses", pInfo->m_playedClasses);
					temp3.InsertKV("team", pInfo->m_teamid);

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
			buff.PutString(",");
			{
				CJsonArray temp2(buff, "chats");
				for (int i = 0; i < chatInfo.Count(); ++i)
				{
					if (i > 0)
					{
						buff.PutString(",");
					}
					CJsonObject temp3(buff);
					const chatInfo_t *pInfo = &chatInfo[i];
					temp3.InsertKV("steamid", pInfo->m_steamid);
					temp3.InsertKV("isTeam", pInfo->m_bTeamChat);
					temp3.InsertKV("time", SCHelpers::RoundDBL(pInfo->m_timestamp));
					const char *pMessage = reinterpret_cast<const char*>(pInfo->m_message.PeekGet());
					temp3.InsertKV("message", pMessage);
				}
			}
		}
	}
}

void CWebStatsHandler::addChatToBuff(const CUtlVector<chatInfo_t> &chatInfo, CUtlBuffer &buff)
{
	buff.SetBufferType(true, true);

	{
		CJsonObject outer(buff);
		{
			CJsonArray temp2(buff, "chats");
			for (int i = 0; i < chatInfo.Count(); ++i)
			{
				if (i > 0)
				{
					buff.PutString(",");
				}
				CJsonObject temp3(buff);
				const chatInfo_t *pInfo = &chatInfo[i];
				temp3.InsertKV("steamid", pInfo->m_steamid);
				temp3.InsertKV("isTeam", pInfo->m_bTeamChat);
				temp3.InsertKV("time", SCHelpers::RoundDBL(pInfo->m_timestamp));
				const char *pMessage = reinterpret_cast<const char*>(pInfo->m_message.PeekGet());
				temp3.InsertKV("message", pMessage);
			}
		}
	}
}

void CWebStatsHandler::createMatchPlayerInfo(CUtlBuffer &buff)
{
	buff.SetBufferType(true, true);
	
	// need to rewrite the json stuff recursively
	{
		CJsonObject outer(buff);
		{
			CJsonObject temp(buff, "stats");

			m_hostInfoMutex.Lock();
			temp.InsertKV("hostname", m_hostInfo.m_hostname);
			temp.InsertKV("apikey", m_apikey);
			temp.InsertKV("map", m_hostInfo.m_mapname);
			temp.InsertKV("bluname", m_hostInfo.m_bluname);
			temp.InsertKV("redname", m_hostInfo.m_redname);
			{
				char buff[16];
				SCHelpers::IntIPToString(m_hostInfo.m_hostip, buff, sizeof(buff));
				temp.InsertKV("hostip", buff);
			}
			temp.InsertKV("ip", m_hostInfo.m_ip);
			temp.InsertKV("hostport", m_hostInfo.m_hostport);
			m_hostInfoMutex.Unlock();

			buff.PutString(",");
			{
				CJsonArray temp2(buff, "players");
				
				m_playerInfoMutex.Lock();
				for (int i = 0; i < m_playerInfo.Count(); ++i)
				{
					if (i > 0)
					{
						buff.PutString(",");
					}
					CJsonObject temp3(buff);
					const playerInfo_t *pInfo = &m_playerInfo[i];
					temp3.InsertKV("name", pInfo->m_name);
					temp3.InsertKV("steamid", pInfo->m_steamid);
					temp3.InsertKV("ip", pInfo->m_ip);
					temp3.InsertKV("mostplayedclass", pInfo->m_mostPlayedClass);
					temp3.InsertKV("team", pInfo->m_teamid);
				}
				m_playerInfoMutex.Unlock();
			}
		}
	}
}
