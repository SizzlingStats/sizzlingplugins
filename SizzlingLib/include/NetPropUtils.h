
#ifndef NETPROP_UTILS_H
#define NETPROP_UTILS_H

#include "dt_send.h"
//#include "dt_recv.h"

#include "Thunk.h"

// return true to call the original proxy fn as well
typedef bool (*SendVarProxyFnCallback)(const SendProp *pProp, const void *pStructBase, const void *pData, DVariant *pOut, int iElement, int objectID);

class CSendPropHook: private CThunk<CSendPropHook>
{
public:
	CSendPropHook():
		m_pProp(NULL),
		m_pOldProxyFn(NULL),
		m_pCallback(NULL)
	{
	    InitThunk(reinterpret_cast<CThunk<CSendPropHook>::ThunkType>(&CSendPropHook::FnCallback), this);
	}

	~CSendPropHook()
	{
		Unhook();
	}

	void Hook( SendProp *pProp, SendVarProxyFnCallback callbackFn )
	{
	    // if we haven't hooked already
		if (!IsHooked() && pProp && callbackFn)
		{
			m_pProp = pProp;
			m_pOldProxyFn = m_pProp->GetProxyFn();
			m_pProp->SetProxyFn(GetThunk<SendVarProxyFn>());
			m_pCallback = callbackFn;
		}
	}

	void Unhook()
	{
		if ( IsHooked() )
		{
			m_pProp->SetProxyFn(m_pOldProxyFn);
			m_pProp = NULL;
			m_pOldProxyFn = NULL;
			m_pCallback = NULL;
		}
	}
	
	// don't cast directly from pointer to bool
	bool IsHooked()
	{
	    return !!m_pProp;
	}

private:
    void FnCallback( const SendProp *pProp, const void *pStructBase, const void *pData, DVariant *pOut, int iElement, int objectID )
    {
        if (m_pCallback(pProp, pStructBase, pData, pOut, iElement, objectID))
        {
            m_pOldProxyFn(pProp, pStructBase, pData, pOut, iElement, objectID);
        }
    }

private:
	SendProp *m_pProp;
	SendVarProxyFn m_pOldProxyFn;
	SendVarProxyFnCallback m_pCallback;
};

#endif // NETPROP_UTILS_H

