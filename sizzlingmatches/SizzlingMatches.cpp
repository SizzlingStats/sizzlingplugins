
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

////////////////////////////////////////////////////////////////////////////////
// Filename: SizzlingMatches.cpp
////////////////////////////////////////////////////////////////////////////////
#include "SizzlingMatches.h"
#include "SC_helpers.h"
#include "tier1/utlvector.h"
#include "tier1/stringpool.h"
#include "MRecipientFilter.h"
#include "UserMessageHelpers.h"

#include "SizzPluginContext.h"

extern IGameEventManager2		*gameeventmanager;

SendTable		*GetDataTable( const char *pTableName, SendTable *pTable );
unsigned int	GetPropOffsetFromTable(const char *pTableName, const char *pPropName, bool &bErr);

SizzlingMatches::SizzlingMatches():
	m_plugin_context(nullptr),
	m_PlayerFlagsOffset(0), 
	m_TeamRoundsWonOffset(0),
	m_nPlayersReady(0),
	m_nPlayersOnTeams(0),
	m_bMatchStarted(0),
	m_bTimer5(0),
	m_nCountdown(0),
	m_n12sectimer(0),
	m_nCurrentPlayers(0),
	m_nCurrentRound(0)
{
	memset(m_aPropOffsets, 0, sizeof(m_aPropOffsets) );
	m_SteamIDToPlayerDataMap.SetLessFunc( SCHelpers::FUIntCmp );
	m_EventRegister.Init( &m_TimedEventMgr, this );
}


SizzlingMatches::~SizzlingMatches()
{	
}

void SizzlingMatches::Load( CSizzPluginContext &context )
{
	m_plugin_context = &context;
}

void SizzlingMatches::SM_LoadCurrentPlayers()
{
	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		SM_InsertPlayer( pEngine->PEntityOfEntIndex( i ) );
	}
}

void SizzlingMatches::SM_SetEventUpdateInterval( float interval )
{
	m_EventRegister.SetUpdateInterval( interval );
}

void SizzlingMatches::SM_SetAllPlayersNotReady()
{
	FOR_EACH_MAP_FAST( m_SteamIDToPlayerDataMap, itMapData )
	{
		SM_PlayerData *pData = m_SteamIDToPlayerDataMap.Element( itMapData );
		pData->SetReadyState( false );
	}
	m_nPlayersReady = 0;
}

void SizzlingMatches::SM_ResetGame()
{
	pEngine->ServerCommand( "mp_tournament 0\n" );
	pEngine->ServerCommand( "mp_timelimit 0\n" );
	pEngine->ServerCommand( "mp_winlimit 0\n" );
	m_bMatchStarted = false;
	SM_SetEventUpdateInterval( 3.0f );
	SM_SetAllPlayersNotReady();
	FireEvent();
	pEngine->ServerCommand( "exec sourcemod/soap_notlive\n" );
}

bool SizzlingMatches::SM_InsertPlayer( edict_t *pEntity )
{
	if( !pEntity || pEntity->IsFree() )
		return false;
	
	IPlayerInfo *pPlayerInfo = playerinfomanager->GetPlayerInfo( pEntity );
	if ( pPlayerInfo )
	{
		bool isConnected = pPlayerInfo->IsConnected();
		SM_Msg( "is this player valid: %s\n", isConnected ? "y" : "n" );
		if ( !isConnected )
		{
			return false;
		}
	} 
	else
	{
		SM_Msg( "player insertion error\n" );
		return false;
	}

	const CSteamID *pSteamID = pEngine->GetClientSteamID( pEntity );
	if (!pSteamID)
	{
		SM_Msg( "client: %s has not auth'd yet\n", pPlayerInfo->GetName() );
		return false;
	}

	unsigned int id = pSteamID->GetAccountID();
	SM_Msg( "player acc id: %i\n", id );

	//if ( !FStrEq( pID, "BOT" ) )
	//{
		const char *name = pPlayerInfo->GetName();
		SM_PlayerData *pData = new SM_PlayerData( pEntity, pPlayerInfo );
		pData->UpdateName( name );

		int teamindex = pPlayerInfo->GetTeamIndex();
		//SM_Msg( "%i\n", teamindex );
		pData->SetTeam( teamindex );
		if ( teamindex > 1 )
		{
			++m_nPlayersOnTeams;
		}
		m_SteamIDToPlayerDataMap.Insert( id, pData );
		SM_Msg( "playerdata for #%i: '%s' created\n", id, name );

		m_nCurrentPlayers += 1;
	//}
	//else
	//{
	//	SM_Msg( "skipping playerdata create for bot: %s\n", pPlayerInfo->GetName() );
	//}
	return true;
}

bool SizzlingMatches::SM_DeletePlayer( edict_t *pEntity )
{
	if( !pEntity || pEntity->IsFree() )
	{
		Msg( "delete entity invalid\n" );
		return false;
	}

	const CSteamID *pSteamID = pEngine->GetClientSteamID( pEntity );
	if (!pSteamID)
	{
		SM_Msg( "client for delete has not auth'd yet\n" );
		return false;
	}

	unsigned int id = pSteamID->GetAccountID();
	int PlayerIndex =  m_SteamIDToPlayerDataMap.Find( id );
	SM_PlayerData *pData = NULL;
	if ( m_SteamIDToPlayerDataMap.IsValidIndex( PlayerIndex ) )
		pData = m_SteamIDToPlayerDataMap.Element( PlayerIndex );

	if ( pData )
	{
		if ( pData->GetTeam() > 1 )
			--m_nPlayersOnTeams;

		if ( pData->GetReadyState() == true )		// need to move these into new func
			--m_nPlayersReady;

		//Msg( "starting delete\n" );
		delete pData;
		pData = NULL;
		SM_Msg( "deleted data #%i\n", id );

		SM_Msg( "size before delete: %i\n", m_SteamIDToPlayerDataMap.Count() );
		m_SteamIDToPlayerDataMap.RemoveAt( PlayerIndex );
		SM_Msg( "size after delete: %i\n", m_SteamIDToPlayerDataMap.Count() );
		
		m_nCurrentPlayers -= 1;
		return true;
	}
	else
	{
		SM_Msg( "datamap error on single player delete\n" );
		return false;
	}
}

bool SizzlingMatches::SM_DeleteAllPlayerData()
{
	if ( ( m_SteamIDToPlayerDataMap.Count() != 0 ) )
	{
		FOR_EACH_MAP_FAST( m_SteamIDToPlayerDataMap, itMapData )
		{
			SM_PlayerData *pData = m_SteamIDToPlayerDataMap.Element( itMapData );

			if ( pData )
			{
				unsigned int id = m_SteamIDToPlayerDataMap.Key( itMapData );
				SM_Msg( "deleting data #%i: %s\n", id, pData->GetPlayerInfo()->GetName() );

				delete pData;
				pData = NULL;

				SM_Msg( "size before delete: %i\n", m_SteamIDToPlayerDataMap.Count() );
				m_SteamIDToPlayerDataMap.RemoveAt( itMapData );
				SM_Msg( "size after delete: %i\n", m_SteamIDToPlayerDataMap.Count() );
			}
			else
			{
				SM_Msg( "datamap error on all player delete\n" );
				return false;
			}
		}
		m_nCurrentPlayers = 0;
	}
	else
	{
		SM_Msg( "nothing to delete in deleteallplayerdata()\n" );
	}
	return true;
}

void SizzlingMatches::SM_Msg( const char *pMsg, ... )
{
	va_list argList;
	va_start( argList, pMsg );
	char message[96];

	V_vsnprintf( message, 96, pMsg, argList );
	Msg( "[SM]: %s", message );

	va_end( argList );
}

void SizzlingMatches::SM_SingleUserChatMessage( edict_t *pEntity, const char *pFormat, ... )
{
	va_list argList;
	va_start( argList, pFormat );

	int ent_index = SCHelpers::EntIndexFromEdict(pEntity);
	if (ent_index != -1)
	{
		CUserMessageHelpers h(m_plugin_context);
		h.SingleUserChatMessageArg(ent_index, pFormat, argList);
	}
	va_end( argList );
}

void SizzlingMatches::SM_AllUserChatMessage( const char *pFormat, ... )
{
	va_list argList;
	va_start( argList, pFormat );
	char message[254];

	V_vsnprintf( message, 254, pFormat, argList );

	CUserMessageHelpers h(m_plugin_context);
	h.AllUserChatMessage("%s%s", "\x04[\x05SizzlingMatches\x04]\x06: \x03", message);
	//CPlayerMessage::AllUserChatMessage( szMessage, "\x01\\x01\x02\\x02\x03\\x03\x04\\x04\x05\\x05\x06\\x06\x07\\x07\x08\\x08\x09\\x09\n" );

	va_end( argList );
}

void SizzlingMatches::SM_StartOfRound()
{
	if ( m_bMatchStarted == true )
	{
		SM_AllUserChatMessage( "bleh minutes left in teh match\n" );
	}
}

void SizzlingMatches::SM_EndOfRound()
{
	if ( m_bMatchStarted == true )
	{
		SM_AllUserChatMessage( "Bleh is leading bleh-bleh or maybe a tie\n" );
	}
}

void SizzlingMatches::SM_GetPropOffsets()
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

	m_PlayerFlagsOffset = GetPropOffsetFromTable( "DT_BasePlayer", "m_fFlags", bError );
	m_TeamRoundsWonOffset = GetPropOffsetFromTable( "DT_Team", "m_iRoundsWon", bError ); 

	m_TeamIsReadyOffset = GetPropOffsetFromTable( "DT_TeamplayRoundBasedRulesProxy", "teamplayroundbased_gamerules_data", bError )
						+ GetPropOffsetFromTable( "DT_TeamplayRoundBasedRules", "m_bTeamReady", bError );

	m_AwaitingReadyRestartOffset = GetPropOffsetFromTable( "DT_TeamplayRoundBasedRulesProxy", "teamplayroundbased_gamerules_data", bError )
						+ GetPropOffsetFromTable( "DT_TeamplayRoundBasedRules", "m_bAwaitingReadyRestart", bError );
}

void SizzlingMatches::SM_GetEntities()
{
	// TODO: this stuff doesn't work yet, cause i need to get the proxy function
	// to give me the ctfgamerulesproxy pointer, look in the tf2dm files for it
	m_pTeamplayRoundBasedRulesProxy = SCHelpers::GetEntityByClassname( "CTFGameRulesProxy" );

	if ( m_pTeamplayRoundBasedRulesProxy )
	{
		m_bTeamReady = ((bool *)(((unsigned char *)m_pTeamplayRoundBasedRulesProxy) + m_TeamIsReadyOffset));
		m_bAwaitingReadyRestart = ((bool *)(((unsigned char *)m_pTeamplayRoundBasedRulesProxy) + m_AwaitingReadyRestartOffset));
	}
}

void SizzlingMatches::SM_PlayerChangeTeam( int userid, int newteamid, int oldteamid )
{
	unsigned int SteamID = m_plugin_context->SteamIDFromUserID(userid);
	int index = m_SteamIDToPlayerDataMap.Find( SteamID );
	if ( !( m_SteamIDToPlayerDataMap.IsValidIndex( index ) ) )
		return;
	SM_PlayerData *pData = m_SteamIDToPlayerDataMap.Element( index );
	pData->SetTeam( newteamid );

	if ( ( oldteamid < 2 ) && ( newteamid > 1 ) ) // if you were previously on spec or just joining
	{
		++m_nPlayersOnTeams;
	}
	else if ( ( oldteamid != 0 ) && ( newteamid < 2 ) ) // if you are joining spec from red or blu
	{
		--m_nPlayersOnTeams;
		if ( pData->GetReadyState() == true )
		{
			--m_nPlayersReady;
			pData->SetReadyState( false );
		}
	}
}

void SizzlingMatches::SM_PlayerChangeName( int userid, const char *szNewName )
{
	unsigned int SteamID = m_plugin_context->SteamIDFromUserID(userid);
	int index = m_SteamIDToPlayerDataMap.Find( SteamID );
	if ( !( m_SteamIDToPlayerDataMap.IsValidIndex( index ) ) )
		return;
	m_SteamIDToPlayerDataMap.Element( index )->UpdateName( szNewName );
}

void SizzlingMatches::SM_PlayerChangeReadyState( int userid, bool state )
{
	unsigned int SteamID = m_plugin_context->SteamIDFromUserID(userid);
	int index = m_SteamIDToPlayerDataMap.Find( SteamID );
	if ( !( m_SteamIDToPlayerDataMap.IsValidIndex( index ) ) )
		return;
	SM_PlayerData *pData = m_SteamIDToPlayerDataMap.Element( index );

	if ( pData->GetTeam() < 2 )
		return;

	if ( pData->GetReadyState() == state )
	{
		SM_SingleUserChatMessage( pData->GetEdict(), "You are already %s\n", state ? "ready." : "not ready." );
		return;
	}
	//pData->SetReadyState( state );
	//SM_AllUserChatMessage( "%s is now %s\n", pData->GetName(), state ? "ready." : "unready." );
	if ( state == true )
	{
		pData->SetReadyState( true );
		SM_AllUserChatMessage( "%s is now ready.\n", pData->GetName() );
		FireEvent();		// probably shouldn't be calling this function directly
		++m_nPlayersReady;
		return;
	}
	else
	{
		pData->SetReadyState( false );
		SM_AllUserChatMessage( "%s is now unready.\n", pData->GetName() );
		FireEvent();
		--m_nPlayersReady;
		return;
	}
	return;
}

//void SizzlingMatches::SM_DisplayNames()				// esea style layout
//{
//	char ready[255] = "", notready[255] = "";
//	colour rgba;
//
//	V_snprintf( ready, 255, "Ready:\n" );
//	V_snprintf( notready, 255, "Not Ready:\n" );
//	//CPlayerMessage::AllUserHudMsg( "Ready", rgba, 3.0f, 0.15, 0.10, 2 );
//	//CPlayerMessage::AllUserHudMsg( "Not Ready", rgba, 3.0f, 0.65, 0.10, 3 );
//	FOR_EACH_MAP_FAST( m_SteamIDToPlayerDataMap, itMapName )
//	{
//		SM_PlayerData *pData = m_SteamIDToPlayerDataMap.Element( itMapName );
//		if ( ( pData->GetTeam() == 2 ) || ( pData->GetTeam() == 3 ) )
//		{
//			if ( pData->GetReadyState() == true )
//			{
//				V_snprintf( ready, 255, "%s%s\n", ready, m_SteamIDToPlayerDataMap.Element( itMapName )->GetName() );
//			}
//			else
//			{
//				V_snprintf( notready, 255, "%s%s\n", notready, m_SteamIDToPlayerDataMap.Element( itMapName )->GetName() );
//			}
//		}
//	}
//	SM_Msg( "%s", ready );
//	SM_Msg( "%s", notready );
//	CPlayerMessage::AllUserHudMsg( ready, rgba, 3.0f, 0.10, 0.09, 4 );
//	CPlayerMessage::AllUserHudMsg( notready, rgba, 3.0f, 0.60, 0.09, 5 );
//	SM_Msg( "refreshtime: %f\n", gpGlobals->curtime );
//}

//void SizzlingMatches::SM_DisplayNames()			// my layout with blue and red coloured names
//{
//	char red[255], blu[255];
//	colour clrred, clrblue;
//	clrred.a = clrblue.a = 0;
//	clrred.b = clrred.g = clrblue.r = clrblue.g = 128;
//	clrred.r = clrblue.b = 255;
//
//	memset( red, 0, sizeof(red) );
//	memset( blu, 0, sizeof(blu) );
//	V_snprintf( red, 255, "Red:" );
//	V_snprintf( blu, 255, "Blu:" );
//	//CPlayerMessage::AllUserHudMsg( "Ready", rgba, 3.0f, 0.15, 0.10, 2 );
//	//CPlayerMessage::AllUserHudMsg( "Not Ready", rgba, 3.0f, 0.65, 0.10, 3 );
//	FOR_EACH_MAP_FAST( m_SteamIDToPlayerDataMap, itMapName )
//	{
//		SM_PlayerData *pData = m_SteamIDToPlayerDataMap.Element( itMapName );
//		if ( pData->GetTeam() == 2 )
//		{
//			V_snprintf( red, 255, "%s\n%s%s", red, pData->GetReadyState() ? "[Ready] " : "[Not Ready] ", pData->GetName() );
//		}
//		else if ( pData->GetTeam() == 3 )
//		{
//			V_snprintf( blu, 255, "%s\n%s%s", blu, pData->GetReadyState() ? "[Ready] " : "[Not Ready] ", pData->GetName() );
//		}
//	}
//	SM_Msg( "%s\n", red );
//	SM_Msg( "%s\n", blu );
//	CPlayerMessage::AllUserHudMsg( red, clrred, 3.0f, 0.05, 0.09, 4 );
//	CPlayerMessage::AllUserHudMsg( blu, clrblue, 3.0f, 0.55, 0.09, 5 );
//	SM_Msg( "refreshtime: %f\n", gpGlobals->curtime );
//}

void SizzlingMatches::SM_GetNamesLists( char RedTeamPlayers[], int RedArraySize, char BluTeamPlayers[], int BluArraySize )
{
	FOR_EACH_MAP_FAST( m_SteamIDToPlayerDataMap, itMapName )
	{
		SM_PlayerData *pData = m_SteamIDToPlayerDataMap.Element( itMapName );
		if ( pData->GetTeam() == 2 )
		{
			if ( pData->GetReadyState() )
			{
				V_snprintf( RedTeamPlayers, RedArraySize, "%s\n%s%s", RedTeamPlayers, "[R] ", pData->GetName() );
			}
			else
			{
				V_snprintf( RedTeamPlayers, RedArraySize, "%s\n%s%s", RedTeamPlayers, "[X] ", pData->GetName() );
			}
		}
		else if ( pData->GetTeam() == 3 )
		{
			if ( pData->GetReadyState() )
			{
				V_snprintf( BluTeamPlayers, BluArraySize, "%s\n%s%s", BluTeamPlayers, "[R] ", pData->GetName() );
			}
			else
			{
				V_snprintf( BluTeamPlayers, BluArraySize, "%s\n%s%s", BluTeamPlayers, "[X] ", pData->GetName() );
			}
		}
	}
}

void SizzlingMatches::SM_DisplayNames( const char *RedTeamPlayers, const char *BluTeamPlayers )			// my layout with green team names and white names
{
	Color clrgreen(0, 85, 0, 0);
	Color clrwhite(85, 85, 85, 0);

	CUserMessageHelpers h(m_plugin_context);
	h.AllUserHudResetMessage();

	hud_msg_cfg_t cfg;
	cfg.screentime = 4.0f;
	cfg.y = 0.09f;

	// y values the same because hudmsg has auto position correction
	cfg.rgba = clrgreen;
	cfg.x = 0.55f;
	cfg.channel = 2;
	h.AllUserHudMessage(cfg, "Red:");

	cfg.rgba = clrwhite;
	cfg.channel = 3;
	h.AllUserHudMessage(cfg, RedTeamPlayers);

	cfg.rgba = clrgreen;
	cfg.x = 0.05f;
	cfg.channel = 4;
	h.AllUserHudMessage(cfg, "Blu:");

	cfg.rgba = clrwhite;
	cfg.channel = 5;
	h.AllUserHudMessage(cfg, BluTeamPlayers);

	if ( m_n12sectimer > 3 )
	{
		SM_AllUserChatMessage( "Type \".ready\" to ready up and \".notready\" to unready.\n" );
		m_n12sectimer = 0;
	}
	else
	{
		++m_n12sectimer;
	}
}

void SizzlingMatches::UpdateMatchStartStatus()
{
	//Msg( "players on teams: %i, players ready: %i\n", m_nPlayersOnTeams, m_nPlayersReady );
	
	int sizzmatch_override = sizzmatch_start_override.GetInt();
	if ( sizzmatch_override != 0 )
	{
		SM_Msg( "override: %i\n", sizzmatch_override );
		if ( sizzmatch_override <= m_nPlayersReady )
		{
			StartGame();
			return;
		}
	}

	if ( m_nPlayersOnTeams > 9 )
	{
		if ( m_nPlayersOnTeams == m_nPlayersReady )
		{
			if ( m_nPlayersOnTeams == 12 )
			{
				StartGame();
				return;
			} 
			else if ( m_nPlayersOnTeams != 12 )
			{
				// vote to start game
				//return;
			}
		}
		//if ( ( m_nPlayersOnTeams == 12 ) && ( m_nPlayersReady == 12 ) )	// ideal match
		//{
		//	StartGame();
		//	return;
		//}
		//else if ( ( m_nPlayersOnTeams != 12 ) && ( m_nPlayersOnTeams == m_nPlayersReady ) )		// if there are more or less than 12, but they are all ready
		//{
		//	// vote to start game
		//	//return;
		//}
		else if ( ( m_nPlayersOnTeams == 12 ) && ( m_nPlayersReady == (m_nPlayersOnTeams - 1) ) )
		{
			// vote to ready the person
			//return;
		}
	}
}

bool SizzlingMatches::SM_IsStarted()
{
	return m_bMatchStarted;
}

void SizzlingMatches::SM_GameOver()
{
	if ( m_bMatchStarted == true )
	{
		SM_AllUserChatMessage( "Match Over\n" );
		SM_ResetGame();
	}
}

void SizzlingMatches::FireEvent()
{
	if ( m_bTimer5 == false )
	{
		char red[255] = "", blu[255] = "";
		//CPlayerMessage::AllUserHudMsg( "test\ntest", rgba, 2.0f, 0.95f, 0.90f, 2 );
		SM_GetNamesLists( red, 255, blu, 255 );
		//SM_Msg( "%s\n", red );
		//SM_Msg( "%s\n", blu );
		SM_DisplayNames( red, blu );
		UpdateMatchStartStatus();
		//SM_Msg( "refreshtime: %f\n", gpGlobals->curtime );
	}
	else
	{
		FOR_EACH_MAP_FAST( m_SteamIDToPlayerDataMap, itMapData )
		{
			pEngine->ClientCommand( m_SteamIDToPlayerDataMap.Element( itMapData )->GetEdict(), "playgamesound Announcer.RoundBegins%iSeconds\n", m_nCountdown );
		}
		if ( m_nCountdown > 1 )
		{
			--m_nCountdown;
		}
		else
		{
			m_EventRegister.StopUpdates();
			m_nCountdown = m_bTimer5 = 0;
			//IGameEvent *event = gameeventmanager->CreateEvent("teamplay_round_restart_seconds");
			//event->SetString( "seconds", "1" );
			//gameeventmanager->FireEvent(event);
			pEngine->ServerCommand( "mp_restartgame 1\n" );
			pEngine->ServerCommand( "exec cevo_push\n" );
		}
	}
}

void SizzlingMatches::LevelInit( char const *pMapName )
{
	SM_ResetGame();
}

void SizzlingMatches::GameFrame()
{
	m_TimedEventMgr.FireEvents();
}

void SizzlingMatches::LevelShutdown()
{
	m_EventRegister.StopUpdates();
	m_nPlayersOnTeams = 0;
	m_nPlayersReady = 0;
	SM_DeleteAllPlayerData();
}

void SizzlingMatches::PlayerDisconnect( edict_t *pEntity )
{
	SM_DeletePlayer( pEntity );
}

void SizzlingMatches::StartGame()
{
	m_EventRegister.StopUpdates();
	m_bTimer5 = true;
	m_nCountdown = 5;
	FireEvent();
		
	m_EventRegister.SetUpdateInterval( 1.0f );

	m_bMatchStarted = true;

	pEngine->ServerCommand( "exec sourcemod/soap_live\n" );
	//pEngine->ServerCommand( "exec cevo_push\n" );
	//pEngine->ServerCommand( "mp_restartgame 5\n" );
	//pEngine->ServerCommand( "mp_restartround 5\n" );
	//IGameEvent *event = gameeventmanager->CreateEvent( "teamplay_round_restart_seconds" );
	//if ( event )
	//{
	//	event->SetInt( "seconds", 5 );
	//	gameeventmanager->FireEvent( event );
	//}
	//pEngine->ServerCommand( "playgamesound Announcer.RoundBegins5Seconds\n" );
	SM_SetAllPlayersNotReady();
}