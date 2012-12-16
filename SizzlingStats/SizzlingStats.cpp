////////////////////////////////////////////////////////////////////////////////
// Filename: SizzlingStats.cpp
////////////////////////////////////////////////////////////////////////////////
#include "SizzlingStats.h"
#include "PlayerMessage.h"
#include "playerdata.h"
#include "SC_helpers.h"
//#include "filesystem.h"
//#include "WebStatsHandler.h"
#include "HtmlGenerator.h"
#include "FtpUploader.h"
#include "utlbuffer.h"
#include "vstdlib/jobthread.h"
#include "ThreadCallQueue.h"
#include "curl/curl.h"
#include "engine/IEngineTrace.h"
#include "mathlib/vector.h"

#include "fasttimer.h"

#define GAME_DLL 1
#include "networkvar.h"

#include "eiface.h"
//#include "engine/iserverplugin.h"
#include "game/server/iplayerinfo.h"

#include "steam/steamclientpublic.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//extern PlayerMessage	*g_pMessage;
extern CGlobalVars		*gpGlobals;
extern IVEngineServer	*pEngine;
extern IPlayerInfoManager *playerinfomanager;
extern CTSCallQueue		*g_pTSCallQueue;
extern IEngineTrace		*enginetrace;

extern IServerGameEnts			*pServerEnts;

JOB_INTERFACE IThreadPool *g_pThreadPool;

#ifndef PUBLIC_RELEASE

static ConVar ftp_user("sizz_stats_ftp_username", "username", FCVAR_DONTRECORD | FCVAR_PROTECTED, "FTP username to connect with when using web stats.");
static ConVar ftp_pass("sizz_stats_ftp_password", "password", FCVAR_DONTRECORD | FCVAR_PROTECTED, "FTP password to connect with when using web stats.");
static ConVar ftp_server("sizz_stats_ftp_hostname", "ftp.myserver.com", FCVAR_DONTRECORD | FCVAR_PROTECTED, "FTP hostname to connect to when using web stats.");
static ConVar ftp_port("sizz_stats_ftp_port", "21", FCVAR_DONTRECORD | FCVAR_PROTECTED, "FTP port to use when using web stats.");
ConVar web_hostname("sizz_stats_web_hostname", "myserver.com", FCVAR_DONTRECORD | FCVAR_PROTECTED, "Hostname of the external public directory of the FTP server.");

static ConVar show_msg("sizz_stats_show_chat_messages", "0", FCVAR_NONE, "If nonzero, shows chat messages by the plugin");

#endif

#pragma warning( push )
#pragma warning( disable : 4351 )
SizzlingStats::SizzlingStats(): m_aPropOffsets(),
								m_PlayerFlagsOffset(0), 
								m_TeamRoundsWonOffset(0),
								m_PlayerClassOffset(0), // what am i using this for yet??
								m_nCurrentRound(0),
								m_playerDataArchive(),
								//m_playerDataArchiveVec(0, 32),
								m_PlayerDataManager(),
								m_pWebStatsHandler(NULL),
								m_refHostname((IConVar*)NULL),
								m_refBlueTeamName((IConVar*)NULL),
								m_refRedTeamName((IConVar*)NULL),
								m_hostInfo(),
								m_flRoundDuration(0),
								m_flMatchDuration(0),
								m_bTournamentMatchRunning(false)
{
	m_playerDataArchive.Init(32);

	// element 0 will never be used, element 33 might be used (in 33 slot servers)
	// we are still allocating for element 0 anyways... who cares
	//
	// should we allocate in here? or should we wait until we insert players
	// lets wait for now
	/*for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		m_pPlayerData[i] = m_PlayerDataMemPool.Alloc();
		m_pEntIndexToExtraData[i] = m_ExtraDataMemPool.Alloc();
	}*/
	//m_SteamidToPlayerDataMap.SetLessFunc( SCHelpers::FUIntCmp );
	//m_entIndexToExtraDataMap.SetLessFunc( SCHelpers::FIntCmp );
}
#pragma warning( pop )

SizzlingStats::~SizzlingStats()
{
}

void SizzlingStats::Load()
{
    GetPropOffsets();
	m_refHostname.Init("hostname", false);
	m_refBlueTeamName.Init("mp_tournament_blueteamname", false);
	m_refRedTeamName.Init("mp_tournament_redteamname", false);
#ifndef PUBLIC_RELEASE
	m_pWebStatsHandler = new CWebStatsHandler();
#else
	m_pWebStatsHandler = new CNullWebStatsHandler();
#endif
}

void SizzlingStats::Unload()
{
	delete m_pWebStatsHandler;
}

void SizzlingStats::LevelInit(const char *pMapName)
{
}

void SizzlingStats::GameFrame()
{
}

void SizzlingStats::PlayerHealed( int entindex, int amount )
{
	playerAndExtra_t data = m_PlayerDataManager.GetPlayerData(entindex);
	data.m_pExtraData->healsrecv += amount;
}

void SizzlingStats::MedPick( int entindex )
{
	playerAndExtra_t data = m_PlayerDataManager.GetPlayerData(entindex);
	data.m_pExtraData->medpicks += 1;
}

void SizzlingStats::UberDropped( int entindex )
{
	playerAndExtra_t data = m_PlayerDataManager.GetPlayerData(entindex);
	data.m_pExtraData->ubersdropped += 1;
}

void SizzlingStats::PlayerChangedClass( int entindex, EPlayerClass player_class )
{
	if (player_class != k_ePlayerClassMedic)
	{
		m_vecMedics.FindAndRemove(entindex);
	}
	else
	{
		m_vecMedics.AddToTail(entindex);
	}

	m_PlayerDataManager.PlayerChangedClass( entindex, player_class, SCHelpers::RoundDBL(Plat_FloatTime()) );
}

void SizzlingStats::ChatEvent( int entindex, const char *pText, bool bTeamChat )
{
	const char *pSteamId = m_PlayerDataManager.GetPlayerData(entindex).m_pPlayerData->GetPlayerInfo()->GetNetworkIDString();
	// during the match, m_flMatchDuration is the Plat_FloatTime() from when the game started
	// so subtracting gets the time since the match started
	m_pWebStatsHandler->PlayerChatEvent(Plat_FloatTime() - m_flMatchDuration, pSteamId, pText, bTeamChat);
}

class CTraceFilterSkipTwo: public ITraceFilter
{
public:
	CTraceFilterSkipTwo( CBaseEntity *pEnt1, CBaseEntity *pEnt2 ):
		m_pEnt1(pEnt1),
		m_pEnt2(pEnt2)
	{
	}

	virtual bool ShouldHitEntity( IHandleEntity *pServerEntity, int contentsMask )
	{ 
		CBaseEntity *pEnt = SCHelpers::BaseHandleToBaseEntity(&pServerEntity->GetRefEHandle());
		if (pEnt == m_pEnt1 || pEnt == m_pEnt2)
		{
			return false;
		}
		return true;
	}

	virtual TraceType_t	GetTraceType() const
	{
		return TRACE_EVERYTHING_FILTER_PROPS;
	}

private:
	CBaseEntity *m_pEnt1;
	CBaseEntity *m_pEnt2;
};

static char *UTIL_VarArgs( char *format, ... )
{
    va_list     argptr;
    static char     string[1024];
    
    va_start (argptr, format);
    Q_vsnprintf(string, sizeof(string), format,argptr);
    va_end (argptr);

    return string;  
}

void SizzlingStats::GiveUber( int entindex )
{
	SS_PlayerData *pData = m_PlayerDataManager.GetPlayerData(entindex).m_pPlayerData;
	if (pData && (pData->GetClass(m_PlayerClassOffset) == 5))
	{
		CBaseHandle *hMedigun = (CBaseHandle*)((unsigned char *)(pData->GetBaseEntity()) + m_iWeaponsOffset + 4); // +4 because we want the medigun slot
		CBaseEntity *pMedigun = SCHelpers::BaseHandleToBaseEntity(hMedigun);

		if (pMedigun)
		{
			float *flChargeLevel = (float*)((unsigned char *)pMedigun + m_iChargeLevelOffset);
			*flChargeLevel = 1.0f;
		}
	}
}

void SizzlingStats::CheckPlayerDropped( int victimIndex )
{
	for (int i = 0; i < m_vecMedics.Count(); ++i)
	{
		int medIndex = m_vecMedics[i];
		SS_PlayerData *pMedData = m_PlayerDataManager.GetPlayerData(medIndex).m_pPlayerData;
		SS_PlayerData *pVictimData = m_PlayerDataManager.GetPlayerData(victimIndex).m_pPlayerData;
		if ( pMedData->GetPlayerInfo()->GetTeamIndex() == pVictimData->GetPlayerInfo()->GetTeamIndex() )
		{
			using namespace SCHelpers;
			CBaseHandle *hMedigun = ByteOffsetFromPointer<CBaseHandle>(pMedData->GetBaseEntity(), m_iWeaponsOffset+4); // +4 because we want the medigun slot
			CBaseEntity *pMedigun = BaseHandleToBaseEntity(hMedigun);

			const char *szWeapon = SCHelpers::GetClassname(pMedigun);
			//if ( SCHelpers::FStrEq(szWeapon, "tf_weapon_medigun") )
			{
				float flChargeLevel = *ByteOffsetFromPointer<float>(pMedigun, m_iChargeLevelOffset);
				uint32 charge = static_cast<uint32>(flChargeLevel);

				bool bReleasingCharge = *ByteOffsetFromPointer<bool>(pMedigun, m_iChargeReleaseOffset);

				if (charge == 1 || bReleasingCharge)
				{
					Vector *victimPos = ByteOffsetFromPointer<Vector>(pVictimData->GetBaseEntity(), m_iOriginOffset);
					Vector *medPos = ByteOffsetFromPointer<Vector>(pMedData->GetBaseEntity(), m_iOriginOffset);
		
					vec_t distance = victimPos->DistToSqr( *medPos );
					SS_AllUserChatMessage( UTIL_VarArgs("distance: %.2f\n", distance) );
					if (static_cast<uint32>(distance) <= 230400) // ~480 units is max target distance for medigun
					{
						Ray_t ray;
						ray.Init(*medPos, *victimPos);
						CTraceFilterSkipTwo traceFilter(pMedData->GetBaseEntity(), pMedigun);
						trace_t trace;
						enginetrace->TraceRay(ray, MASK_SHOT_HULL, &traceFilter, &trace);
						if (!trace.DidHit())
						{
							SS_AllUserChatMessage( "player dropped\n" );
						}
					}
				}
			}
		}
	}
}

void SizzlingStats::CapFix( const char *cappers, int length )
{
	for (int i = 0; i < length; ++i)
	{
		int index = cappers[i];
		m_PlayerDataManager.SetCapFix(index);
	}
}

bool SizzlingStats::SS_InsertPlayer( edict_t *pEdict )
{
	Msg( "SS_InsertPlayer\n" );
	engineContext_t context = { playerinfomanager, pEngine };
	return m_PlayerDataManager.InsertPlayer(context, pEdict);
	
/*
	PROFILE_SCOPE( new_insert_player );
	Msg( "SS_InsertPlayer\n" );
	if ( !pEdict || pEdict->IsFree() )
	{
		SS_Msg("invalid edict, aborting insert\n");
		return false;
	}

	IPlayerInfo *pPlayerInfo = playerinfomanager->GetPlayerInfo(pEdict);
	if (pPlayerInfo)
	{
		if (!(pPlayerInfo->IsConnected()))
		{
			SS_Msg("error: player not yet connected, aborting insert\n");
		}
	}
	else
	{
		SS_Msg("error: could not get player info, aborting insert\n");
		return false;
	}

	const CSteamID *pSteamID = pEngine->GetClientSteamID(pEdict);
	if (!pSteamID)
	{
		SS_Msg("error: client %s not authenticated with steam, aborting insert\n", pPlayerInfo->GetName());
		return false;
	}

	//unsigned int accountId = pSteamID->GetAccountID();
	// check if info is in the archive (hashmap)
	// if it is, move the data to the ptr array
	// null the entry in the hashmap
	UtlHashFastHandle_t hashHandle = m_playerDataArchive.InvalidHandle();//.Find(accountId);
	// if the data is not in our archive
	if (hashHandle == m_playerDataArchive.InvalidHandle())
	{
		// for when we allocate at the beginning
		//m_pPlayerData[pEngine->IndexOfEdict(pEdict)]->SetBaseData(pEdict, pPlayerInfo);
		int entIndex = pEngine->IndexOfEdict(pEdict);
		m_pPlayerData[entIndex] = m_PlayerDataMemPool.Alloc();
		m_pPlayerData[entIndex]->SetBaseData(pEdict, pPlayerInfo);
		m_pEntIndexToExtraData[entIndex] = m_ExtraDataMemPool.Alloc();

		m_nCurrentPlayers += 1;
		SS_Msg( "current players: %i\n", m_nCurrentPlayers );
		SS_Msg( "Stats for player #%i: '%s' will be tracked\n", pSteamID->GetAccountID(), pPlayerInfo->GetName() );
	}
	else // we have an archive of the player
	{
		// if the hash handle is valid, i guess this pointer will be too
		// TODO: need to think about how to restore stats if the person is rejoining a round
		playerAndExtra &data = m_playerDataArchive.Element(hashHandle);
		m_playerDataArchive.Remove(hashHandle);
		// we need to remove the archive mempool pointers from the vector

		int entIndex = pEngine->IndexOfEdict(pEdict);
		m_pPlayerData[entIndex] = data.m_pPlayerData;
		m_pPlayerData[entIndex]->SetBaseData(pEdict, pPlayerInfo);
		m_pEntIndexToExtraData[entIndex] = data.m_pExtraData;
	}
	return true;
*/
}

void SizzlingStats::SS_DeletePlayer( edict_t *pEdict )
{
	Msg( "SS_DeletePlayer\n" );
	int entindex = pEngine->IndexOfEdict(pEdict);
	m_vecMedics.FindAndRemove(entindex);
	engineContext_t context = { playerinfomanager, pEngine };
	m_PlayerDataManager.RemovePlayer(context, pEdict);
	
/*
	if ( !pEdict || pEdict->IsFree() )
	{
		SS_Msg("error: invalid edict, aborting delete\n");
		return false;
	}

	const CSteamID *pSteamID = pEngine->GetClientSteamID(pEdict);
	if (!pSteamID)
	{
		// TODO: verify that this can happen
		SS_Msg("error: client not authenticated with steam, aborting delete\n");
		return false;
	}

	int entIndex = pEngine->IndexOfEdict(pEdict);
	//playerAndExtra data = {m_pPlayerData[entIndex], m_pEntIndexToExtraData[entIndex]};

	// insert vs fastinsert, insert checks for duplicates
	//m_playerDataArchive.FastInsert(pSteamID->GetAccountID(), data);
	m_PlayerDataMemPool.Free(m_pPlayerData[entIndex]);
	m_pPlayerData[entIndex] = NULL;
	m_ExtraDataMemPool.Free(m_pEntIndexToExtraData[entIndex]);
	m_pEntIndexToExtraData[entIndex] = NULL;

	SS_Msg( "deleted data index #%i\n", entIndex );
	SS_Msg( "size before delete: %i\n", m_nCurrentPlayers );
	m_nCurrentPlayers -= 1;
	SS_Msg( "size after delete: %i\n", m_nCurrentPlayers );
	return true;
*/
}

void SizzlingStats::SS_DeleteAllPlayerData()
{
	SS_Msg( "deleting all data\n" );
	m_vecMedics.RemoveAll();
	engineContext_t context = { playerinfomanager, pEngine };
	m_PlayerDataManager.RemoveAllPlayers(context);
	
/*
	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		if (m_pPlayerData[i])
		{
			m_PlayerDataMemPool.Free(m_pPlayerData[i]);
			m_pPlayerData[i] = NULL;
			m_ExtraDataMemPool.Free(m_pEntIndexToExtraData[i]);
			m_pEntIndexToExtraData[i] = NULL;

			SS_Msg( "deleted data index #%i\n", i );
			SS_Msg( "size before delete: %i\n", m_nCurrentPlayers );
			m_nCurrentPlayers -= 1;
			SS_Msg( "size after delete: %i\n", m_nCurrentPlayers );
		}
	}
	
	m_playerDataArchive.RemoveAll();

	// clear messes everything up
	// need to reinit after a clear
	// but init is protected
	//m_PlayerDataMemPool.Clear();
	//m_ExtraDataMemPool.Clear();
	
	SS_Msg( "all data successfully cleared\n" );

	return true;
*/
}

void SizzlingStats::SS_Msg( const char *pMsg, ... )
{
	va_list argList;
	va_start( argList, pMsg );
	char message[96];

	V_vsnprintf( message, 96, pMsg, argList );
	Msg( "[SS]: %s", message );

	va_end( argList );
}

void SizzlingStats::SS_SingleUserChatMessage( edict_t *pEntity, const char *szMessage )
{
#ifndef PUBLIC_RELEASE
	if (show_msg.GetInt() != 0)
#endif
	CPlayerMessage::SingleUserChatMessage( pEntity, szMessage );
}

void SizzlingStats::SS_AllUserChatMessage( const char *szMessage )
{
#ifndef PUBLIC_RELEASE
	if (show_msg.GetInt() != 0)
#endif
	CPlayerMessage::AllUserChatMessage( szMessage, "\x04[\x05SizzlingStats\x04]\x06: \x03" );
	//g_pMessage->AllUserChatMessage( szMessage, "\x01\\x01\x02\\x02\x03\\x03\x04\\x04\x05\\x05\x06\\x06\x07\\x07\x08\\x08\x09\\x09\n" );
}

void SizzlingStats::SS_TournamentMatchStarted()
{
	Msg( "tournament match started\n" );
	m_bTournamentMatchRunning = true;
	m_flMatchDuration = Plat_FloatTime();

	V_strncpy(m_hostInfo.m_hostname, m_refHostname.GetString(), 64);
	V_strncpy(m_hostInfo.m_mapname, gpGlobals->mapname.ToCStr(), 64);
	V_strncpy(m_hostInfo.m_bluname, m_refBlueTeamName.GetString(), 32);
	V_strncpy(m_hostInfo.m_redname, m_refRedTeamName.GetString(), 32);
	m_hostInfo.m_roundduration = m_flRoundDuration;
	m_pWebStatsHandler->SetHostData(m_hostInfo);

	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		playerAndExtra_t data = m_PlayerDataManager.GetPlayerData(i);
		if (data.m_pPlayerData)
		{
			playerInfo_t info;
			V_strncpy(info.m_name, data.m_pPlayerData->GetPlayerInfo()->GetName(), 32);
			V_strncpy(info.m_steamid, data.m_pPlayerData->GetPlayerInfo()->GetNetworkIDString(), 32);
			info.m_teamid = data.m_pPlayerData->GetPlayerInfo()->GetTeamIndex();
			info.m_mostPlayedClass = data.m_pPlayerData->GetClass(m_PlayerClassOffset);
			m_pWebStatsHandler->EnqueuePlayerInfo(info);
		}
	}

	m_pWebStatsHandler->SendGameStartEvent();
}

void SizzlingStats::SS_TournamentMatchEnded()
{
	Msg( "tournament match ended\n" );
	m_bTournamentMatchRunning = false;
	m_flMatchDuration = Plat_FloatTime() - m_flMatchDuration;
	m_pWebStatsHandler->SendGameOverEvent(m_flMatchDuration);
	SetTeamScores(0, 0);
}

void SizzlingStats::SS_PreRoundFreeze()
{
	Msg( "pre-round started\n" );
	SS_ResetData();
	double curtime = Plat_FloatTime();
	m_flRoundDuration = curtime;
	m_PlayerDataManager.ResetAndStartClassTracking(m_PlayerClassOffset, SCHelpers::RoundDBL(curtime));
}

void SizzlingStats::SS_RoundStarted()
{
	Msg( "round started\n" );
	SS_AllUserChatMessage( "Stats Recording Started\n" );
}

void SizzlingStats::SS_RoundEnded()
{
	Msg( "round ended\n" );
	double curtime = Plat_FloatTime();
	m_flRoundDuration = curtime - m_flRoundDuration;
	m_PlayerDataManager.StopClassTracking(SCHelpers::RoundDBL(curtime));
	SS_AllUserChatMessage( "Stats Recording Stopped\n" );
	SS_EndOfRound();
}

void SizzlingStats::SS_DisplayStats( SS_PlayerData &playerData )
{
	char pText[64] = {};
	int kills = playerData.GetStat(m_nCurrentRound, Kills);
	int assists = playerData.GetStat(m_nCurrentRound, KillAssists);
	int deaths = playerData.GetStat(m_nCurrentRound, Deaths);
	int damagedone = playerData.GetStat(m_nCurrentRound, DamageDone);
	int amounthealed = playerData.GetStat(m_nCurrentRound, HealPoints);
	int points = playerData.GetStat(m_nCurrentRound, Points);
	int backstabs = playerData.GetStat(m_nCurrentRound, Backstabs);
	int headshots = playerData.GetStat(m_nCurrentRound, Headshots);
	int captures = playerData.GetStat(m_nCurrentRound, Captures);
	int defenses = playerData.GetStat(m_nCurrentRound, Defenses);
	int healsrecv = playerData.GetStat(m_nCurrentRound, HealsReceived);
	int ubers = playerData.GetStat(m_nCurrentRound, Invulns);
	int drops = playerData.GetStat(m_nCurrentRound, UbersDropped);
	int medpicks = playerData.GetStat(m_nCurrentRound, MedPicks);
	
	edict_t *pEntity = playerData.GetEdict();

	V_snprintf( pText, 64, "\x04[\x05SizzlingStats\x04]\x06: \x03%s\n", playerData.GetPlayerInfo()->GetName() );
	SS_SingleUserChatMessage( pEntity, pText );

	memset( pText, 0, sizeof(pText) );
	if ( deaths != 0 )
		V_snprintf( pText, 64, "K/D: %i:%i (%.2f), Assists: %i\n", kills, deaths, (double)kills/(double)deaths, assists );
	else
		V_snprintf( pText, 64, "K/D: %i:%i, Assists: %i\n", kills, deaths, assists );
	SS_SingleUserChatMessage( pEntity, pText );

	memset( pText, 0, sizeof(pText) );
	//Msg( "class: %i\n", *((int *)(((unsigned char *)playerData.GetBaseEntity()) + m_PlayerClassOffset)) );
	if ( (playerData.GetClass(m_PlayerClassOffset) - 5) != 0 ) // if the player isn't a medic
	{
		V_snprintf( pText, 64, "Damage Done: %i, Heals Received (not incl. buffs): %i\n", damagedone, healsrecv );
	}
	else
	{
		V_snprintf( pText, 64, "Ubers/Kritz Used: %i, Ubers/Kritz Dropped: %i", ubers, drops );
		//SS_SingleUserChatMessage( pEntity, pText );
		//memset( pText, 0, size );
		//V_snprintf( pText, 64, "Avg Charge Time: %i seconds", chargetime );
		//SS_SingleUserChatMessage( pEntity, pText );
	}
	SS_SingleUserChatMessage( pEntity, pText );

	if ( amounthealed != 0 )
	{
		memset( pText, 0, sizeof(pText) );
		V_snprintf( pText, 64, "Amount Healed: %i\n", amounthealed );
		SS_SingleUserChatMessage( pEntity, pText );
	}

	if ( medpicks != 0 )
	{
		memset( pText, 0, sizeof(pText) );
		V_snprintf( pText, 64, "Medic Picks: %i\n", medpicks );
		SS_SingleUserChatMessage( pEntity, pText );
	}

	if ( (backstabs != 0) && (headshots != 0) )
	{
		memset( pText, 0, sizeof(pText) );
		V_snprintf( pText, 64, "Backstabs: %i, Headshots: %i\n", backstabs, headshots );
		SS_SingleUserChatMessage( pEntity, pText );
	}
	else if ( (backstabs != 0) && (headshots == 0) )
	{
		memset( pText, 0, sizeof(pText) );
		V_snprintf( pText, 64, "Backstabs: %i\n", backstabs );
		SS_SingleUserChatMessage( pEntity, pText );
	}
	else if ( (backstabs == 0) && (headshots != 0) )
	{
		memset( pText, 0, sizeof(pText) );
		V_snprintf( pText, 64, "Headshots: %i\n", headshots );
		SS_SingleUserChatMessage( pEntity, pText );
	}

	memset( pText, 0, sizeof(pText) );
	V_snprintf( pText, 64, "Captures: %i, Defenses: %i\n", captures, defenses );
	SS_SingleUserChatMessage( pEntity, pText );

	memset( pText, 0, sizeof(pText) );
	V_snprintf( pText, 64, "Round Score: %i\n", points );
	SS_SingleUserChatMessage( pEntity, pText );
}

void SizzlingStats::SS_EndOfRound()
{
	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		playerAndExtra_t data = m_PlayerDataManager.GetPlayerData(i);
		if (data.m_pPlayerData)
		{
			// UpdateRoundData needs to be before the UpdateExtraData or crash cause no vector
			data.m_pPlayerData->UpdateRoundData(m_nCurrentRound, m_aPropOffsets);
			data.m_pPlayerData->UpdateExtraData(m_nCurrentRound, *data.m_pExtraData);

			if (data.m_pPlayerData->GetPlayerInfo()->GetTeamIndex() > 1)
			{
				if (m_bTournamentMatchRunning)
				{
					playerWebStats_t stats;
					stats.m_scoreData = data.m_pPlayerData->GetScoreData(m_nCurrentRound);
					V_strncpy(stats.m_playerInfo.m_name, data.m_pPlayerData->GetPlayerInfo()->GetName(), 32);
					V_strncpy(stats.m_playerInfo.m_steamid, data.m_pPlayerData->GetPlayerInfo()->GetNetworkIDString(), 32);
					stats.m_playerInfo.m_teamid = data.m_pPlayerData->GetPlayerInfo()->GetTeamIndex();
					CPlayerClassTracker *pTracker = data.m_pPlayerData->GetClassTracker();
					stats.m_playerInfo.m_mostPlayedClass = pTracker->GetMostPlayedClass();
					stats.m_playerInfo.m_playedClasses = pTracker->GetPlayedClasses();
					m_pWebStatsHandler->EnqueuePlayerStats(stats);
				}
				SS_DisplayStats( *data.m_pPlayerData );
			}
			data.m_pPlayerData->UpdateTotalData( m_nCurrentRound );
			//data.m_pPlayerData->ResetExtraData( m_nCurrentRound ); // this happens in pre-round, so test commenting this
		}
	}

	if (m_bTournamentMatchRunning)
	{
		V_strncpy(m_hostInfo.m_hostname, m_refHostname.GetString(), 64);
		V_strncpy(m_hostInfo.m_mapname, gpGlobals->mapname.ToCStr(), 64);
		V_strncpy(m_hostInfo.m_bluname, m_refBlueTeamName.GetString(), 32);
		V_strncpy(m_hostInfo.m_redname, m_refRedTeamName.GetString(), 32);
		m_hostInfo.m_roundduration = m_flRoundDuration;
		m_pWebStatsHandler->SetHostData(m_hostInfo);
		m_pWebStatsHandler->SendStatsToWeb();
	}
}

void SizzlingStats::SS_ResetData()
{
	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		playerAndExtra_t data = m_PlayerDataManager.GetPlayerData(i);
		if (data.m_pPlayerData)
		{
			data.m_pPlayerData->ResetExtraData(m_nCurrentRound);
			data.m_pExtraData->operator=(0);
		}
	}
}

void SizzlingStats::SS_Credits( int entindex, const char *pszVersion )
{
	char version[32] = {};
	V_snprintf( version, 32, "\x03SizzlingStats v%s\n", pszVersion );
	CPlayerMessage::SingleUserChatMessage( entindex, "========================\n" );
	CPlayerMessage::SingleUserChatMessage( entindex, version );
	CPlayerMessage::SingleUserChatMessage( entindex, "\x03\x42y:\n" );
	CPlayerMessage::SingleUserChatMessage( entindex, "\x03\tSizzlingCalamari\n" );
	CPlayerMessage::SingleUserChatMessage( entindex, "\x03\tTechnosex\n" );
	CPlayerMessage::SingleUserChatMessage( entindex, "========================\n" );
}

#ifndef PUBLIC_RELEASE

static void messagethis()
{
	CPlayerMessage::AllUserChatMessage("thread completed execution\n");
}

static void messagemain()
{
	CPlayerMessage::AllUserChatMessage("message from main\n");
}

static void messagethread()
{
	CPlayerMessage::AllUserChatMessage("message from thread\n");
}

static void threadstuff()
{
	CUtlBuffer html;
	CHtmlGenerator htmlGen(html);
	htmlGen.StartFile();
	htmlGen.StartHead();
	htmlGen.EndHead();
	htmlGen.StartBody();
	htmlGen.WriteToBuffer("this is a test2\n");
	htmlGen.EndBody();
	htmlGen.EndFile();
	char temp[256];
	V_snprintf(temp, sizeof(temp), "ftp://%s:%s@%s/public/sizzlingstats/asdf.html.uploading", ftp_user.GetString(), ftp_pass.GetString(), ftp_server.GetString());
	CFtpUploader::UploadFile(temp, html);
	Msg("size of html buff: %i\n", html.GetBytesRemaining());
	CFunctor *pFunctor = CreateFunctor(messagethis);
	// do i need to delete functors?
	// it looks like they are reference counted... so no, but not sure.....
	g_pTSCallQueue->EnqueueFunctor( pFunctor );
}

static void loopmessage()
{
	for (int i = 0; i < 100; ++i)
	{
		g_pTSCallQueue->EnqueueFunctor(CreateFunctor(messagethread));
	}
}

#endif

void SizzlingStats::SetTeamScores( int redscore, int bluscore )
{
	m_hostInfo.m_redscore = redscore;
	m_hostInfo.m_bluscore = bluscore;
}

#ifndef PUBLIC_RELEASE

void SizzlingStats::SS_TestThreading()
{
	g_pThreadPool->AddCall(loopmessage);
	for (int i = 0; i < 100; ++i)
	{
		g_pTSCallQueue->EnqueueFunctor(CreateFunctor(messagemain));
	}
}

void SizzlingStats::SS_UploadStats()
{
	g_pThreadPool->AddCall(threadstuff);
}

void SizzlingStats::SS_ShowHtmlStats( int entindex )
{
	//V_snprintf(temp, 256, "%s/sizzlingstats/asdf.html", web_hostname.GetString());
	if (m_pWebStatsHandler->HasMatchUrl())
	{
		char temp[128] = {};
		m_pWebStatsHandler->GetMatchUrl(temp, 128);
		CPlayerMessage::SingleUserVGUIMenu( entindex, "SizzlingStats", temp );
	}
	else
	{
		CPlayerMessage::SingleUserChatMessage( entindex, "\x03No match stats to view.\n" );
	}
}

#endif

void SizzlingStats::GetPropOffsets()
{
//DT_TFPlayerScoringDataExclusive
//	4, Captures				0
//	8, Defenses				1
//	12, Kills				2
//	16, Deaths				3
//	20, Suicides			4
//	24, Dominations			5
//	28, Revenge				6
//	32, BuildingsBuilt		7
//	36, BuildingsDestroyed	8
//	40, Headshots			9
//	44, Backstabs			10
//	48, HealPoints			11
//	52, Invulns				12
//	56, Teleports			13
//	60, DamageDone			14
//	64, Crits				15
//	68, ResupplyPoints		16
//	72, KillAssists			17
//	76, BonusPoints			18
//	80, Points				19
	using namespace SCHelpers;
	bool bError = false;
	int iTFPlayerScoreingDataExclusiveOffset = GetPropOffsetFromTable( "DT_TFPlayer", "m_Shared", bError ) 
											+ GetPropOffsetFromTable( "DT_TFPlayerShared", "tfsharedlocaldata", bError )
											+ GetPropOffsetFromTable( "DT_TFPlayerSharedLocal", "m_RoundScoreData", bError );

	m_aPropOffsets[0] = iTFPlayerScoreingDataExclusiveOffset + GetPropOffsetFromTable( "DT_TFPlayerScoringDataExclusive", "m_iCaptures", bError );
	m_aPropOffsets[1] = iTFPlayerScoreingDataExclusiveOffset + GetPropOffsetFromTable( "DT_TFPlayerScoringDataExclusive", "m_iDefenses", bError );
	m_aPropOffsets[2] = iTFPlayerScoreingDataExclusiveOffset + GetPropOffsetFromTable( "DT_TFPlayerScoringDataExclusive", "m_iKills", bError );
	m_aPropOffsets[3] = iTFPlayerScoreingDataExclusiveOffset + GetPropOffsetFromTable( "DT_TFPlayerScoringDataExclusive", "m_iDeaths", bError );
	m_aPropOffsets[4] = iTFPlayerScoreingDataExclusiveOffset + GetPropOffsetFromTable( "DT_TFPlayerScoringDataExclusive", "m_iSuicides", bError );
	m_aPropOffsets[5] = iTFPlayerScoreingDataExclusiveOffset + GetPropOffsetFromTable( "DT_TFPlayerScoringDataExclusive", "m_iDominations", bError );
	m_aPropOffsets[6] = iTFPlayerScoreingDataExclusiveOffset + GetPropOffsetFromTable( "DT_TFPlayerScoringDataExclusive", "m_iRevenge", bError );
	m_aPropOffsets[7] = iTFPlayerScoreingDataExclusiveOffset + GetPropOffsetFromTable( "DT_TFPlayerScoringDataExclusive", "m_iBuildingsBuilt", bError );
	m_aPropOffsets[8] = iTFPlayerScoreingDataExclusiveOffset + GetPropOffsetFromTable( "DT_TFPlayerScoringDataExclusive", "m_iBuildingsDestroyed", bError );
	m_aPropOffsets[9] = iTFPlayerScoreingDataExclusiveOffset + GetPropOffsetFromTable( "DT_TFPlayerScoringDataExclusive", "m_iHeadshots", bError );
	m_aPropOffsets[10] = iTFPlayerScoreingDataExclusiveOffset + GetPropOffsetFromTable( "DT_TFPlayerScoringDataExclusive", "m_iBackstabs", bError );
	m_aPropOffsets[11] = iTFPlayerScoreingDataExclusiveOffset + GetPropOffsetFromTable( "DT_TFPlayerScoringDataExclusive", "m_iHealPoints", bError );
	m_aPropOffsets[12] = iTFPlayerScoreingDataExclusiveOffset + GetPropOffsetFromTable( "DT_TFPlayerScoringDataExclusive", "m_iInvulns", bError );
	m_aPropOffsets[13] = iTFPlayerScoreingDataExclusiveOffset + GetPropOffsetFromTable( "DT_TFPlayerScoringDataExclusive", "m_iTeleports", bError );
	m_aPropOffsets[14] = iTFPlayerScoreingDataExclusiveOffset + GetPropOffsetFromTable( "DT_TFPlayerScoringDataExclusive", "m_iDamageDone", bError );
	m_aPropOffsets[15] = iTFPlayerScoreingDataExclusiveOffset + GetPropOffsetFromTable( "DT_TFPlayerScoringDataExclusive", "m_iCrits", bError );
	m_aPropOffsets[16] = iTFPlayerScoreingDataExclusiveOffset + GetPropOffsetFromTable( "DT_TFPlayerScoringDataExclusive", "m_iResupplyPoints", bError );
	m_aPropOffsets[17] = iTFPlayerScoreingDataExclusiveOffset + GetPropOffsetFromTable( "DT_TFPlayerScoringDataExclusive", "m_iKillAssists", bError );
	m_aPropOffsets[18] = iTFPlayerScoreingDataExclusiveOffset + GetPropOffsetFromTable( "DT_TFPlayerScoringDataExclusive", "m_iBonusPoints", bError );
	m_aPropOffsets[19] = iTFPlayerScoreingDataExclusiveOffset + GetPropOffsetFromTable( "DT_TFPlayerScoringDataExclusive", "m_iPoints", bError );

	m_PlayerClassOffset = GetPropOffsetFromTable( "DT_TFPlayer", "m_PlayerClass", bError ) + GetPropOffsetFromTable( "DT_TFPlayerClassShared", "m_iClass", bError );
	m_PlayerFlagsOffset = GetPropOffsetFromTable( "DT_BasePlayer", "m_fFlags", bError );
	m_TeamRoundsWonOffset = GetPropOffsetFromTable( "DT_Team", "m_iRoundsWon", bError ); 
	m_iWeaponsOffset = GetPropOffsetFromTable( "DT_BaseCombatCharacter", "m_hMyWeapons", bError ); // should get the 0 offsets before it incase something changes
	m_iChargeLevelOffset = GetPropOffsetFromTable( "DT_LocalTFWeaponMedigunData", "m_flChargeLevel", bError ); // should get the 0 offsets before it incase something changes
	m_iOriginOffset = GetPropOffsetFromTable( "DT_BaseEntity", "m_vecOrigin", bError );
	m_iChargeReleaseOffset = GetPropOffsetFromTable( "DT_WeaponMedigun", "m_bChargeRelease", bError );

	//oKills = m_PlayerFlagsOffset;

	//CBaseEntity *pEntity = gEntList.FindEntityByClassname( pServerEnts->EdictToBaseEntity( engine->PEntityOfEntIndex( gpGlobals->maxClients ) ), "CWeaponIFMSteadyCam" );
}

