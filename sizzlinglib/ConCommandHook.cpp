
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

#include "ConCommandHook.h"

bool CConCommandHook::Hook( ICommandHookCallback *pThis, ICvar *pCvar, const char *pszCommandToHook )
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
	
void CConCommandHook::Unhook()
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
	
void CConCommandHook::CommandCallback( const CCommand &command )
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
