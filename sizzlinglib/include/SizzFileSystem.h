
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

// ---------------------
//
//
// ---------------------

#ifndef SIZZ_FILE_SYSTEM_H
#define SIZZ_FILE_SYSTEM_H

#include <stdio.h>
#include <sys/types.h>

namespace sizzFile
{
	typedef void *FileHandle_t;

	namespace SizzFileSystem
	{
		FileHandle_t	OpenFile( const char *pszPath, const char *pszOptions );
		void			CloseFile( FileHandle_t file );

		bool			CreateDirectory( const char* pszPath );
		bool			Exists( const char *pszPath );
		bool			RemoveFile( const char *pszPath );
		bool			RenameFile( const char *pszOldPath, const char *pszNewPath );

		bool			IsOk( FileHandle_t file );

		off_t			GetFileSize(FileHandle_t file);
		off_t			GetFileSize(const char *pszPath);

		unsigned int	Read( void *pOutput, unsigned int numBytes, FileHandle_t file );
		unsigned int	Write( const void *pInput, unsigned int numBytes, FileHandle_t file );
		void			ReadToMem( void *pMem, unsigned int maxsize, FileHandle_t file );
	}

	class CBaseFile
	{
	public:
		FileHandle_t m_FileHandle;

		CBaseFile(void)
		{
			m_FileHandle = NULL;
		}

		virtual ~CBaseFile( void )
		{
			Close();
		}

		void Close( void )
		{
			if ( m_FileHandle != NULL )
				SizzFileSystem::CloseFile( m_FileHandle );
			m_FileHandle = NULL;
		}

		void Open( char const *fname, char const *modes )
		{
			Close();
			m_FileHandle = SizzFileSystem::OpenFile( fname, modes );
		}
		/*
		char *ReadLine( char *pOutput, int maxChars )
		{
			return g_pFullFileSystem->ReadLine( pOutput, maxChars, m_FileHandle );
		}*/

		int Read( void* pOutput, int size )
		{
			return SizzFileSystem::Read( pOutput, size, m_FileHandle );
		}

		/*void MustRead( void* pOutput, int size )
		{
			int ret=Read( pOutput, size );
			if (ret != size )
				Error("failed to read %d bytes\n");
		}*/
	
		int Write( void const* pInput, int size )
		{
			return SizzFileSystem::Write( pInput, size, m_FileHandle );
		}

		/*
		// {Get|Put}{Int|Float} read and write ints and floats from a file in x86 order, swapping on
		// input for big-endian systems.
		void PutInt( int n )
		{
			int n1=LittleDWord( n );
			Write(&n1, sizeof( n1 ) );
		}

		int GetInt( void )
		{
			int ret;
			MustRead( &ret, sizeof( ret ));
			return LittleDWord( ret );
		}

		float GetFloat( void )
		{
			float ret;
			MustRead( &ret, sizeof( ret ));
			LittleFloat( &ret, &ret );
			return ret;
		}
		void PutFloat( float f )
		{
			LittleFloat( &f, &f );
			Write( &f, sizeof( f ) );
		}*/

		bool IsOk( void )
		{
			return ( m_FileHandle != NULL) &&
				( SizzFileSystem::IsOk( m_FileHandle ) );
		}
	};

	class CInputFile : public CBaseFile
	{
	public:
		void Open( char const *pFname )
		{
			CBaseFile::Open( pFname, "rb" );
		}

		CInputFile( char const *pFname ) : CBaseFile()
		{
			Open( pFname );
		}

		CInputFile( void ) : CBaseFile()
		{
		}
	};

	class COutputFile : public CBaseFile
	{
	public:
		void Open( char const *pFname )
		{
			CBaseFile::Open( pFname, "wb" );
		}

		COutputFile( char const *pFname ) : CBaseFile()
		{
			Open( pFname );
		}

		COutputFile( void ) : CBaseFile()
		{
		}
	};

}
#endif // SIZZ_FILE_SYSTEM_H