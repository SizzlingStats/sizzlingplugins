
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

#include "LogStats.h"
#include "convar.h"

class CGlobalVars;
#include "game/server/iplayerinfo.h"

#include "SizzPluginContext.h"

static ConVar ss_logging("sizz_stats_disable_logging", "0", FCVAR_NONE, "If nonzero, 'suppstats' logging is disabled from SizzlingStats. Used to resolve conflicts from other logging plugins.");

struct playerInfo
{
	playerInfo()
	{
		reset();
	}
	// which ones do i really need to reset? (instead of doing all of them)
	void reset()
	{
		memset( steamid, 0, 64 );
		memset( name, 0, 32 );
		teamid = 0;
		classid = 0;
		pPlayerInfo = NULL;
		userid = -1;
	}
	char steamid[64];
	char name[32];
	int teamid;
	int classid;
	IPlayerInfo *pPlayerInfo;
	int userid;
};

static const char *teamNames[] =
{
	"Unassigned",
	"Spectator",
	"Red",
	"Blue"
};

static const char *classNames[] = 
{
	"undefined",
	"scout",
	"sniper",
	"soldier",
	"demoman",
	"medic",
	"heavyweapons",
	"pyro",
	"spy",
	"engineer"
};

inline bool FStrEq(const char *sz1, const char *sz2)
{
	return(Q_stricmp(sz1, sz2) == 0);
}

CLogStats::CLogStats():
	m_context(nullptr),
	m_entIndexToPlayerInfo(new playerInfo[33])
{
}

CLogStats::~CLogStats()
{
	delete [] m_entIndexToPlayerInfo;
}

bool CLogStats::Load( CSizzPluginContext &plugin_context )
{
	m_context = &plugin_context;

	m_context->AddListener( this, "item_pickup", true );
	m_context->AddListener( this, "player_hurt", true );
	m_context->AddListener( this, "player_healed", true );
	m_context->AddListener( this, "player_spawn", true );
	m_context->AddListener( this, "player_team", true );
	m_context->AddListener( this, "player_changename", true );
	m_context->AddListener( this, "player_changeclass", true );

	m_context->ServerCommand( "log on\n" );
	m_context->ServerCommand("sm plugins unload supstats\n");
	m_context->ServerExecute();
	return true;
}

void CLogStats::Unload()
{
	if (m_context->GetEngine() && m_context->GetGameEventManager())
	{
		m_context->RemoveListener(this);
		m_context->ServerCommand("sm plugins load supstats\n");
		m_context->ServerExecute();
	}
}

void CLogStats::LevelInit( const char *pMapName )
{
	m_context->ServerCommand( "log on\n" );
	m_context->ServerExecute();

	char maplog[64];
	Q_snprintf( maplog, 64, "Loading map \"%s\"\n", pMapName );
	WriteLog( maplog );
}

void CLogStats::ClientActive( int ent_index )
{
	IPlayerInfo *pPlayerInfo = m_context->GetPlayerInfo(ent_index);
	if ( pPlayerInfo && pPlayerInfo->IsConnected() )
	{
		playerInfo &pInfo = m_entIndexToPlayerInfo[ent_index];
		pInfo.pPlayerInfo = pPlayerInfo;
		Q_snprintf( pInfo.name, 32, "%s", pPlayerInfo->GetName() );
		pInfo.teamid = pPlayerInfo->GetTeamIndex();
		pInfo.classid = 0;

		pInfo.userid = m_context->UserIDFromEntIndex(ent_index);
		m_context->GetSteamIDString(pInfo.userid, pInfo.steamid, sizeof(pInfo.steamid));
	}
}

void CLogStats::ClientDisconnect( int ent_index )
{
	m_entIndexToPlayerInfo[ent_index].reset();
}

void CLogStats::TournamentMatchStarted( const char *RESTRICT hostname, 
									   const char *RESTRICT mapname, 
									   const char *RESTRICT bluname, 
									   const char *RESTRICT redname )
{
	m_context->ServerCommand( "logaddress_add sizzlingstats.com:8006\n" );
	m_context->ServerCommand("sm plugins unload supstats\n");
	m_context->ServerExecute();
	m_context->LogPrint( "[SizzlingStats]: Match Started\n" );
	//char temp[256];
	// DELIMIT THE STRINGS
	//Q_snprintf(temp, 256, "[SizzlingStats]: Match Started<%s><%s><%s><%s>\n", hostname, mapname, bluname, redname);
	//pEngine->LogPrint(temp);
	char temp[128];
	int max_clients = m_context->GetMaxClients();
	for (int i = 1; i < max_clients; ++i)
	{
		playerInfo *pInfo = &m_entIndexToPlayerInfo[i];
		int userid = m_context->UserIDFromEntIndex(i);
		if (userid != -1)
		{
			const char *RESTRICT teamname = teamNames[pInfo->teamid];
			const char *RESTRICT classname = classNames[pInfo->classid];

			Q_snprintf(temp, 128, "[SizzlingStats]: player \"%s\"<%i><%s><%s> is role %s\n", pInfo->name, userid, pInfo->steamid, teamname, classname);
			m_context->LogPrint(temp);
		}
	}
}

void CLogStats::TournamentMatchEnded()
{
	m_context->LogPrint("[SizzlingStats]: Match Ended\n");
	m_context->ServerExecute();
	m_context->ServerCommand( "logaddress_del sizzlingstats.com:8006\n" );
	m_context->ServerExecute();
}

void CLogStats::PreRoundFreezeStarted( bool bTournamentModeOn )
{
	if (bTournamentModeOn)
	{
		m_context->ServerCommand("sm plugins unload supstats\n");
		m_context->ServerExecute();
	}
}

void CLogStats::FireGameEvent( IGameEvent *event )
{
	const char *RESTRICT name = event->GetName();

	if ( FStrEq( name, "player_hurt" ) )
	{
		int damageamount = event->GetInt( "damageamount" );
		if ( damageamount == 0 )
		{
			return;
		}
		int attackerid = event->GetInt( "attacker" );
		if ( ( event->GetInt( "userid" ) != attackerid ) && attackerid != 0 )
		{
			int index = m_context->EntIndexFromUserID(attackerid);
			playerInfo &pInfo = m_entIndexToPlayerInfo[index];

			if ( pInfo.steamid[0] == '\0' )
			{
				m_context->GetSteamIDString(pInfo.userid, pInfo.steamid, sizeof(pInfo.steamid));
			}

			char log[128];
			Q_snprintf( log, 128, "\"%s<%d><%s><%s>\" triggered \"damage\" (damage \"%d\")\n",
				pInfo.name,
				attackerid,
				pInfo.steamid,
				teamNames[pInfo.teamid],
				damageamount );
			WriteLog( log );
			//L 03/21/2011 - 02:32:11: "rline<326><STEAM_0:1:796515><Blue>" triggered "damage" (damage "37")
		}
	}
	else if ( FStrEq( name, "player_healed" ) )
	{
		int healer = event->GetInt( "healer" );
		int index1 = m_context->EntIndexFromUserID(healer);
		playerInfo &pInfo1 = m_entIndexToPlayerInfo[index1];

		int patient = event->GetInt( "patient" );
		int index2 = m_context->EntIndexFromUserID(patient);
		playerInfo &pInfo2 = m_entIndexToPlayerInfo[index2];

		if ( pInfo1.steamid[0] == '\0' )
		{
			m_context->GetSteamIDString(pInfo1.userid, pInfo1.steamid, sizeof(pInfo1.steamid));
		}

		if ( pInfo2.steamid[0] == '\0' )
		{
			m_context->GetSteamIDString(pInfo2.userid, pInfo2.steamid, sizeof(pInfo2.steamid));
		}
					
		char log[196];
		Q_snprintf( log, 196, "\"%s<%d><%s><%s>\" triggered \"healed\" against \"%s<%d><%s><%s>\" (healing \"%d\")\n", 
			pInfo1.name,
			healer,
			pInfo1.steamid,
			teamNames[pInfo1.teamid],
			pInfo2.name,
			patient,
			pInfo2.steamid,
			teamNames[pInfo2.teamid],
			event->GetInt( "amount" ) );
		WriteLog( log );
		//L 03/21/2011 - 02:35:55: "HackLimit2. MixMixMixMix<333><STEAM_0:1:15579670><Blue>" triggered "healed" against "AI kaytee fbs!!<331><STEAM_0:0:9786107><Blue>" (healing "73")
	}
	else if ( FStrEq( name, "item_pickup" ) )
	{
		int userid = event->GetInt( "userid" );
		int index = m_context->EntIndexFromUserID(userid);

		playerInfo &pInfo = m_entIndexToPlayerInfo[index];
		if ( pInfo.steamid[0] == '\0' )
		{
			m_context->GetSteamIDString(pInfo.userid, pInfo.steamid, sizeof(pInfo.steamid));
		}

		char log[128];
		Q_snprintf( log, 128, "\"%s<%d><%s><%s>\" picked up item \"%s\"\n", 
			pInfo.name,
			userid,
			pInfo.steamid,
			teamNames[pInfo.teamid],
			event->GetString( "item" ) );
		WriteLog( log );
		//L 03/21/2011 - 02:35:56: "GooB<330><STEAM_0:1:23384772><Blue>" picked up item "tf_ammo_pack"
	}
	else if ( FStrEq( name, "player_spawn" ) )
	{
		int userid = event->GetInt( "userid" );
		int index = m_context->EntIndexFromUserID(userid);
		playerInfo &pInfo = m_entIndexToPlayerInfo[index];

		if ( pInfo.steamid[0] == '\0' )
		{
			m_context->GetSteamIDString(pInfo.userid, pInfo.steamid, sizeof(pInfo.steamid));
		}

		char log[128];
		Q_snprintf( log, 128, "\"%s<%d><%s><%s>\" spawned as \"%s\"\n", 
			pInfo.name,
			userid,
			pInfo.steamid,
			teamNames[ pInfo.teamid ],
			classNames[ pInfo.classid ] );
		WriteLog( log );

		        //LogToGame("\"%s<%d><%s><%s>\" spawned as \"%s\"",
          //        clientname,
          //        user,
          //        steamid,
          //        team,
          //        classes[clss]);
	}
	else if ( FStrEq( name, "player_changeclass" ) )
	{
		int userid = event->GetInt( "userid" );
		int index = m_context->EntIndexFromUserID(userid);
		m_entIndexToPlayerInfo[index].classid = event->GetInt( "class" );
	}
	else if ( FStrEq( name, "player_team" ) )
	{
		int userid = event->GetInt( "userid" );
		int index = m_context->EntIndexFromUserID(userid);
		m_entIndexToPlayerInfo[index].teamid = event->GetInt( "team" );
		Q_snprintf( m_entIndexToPlayerInfo[index].name, 32, "%s", event->GetString( "name" ) );
	}
	else if ( FStrEq( name, "player_changename" ) )
	{
		int userid = event->GetInt( "userid" );
		int index = m_context->EntIndexFromUserID(userid);
		Q_snprintf( m_entIndexToPlayerInfo[index].name, 32, "%s", event->GetString( "newname" ) );
	}
}

void CLogStats::WriteLog( const char *msg )
{
	if (!ss_logging.GetBool())
	{
		m_context->LogPrint(msg);
	}
}
