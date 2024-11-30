////////////////////////////////////////////////////////////////////////////////
// This source file is part of the ZipArchive library source distribution and
// is Copyrighted 2000 - 2007 by Artpol Software - Tadeusz Dracz
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// For the licensing details refer to the License.txt file.
//
// Web Site: http://www.artpol-software.com
////////////////////////////////////////////////////////////////////////////////

#include "_platform.h"

#ifdef ZIP_ARCHIVE_STL

#include "stdafx.h"
#include "ZipFile.h"
#include "ZipException.h"
#include "ZipPlatform.h"

#include <fcntl.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CZipFile::CZipFile()
{
	m_hFile = -1;
}

void CZipFile::ThrowError() const
{
	CZipException::Throw(errno, m_szFileName);
}


ULONGLONG CZipFile::GetLength() const
{
	// cannot use Seek here, Seek is not const
	ZIP_SIZE_TYPE lLen, lCur;
#if defined _ZIP64 && !defined __APPLE__
	lCur = _lseeki64(m_hFile, 0, current);
#else
	lCur = _lseek(m_hFile, 0, current);
#endif
	if (lCur == (ZIP_SIZE_TYPE)-1)
		ThrowError();
#if defined _ZIP64 && !defined __APPLE__
	lLen = _lseeki64(m_hFile, 0, end);
#else
	lLen = _lseek(m_hFile, 0, end);
#endif

	// first go back
#if defined _ZIP64 && !defined __APPLE__
	bool err = _lseeki64(m_hFile, lCur, begin) == -1;
#else
	bool err = _lseek(m_hFile, lCur, begin) == -1;
#endif

	if (err || lLen == (ZIP_SIZE_TYPE)-1)
		ThrowError();
	return lLen;

}


bool CZipFile::Open(LPCTSTR lpszFileName, UINT openFlags, bool bThrow)
{
	if (!IsClosed())
		Close();

#ifdef O_BINARY
	UINT iNewFlags = O_BINARY;
#else
	UINT iNewFlags = 0;
#endif

	bool bReadOnly = false;
	if (openFlags & CZipFile::modeCreate)
		iNewFlags |= O_CREAT;
	if ((openFlags & CZipFile::modeReadWrite) == CZipFile::modeReadWrite)
		iNewFlags |= O_RDWR;
	else if (openFlags & CZipFile::modeRead)
	{
		// O_RDONLY is defined as 0
		bReadOnly = true;
		iNewFlags |= O_RDONLY;
	}
	else if (openFlags & CZipFile::modeWrite)
		iNewFlags |= O_WRONLY;

	if (!(openFlags & CZipFile::modeNoTruncate) && !bReadOnly)
		iNewFlags |= O_TRUNC;
	m_hFile = ZipPlatform::OpenFile(lpszFileName, iNewFlags, openFlags & 0x1C);
	if (m_hFile == -1)
		if (bThrow)
			ThrowError();
		else
			return false;
	m_szFileName = lpszFileName;
	return true;
}


void CZipFile::SetLength(ULONGLONG uNewLen)
{
	ZipPlatform::TruncateFile(m_hFile, uNewLen);
}

ZIP_FILE_USIZE CZipFile::GetPosition() const
{
#if defined _ZIP64 && !defined __APPLE__
	#ifndef __GNUC__
		ZIP_FILE_USIZE ret = (ZIP_FILE_USIZE)_telli64(m_hFile);
	#else
		ZIP_FILE_USIZE ret = (ZIP_FILE_USIZE)lseek64(m_hFile, 0, SEEK_CUR);
	#endif		
#else
	#ifndef __GNUC__
		ZIP_FILE_USIZE ret = _tell(m_hFile);
	#else
		ZIP_FILE_USIZE ret = lseek(m_hFile, 0, SEEK_CUR);
	#endif
#endif
		if (ret == (ZIP_FILE_USIZE)-1)
			ThrowError();
		return ret;
}

ZIP_FILE_USIZE CZipFile::Seek(ZIP_FILE_SIZE dOff, int nFrom)
{
	// restricted to signed
#if defined _ZIP64 && !defined __APPLE__
	ZIP_FILE_SIZE ret = (ZIP_FILE_SIZE)_lseeki64(m_hFile, dOff, nFrom);
#else
	ZIP_FILE_SIZE ret = (ZIP_FILE_SIZE)_lseek(m_hFile, (long)dOff, nFrom);
#endif
	if (ret == -1)
		ThrowError();
	return (ZIP_FILE_USIZE)ret;
}

void  CZipFile::Flush()
{
	if (!ZipPlatform::FlushFile(m_hFile)) 
		ThrowError();
}

CZipFile::operator HANDLE()
{
	intptr_t fh = ZipPlatform::GetFileSystemHandle(m_hFile);
	if (fh == -1)
		ThrowError();
#if _MSC_VER >= 1300
	return (HANDLE)fh;
#else
	return (HANDLE)fh;
#endif
}

#endif
