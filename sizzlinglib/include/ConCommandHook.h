
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

#ifndef CON_COMMAND_HOOK_H
#define CON_COMMAND_HOOK_H

#include "convar.h"

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
	
	bool Hook( ICommandHookCallback *pThis, ICvar *pCvar, const char *pszCommandToHook );
	void Unhook();
	
	// after the hook is set up, the concommand will call this
	// instead of whatever it was calling before.
	// we saved the data so we can choose whether to call the
	// old callback after we are done or not.
	virtual void CommandCallback( const CCommand &command );
	
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
		FnCommandCallbackVoid_t m_fnCommandCallbackV1;
		FnCommandCallback_t m_fnCommandCallback;
		ICommandCallback *m_pCommandCallback; 
	};
	bool m_bUsingNewCommandCallback : 1;
	bool m_bUsingCommandCallbackInterface : 1;
};

#endif // CON_COMMAND_HOOK_H
