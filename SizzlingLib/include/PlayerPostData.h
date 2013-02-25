
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

//====================================================
//
// reference counted data to pass to curl
// for transfer to the web server
//
//====================================================

#ifndef PLAYER_POST_DATA_H
#define PLAYER_POST_DATA_H

#include "utlbuffer.h"
#include "utlvector.h"

#pragma warning( disable : 4351 )
#pragma warning( default : 4351 )

#include "playerdata.h"

// don't conflict with player_info_t from cdll_int.h
typedef struct playerInfo_s
{
	char m_name[32];
	char m_steamid[32];
	unsigned char m_teamid;
} playerInfo_t;

typedef struct hostInfo_s
{
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

class CStatsJsonData//: public CRefCounted<>
{
public:
	CStatsJsonData():
		m_postString()
	{
		m_postString.SetBufferType(true, true);
	}

	~CStatsJsonData()
	{
	}

	CUtlBuffer &producePostString(const hostInfo_t &host, const CUtlVector<playerWebStats_t> &data)
	{
		m_postString.PutString("{\"stats\":{\"sessionid\":0,\"map\":\"");
		m_postString.PutString(host.m_mapname);
		m_postString.PutString("\",\"hostname\":\"");
		m_postString.PutString(host.m_hostname);
		m_postString.PutString("\",\"bluname\":\"");
		m_postString.PutString(host.m_bluname);
		m_postString.PutString("\",\"redname\":\"");
		m_postString.PutString(host.m_redname);
		m_postString.PutString("\",\"bluscore\":");
		{
			char temp[4];
			V_snprintf(temp, 4, "%i", host.m_bluscore);
			m_postString.PutString(temp);
		}
		m_postString.PutString(",\"redscore\":");
		{
			char temp[4];
			V_snprintf(temp, 4, "%i", host.m_redscore);
			m_postString.PutString(temp);
		}
		
		m_postString.PutString(",\"players\":[");
		for (int i = 0; i < data.Count(); ++i)
		{
			if (i > 0)
			{
				m_postString.PutString(",");
			}
			m_postString.PutString("{\"steamid\":\"");
			m_postString.PutString(data[i].m_playerInfo.m_steamid);
			m_postString.PutString("\",\"team\":");
			{
				char temp[2];
				V_snprintf(temp, 2, "%i", data[i].m_playerInfo.m_teamid);
				m_postString.PutString(temp);
			}
			m_postString.PutString(",\"name\":\"");
			m_postString.PutString(data[i].m_playerInfo.m_name);
			m_postString.PutString("\",\"kills\":");
			{
				char temp[16];
				V_snprintf(temp, 16, "%i", data[i].m_scoreData.getStat(Kills));
				m_postString.PutString(temp);
			}
			m_postString.PutString(",\"deaths\":");
			{
				char temp[16];
				V_snprintf(temp, 16, "%i", data[i].m_scoreData.getStat(Deaths));
				m_postString.PutString(temp);
			}
			m_postString.PutString("}");
		}
		m_postString.PutString("]}}");

		return m_postString;
	}
	
private:
	CUtlBuffer	m_postString;
};

#endif