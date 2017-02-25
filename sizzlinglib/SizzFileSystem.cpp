
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

////////////////////////////////////////////////////////////////////////////////
// Filename: SizzFileSystem.cpp
////////////////////////////////////////////////////////////////////////////////
#include "SizzFileSystem.h"
#include "dbg.h"
#include <stdio.h>
#include <sys/stat.h>

#include <cassert>

#ifdef _WIN32
#include <direct.h>
#endif

// do i need to error check these for NULL?

using namespace sizzFile;

FileHandle_t SizzFileSystem::OpenFile( const char *pszPath, const char *pszOptions )
{
	return fopen( pszPath, pszOptions );
}

void SizzFileSystem::CloseFile( FileHandle_t file )
{
	fclose(reinterpret_cast<FILE*>(file));
	file = nullptr;
}

bool SizzFileSystem::CreateDirectory( const char* pszPath )
{
#ifdef _WIN32
    return(_mkdir(pszPath) != -1);
#else
    // -rwxr--r--
    return(mkdir(pszPath, S_IRWXU | S_IRGRP | S_IROTH) != -1);
#endif
}

bool SizzFileSystem::Exists( const char *pszPath )
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

off_t SizzFileSystem::GetFileSize( FileHandle_t file )
{
	FILE *pFile = reinterpret_cast<FILE*>(file);
	int fd = fileno(pFile);
	struct stat buf;
	int retval = fstat(fd, &buf);
	return retval == 0 ? buf.st_size : -1;
}

off_t SizzFileSystem::GetFileSize(const char *pszPath)
{
	struct stat buf;
	int retval = stat(pszPath, &buf);
	return retval == 0 ? buf.st_size : -1;
}

unsigned int SizzFileSystem::Read( void *pOutput, unsigned int numBytes, FileHandle_t file )
{
	return fread(pOutput, 1, numBytes, reinterpret_cast<FILE*>(file));
}

unsigned int SizzFileSystem::Write( const void *pInput, unsigned int numBytes, FileHandle_t file )
{
	return fwrite(pInput, 1, numBytes, reinterpret_cast<FILE*>(file));
}

void SizzFileSystem::ReadToMem( void *pMem, unsigned int maxsize, FileHandle_t file )
{
	assert(file);
	assert(pMem);
	FILE *pFile = reinterpret_cast<FILE*>(file);
	fpos_t oldPos;
	fgetpos(pFile, &oldPos);
	rewind(pFile);
	fread(pMem, 1, maxsize, pFile);
	fsetpos(pFile, &oldPos);
}