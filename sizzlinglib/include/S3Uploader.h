
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

////////////////////////////////////////////////////////////////////////////////
// Filename: S3Uploader.h
////////////////////////////////////////////////////////////////////////////////
#ifndef S3_UPLOADER_H
#define S3_UPLOADER_H

#include "stddef.h"
#include "threading.h"

class CUtlBuffer;

typedef struct S3UploadInfo_s
{
	char uploadUrl[256];
	char sourceDir[256];
	char sourceFile[256];
} S3UploadInfo_t;

class CS3Uploader
{
public:
	void SetUploadInfo( const S3UploadInfo_t &info ) { m_info = info; }
	bool UploadFile();

private:
	bool SendMemory(void* mem, size_t size);

private:
	S3UploadInfo_t m_info;
};

class CS3UploaderThread: public sizz::CThread
{
public:
	CS3UploaderThread():
		m_finished_callback(nullptr)
	{
	}

	virtual ~CS3UploaderThread()
	{
	}

	void StartThread()
	{
		Join();
		Start();
	}

	void SetOnFinishedS3UploadCallback( std::function<void(bool)> callback )
	{
		m_finished_callback = std::move(callback);
	}

	virtual int Run()
	{
		m_infoLock.Lock();
		bool ret = m_uploader.UploadFile();
		m_infoLock.Unlock();
		if (m_finished_callback)
		{
			m_finished_callback(ret);
		}
		return 0;
	}

	void ShutDown()
	{
		Join();
	}

	void SetUploadInfo( const S3UploadInfo_t &info ) 
	{
		m_infoLock.Lock();
		m_uploader.SetUploadInfo(info);
		m_infoLock.Unlock();
	}

private:
	CS3Uploader m_uploader;
	sizz::CThreadMutex m_infoLock;
	std::function<void(bool)> m_finished_callback;
};

#endif // S3_UPLOADER_H

