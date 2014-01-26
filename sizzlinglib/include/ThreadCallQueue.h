
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

#ifndef THREAD_CALL_QUEUE_H
#define THREAD_CALL_QUEUE_H

#include "tslist.h"
#include "functors.h"

class CTSCallQueue
{
public:
	CTSCallQueue();
	~CTSCallQueue();

	void CallAll();
	void callQueueGameFrame();
	void EnqueueFunctor( CFunctor *pFunctor );

private:
	void CallOne();
	void nopFunc();
	void QueueFunctorInternal( CFunctor *pFunctor );

private:
	CFunctor	*m_pFuncCommandCaller;
	CFunctor	*m_pCallQueueFunctor;
	CFunctor	*m_pNopFunctor;
	CTSQueue<CFunctor *>	m_queue;
};

#pragma warning( push )
#pragma warning( disable : 4355 )
inline CTSCallQueue::CTSCallQueue():
	m_pFuncCommandCaller( CreateFunctor(this, &CTSCallQueue::nopFunc) ),
	m_pCallQueueFunctor( CreateFunctor(this, &CTSCallQueue::CallOne) ),
	m_pNopFunctor( m_pFuncCommandCaller )
{
}
#pragma warning( pop )

inline CTSCallQueue::~CTSCallQueue()
{
	while ( m_queue.Count() )
	{
		CFunctor *pFunctor = NULL;
		if ( m_queue.PopItem( &pFunctor ) )
		{
			pFunctor->Release();
		}
	}
	m_pNopFunctor->Release();
	m_pCallQueueFunctor->Release();
}

inline void CTSCallQueue::CallAll()
{
	while ( m_queue.Count() )
	{
		CFunctor *pFunctor = NULL;
		if ( m_queue.PopItem( &pFunctor ) )
		{
			(*pFunctor)();
			pFunctor->Release();
		}
	}
}

inline void CTSCallQueue::callQueueGameFrame()
{
	(*m_pFuncCommandCaller)();
}

inline void CTSCallQueue::EnqueueFunctor( CFunctor *pFunctor )
{
	QueueFunctorInternal( RetAddRef( pFunctor ) );
	ThreadInterlockedCompareExchangePointer( (void* volatile*)(&m_pFuncCommandCaller), m_pCallQueueFunctor, m_pNopFunctor );
}

inline void CTSCallQueue::CallOne()
{
	if ( m_queue.Count() )
	{
		CFunctor *pFunctor = NULL;
		if ( m_queue.PopItem( &pFunctor ) )
		{
			(*pFunctor)();
			pFunctor->Release();
		}
	}
	else
	{
		ThreadInterlockedExchangePointer( (void* volatile*)(&m_pFuncCommandCaller), m_pNopFunctor );
	}
}

inline void CTSCallQueue::nopFunc()
{
}

inline void CTSCallQueue::QueueFunctorInternal( CFunctor *pFunctor )
{
	m_queue.PushItem( pFunctor );
}

#endif // THREAD_CALL_QUEUE_H
