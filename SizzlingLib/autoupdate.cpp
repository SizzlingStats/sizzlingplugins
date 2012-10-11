////////////////////////////////////////////////////////////////////////////////
// Filename: autoupdate.cpp
////////////////////////////////////////////////////////////////////////////////

#include "autoupdate.h"

#include "eiface.h"
#include "SC_helpers.h"
#include "functors.h"

#include "SizzFileSystem.h"
#include "ThreadCallQueue.h"

#include "utlbuffer.h"

//#include "lzss.h"
//#include "zip/XUnzip.h"
//#include "lzmaDecoder.h"

//#include "LzmaDec.h"
//#include "LzmaEnc.h"

using namespace sizzFile;

// Interfaces from the engine
extern IVEngineServer		*pEngine;
extern CTSCallQueue			*g_pTSCallQueue;

void CAutoUpdater::PerformUpdateIfAvailable( const char *pUpdateInfo[] )
{
	if (m_bWaitingForUnload)
		return;

	const char *pluginPath = pUpdateInfo[k_eLocalPluginPath];
	const char *pluginNameNoExtension = pUpdateInfo[k_ePluginNameNoExtension];
	const char *pluginExtension = pUpdateInfo[k_ePluginExtension];

	char oldPluginPath[512];
	V_snprintf(oldPluginPath, 512, "%s%s_old%s", pluginPath, pluginNameNoExtension, pluginExtension);
	RemoveFile(oldPluginPath);

	bool isUpdate = CheckForUpdate();
	if ( !isUpdate )
		return;

	const char *pluginName = pUpdateInfo[k_ePluginName];
	char currentPluginPath[512];
	V_snprintf(currentPluginPath, 512, "%s%s", pluginPath, pluginName);

	Msg( "[SS]: Downloading update.\n" );

	CUtlBuffer updatedFile;
	bool success = m_downloader.DownloadFileAndVerify( m_info.fileUrl, m_info.fileCRC, updatedFile );
	if ( success )
	{
		// we can rename the current plugin, but not remove it
		SizzFileSystem::RenameFile( currentPluginPath, oldPluginPath );

		// write the file to disk
		sizzFile::COutputFile file( m_info.fileName );
		if ( file.IsOk() )
		{
			file.Write( updatedFile.Base(), updatedFile.GetBytesRemaining() );
			file.Close();

			const char *pluginDescriptionPart = pUpdateInfo[k_ePluginDescriptionPart];
			int index = SCHelpers::GetThisPluginIndex(pluginDescriptionPart);
			static char temp[256] = {};

			// unload the old plugin, load the new plugin
			V_snprintf( temp, 256, "plugin_unload %i; plugin_load %s\n", index, currentPluginPath);

			CLateBoundPtr<IVEngineServer> ppEngine(&pEngine);
			g_pTSCallQueue->EnqueueFunctor( CreateFunctor(ppEngine, &IVEngineServer::ServerCommand, (const char *)temp) );
			m_bWaitingForUnload = true;
			// the plugin will be unloaded when tf2 executes the command,
			// which then also loads the new version of the plugin.
			// the new version runs the updater which checks for plugin_old
			// and deletes it.
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
	if ( SizzFileSystem::FileExists( pRelativePath ) )
	{
		SizzFileSystem::RemoveFile( pRelativePath );
	}
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
