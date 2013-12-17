
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
	char sourcePath[256];
	char destPath[256];
} S3UploadInfo_t;

namespace S3Uploader
{
	bool UploadFile( S3UploadInfo_t const &info );

	class CS3UploaderThread: public sizz::CThread
	{
	public:
		CS3UploaderThread():
			m_finished_callback(nullptr)
		{
		}

		void SetUploadUrl( const char *url )
		{
			strcpy(m_info.uploadUrl, url);
		}

		void SetSourcePath( const char *sp )
		{
			strcpy(m_info.sourcePath, sp);
		}

		void SetDestPath( const char *dp )
		{
			strcpy(m_info.destPath, dp);
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
			bool ret = UploadFile(m_info);
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

	private:
		S3UploadInfo_t m_info;
		std::function<void(bool)> m_finished_callback;
	};
};

#endif // S3_UPLOADER_H

