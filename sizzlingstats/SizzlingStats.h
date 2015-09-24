
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

////////////////////////////////////////////////////////////////////////////////
// Filename: SizzlingStats.h
////////////////////////////////////////////////////////////////////////////////
#ifndef SIZZLING_STATS_H
#define SIZZLING_STATS_H

#include "WebStatsHandler.h"
#include "convar.h"
#include "PluginDefines.h"
#include "PlayerDataManager.h"
#include "sizzstring.h"
#include "STVRecorder.h"
#include "S3Uploader.h"

#include "tier1/utlvector.h"

class CFuncQueueThread;
class CSizzPluginContext;
struct edict_t;

class SizzlingStats
{
public:
	SizzlingStats(void);
	~SizzlingStats(void);

	// called when the plugin is loaded
	void	Load( CSizzPluginContext *pPluginContext );

	// called when the plugin is unloaded
	void	Unload( CSizzPluginContext *pPluginContext );

	// called on level start
	void	LevelInit( CSizzPluginContext *pPluginContext, const char *pMapName );

	void	ServerActivate( CSizzPluginContext *pPluginContext );

	void	GameFrame();

	void	LoadConfig( CSizzPluginContext *pPluginContext );
	
	void	PlayerHealed( int entindex, int amount );
	void	MedPick( int entindex );
	void	UberDropped( int entindex );
	void	OnPlayerDeath(int inflictorEntIndex, int victimEntIndex);
	
	// this needs to be called whenever the player switches classes or goes to spec
	// set player_class to k_ePlayerClassUndefined if switching to spec
	void	PlayerChangedClass( int entindex, EPlayerClass player_class );
	
	void	ChatEvent( CSizzPluginContext *pPluginContext, int entindex, const char *pText, bool bTeamChat );

	void	TeamCapped( int team_index );

	void	GiveUber( CSizzPluginContext *pPluginContext, int entindex );

	// checks to see if any of the medics with uber on the same team 
	// as the player that died dropped him
	void	CheckPlayerDropped( CSizzPluginContext *pPluginContext, int victimIndex );

	void	CapFix( const char *cappers, int length );

	// insert and player and add them to the map
	bool	SS_PlayerConnect( CSizzPluginContext *pPluginContext, edict_t *pEdict );

	// delete a player and remove them from the map
	void	SS_PlayerDisconnect( CSizzPluginContext *pPluginContext, edict_t *pEdict );

	// deletes all player data and empties the map
	void	SS_DeleteAllPlayerData();

	//	console message with "[SS]: " prefix
	void	SS_Msg( const char *pMsg, ... );

	//	chat message to be sent to one user
	void	SS_SingleUserChatMessage( CSizzPluginContext *pPluginContext, int ent_index, const char *szMessage );

	//	chat message to be sent to all users
	void	SS_AllUserChatMessage( CSizzPluginContext *pPluginContext, const char *szMessage );

	void	SS_TournamentMatchStarted( CSizzPluginContext *pPluginContext );

	void	SS_TournamentMatchEnded( CSizzPluginContext *pPluginContext );

	void	SS_PreRoundFreeze( CSizzPluginContext *pPluginContext );

	void	SS_RoundStarted( CSizzPluginContext *pPluginContext );

	void	SS_RoundEnded( CSizzPluginContext *pPluginContext );

	//	displays the stats for use at the end of a round
	void	SS_DisplayStats( CSizzPluginContext *pPluginContext, int ent_index );

	//	called when the round ends
	void	SS_EndOfRound( CSizzPluginContext *pPluginContext );

	// resets the player data
	void	SS_ResetData( CSizzPluginContext *pPluginContext );

	//	called when a user types ".ss_credits"
	void	SS_Credits( CSizzPluginContext *pPluginContext, int entindex, const char *pszVersion );

	void	SetTeamScores( int redscore, int bluscore );

	void	SS_TestThreading();

	void	SS_UploadStats();

	void	SS_ShowHtmlStats( CSizzPluginContext *pPluginContext, int entindex, bool reload_page );

	void	SS_HideHtmlStats( CSizzPluginContext *pPluginContext, int endindex );

private:
	void	OnSessionIdReceived( CSizzPluginContext *pPluginContext, sizz::CString sessionid );
	void	LogSessionId( CSizzPluginContext *pPluginContext, const sizz::CString &str );

	void	OnMatchUrlReceived( CSizzPluginContext *pPluginContext, sizz::CString matchurl );
	void	CacheSiteOnPlayer( CSizzPluginContext *pPluginContext, const sizz::CString &match_url );

	void	OnSTVUploadUrlReceived( CSizzPluginContext *pPluginContext, sizz::CString stvuploadurl );
	void	LogSTVUploadUrl( CSizzPluginContext *pPluginContext, const sizz::CString &str );

	//	gets the prop offests
	void	GetPropOffsets();

	void	GetEntities( CSizzPluginContext *pPluginContext );

	void	OnS3UploadReturn( const sizz::CString &sessionid, bool bUpdateSuccessful );

private:
	unsigned int	m_aPropOffsets[20];

	CBaseEntity		*m_pRedTeam;
	CBaseEntity		*m_pBluTeam;
	uint16			m_iOldRedScore;
	uint16			m_iOldBluScore;

	CUtlVector<char> m_vecMedics; //ent index of medics
private:
	CS3UploaderThread	*m_pS3UploaderThread;
	CPlayerDataManager m_PlayerDataManager;
	CWebStatsHandler *m_pWebStatsHandler;
	CSTVRecorder m_STVRecorder;
	ConVarRef m_refHostIP;
	ConVarRef m_refIP;
	ConVarRef m_refHostPort;
	hostInfo_t *m_pHostInfo;
	double m_flRoundDuration;
	double m_flMatchDuration;
	bool m_bTournamentMatchRunning;
	bool m_bFirstCapOfRound;
};

#endif // SIZZLING_STATS_H
