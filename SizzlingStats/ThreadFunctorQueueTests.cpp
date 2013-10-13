
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

#include "ThreadFunctorQueue.h"
#include "convar.h"

class CQueueTest: public sizz::CThread
{
public:
	CQueueTest():
		m_pQueue(NULL)
	{
	}

	~CQueueTest()
	{
		Join();
	}

	void RunTests( CSizzFuncQueueThread *pQueue, int iterations )
	{
		m_pQueue = pQueue;
		m_iterations = iterations;
		Start();
	}

	virtual int Run()
	{
		for ( int i = 0; i < m_iterations; ++i )
		{
			m_pQueue->EnqueueFunctor( CreateFunctor(&CQueueTest::TestFunc, i) );
		}
		return 0;
	}

	static void TestFunc( int num )
	{
		Msg( "%i\n", num );
	}

private:
	CSizzFuncQueueThread *m_pQueue;
	int m_iterations;
};

static void TestNoItems()
{
	CSizzFuncQueueThread queue;
	queue.JoinQueue();
}

static void TestSingleThreadItems( int num_iterations )
{
	CSizzFuncQueueThread queue;
	for (int i = 0; i < num_iterations; ++i)
	{
		queue.EnqueueFunctor( CreateFunctor(&CQueueTest::TestFunc, i) );
	}
	queue.JoinQueue();
}

static void TestMultiThreadItems( int threads, int num_iterations )
{
	CQueueTest *pTests = new CQueueTest[threads];
	CSizzFuncQueueThread queue;
	for (int i = 0; i < threads; ++i)
	{
		pTests->RunTests(&queue, num_iterations);
	}
	for (int i = 0; i < threads; ++i)
	{
		pTests->Join();
	}
	queue.JoinQueue();
	delete [] pTests;
}

static void TestQueueRestart( int num_restarts )
{
	CSizzFuncQueueThread queue;
	for (int i = 0; i < num_restarts; ++i)
	{
		for (int i = 0; i < 10000; ++i)
		{
			queue.EnqueueFunctor( CreateFunctor(&CQueueTest::TestFunc, i) );
		}
		queue.JoinQueue();
		queue.RestartQueue();
	}
	queue.JoinQueue();
}

static void RunQueueTests()
{
	Msg( "starting queue tests\n" );
	TestNoItems();
	TestSingleThreadItems(1000);
	for (int i = 0; i < 1; ++i)
	{
		TestMultiThreadItems(4, 1000);
	}
	TestQueueRestart(5);
	Msg("tests over\n");
}

static ConCommand tests("sizz_queue_tests", &RunQueueTests, "Runs Sizzling's queue tests" );
