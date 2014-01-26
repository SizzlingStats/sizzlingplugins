
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

#pragma once

#include <mutex>
#include <condition_variable>
#include <thread>

namespace sizz
{
	class CThreadMutex
	{
	public:
		// locks the mutex
		void Lock();

		// unlocks the mutex
		void Unlock();

		// tries to lock the mutex
		// returns immediately on any condition
		// returns true if locked, false if not locked
		bool TryLock();

	private:
		std::mutex m_mutex;
	};

	inline void CThreadMutex::Lock()
	{
		m_mutex.lock();
	}

	inline void CThreadMutex::Unlock()
	{
		m_mutex.unlock();
	}

	inline bool CThreadMutex::TryLock()
	{
		return m_mutex.try_lock();
	}

	class CAutoLock
	{
	public:
		CAutoLock( CThreadMutex &mutex );
		~CAutoLock();

	private:
		CAutoLock( const CAutoLock &other );
		CAutoLock &operator=( const CAutoLock &other );

	private:
		CThreadMutex &m_mutex;
	};

	inline CAutoLock::CAutoLock( CThreadMutex &mutex ):
		m_mutex(mutex)
	{
		m_mutex.Lock();
	}

	inline CAutoLock::~CAutoLock()
	{
		m_mutex.Unlock();
	}

	class CThreadEvent
	{
	public:
		CThreadEvent();

		// set the signal
		void Set();

		// unset the signal
		void Reset();

		// returns true if the signal is set
		bool Check();

		// waits until the signal is set
		void Wait();

	private:
		std::condition_variable m_cond;
		std::mutex m_mutex;
		bool m_set;
	};

	inline CThreadEvent::CThreadEvent():
		m_set(false)
	{
	}

	inline void CThreadEvent::Set()
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		m_set = true;
		m_cond.notify_all();
	}

	inline void CThreadEvent::Reset()
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		m_set = false;
	}

	inline bool CThreadEvent::Check()
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		return m_set;
	}

	inline void CThreadEvent::Wait()
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		if (!m_set)
		{
			m_cond.wait(lock, [&]{ return m_set; });
		}
	}

	class CThread
	{
	public:
		virtual ~CThread();

		void Start();

		void Stop();

		void Join();

		bool IsAlive() const;

	protected:
		virtual int Run() = 0;

	private:
		std::thread m_thread;
	};

	inline CThread::~CThread()
	{
		Join();
	}

	inline void CThread::Start()
	{
		if (!IsAlive())
		{
			m_thread = std::thread(std::mem_fun(&CThread::Run), this);
		}
	}

	inline void CThread::Stop()
	{
		Join();
	}

	inline void CThread::Join()
	{
		if (IsAlive())
		{
			m_thread.join();
		}
	}

	inline bool CThread::IsAlive() const
	{
		return m_thread.joinable();
	}
}
