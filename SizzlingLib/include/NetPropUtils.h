
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

#ifndef NETPROP_UTILS_H
#define NETPROP_UTILS_H

//#include "dt_send.h"
#include "dt_recv.h"

#include "Thunk.h"

// callback class for classes that hook client props
//
// inherit from this and provide the callback
class IRecvPropHookCallback
{
public:
	// return true to call the default 
	// game function as well
	virtual bool RecvPropHookCallback( const CRecvProxyData *pData, void *pStruct, void *pOut ) = 0;
};

class CRecvPropHook: private CThunkCDecl<CRecvPropHook>
{
public:
	CRecvPropHook():
		m_pProp(NULL),
		m_pOldProxyFn(NULL),
		m_pThis(NULL)
	{
		InitThunk(reinterpret_cast<CThunkCDecl<CRecvPropHook>::ThunkType>(&CRecvPropHook::FnCallback), this);
	}

	~CRecvPropHook()
	{
		Unhook();
	}

	void Hook( RecvProp *pProp, IRecvPropHookCallback *pThis )
	{
		// if we haven't hooked already
		if (!IsHooked() && pProp && pThis)
		{
			m_pProp = pProp;
			m_pOldProxyFn = m_pProp->GetProxyFn();
			m_pProp->SetProxyFn(GetThunk<RecvVarProxyFn>());
			m_pThis = pThis;
		}
	}

	void Unhook()
	{
		if ( IsHooked() )
		{
			m_pProp->SetProxyFn(m_pOldProxyFn);
			m_pProp = NULL;
			m_pOldProxyFn = NULL;
			m_pThis = NULL;
		}
	}

	bool IsHooked()
	{
		// don't cast directly from pointer to bool
		// for returning from functions
		return !!m_pProp;
	}

private:
	void __cdecl FnCallback( const CRecvProxyData *pData, void *pStruct, void *pOut )
	{
		if (m_pThis->RecvPropHookCallback(pData, pStruct, pOut))
		{
			m_pOldProxyFn(pData, pStruct, pOut);
		}
	}

private:
	RecvProp *m_pProp;
	RecvVarProxyFn m_pOldProxyFn;
	IRecvPropHookCallback *m_pThis;
};

#endif // NETPROP_UTILS_H

