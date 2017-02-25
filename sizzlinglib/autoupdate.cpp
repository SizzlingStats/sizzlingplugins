
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
#include "tier0/valve_minmax_off.h"
#include "miniz.h"
#include <rapidjson/document.h>

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
	metaBuffer.SetBufferType(true, true);
	if (!SizzDownloader::DownloadFile(m_info.metaUrl, metaBuffer))
	{
		return false;
	}

	rapidjson::Document doc;
	doc.Parse(metaBuffer.String());
	if (doc.HasParseError())
	{
		Msg("[SS]: Error parsing metadata.\n");
		return false;
	}

	rapidjson::Document::MemberIterator versionIt = doc.FindMember("v");
	rapidjson::Document::MemberIterator crcIt = doc.FindMember("c");
	if (versionIt == doc.MemberEnd() || crcIt == doc.MemberEnd())
	{
		Msg("[SS]: Invalid metadata format.\n");
		return false;
	}

	rapidjson::Value& version = versionIt->value;
	rapidjson::Value& crc = crcIt->value;
	if (!version.IsString() || !crc.IsNumber() || (7 != version.GetStringLength()))
	{
		Msg("[SS]: Error parsing metadata version info.\n");
		return false;
	}

	if (!CompareVersions(version.GetString(), m_info.currentVersion))
	{
		Msg("[SS]: No update available.\n");
		return false;
	}

	m_info.fileCRC = crc.GetUint();
	Msg("[SS]: Update available. Preparing to download...\n");
	return true;
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
	if ( SizzFileSystem::Exists( pRelativePath ) )
	{
		SizzFileSystem::RemoveFile( pRelativePath );
	}
}
