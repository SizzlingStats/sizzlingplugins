
#ifndef FUNC_QUEUE_THREAD_H
#define FUNC_QUEUE_THREAD_H

#include "queuethread.h"
#include "functors.h"

class CFuncQueueThread : public CThread 
{
public:
    CFuncQueueThread()
    {
        SOPDLog("[QUEUETHREAD] CQueueThread()\n");
        m_bExit = false;
        m_Waiting.Purge();
        SOPDLog("[QUEUETHREAD] m_queueEmptyMutex.Lock()\n");
        m_queueEmptyMutex.Lock();
        Start();
    }

    virtual ~CFuncQueueThread()
    {
        SOPDLog("[QUEUETHREAD] ~CQueueThread()\n");
        JoinQueue();
    }

    // the thread is going to begin servicing the queue
    virtual bool BeginQueue() { return true; }
    
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

    // Adds an item to the waiting queue
    void EnqueueItem(CFunctor const *pFunc)
    {
        SOPDLog("[QUEUETHREAD] EnqueueItem()\n");
        if(!pFunc || m_bExit)
            return;

        SOPDLog("[QUEUETHREAD] QueueCount()\n");
        SOPDLog("[QUEUETHREAD] m_listMutex.Lock()\n");
        m_listMutex.Lock();
        if(m_Waiting.Count())
        {
            // worker already working on the queue, so just append to the
            // existing queue. the worker will pick this item up next.
            SOPDLog("[QUEUETHREAD] Appending item to exiting queue.\n");
            m_Waiting.AddToTail(pFunc);
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
            m_Waiting.AddToTail(pFunc);
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
                    CFunctor *pCurrentFunc = m_Waiting.Element(0);
                    SOPDLog("[QUEUETHREAD] m_listMutex.Unlock()\n");
                    m_listMutex.Unlock();

                    // if the servicing fails, start over
                    // unless max fails reached
                    SOPDLog("[QUEUETHREAD] Run calling ServiceItem\n");
                    failed = !((*pCurrentFunc)());
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
    CUtlVector<CFunctor*> m_Waiting;
    bool m_bExit;
};

#endif // FUNC_QUEUE_THREAD_H

