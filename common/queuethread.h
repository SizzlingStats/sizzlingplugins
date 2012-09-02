/*
    This file is part of SourceOP.

    SourceOP is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    SourceOP is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with SourceOP.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef QUEUETHREAD_H
#define QUEUETHREAD_H

/**
 * A thread that will process a queue of items. When the queue is depleted,
 * EndQueue() is called and the thread will wait for more items.
 *
 * @author Tony Paloma
 * @version 6/1/2010
 */

#ifdef Yield
#undef Yield
#endif
#include "vstdlib/jobthread.h"

#define QUEUE_ITEM_MAX_FAILS 5

//#include "AdminOP.h"
#ifdef SOPDLog
#undef SOPDLog
#endif
#define SOPDLog(s) ((void)0)

#ifndef _WIN32
#include <pthread.h>
#include "tier0/memdbgon.h"
class CThreadMutexPthread
{
public:
    CThreadMutexPthread()
    {
        pthread_mutex_init(&m_mutex, 0);
    }

    ~CThreadMutexPthread()
    {
        pthread_mutex_destroy(&m_mutex);
    }

    void Lock()
    {
        pthread_mutex_lock(&m_mutex);
    }

    void Unlock()
    {
        pthread_mutex_unlock(&m_mutex);
    }
private:
    pthread_mutex_t m_mutex;
};
#else
#include <Windows.h>
#include "tier0/memdbgon.h"
class CThreadMutexPthread
{
public:
    CThreadMutexPthread()
    {
        m_binarySemaphore = CreateSemaphore(NULL, 1, 1, NULL);
    }

    ~CThreadMutexPthread()
    {
        CloseHandle(m_binarySemaphore);
    }

    void Lock()
    {
        WaitForSingleObject(m_binarySemaphore, INFINITE);
    }

    void Unlock()
    {
        ReleaseSemaphore(m_binarySemaphore, 1, NULL);
    }
private:
    HANDLE m_binarySemaphore;
};
#endif

template<class T> class CQueueThread : public CThread 
{
public:
    CQueueThread()
    {
        SOPDLog("[QUEUETHREAD] CQueueThread()\n");
        m_bExit = false;
        m_Waiting.Purge();
        SOPDLog("[QUEUETHREAD] m_queueEmptyMutex.Lock()\n");
        m_queueEmptyMutex.Lock();
        Start();
    }

    virtual ~CQueueThread()
    {
        SOPDLog("[QUEUETHREAD] ~CQueueThread()\n");
        JoinQueue();
    }

    // the thread is going to begin servicing the queue
    virtual bool BeginQueue() { return true; }
    // service one item
    virtual bool ServiceItem(T const &item) = 0;
    // all items in the queue have been depleted
    virtual void EndQueue() { }
    // thread is starting
    virtual void ThreadInit() { }
    // thread is ending
    virtual void ThreadEnd() { }
    // stop accepting queue tasks and wait for queue to finish current task
    void JoinQueue()
    {
        SOPDLog("[QUEUETHREAD] JoinQueue()\n");
        if(!m_bExit)
        {
            m_bExit = true;
            SOPDLog("[QUEUETHREAD]  - m_queueEmptyMutex.Unlock()\n");
            m_queueEmptyMutex.Unlock();
            SOPDLog("[QUEUETHREAD]  - calling Join()\n");
            Join();
        }
    }
    // how many tasks are queued
    int QueueCount()
    {
        int ret = 0;

        SOPDLog("[QUEUETHREAD] QueueCount()\n");
        SOPDLog("[QUEUETHREAD] m_listMutex.Lock()\n");
        m_listMutex.Lock();
        ret = m_Waiting.Count();
        SOPDLog("[QUEUETHREAD] m_listMutex.Unlock()\n");
        m_listMutex.Unlock();

        return ret;
    }
    // get a particular queue element. unreliable if the queue is running.
    const T& QueueElement(int elem)
    {
        SOPDLog("[QUEUETHREAD] QueueElement(int elem)\n");
        return m_Waiting.Element(elem);
    }

    // Adds an item to the waiting queue
    void EnqueueItem(T const &item)
    {
        SOPDLog("[QUEUETHREAD] EnqueueItem()\n");
        if(m_bExit)
            return;

        SOPDLog("[QUEUETHREAD] QueueCount()\n");
        SOPDLog("[QUEUETHREAD] m_listMutex.Lock()\n");
        m_listMutex.Lock();
        if(m_Waiting.Count())
        {
            // worker already working on the queue, so just append to the
            // existing queue. the worker will pick this item up next.
            SOPDLog("[QUEUETHREAD] Appending item to exiting queue.\n");
            m_Waiting.AddToTail(item);
            SOPDLog("[QUEUETHREAD] m_listMutex.Unlock()\n");
            m_listMutex.Unlock();
            if(!IsAlive())
            {
                SOPDLog("[QUEUETHREAD] Thread not started but should be. Starting thread.\n");
                Msg("[SOURCEOP] Thread not started but should be. Starting thread.\n");
                bool r = Start();
                if(!r)
                    SOPDLog("[QUEUETHREAD] Start failed!\n");
            }
        }
        else
        {
            SOPDLog("[QUEUETHREAD] Adding first item to queue.\n");
            m_Waiting.AddToTail(item);
            SOPDLog("[QUEUETHREAD] m_listMutex.Unlock()\n");
            m_listMutex.Unlock();
            // signal worker to begin on this queue
            SOPDLog("[QUEUETHREAD] signal worker to begin\n");
            SOPDLog("[QUEUETHREAD] m_queueEmptyMutex.Unlock()\n");
            m_queueEmptyMutex.Unlock();
        }

        SOPDLog("[QUEUETHREAD] EnqueueItem exiting\n");
    }

    // Do not call this directly.
    int Run()
    {
        ThreadInit();
        SOPDLog("[QUEUETHREAD] Run()\n");
        while(!m_bExit)
        {
            // try again?
            bool failed = true;
            int failcount = 0;

            SOPDLog("[QUEUETHREAD] Permaloop\n");
            SOPDLog("[QUEUETHREAD] m_queueEmptyMutex.Lock()\n");
            m_queueEmptyMutex.Lock();
            SOPDLog("[QUEUETHREAD] m_queueEmptyMutex.Unlock() after lock\n");
            m_queueEmptyMutex.Unlock();

            // it's possible we are here when exit is true, so check that there
            // is actually work to do before attempting to BeginQueue and
            // potentially cause a mysql connection or something
            if(m_bExit)
            {
                SOPDLog("[QUEUETHREAD] m_listMutex.Lock()\n");
                m_listMutex.Lock();
                bool bActuallyHasItemsWaiting = m_Waiting.Count() > 0;
                SOPDLog("[QUEUETHREAD] m_listMutex.Unlock()\n");
                m_listMutex.Unlock();

                if(!bActuallyHasItemsWaiting)
                {
                    // no items waiting, do not start the main loop
                    failed = false;
                }
            }

            while(failed)
            {
                SOPDLog("[QUEUETHREAD] Run() main loop\n");
                failed = false;
                if(!BeginQueue())
                {
                    SOPDLog("[QUEUETHREAD] BeginQueue failed.\n");
                    failed = true;
                    SOPDLog("[QUEUETHREAD] Run calling EndQueue on failure\n");
                    EndQueue();
                    SOPDLog("[QUEUETHREAD] Run sleeping on failire\n");
                    Sleep(250);
                    continue;
                }
                SOPDLog("[QUEUETHREAD] m_listMutex.Lock()\n");
                m_listMutex.Lock();
                bool bRunLoop = m_Waiting.Count() > 0;
                SOPDLog("[QUEUETHREAD] m_listMutex.Unlock()\n");
                m_listMutex.Unlock();
                while(bRunLoop)
                {
                    SOPDLog("[QUEUETHREAD] Run inner loop\n");
                    SOPDLog("[QUEUETHREAD] m_listMutex.Lock()\n");
                    m_listMutex.Lock();
                    SOPDLog("[QUEUETHREAD] Accessing top item\n");
                    T curEntry = m_Waiting.Element(0);
                    SOPDLog("[QUEUETHREAD] m_listMutex.Unlock()\n");
                    m_listMutex.Unlock();

                    // if the servicing fails, start over
                    // unless max fails reached
                    SOPDLog("[QUEUETHREAD] Run calling ServiceItem\n");
                    failed = !ServiceItem(curEntry);
                    if(failed && failcount < QUEUE_ITEM_MAX_FAILS)
                    {
                        SOPDLog("[QUEUETHREAD] ServiceItem failed\n");
                        failcount++;
                        Sleep(250);
                        break;
                    }

                    failcount = 0;
                    SOPDLog("[QUEUETHREAD] m_listMutex.Lock()\n");
                    m_listMutex.Lock();
                    SOPDLog("[QUEUETHREAD] Removing top item\n");
                    m_Waiting.Remove(0);
                    // stop looping if the list is now empty and wait for another call
                    bRunLoop = m_Waiting.Count() > 0;
                    if(!bRunLoop)
                    {
                        SOPDLog("[QUEUETHREAD] m_queueEmptyMutex.Lock()\n");
                        m_queueEmptyMutex.Lock();
                    }
                    SOPDLog("[QUEUETHREAD] m_listMutex.Unlock()\n");
                    m_listMutex.Unlock();
                    SOPDLog("[QUEUETHREAD] Mutex unlocked. Returning to top of loop.\n");
                }
                SOPDLog("[QUEUETHREAD] Run calling EndQueue since all items serviced\n");
                EndQueue();
                SOPDLog("[QUEUETHREAD] EndQueue finished\n");
            }
        }
        SOPDLog("[QUEUETHREAD] Run exiting\n");
        ThreadEnd();
        return 0;
    }

private:
    CThreadMutexPthread m_listMutex;
    CThreadMutexPthread m_queueEmptyMutex;
    CUtlVector<T> m_Waiting;
    bool m_bExit;
};

#endif // QUEUETHREAD_H
