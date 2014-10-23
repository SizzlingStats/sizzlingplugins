
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

////////////////////////////////////////////////////////////////////////////////
// Filename: autoupdate.cpp
////////////////////////////////////////////////////////////////////////////////

#include "autoupdate.h"

#include "SC_helpers.h"
#include "SizzFileSystem.h"
#include "utlbuffer.h"
#include "miniz.h"

using namespace sizzFile;

bool CAutoUpdater::PerformUpdateIfAvailable( const char *pUpdateInfo[] )
{
	if (m_bWaitingForUnload)
	{
		return false;
	}

	const char *pluginPath = pUpdateInfo[k_eLocalPluginPath];
	const char *pluginNameNoExtension = pUpdateInfo[k_ePluginNameNoExtension];
	const char *pluginExtension = pUpdateInfo[k_ePluginExtension];

	char oldPluginPath[512] = {};
	V_snprintf(oldPluginPath, 512, "%s%s_old%s", pluginPath, pluginNameNoExtension, pluginExtension);
	RemoveFile(oldPluginPath);

	bool isUpdate = CheckForUpdate();
	if ( !isUpdate )
	{
		return false;
	}

	const char *pluginName = pUpdateInfo[k_ePluginName];
	char currentPluginPath[512] = {};
	V_snprintf(currentPluginPath, 512, "%s%s", pluginPath, pluginName);

	Msg( "[SS]: Downloading update.\n" );

	CUtlBuffer updatedFile;
	bool success = SizzDownloader::DownloadFileAndVerify( m_info.fileUrl, m_info.fileCRC, updatedFile );
	if ( success )
	{
		// init the zip struct
		mz_zip_archive zip;
		V_memset(&zip, 0, sizeof(zip));

		// open the zipped update file
		mz_bool ret = mz_zip_reader_init_mem(&zip, updatedFile.Base(), updatedFile.GetBytesRemaining(), 0);
		if (MZ_TRUE != ret)
		{
			return false;
		}

		// we can rename the current plugin, but not remove it
		SizzFileSystem::RenameFile(currentPluginPath, oldPluginPath);

		// unzip the file to disk
		ret = mz_zip_reader_extract_file_to_file(&zip, pluginName, currentPluginPath, 0);
		if (MZ_TRUE != ret)
		{
			return false;
		}

		// close the zip file
		mz_zip_reader_end(&zip);

		// set the flag that we're waiting to update
		m_bWaitingForUnload = true;

		return true;
	}
	return false;
}

bool CAutoUpdater::CheckForUpdate()
{
	CUtlBuffer metaBuffer;
	// gets the version and crc like this: "x.x.x.x\nffffffff"
	bool downloadOk = SizzDownloader::DownloadFile( m_info.metaUrl, metaBuffer );
	if ( !downloadOk )
	{
		return false;
	}
	
	char version[8];
	metaBuffer.Get( version, 7 );
	version[7] = '\0';

	// if the current version is newer or the same
	// as the downloaded version string
	bool isUpdate = CompareVersions( version, m_info.currentVersion );
	//Msg( "current: %s, new: %s\n", m_info.currentVersion, version );
	if ( !isUpdate )
	{
		Msg( "[SS]: No update available.\n" );// no update needed/available
		return false;
	}
	else
	{
		Msg( "[SS]: Update available. Preparing to download...\n" );
		// update available

		 // get rid of the CR LF
		metaBuffer.GetChar();
		metaBuffer.GetChar();

		char crc[9];
		metaBuffer.Get( crc, 8 );

		// not sure if we need to null terminate this string since
		// the function we use it in goes by the count we give it
		crc[8] = '\0';

		// convert the string representation of the hex value to an int
		SCHelpers::S_littleendianhextobinary( crc, 8, (unsigned char*)(&m_info.fileCRC), sizeof(m_info.fileCRC) );
		return true;
	}
}

// returns true if v1 is newer than v2
bool CAutoUpdater::CompareVersions( const char *v1, const char *v2 )
{
	// assuming the strings conform to the spec "x.x.x.x\0"
	for ( int i = 0; i < 3; ++i )
	{
		if ( *v1 > *v2 )
		{
			return true;
		}
		else if ( *v1 < *v2 )
		{
			return false;
		}
		v1+=2;
		v2+=2;
	}
	if ( *v1 > *v2 )
	{
		return true;
	}
	else 
	{
		return false;
	}
}

void CAutoUpdater::RemoveFile( const char *pRelativePath )
{
	if ( SizzFileSystem::FileExists( pRelativePath ) )
	{
		SizzFileSystem::RemoveFile( pRelativePath );
	}
}
