////////////////////////////////////////////////////////////////////////////////
// Filename: SizzFileSystem.cpp
////////////////////////////////////////////////////////////////////////////////
#include "SizzFileSystem.h"
#include <stdio.h>
#include <sys/stat.h>

CSizzFileSystem::CSizzFileSystem()
{
}

CSizzFileSystem::~CSizzFileSystem()
{
}

// do i need to error check these for NULL?

FileHandle_t CSizzFileSystem::OpenFile( const char *pszPath, const char *pszOptions )
{
	return fopen( pszPath, pszOptions );
}

void CSizzFileSystem::CloseFile( FileHandle_t file )
{
	fclose(reinterpret_cast<FILE*>(file));
}

bool CSizzFileSystem::FileExists( const char *pszPath )
{
	struct stat buf;
	return (stat(pszPath, &buf) != -1);
}

bool CSizzFileSystem::RemoveFile( const char *pszPath )
{
	return !remove(pszPath);
}

bool CSizzFileSystem::RenameFile( const char *pszOldPath, const char *pszNewPath )
{
	return !rename(pszOldPath, pszNewPath);
}

bool CSizzFileSystem::IsOk( FileHandle_t file )
{
	return !ferror(reinterpret_cast<FILE*>(file));
}

unsigned int CSizzFileSystem::Write( const void *pInput, unsigned int numBytes, FileHandle_t file )
{
	return fwrite(pInput, 1, numBytes, reinterpret_cast<FILE*>(file));
}

namespace sizzFile
{

	static CSizzFileSystem s_SizzFileSystem;

	CSizzFileSystem *GetSizzFileSystem()
	{
		return &s_SizzFileSystem;
	}

}