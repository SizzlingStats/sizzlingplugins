
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
private:
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
	
	~CConCommandHook();
	
	bool Hook( ICommandHookCallback *pThis, ICvar *pCvar, const char *pszCommandToHook )
	{
		// if the cvar pointer and the string are valid
		// and a command isn't already hooked
		if (pCvar && pszCommandToHook && !m_pCommand)
		{
			// find the current registered concommand that you want to hook
			m_pCommand = pCvar->FindCommand( pszCommandToHook );
			// if it was valid, return true. else false
			if (m_pCommand)
			{
				if (m_pCommand->m_bUsingCommandCallbackInterface)
				{
					m_pCommandCallback = m_pCommand->m_pCallback;
					m_bUsingCommandCallbackInterface = true;
					m_bUsingNewCommandCallback = false;
				}
				else
				{
					m_bUsingCommandCallbackInterface = false;
					if (m_pCommand->m_bUsingNewCommandCallback)
					{
						m_fnCommandCallback = m_pCommand->m_fnCommandCallback;
						m_bUsingNewCommandCallback = true;
					}
					else
					{
						m_fnCommandCallbackV1 = m_pCommand->m_fnCommandCallbackV1;
						m_bUsingNewCommandCallback = false;
					}
				}
				
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
		m_pCommand->m_bUsingCommandCallbackInterface = m_bUsingCommandCallbackInterface;
		m_pCommand->m_bUsingNewCommandCallback = m_bUsingNewCommandCallback;
		if (m_pCommand->m_bUsingCommandCallbackInterface)
		{
			m_pCommand->m_pCommandCallback = m_pCallback;
		}
		else
		{
			if (m_pCommand->m_bUsingNewCommandCallback)
			{
				m_pCommand->m_fnCommandCallback = m_fnCommandCallback;
			}
			else
			{
				m_pCommand->m_fnCommandCallbackV1 = m_fnCommandCallbackV1;
			}
		}
		m_pCommand = NULL;
		m_pCallback = NULL;
	}
	
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
	ConCommand *m_pCommand;
	// only support the class based callback for now
	ICommandHookCallback *m_pCallback;
	
	// data that we need to preserve before changing
	union
	{
		FnCommandCallbackV1_t m_fnCommandCallbackV1;
		FnCommandCallback_t m_fnCommandCallback;
		ICommandCallback *m_pCommandCallback; 
	};
	bool m_bUsingNewCommandCallback : 1;
	bool m_bUsingCommandCallbackInterface : 1;
};

class CConCommandHook: public ICommandCallback
{
public:
	CConCommandHook():
		m_pOriginalCommand(NULL),
		m_pNewCommand(NULL)
	{
	}

	~CConCommandHook()
	{
		Unhook(NULL);
	}

	// call this once when you have a valid ICvar global to pass in.
	// hooks the command so we can execute before and after it
	bool Hook( ICvar *pCvar, const char *pszCommandToHook )
	{
		// if the cvar pointer and the string are valid
		if (pCvar && pszCommandToHook)
		{
			// find the current registered concommand that you want to hook
			m_pOriginalCommand = pCvar->FindCommand( pszCommandToHook );
			// if it was valid
			if (m_pOriginalCommand)
			{
				// cast it to a base concommand, then unregister it from the game
				ConCommandBase *pCommandBase = static_cast<ConCommandBase*>(m_pOriginalCommand);
				pCvar->UnregisterConCommand(pCommandBase);
				// Uses the other command's 'name' and 'helptext' memory for the strings.
				// Should maybe copy it in, but since it's static data for globally defined 
				// ConCommands, then it should be fine to use it as is. Unless we want cache 
				// locality for the class and it's strings...
				//
				// construct our new con command with that name, so the game calls ours instead
				// then we can pass execution to the old one if we want since we still have 
				// a pointer to it.
				m_pNewCommand = new(&m_CommandMem) ConCommand(pCommandBase->GetName(), 
												static_cast<ICommandCallback*>(this), 
												pCommandBase->GetHelpText(), 
												pCommandBase->m_nFlags);
			}
		}
		return true;
	}

	// unhooks the command and leaves it in a state the same 
	// as before we touched it
	void Unhook( ICvar *pCvar )
	{
		// if we previously hooked the command...
		if (pCvar && m_pNewCommand)
		{
			// unregister our version of the command
			pCvar->UnregisterConCommand( static_cast<ConCommandBase*>(m_pNewCommand) );
			// re-register the previous default one
			pCvar->RegisterConCommand( static_cast<ConCommandBase*>(m_pOriginalCommand) );
			
			// call the destructor and set the pointer to null
			m_pNewCommand->~ConCommand();
			m_pNewCommand = NULL;
		}
	}

	// define this
	// return false to interrupt normal execution of ConCommand
	virtual bool OnCommandPreExecute( const CCommand &args ) = 0;

	virtual void OnCommandPostExecute( const CCommand &args, bool bWasCommandExecuted ) = 0;

private:
	// private so noone overrides this method in a derived class
	virtual void CommandCallback( const CCommand &command )
	{
		if (m_pOriginalCommand)
		{
			bool bDispatch = OnCommandPreExecute(command);
			if (bDispatch)
			{
				m_pOriginalCommand->Dispatch(command);
			}
			OnCommandPostExecute(command, bDispatch);
		}
	}

private:
	ConCommand *m_pOriginalCommand;
	ConCommand *m_pNewCommand;
	// this memory is for the ConCommand so we can choose when 
	// we want it's constructor called. Since calling the constructor 
	// registers the command with the game, we want to delay that call 
	// until we have unregistered the default game command with that 
	// name. If we don't, only bad things can come of having two 
	// registered concommands with the same name.
	unsigned char m_CommandMem[sizeof(ConCommand)];
};

class CSayHook: public CConCommandHook
{
public:
	CSayHook():
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

