////////////////////////////////////////////////////////////////////////////////
// Filename: SizzFileSystem.cpp
////////////////////////////////////////////////////////////////////////////////
#include "SizzFileSystem.h"
#include <stdio.h>
#include <sys/stat.h>

// do i need to error check these for NULL?

using namespace sizzFile;

FileHandle_t SizzFileSystem::OpenFile( const char *pszPath, const char *pszOptions )
{
	return fopen( pszPath, pszOptions );
}

void SizzFileSystem::CloseFile( FileHandle_t file )
{
	fclose(reinterpret_cast<FILE*>(file));
}

bool SizzFileSystem::FileExists( const char *pszPath )
{
	struct stat buf;
	return (stat(pszPath, &buf) != -1);
}

bool SizzFileSystem::RemoveFile( const char *pszPath )
{
	return !remove(pszPath);
}

bool SizzFileSystem::RenameFile( const char *pszOldPath, const char *pszNewPath )
{
	return !rename(pszOldPath, pszNewPath);
}

bool SizzFileSystem::IsOk( FileHandle_t file )
{
	return !ferror(reinterpret_cast<FILE*>(file));
}

unsigned int SizzFileSystem::Write( const void *pInput, unsigned int numBytes, FileHandle_t file )
{
	return fwrite(pInput, 1, numBytes, reinterpret_cast<FILE*>(file));
}
