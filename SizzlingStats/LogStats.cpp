
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

#include "LogStats.h"
#include "PluginContext.h"
#include "strtools.h"
#include "UserIdTracker.h"
#include "platform.h"

struct playerInfo
{
	playerInfo()
	{
		reset();
	}
	// which ones do i really need to reset? (instead of doing all of them)
	void reset() {
		memset( steamid, 0, 64 );
		memset( name, 0, 32 );
		teamid = 0;
		classid = 0;
		pEdict = NULL;
		pPlayerInfo = NULL;
		pSteamId = NULL;
	}
	char steamid[64];
	char name[32];
	int teamid;
	int classid;
	edict_t *pEdict;
	IPlayerInfo *pPlayerInfo;
	const CSteamID *pSteamId;
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

CLogStats::CLogStats( const CPluginContext &plugin_context ):
	m_context(plugin_context),
	m_entIndexToPlayerInfo(new playerInfo[33])
{
}

CLogStats::~CLogStats()
{
	delete [] m_entIndexToPlayerInfo;
}

bool CLogStats::Load()
{
	IVEngineServer *pEng = m_context.GetEngine();
	IGameEventManager2 *pGE = m_context.GetEventMgr();
	if (pEng && pGE)
	{
		pGE->AddListener( this, "item_pickup", true );
		pGE->AddListener( this, "player_hurt", true );
		pGE->AddListener( this, "player_healed", true );
		pGE->AddListener( this, "player_spawn", true );
		pGE->AddListener( this, "player_team", true );
		pGE->AddListener( this, "player_changename", true );
		pGE->AddListener( this, "player_changeclass", true );

		pEng->ServerCommand( "log on\n" );
		pEng->ServerCommand("sm plugins unload supstats\n");
		pEng->ServerExecute();
		return true;
	}
	return false;
}

void CLogStats::Unload()
{
	IVEngineServer *pEng = m_context.GetEngine();
	IGameEventManager2 *pGE = m_context.GetEventMgr();
	if (pEng && pGE)
	{
		pGE->RemoveListener(this);
		pEng->ServerCommand("sm plugins load supstats\n");
		pEng->ServerExecute();
	}
}

void CLogStats::LevelInit( const char *pMapName )
{
	IVEngineServer *pEng = m_context.GetEngine();
	pEng->ServerCommand( "log on\n" );
	pEng->ServerExecute();

	char maplog[64];
	Q_snprintf( maplog, 64, "Loading map \"%s\"\n", pMapName );
	pEng->LogPrint( maplog );
}

void CLogStats::ClientActive( edict_t *pEdict, int ent_index )
{
	IPlayerInfo *pPlayerInfo = m_context.GetPlayerInfoMgr()->GetPlayerInfo( pEdict );
	if ( pPlayerInfo && pPlayerInfo->IsConnected() )
	{
		playerInfo &pInfo = m_entIndexToPlayerInfo[ent_index];
		pInfo.pEdict = pEdict;
		pInfo.pPlayerInfo = pPlayerInfo;
		Q_snprintf( pInfo.name, 32, "%s", pPlayerInfo->GetName() );
		pInfo.teamid = pPlayerInfo->GetTeamIndex();
		pInfo.classid = 0;
		pInfo.pSteamId = m_context.GetEngine()->GetClientSteamID( pEdict );
		if ( pInfo.pSteamId )
		{
			Q_snprintf( pInfo.steamid, 64, "%s", pInfo.pPlayerInfo->GetNetworkIDString() );
		}
	}
}

void CLogStats::ClientDisconnect( edict_t *pEdict )
{
	int index = m_context.GetEngine()->IndexOfEdict( pEdict );
	m_entIndexToPlayerInfo[index].reset();
}

void CLogStats::TournamentMatchStarted( const char *RESTRICT hostname, 
									   const char *RESTRICT mapname, 
									   const char *RESTRICT bluname, 
									   const char *RESTRICT redname )
{
	IVEngineServer *pEngine = m_context.GetEngine();
	pEngine->ServerCommand( "logaddress_add sizzlingstats.com:8006\n" );
	pEngine->ServerCommand("sm plugins unload supstats\n");
	pEngine->ServerExecute();
	pEngine->LogPrint( "[SizzlingStats]: Match Started\n" );
	//char temp[256];
	// DELIMIT THE STRINGS
	//Q_snprintf(temp, 256, "[SizzlingStats]: Match Started<%s><%s><%s><%s>\n", hostname, mapname, bluname, redname);
	//pEngine->LogPrint(temp);
	char temp[128];
	for (int i = 0; i < 33; ++i)
	{
		playerInfo *pInfo = &m_entIndexToPlayerInfo[i];
		if (pInfo->pEdict)
		{
			Q_strncpy(pInfo->name, pInfo->pPlayerInfo->GetName(), 32);
			int userid = pEngine->GetPlayerUserId(pInfo->pEdict);
			const char *RESTRICT teamname = teamNames[pInfo->teamid];
			const char *RESTRICT classname = classNames[pInfo->classid];
			Q_snprintf(temp, 128, "[SizzlingStats]: player \"%s\"<%i><%s><%s> is role %s\n", pInfo->name, userid, pInfo->steamid, teamname, classname);
			pEngine->LogPrint(temp);
		}
	}
}

void CLogStats::TournamentMatchEnded()
{
	IVEngineServer *pEngine = m_context.GetEngine();
	pEngine->LogPrint("[SizzlingStats]: Match Ended\n");
	pEngine->ServerExecute();
	pEngine->ServerCommand( "logaddress_del sizzlingstats.com:8006\n" );
	pEngine->ServerExecute();
}

void CLogStats::PreRoundFreezeStarted( bool bTournamentModeOn )
{
	if (bTournamentModeOn)
	{
		IVEngineServer *pEngine = m_context.GetEngine();
		pEngine->ServerCommand("sm plugins unload supstats\n");
		pEngine->ServerExecute();
	}
}

void CLogStats::FireGameEvent( IGameEvent *event )
{
	const char *RESTRICT name = event->GetName();
	IVEngineServer *pEngine = m_context.GetEngine();

	if ( FStrEq( name, "player_hurt" ) )
	{
		int damageamount = event->GetInt( "damageamount" );
		if ( damageamount == 0 )
			return;
		int attackerid = event->GetInt( "attacker" );
		if ( ( event->GetInt( "userid" ) != attackerid ) && attackerid != 0 )
		{
			int index = g_pUserIdTracker->GetEntIndex(attackerid);
			playerInfo &pInfo = m_entIndexToPlayerInfo[index];

			if ( !pInfo.pSteamId )
			{
				pInfo.pSteamId = pEngine->GetClientSteamID( pInfo.pEdict );
				if ( !pInfo.pSteamId )
					return;
				Q_snprintf( pInfo.steamid, 64, "%s", pInfo.pPlayerInfo->GetNetworkIDString() );
			}

			char log[128];
			Q_snprintf( log, 128, "\"%s<%d><%s><%s>\" triggered \"damage\" (damage \"%d\")\n",
				pInfo.name,
				attackerid,
				pInfo.steamid,
				teamNames[pInfo.teamid],
				damageamount );
			pEngine->LogPrint( log );
			//L 03/21/2011 - 02:32:11: "rline<326><STEAM_0:1:796515><Blue>" triggered "damage" (damage "37")
		}
	}
	else if ( FStrEq( name, "player_healed" ) )
	{
		int healer = event->GetInt( "healer" );
		int index1 = g_pUserIdTracker->GetEntIndex( healer );
		playerInfo &pInfo1 = m_entIndexToPlayerInfo[index1];

		int patient = event->GetInt( "patient" );
		int index2 = g_pUserIdTracker->GetEntIndex( patient );
		playerInfo &pInfo2 = m_entIndexToPlayerInfo[index2];

		if ( !pInfo1.pSteamId )
		{
			pInfo1.pSteamId = pEngine->GetClientSteamID( pInfo1.pEdict );
			if ( !pInfo1.pSteamId )
				return;
			Q_snprintf( pInfo1.steamid, 64, "%s", pInfo1.pPlayerInfo->GetNetworkIDString() );
		}

		if ( !pInfo2.pSteamId )
		{
			pInfo2.pSteamId = pEngine->GetClientSteamID( pInfo2.pEdict );
			if ( !pInfo2.pSteamId )
				return;
			Q_snprintf( pInfo2.steamid, 64, "%s", pInfo2.pPlayerInfo->GetNetworkIDString() );
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
		pEngine->LogPrint( log );
		//L 03/21/2011 - 02:35:55: "HackLimit2. MixMixMixMix<333><STEAM_0:1:15579670><Blue>" triggered "healed" against "AI kaytee fbs!!<331><STEAM_0:0:9786107><Blue>" (healing "73")
	}
	else if ( FStrEq( name, "item_pickup" ) )
	{
		int userid = event->GetInt( "userid" );
		int index = g_pUserIdTracker->GetEntIndex( userid );

		playerInfo &pInfo = m_entIndexToPlayerInfo[index];
		if ( !pInfo.pSteamId )
		{
			pInfo.pSteamId = pEngine->GetClientSteamID( pInfo.pEdict );
			if ( !pInfo.pSteamId )
				return;
			Q_snprintf( pInfo.steamid, 64, "%s", pInfo.pPlayerInfo->GetNetworkIDString() );
		}

		char log[128];
		Q_snprintf( log, 128, "\"%s<%d><%s><%s>\" picked up item \"%s\"\n", 
			pInfo.name,
			userid,
			pInfo.steamid,
			teamNames[pInfo.teamid],
			event->GetString( "item" ) );
		pEngine->LogPrint( log );
		//L 03/21/2011 - 02:35:56: "GooB<330><STEAM_0:1:23384772><Blue>" picked up item "tf_ammo_pack"
	}
	else if ( FStrEq( name, "player_spawn" ) )
	{
		int userid = event->GetInt( "userid" );
		int index = g_pUserIdTracker->GetEntIndex( userid );
		playerInfo &pInfo = m_entIndexToPlayerInfo[index];

		if ( !pInfo.pSteamId )
		{
			pInfo.pSteamId = pEngine->GetClientSteamID( pInfo.pEdict );
			if ( !pInfo.pSteamId )
				return;
			Q_snprintf( pInfo.steamid, 64, "%s", pInfo.pPlayerInfo->GetNetworkIDString() );
		}

		char log[128];
		Q_snprintf( log, 128, "\"%s<%d><%s><%s>\" spawned as \"%s\"\n", 
			pInfo.name,
			userid,
			pInfo.steamid,
			teamNames[ pInfo.teamid ],
			classNames[ pInfo.classid ] );
		pEngine->LogPrint( log );

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
		int index = g_pUserIdTracker->GetEntIndex( userid );
		m_entIndexToPlayerInfo[index].classid = event->GetInt( "class" );
	}
	else if ( FStrEq( name, "player_team" ) )
	{
		int userid = event->GetInt( "userid" );
		int index = g_pUserIdTracker->GetEntIndex( userid );
		m_entIndexToPlayerInfo[index].teamid = event->GetInt( "team" );
	}
	else if ( FStrEq( name, "player_changename" ) )
	{
		int userid = event->GetInt( "userid" );
		int index = g_pUserIdTracker->GetEntIndex( userid );
		Q_snprintf( m_entIndexToPlayerInfo[index].name, 32, "%s", event->GetString( "newname" ) );
	}
}
