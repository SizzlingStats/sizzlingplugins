
#ifndef NETPROP_UTILS_H
#define NETPROP_UTILS_H

#include "dt_send.h"
//#include "dt_recv.h"

class CSendPropHook
{
public:
	CSendPropHook():
		m_pProp(NULL),
		m_pOldProxyFn(NULL)
	{
	}

	~CSendPropHook()
	{
		Unhook();
	}

	void Hook( SendProp *pProp, SendVarProxyFn fn )
	{
		if (pProp && fn)
		{
			m_pProp = pProp;
			m_pOldProxyFn = m_pProp->GetProxyFn();
			m_pProp->SetProxyFn(&SendVarProxyFnCallback);
		}
	}

	void Unhook()
	{
		if ( m_pProp )
		{
			m_pProp->SetProxyFn(m_pOldProxyFn);
			m_pProp = NULL;
			m_pOldProxyFn = NULL;
		}
	}

private:
	static void SendVarProxyFnCallback( const SendProp *pProp, const void *pStructBase, const void *pData, DVariant *pOut, int iElement, int objectID )
	{
	}

private:
	SendProp *m_pProp;
	SendVarProxyFn m_pOldProxyFn;
};

#endif // NETPROP_UTILS_H
