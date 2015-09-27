
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

#ifndef THREAD_FUNC_QUEUE
#define THREAD_FUNC_QUEUE

#include "threading.h"
#include "functors.h"
#include "tier0/tslist.h"

class CSizzFuncQueueThread: public sizz::CThread
{
public:
	CSizzFuncQueueThread():
		m_bJoining(false)
	{
	}

	virtual ~CSizzFuncQueueThread()
	{
		JoinQueue();
	}

	void RestartQueue()
	{
		if (m_bJoining)
		{
			m_bJoining = false;
			m_waitingForItems.Reset();
		}
		Start();
	}

	// there might be a deadlock
	// somewhere around when this 
	// is called... need to test
	void JoinQueue()
	{
		if (!m_bJoining)
		{
			m_bJoining = true;
			//Msg( "=== joining\n" );
			// let the queue run past the 
			// lock so it can exit
			m_waitingForItems.Set();
			Join();
		}
	}

	virtual int Run()
	{
		while (!m_bJoining)
		{
			// stall here while there aren't any functors to run
			//Msg( "waiting lock\n" );
			m_waitingForItems.Wait();

			//Msg( "processing functors\n" );
			// process the current number of functors
			ProcessFunctors();

			//Msg( "second lock\n" );
			// lock to make sure the thread becomes 
			// stalled if there are no more elements
			m_waitingForItems.Reset();

			//Msg( "check second lock\n" );
			// but, if there are elements, 
			// let the thread continue
			if (m_queue.Count())
			{
				//Msg( "release second lock\n" );
				m_waitingForItems.Set();
			}
		}
		// process the rest of the 
		// functors before exiting
		ProcessFunctors();
		return 0;
	}
	
	bool EnqueueFunctor( CFunctor *pFunctor )
	{
		if (pFunctor && !m_bJoining)
		{
			//Msg("enqueue\n");
			// add it to the queue
			m_queue.PushItem(pFunctor);

			// if queue was empty and is waiting
			//Msg("enqueue trylock\n");
			if (!m_waitingForItems.Check())
			{
				// let the queue run
				//Msg("unlocking from enqueue\n");
				m_waitingForItems.Set();

				// if the queue hasn't started up yet
				if (!this->IsAlive())
				{
					// start it
					Start();
				}
			}
			return true;
		}
		return false;
	}

	int32 NumQueuedFunctors()
	{
		return m_queue.Count();
	}

private:
	void ProcessFunctors()
	{
		CFunctor *pFunc = NULL;
		while (m_queue.PopItem(&pFunc))
		{
			// call the functor
			(*pFunc)();
			pFunc->Release();
		}
	}

private:
	CTSQueue<CFunctor*> m_queue;
	sizz::CThreadEvent m_waitingForItems;
	bool m_bJoining;
};

#endif // THREAD_FUNC_QUEUE
