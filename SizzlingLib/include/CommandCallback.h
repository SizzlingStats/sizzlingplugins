
#include "convar.h"
#include "icvar.h"
#include "eiface.h"

extern IVEngineServer *pEngine;

static ConVar enable( "sizz_protect_enable", "1", FCVAR_NOTIFY, "If set to 1, enables exploit message blocking" );
static ConVar mode( "sizz_protect_mode", "0", FCVAR_NOTIFY | FCVAR_ARCHIVE, "If set to 0, only blocks exploit messages; If set to 1, blocks and kicks the offending client" );

static bool isExploit( const CCommand &args, int ClientCommandIndex );
static bool validString( const char **pChar );

// Special hook for say commands
class CSayHook : public ConCommand
{
	// This will hold the pointer original gamedll say command
	ConCommand *m_pGameDLLSayCommand;
	int	m_iClientCommandIndex;
public:
	CSayHook(): ConCommand("say", FnCommandCallback_t(0), "Display player message", FCVAR_GAMEDLL), m_pGameDLLSayCommand(NULL)
	{
	}

	// Override Init
	void Init()
	{
		// Find the gamedll say command
		m_pGameDLLSayCommand = g_pCVar->FindCommand( "say" );

		// Call base class' init function
		ConCommand::Init();
	}

	void Dispatch( const CCommand &args )
	{
		// Do the normal stuff, return if you want to override the say

		if ( enable.GetInt() == 1 )
		{
			if ( isExploit( args, m_iClientCommandIndex ) )
			{
				return;
			}
		}

		// Forward to gamedll
		m_pGameDLLSayCommand->Dispatch( args );
	}

	void SetCommandClient( int index )
	{
		m_iClientCommandIndex = index;
	}
};

// Don't forget to make an instance
static CSayHook		g_SayHook;
CSayHook			*g_pSayHook = &g_SayHook;

class CSayTeamHook : public ConCommand
{
	// This will hold the pointer original gamedll say command
	ConCommand *m_pGameDLLSayCommand;
	int	m_iClientCommandIndex;
public:
	CSayTeamHook(): ConCommand("say_team", FnCommandCallback_t(0), "Display player message to team", FCVAR_GAMEDLL), m_pGameDLLSayCommand(NULL)
	{
	}

	// Override Init
	void Init()
	{
		// Find the gamedll say command
		m_pGameDLLSayCommand = g_pCVar->FindCommand( "say_team" );

		// Call base class' init function
		ConCommand::Init();
	}

	void Dispatch( const CCommand &args )
	{
		// Do the normal stuff, return if you want to override the say

		if ( enable.GetInt() == 1 )
		{
			if ( isExploit( args, m_iClientCommandIndex ) )
			{
				return;
			}
		}

		// Forward to gamedll
		m_pGameDLLSayCommand->Dispatch( args );
	}

	void SetCommandClient( int index )
	{
		m_iClientCommandIndex = index;
	}
};

// Don't forget to make an instance
static CSayTeamHook		g_SayTeamHook;
CSayTeamHook			*g_pSayTeamHook = &g_SayTeamHook;

bool isExploit( const CCommand &args, int ClientCommandIndex )
{
	const char *pChar = V_strstr( args.ArgS(), "%" );
	while ( pChar )
	{
		if ( !validString(&pChar) )
		{
			if ( mode.GetInt() == 1 )
			{
				char temp[128];
				int userid = pEngine->GetPlayerUserId( pEngine->PEntityOfEntIndex( ClientCommandIndex ) );
				V_snprintf( temp, 128, "kickid %i %s\n", userid, "Attempted crash exploit" );
				pEngine->ServerCommand( temp );
			}
			return true;
		}
		pChar = V_strstr( pChar, "%" );
	}
	return false;
}

bool validString( const char **pChar )
{
	if ( *pChar )
	{
		while ( *pChar != '\0' )
		{
			switch ( **pChar )
			{
			case ' ':
			case '\n':
			case '%':
				++(*pChar);
				continue;
			case 'n':
			case 'N':
			case 's':
			case 'S':
				return false;
			default:
				return true;
			}
		}
	}
	return true;
}