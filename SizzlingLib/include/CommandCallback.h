
#include "convar.h"
#include "icvar.h"
#include "eiface.h"

extern IVEngineServer *pEngine;

static ConVar enable( "sizz_protect_enable", "1", FCVAR_NOTIFY, "If set to 1, enables exploit message blocking" );
static ConVar mode( "sizz_protect_mode", "0", FCVAR_NOTIFY | FCVAR_ARCHIVE, "If set to 0, only blocks exploit messages; If set to 1, blocks and kicks the offending client" );

static bool isExploit( const CCommand &args, int ClientCommandIndex );
static bool validString( const char **pChar );

class ICommandHookCallback
{
public:
	virtual bool CommandPreExecute( const CCommand &args ) = 0;
	virtual void CommandPostExecute( const CCommand &args, bool bWasCommandExecuted ) = 0;
};

class CConCommandHook: public ICommandCallback
{
public:
	CConCommandHook():
		m_pCommand(NULL),
		m_pCallback(NULL)
	{
	}
	
	virtual ~CConCommandHook()
	{
		Unhook();
	}
	
	bool Hook( ICommandHookCallback *pThis, ICvar *pCvar, const char *pszCommandToHook )
	{
		// if the cvar pointer and the string are valid
		// and a command isn't already hooked
		if (pCvar && pszCommandToHook && !m_pCommand)
		{
			m_pCallback = pThis;
			// find the current registered concommand that you want to hook
			m_pCommand = pCvar->FindCommand( pszCommandToHook );
			// if it was valid, hook the callback
			if (m_pCommand)
			{
				// first save the old callback flags
				m_bUsingCommandCallbackInterface = m_pCommand->m_bUsingCommandCallbackInterface;
				m_bUsingNewCommandCallback = m_pCommand->m_bUsingNewCommandCallback;

				// now figure out which function it called for the callback
				// and save that
				if (m_bUsingCommandCallbackInterface)
				{
					m_pCommandCallback = m_pCommand->m_pCommandCallback;
				}
				else
				{
					if (m_bUsingNewCommandCallback)
					{
						m_fnCommandCallback = m_pCommand->m_fnCommandCallback;
					}
					else
					{
						m_fnCommandCallbackV1 = m_pCommand->m_fnCommandCallbackV1;
					}
				}
				
				// now that all the old callback info is saved,
				// change the command to use our class as it's callback
				m_pCommand->m_bUsingCommandCallbackInterface = true;
				m_pCommand->m_bUsingNewCommandCallback = false;
				m_pCommand->m_pCommandCallback = this;
				return true;
			}
		}
		return false;
	}
	
	void Unhook()
	{
		// if there was a command already hooked
		if (m_pCommand)
		{
			// restore the old callback info from our saved data
			m_pCommand->m_bUsingCommandCallbackInterface = m_bUsingCommandCallbackInterface;
			m_pCommand->m_bUsingNewCommandCallback = m_bUsingNewCommandCallback;
			if (m_bUsingCommandCallbackInterface)
			{
				m_pCommand->m_pCommandCallback = m_pCommandCallback;
			}
			else
			{
				if (m_bUsingNewCommandCallback)
				{
					m_pCommand->m_fnCommandCallback = m_fnCommandCallback;
				}
				else
				{
					m_pCommand->m_fnCommandCallbackV1 = m_fnCommandCallbackV1;
				}
			}

			// null our pointers
			m_pCommand = NULL;
			m_pCallback = NULL;
		}
	}
	
	// after the hook is set up, the concommand will call this
	// instead of whatever it was calling before.
	// we saved the data so we can choose whether to call the
	// old callback after we are done or not.
	virtual void CommandCallback( const CCommand &command )
	{
		bool bDispatch = m_pCallback->CommandPreExecute( command );
		if ( m_bUsingNewCommandCallback )
		{
			if ( m_fnCommandCallback )
			{
				if (bDispatch)
				{
					( *m_fnCommandCallback )( command );
				}
				m_pCallback->CommandPostExecute( command, bDispatch );
				return;
			}
		}
		else if ( m_bUsingCommandCallbackInterface )
		{
			if ( m_pCommandCallback )
			{
				if (bDispatch)
				{
					m_pCommandCallback->CommandCallback( command );
				}
				m_pCallback->CommandPostExecute( command, bDispatch );
				return;
			}
		}
		else
		{
			if ( m_fnCommandCallbackV1 )
			{
				if (bDispatch)
				{
					( *m_fnCommandCallbackV1 )();
				}
				m_pCallback->CommandPostExecute( command, bDispatch );
				return;
			}
		}

		// Command without callback!!!
		AssertMsg( 0, ( "Encountered ConCommand without a callback!\n" ) );
	}
	
private:
	// the command that we are hooking
	ConCommand *m_pCommand;

	// only support the class based callback for now
	// this pointer provides the new pre and post callbacks
	ICommandHookCallback *m_pCallback;
	
	// data that we need to preserve before hooking
	// so we can restore it when we unhook
	union
	{
		FnCommandCallbackV1_t m_fnCommandCallbackV1;
		FnCommandCallback_t m_fnCommandCallback;
		ICommandCallback *m_pCommandCallback; 
	};
	bool m_bUsingNewCommandCallback : 1;
	bool m_bUsingCommandCallbackInterface : 1;
};

/*
class CSayHook: public ICommandHookCallback
{
public:
	CSayHook():
		m_iClientCommandIndex(0)
	{
	}

	void SetCommandClient( int index )
	{
		m_iClientCommandIndex = index;
	}

	virtual bool OnCommandPreExecute( const CCommand &args )
	{
		if ( enable.GetInt() == 1 )
		{
			if ( isExploit( args, m_iClientCommandIndex ) )
			{
				return false;
			}
		}
		return true;
	}

	virtual void OnCommandPostExecute( const CCommand &args, bool bWasCommandExecuted )
	{
	}

private:
	int m_iClientCommandIndex;
};

class CSayTeamHook: public CConCommandHook
{
public:
	CSayTeamHook():
		CConCommandHook(),
		m_iClientCommandIndex(0)
	{
	}

	void SetCommandClient( int index )
	{
		m_iClientCommandIndex = index;
	}

	virtual bool OnCommandPreExecute( const CCommand &args )
	{
		if ( enable.GetInt() == 1 )
		{
			if ( isExploit( args, m_iClientCommandIndex ) )
			{
				return false;
			}
		}
		return true;
	}

	virtual void OnCommandPostExecute( const CCommand &args, bool bWasCommandExecuted )
	{
	}

private:
	int m_iClientCommandIndex;
};
*/

static bool isExploit( const CCommand &args, int ClientCommandIndex )
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

static bool validString( const char **pChar )
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