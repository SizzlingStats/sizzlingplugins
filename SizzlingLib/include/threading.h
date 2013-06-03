
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

namespace sizz
{
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
}
