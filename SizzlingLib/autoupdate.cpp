////////////////////////////////////////////////////////////////////////////////
// Filename: autoupdate.cpp
////////////////////////////////////////////////////////////////////////////////

#include "autoupdate.h"

#include "engine/iserverplugin.h"
#include "eiface.h"
#include "game/server/iplayerinfo.h"
#include "tier2/fileutils.h"
#include "SC_helpers.h"
#include "functors.h"

#define USING_SIZZ_FILE_SYSTEM

#ifdef USING_SIZZ_FILE_SYSTEM
#include "SizzFileSystem.h"

using namespace sizzFile;
#endif

#ifdef USE_QUEUE
#include "ThreadCallQueue.h"
#endif
//#include "lzss.h"
//#include "curl\curl.h"
//#include "zip/XUnzip.h"
//#include "lzmaDecoder.h"

//#include "LzmaDec.h"
//#include "LzmaEnc.h"

#include "utlstring.h"

// Interfaces from the engine
extern IVEngineServer			*pEngine;
#ifdef USE_QUEUE
extern CTSCallQueue			*g_pTSCallQueue;
#endif
void CAutoUpdater::PerformUpdateIfAvailable( const char *pluginPath,
											 const char *pluginName,
											 const char *pluginNameNoExtension,
											 const char *pluginExtension,
											 const char *pluginDescriptionPart )
{
#ifdef USE_QUEUE
	CFunctor *pRemoveFileFunc = CreateFunctor( this, &CAutoUpdater::RemoveFile, PLUGIN_PATH PLUGIN_NAME_NO_EX "_old" PLUGIN_EXTENSION );
	g_pTSCallQueue->EnqueueFunctor( pRemoveFileFunc );
#else
	char oldPluginPath[512];
	V_snprintf(oldPluginPath, 512, "%s%s_old%s", pluginPath, pluginNameNoExtension, pluginExtension);
	RemoveFile(oldPluginPath);
#endif

	bool isUpdate = CheckForUpdate();
	if ( !isUpdate )
		return;

	char currentPluginPath[512];
	V_snprintf(currentPluginPath, 512, "%s%s", pluginPath, pluginName);

	Msg( "[SS]: Downloading update.\n" );
	// this file will be zipped when downloaded because i made it that way
	// not yet, cause zipishard
	CUtlBuffer updatedFile;
	bool success = m_downloader.DownloadFileAndVerify( m_info.fileUrl, m_info.fileCRC, updatedFile );
	if ( success )
	{
		// now, unzip the file!
		//CUtlMemory<unsigned char> unzippedFile;
		//UnzipFile( m_info.fileName, updatedFile, unzippedFile );

		// we can rename the current plugin, but not remove it
#ifdef USE_QUEUE
		CFunctor *pRenameFunc = CreateFunctor( g_pFullFileSystem, &IFileSystem::RenameFile, PLUGIN_PATH PLUGIN_NAME, PLUGIN_PATH PLUGIN_NAME_NO_EX "_old" PLUGIN_EXTENSION, (const char *)0 );
		g_pTSCallQueue->EnqueueFunctor( pRenameFunc );
#else
#ifdef USING_SIZZ_FILE_SYSTEM
		GetSizzFileSystem()->RenameFile( currentPluginPath, oldPluginPath );
#else
		g_pFullFileSystem->RenameFile( currentPluginPath, oldPluginPath );
#endif
#endif
		// write the file to disk
#ifdef USING_SIZZ_FILE_SYSTEM
		sizzFile::COutputFile file( m_info.fileName );
#else
		::COutputFile file( m_info.fileName );
#endif
		if ( file.IsOk() )
		{
			file.Write( updatedFile.Base(), updatedFile.GetBytesRemaining() );
			file.Close();

			int index = SCHelpers::GetThisPluginIndex(pluginDescriptionPart);
			char temp[256];

			// unload the old plugin, load the new plugin
			V_snprintf( temp, 256, "plugin_unload %i; plugin_load %s\n", index, currentPluginPath);
			//pEngine->ServerCommand( temp );
#ifdef USE_QUEUE
			CFunctor *pUnloadFunc = CreateFunctor( pEngine, &IVEngineServer::ServerCommand, (const char *)temp );
			g_pTSCallQueue->EnqueueFunctor( pUnloadFunc );
#else
#ifndef REQUIRE_RESTART_FOR_UPDATES
			pEngine->ServerCommand( temp );
#endif
#endif

			// we are done with this plugin now
			// it will be unloaded when the stack unwinds
			// the new plugin is now loaded and will check for the plugin_old and delete them to finish
		}
		else
		{
			return;
		}
	}
	else
	{
		return;
	}
}

bool CAutoUpdater::CheckForUpdate()
{
	CUtlBuffer metaBuffer;
	// gets the version and crc like this: "x.x.x.x\nffffffff"
	bool downloadOk = m_downloader.DownloadFile( m_info.metaUrl, metaBuffer );
	if ( !downloadOk )
		return false;
	
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
#ifdef USING_SIZZ_FILE_SYSTEM
	if ( GetSizzFileSystem()->FileExists( pRelativePath ) )
	{
		GetSizzFileSystem()->RemoveFile( pRelativePath );
	}
#else
	if ( g_pFullFileSystem->FileExists( pRelativePath ) )
	{
		g_pFullFileSystem->RemoveFile( pRelativePath );
	}
#endif
}
//static void *SzAlloc(void */*p*/, size_t size)
//{
//	return new unsigned char [size];
//
//}
//static void SzFree(void */*p*/, void *address)
//{
//	delete []address;
//}
//
//static SRes SzProgress(void */*p*/, UInt64 /*inSize*/, UInt64 /*outSize*/)
//{
//	return 0;
//}
//
//bool CAutoUpdater::UnzipFile( const char *pRelativeName, CUtlBuffer &src, CUtlMemory<unsigned char> &dest )
//{
//	size_t srcLen = src.GetBytesRemaining();
//	ELzmaStatus status;
//
//	ISzAlloc alloc = { SzAlloc, SzFree };
//
//	/* header: 5 bytes of LZMA properties and 8 bytes of uncompressed size */
//	unsigned char header[LZMA_PROPS_SIZE + 8];
//
//	/* Read and parse header */
//
//	src.Get( header, sizeof(header) );
//
//	size_t destLen = 0;
//	for ( int i = 0; i < 8; ++i )
//	{
//		destLen += (UInt64)header[LZMA_PROPS_SIZE + i] << (i * 8);
//	}
//
//	unsigned char *pDest = new unsigned char [destLen];
//
//	LzmaDecode( pDest, &destLen, (const byte*)src.Base(), &srcLen,
//    (const Byte*)header, LZMA_PROPS_SIZE, LZMA_FINISH_END,
//    &status, &alloc);
//
//	dest.SetExternalBuffer( pDest, destLen );
//	return true;
//}
//
//unsigned int CAutoUpdater::Compress( CUtlBuffer &src, CUtlMemory<unsigned char> &dest )
//{
//	unsigned char *pDest = NULL;
//	unsigned int destLen = 0;
//
//	CLzmaEncProps props;
//	props.level = 5;
//	props.reduceSize = 0xffffffff;
//	props.lc = 3;
//	props.lp = 0;
//	props.pb = 2;
//	props.algo = 1;
//	props.fb = 32;
//	props.btMode = 1;
//	props.numHashBytes = 4;
//	props.mc = 32;
//	props.writeEndMark = 0;
//	props.numThreads = 1;
//
//	ISzAlloc alloc = { SzAlloc, SzFree };
//
//	unsigned char *pProps = NULL;
//	unsigned int propsLen = 0;
//
//	ICompressProgress progress = { SzProgress };
//
//	LzmaEncode( pDest, &destLen, (const Byte *)src.Base(), src.GetBytesRemaining(),
//    &props, pProps, &propsLen, 0,
//    &progress, &alloc, &alloc);
//
//	return destLen;
//}
//
//bool CDownloader::UnzipFile( const char *pRelativeName, bool bTextMode, CUtlMemory<unsigned char> &buf )
//{
//	HZIP hZip = OpenZip( const_cast<void*>(m_fileBuffer.PeekGet()), m_fileBuffer.GetBytesRemaining(), ZIP_MEMORY );
//	ZIPENTRY zipEntry;
//	GetZipItem( hZip, 0, &zipEntry );
//	buf.EnsureCapacity( zipEntry.unc_size );
//	UnzipItem( hZip, 0, buf.Base(), zipEntry.unc_size, ZIP_MEMORY );
//	return true;
//}
