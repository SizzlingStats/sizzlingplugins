
#ifndef NETPROP_UTILS_H
#define NETPROP_UTILS_H

#include "dt_send.h"
//#include "dt_recv.h"

#include "Thunk.h"

// callback class for classes that hook props
//
// inherit from this and provide the callback
class IPropHookCallback
{
public:
	// return true to call the default 
	// game function as well
	virtual bool SendPropHookCallback( const SendProp *pProp, const void *pStructBase, const void *pData, DVariant *pOut, int iElement, int objectID ) = 0;
};

class CSendPropHook: private CThunkCDecl<CSendPropHook>
{
public:
	CSendPropHook():
		m_pProp(NULL),
		m_pOldProxyFn(NULL),
		m_pThis(NULL)
	{
		InitThunk(reinterpret_cast<CThunkCDecl<CSendPropHook>::ThunkType>(&CSendPropHook::FnCallback), this);
	}

	~CSendPropHook()
	{
		Unhook();
	}

	void Hook( SendProp *pProp, IPropHookCallback *pThis )
	{
		// if we haven't hooked already
		if (!IsHooked() && pProp && pThis)
		{
			m_pProp = pProp;
			m_pOldProxyFn = m_pProp->GetProxyFn();
			m_pProp->SetProxyFn(GetThunk<SendVarProxyFn>());
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

	// don't cast directly from pointer to bool
	bool IsHooked()
	{
		return !!m_pProp;
	}

private:
	void __cdecl FnCallback( const SendProp *pProp, const void *pStructBase, const void *pData, DVariant *pOut, int iElement, int objectID )
	{
		if (m_pThis->SendPropHookCallback(pProp, pStructBase, pData, pOut, iElement, objectID))
		{
			m_pOldProxyFn(pProp, pStructBase, pData, pOut, iElement, objectID);
		}
	}

private:
	SendProp *m_pProp;
	SendVarProxyFn m_pOldProxyFn;
	IPropHookCallback *m_pThis;
};

#endif // NETPROP_UTILS_H

