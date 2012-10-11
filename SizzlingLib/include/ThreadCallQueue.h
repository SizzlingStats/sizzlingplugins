
#ifndef THREAD_CALL_QUEUE_H
#define THREAD_CALL_QUEUE_H

#include "callqueue.h"
//#include "queuethread.h"

class CTSCallQueue
{
public:

#pragma warning( push )
#pragma warning( disable : 4355 )
	CTSCallQueue(): m_pNopFunctor( CreateFunctor(this, &CTSCallQueue::nopFunc) ),
					m_pCallQueueFunctor( CreateFunctor(this, &CTSCallQueue::CallOne) ),
					m_pFuncCommandCaller( m_pNopFunctor )
	{
	}
#pragma warning( pop )

	~CTSCallQueue()
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

	void CallAll()
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

	inline void callQueueGameFrame()
	{
	    (*m_pFuncCommandCaller)();
	}

	void EnqueueFunctor( CFunctor *pFunctor )
	{
		QueueFunctorInternal( RetAddRef( pFunctor ) );
		ThreadInterlockedCompareExchangePointer( (void* volatile*)(&m_pFuncCommandCaller), m_pCallQueueFunctor, m_pNopFunctor );
	}

	FUNC_GENERATE_QUEUE_METHODS();

private:

    void CallOne()
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

    inline void nopFunc()
    {
    }

	void QueueFunctorInternal( CFunctor *pFunctor )
	{
		m_queue.PushItem( pFunctor );
	}

	CFunctor	*m_pNopFunctor;
	CFunctor	*m_pCallQueueFunctor;
	CTSQueue<CFunctor *>	m_queue;
public:
	CFunctor	*m_pFuncCommandCaller;
	//CThreadMutexPthread		m_FnCommandMutex;
};

#endif // THREAD_CALL_QUEUE_H