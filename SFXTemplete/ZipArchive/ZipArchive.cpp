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

#include "stdafx.h"
#include "ZipArchive.h"
#include "ZipPlatform.h"
#include "ZipCompatibility.h"
#include "Wildcard.h"
#include "BytesWriter.h"

#include <time.h>

using namespace ZipArchiveLib;

const char CZipArchive::m_gszCopyright[] = {"ZipArchive Library Copyright (C) 2000 - 2007 Artpol Software - Tadeusz Dracz"};
const char CZipArchive::m_gszVersion[] = {"3.0.2"};

void CZipAddNewFileInfo::Defaults()
{
	m_iSmartLevel = CZipArchive::zipsmSafeSmart;
	m_uReplaceIndex = ZIP_FILE_INDEX_UNSPECIFIED;
	m_nBufSize = 65536;
	m_iComprLevel = -1; // default
}



CZipArchive::CZipArchive()
{
	Initialize();
}

void CZipArchive::Initialize()
{
	m_bRemoveDriveLetter = true;
	m_bAutoFlush = false;
	m_centralDir.m_pStorage= &m_storage;	
	m_centralDir.m_pCallbacks = &m_callbacks;
	m_iFileOpened = nothing;
	SetCaseSensitivity(ZipPlatform::GetSystemCaseSensitivity());
	m_uCompressionMethod = CZipCompressor::methodDeflate;
	m_iEncryptionMethod = CZipCryptograph::encStandard;	
	m_pCryptograph = NULL;
	m_pCompressor = NULL;
	m_iBufferSize = 65536;
}


CZipArchive::~CZipArchive()
{
	// 	Close(); // cannot be here: if an exception is thrown strange things can happen	
	ClearCompressor();
	ClearCryptograph();	
}

void CZipArchive::Open(LPCTSTR szPathName, int iMode, ZIP_SIZE_TYPE uVolumeSize)
{
	if (!IsClosed())
	{
		ZIPTRACE("%s(%i) : ZipArchive already opened.\n");
		return;
	}
	m_storage.Open(szPathName, iMode, uVolumeSize);
	OpenInternal(iMode);
}

void CZipArchive::Open(CZipAbstractFile& af, int iMode)
{
	if (!IsClosed())
	{
		ZIPTRACE("%s(%i) : ZipArchive is already opened.\n");
		return;
	}
	if (iMode != zipOpen && iMode != zipOpenReadOnly && iMode != zipCreate)
	{
		ZIPTRACE("%s(%i) : Mode is not supported.\n");
		return;
	}
	m_storage.Open(af, iMode);
	OpenInternal(iMode);
}


void CZipArchive::OpenInternal(int iMode)
{
	m_pszPassword.Release();
	m_iFileOpened = nothing;
	m_iArchiveSystCompatib = ZipPlatform::GetSystemID();
	m_centralDir.Init();	
	m_szRootPath.Empty();

	if ((iMode == zipOpen) ||(iMode == zipOpenReadOnly))
	{
		m_centralDir.Read();
		// if there is at least one file, get system comp. from the first one
		if (m_centralDir.IsValidIndex(0))
		{			
			int iSystemComp = m_centralDir[0]->GetSystemCompatibility();
			if (ZipCompatibility::IsPlatformSupported(iSystemComp))
				m_iArchiveSystCompatib = iSystemComp;
		}
	}
}

void CZipArchive::ThrowError(int err)
{
	CZipException::Throw(err, IsClosed() ? _T("") : (LPCTSTR)m_storage.m_pFile->GetFilePath());
}

bool CZipArchive::GetFileInfo(CZipFileHeader & fhInfo, ZIP_INDEX_TYPE uIndex) const
{
	if (IsClosed())
	{
		ZIPTRACE("%s(%i) : ZipArchive is closed.\n");
		return false;
	}
	
	if (!m_centralDir.IsValidIndex(uIndex))
		return false;
	
	fhInfo = *(m_centralDir[uIndex]);
	return true;
}

CZipFileHeader* CZipArchive::GetFileInfo(ZIP_INDEX_TYPE uIndex)
{
	if (IsClosed())
	{
		ZIPTRACE("%s(%i) : ZipArchive is closed.\n");
		return NULL;
	}
	
	if (!m_centralDir.IsValidIndex(uIndex))
		return NULL;	
	return m_centralDir[uIndex];
}

const CZipFileHeader* CZipArchive::GetFileInfo(ZIP_INDEX_TYPE uIndex) const
{
	if (IsClosed())
	{
		ZIPTRACE("%s(%i) : ZipArchive is closed.\n");
		return NULL;
	}
	
	if (!m_centralDir.IsValidIndex(uIndex))
		return NULL;
	return m_centralDir[uIndex];
}



ZIP_INDEX_TYPE CZipArchive::FindFile(LPCTSTR lpszFileName, int iCaseSensitive, bool bFileNameOnly)
{
	if (IsClosed())
	{
		ZIPTRACE("%s(%i) : ZipArchive is closed.\n");
		return ZIP_FILE_INDEX_NOT_FOUND;
	}
	bool bCS;
	bool bSporadically;
	switch (iCaseSensitive)
	{
	case ffCaseSens:
		bCS = true;
		bSporadically = true;
		break;
	case ffNoCaseSens:
		bCS = false;
		bSporadically = true;
		break;
	default:
		bCS = m_bCaseSensitive;
		bSporadically = false;
	}
	return m_centralDir.FindFile(lpszFileName, bCS, bSporadically, bFileNameOnly);
}

bool CZipArchive::OpenFile(ZIP_INDEX_TYPE uIndex)
{	
	if (!m_centralDir.IsValidIndex(uIndex))
	{
		ASSERT(FALSE);
		return false;
	}
	if (m_storage.IsSegmented() == 1)
	{
		ZIPTRACE("%s(%i) : You cannot extract from a segmented archive in creation.\n");
		return false;
	}	
	
	if (m_iFileOpened)
	{
		ZIPTRACE("%s(%i) : A file already opened.\n");
		return false;
	}
	
	InitBuffer();
	m_centralDir.OpenFile(uIndex);
	if (CurrentFile()->IsEncrypted())
	{		
		if (m_pszPassword.GetSize() == 0)
		{
			ZIPTRACE("%s(%i) : Password not set for the encrypted file.\n");
				ThrowError(CZipException::badPassword);
		}
		CreateCryptograph(CurrentFile()->m_uEncryptionMethod);
		if (!m_pCryptograph->InitDecode(m_pszPassword, *CurrentFile(), m_storage))
			ThrowError(CZipException::badPassword); 

	}
	else if (m_pszPassword.GetSize() != 0)
	{
		ZIPTRACE("%s(%i) : Password set for a not encrypted file. Ignoring password.\n");
	}

	// should be validated already in CZipFileHeader::ReadLocal (from CZipCentralDir::OpenFile)
	ASSERT(CZipCompressor::IsCompressionSupported(CurrentFile()->m_uMethod));
	
	CreateCompressor(CurrentFile()->m_uMethod);
	m_pCompressor->InitDecompression(CurrentFile(), m_pCryptograph);
	
	m_iFileOpened = extract;
	return true;
}

CZipFileHeader* CZipArchive::CurrentFile()
{
	ASSERT(m_centralDir.m_pOpenedFile);
	return m_centralDir.m_pOpenedFile;
}

DWORD CZipArchive::ReadFile(void *pBuf,     
                            DWORD uSize)
{
	if (m_iFileOpened != extract)
	{
		ZIPTRACE("%s(%i) : Current file must be opened.\n");
		return 0;
	}
	
	if (!pBuf || !uSize)
		return 0;

	return m_pCompressor->Decompress(pBuf, uSize);
}

void CZipArchive::Close(int iAfterException, bool bUpdateTimeStamp)
{
	// if after an exception - the archive may be closed, but the file may be opened
	if (IsClosed() && (!iAfterException || IsClosed(false)))
	{
		ZIPTRACE("%s(%i) : ZipArchive is already closed.\n");
		return;
	}

	if (m_iFileOpened == extract)
		CloseFile(NULL, iAfterException != afNoException);

	if (m_iFileOpened == compress)
		CloseNewFile(iAfterException != afNoException);

	if (iAfterException != afAfterException && !IsClosed(false)) // in disk spanning when user aborts 
		WriteCentralDirectory(false);  // we will flush in CZipStorage::Close

	time_t tNewestTime = 0;

	if (bUpdateTimeStamp)
	{
		ZIP_INDEX_TYPE iSize = (ZIP_INDEX_TYPE)m_centralDir.m_headers.GetSize();
		for (ZIP_INDEX_TYPE i = 0; i < iSize; i++)
		{
			time_t tFileInZipTime = m_centralDir[i]->GetTime();
			if (tFileInZipTime > tNewestTime)
				tNewestTime = tFileInZipTime;
		}
	}
	m_centralDir.Clear();
	m_centralDir.m_stringSettings.Reset();
	CZipString szFileName = m_storage.Close(iAfterException == afAfterException);
	if (bUpdateTimeStamp && !szFileName.IsEmpty())
		ZipPlatform::SetFileModTime(szFileName, tNewestTime);
}

void CZipArchive::WriteCentralDirectory(bool bFlush)
{
	m_centralDir.Write();
	if (bFlush)
		m_storage.Flush();
}

void CZipArchive::SetAdvanced(int iWriteBuffer, int iGeneralBuffer, int iSearchBuffer)
{
	if (!IsClosed())
	{
		ZIPTRACE("%s(%i) : Set this options before opening the archive.\n");
		return;
	}
	
	m_storage.m_iWriteBufferSize = iWriteBuffer < 1024 ? 1024 : iWriteBuffer;
	m_iBufferSize = iGeneralBuffer < 1024 ? 1024 : iGeneralBuffer;
	m_storage.m_iLocateBufferSize = iSearchBuffer < 1024 ? 1024 : iSearchBuffer;
}

int CZipArchive::CloseFile(CZipFile &file)
{
	CZipString temp = file.GetFilePath();
	file.Close();
	return CloseFile(temp);
}

int CZipArchive::CloseFile(LPCTSTR lpszFilePath, bool bAfterException)
{
	if (m_iFileOpened != extract)
	{
		ZIPTRACE("%s(%i) : No opened file.\n");
		return 0;
	}

	int iRet = 1;
	if (bAfterException)
		m_pCompressor->FinishDecompression(true);
	else
	{
		if (m_pCompressor->m_uUncomprLeft == 0)
		{
			if (m_centralDir.IsConsistencyCheckOn(checkCRC)
				&& !CurrentFile()->m_bIgnoreCrc32
				&& m_pCompressor->m_uCrc32 != CurrentFile()->m_uCrc32)
					ThrowError(CZipException::badCrc);
		}
		else
			iRet = -1;

		m_pCompressor->FinishDecompression(false);
		
		if (lpszFilePath)
		{			
			if (!ZipPlatform::SetFileModTime(lpszFilePath, CurrentFile()->GetTime())
				||!ZipPlatform::SetFileAttr(lpszFilePath, CurrentFile()->GetSystemAttr()))
					iRet = -2;
		}
		if (m_pCryptograph)
			m_pCryptograph->FinishDecode(*CurrentFile(), m_storage);
	}

	m_centralDir.CloseFile(bAfterException);
		
	ReleaseBuffer();
	m_iFileOpened = nothing;	
	ClearCryptograph();	
	return iRet;
}

bool CZipArchive::OpenNewFile(CZipFileHeader & header, int iLevel, LPCTSTR lpszFilePath,
							  ZIP_INDEX_TYPE uReplaceIndex)
{
	if (IsClosed())
	{
		ZIPTRACE("%s(%i) : ZipArchive is closed.\n");
		return false;
	}
	
	if (m_iFileOpened)
	{
		ZIPTRACE("%s(%i) : A file already opened.\n");
		return false;
	}
	
	if (m_storage.IsSegmented() == -1)
	{
		ZIPTRACE("%s(%i) : You cannot add files to an existing segmented archive.\n");
		return false;
	}

	if (GetCount() ==(WORD)USHRT_MAX)
	{
		ZIPTRACE("%s(%i) : Maximum file count inside archive reached.\n");
		return false;
	}
	
	DWORD uAttr = 0;
	time_t ttime;
	if (lpszFilePath)
	{
		if (!ZipPlatform::GetFileAttr(lpszFilePath, uAttr))
			// do not continue - if the file was a directory then not recognizing it will cause 
			// serious errors (need uAttr to recognize it)
			return false;
		ZipPlatform::GetFileModTime(lpszFilePath, ttime);
	}

	InitBuffer();
	
	
	if (lpszFilePath)
	{
		header.SetTime(ttime);
		SetFileHeaderAttr(header, uAttr); // set system compatibility as well
	}
	else
	{
		header.SetSystemCompatibility(m_iArchiveSystCompatib);
		if (!header.HasTime())
			header.SetTime(time(NULL));
	}

	CZipString szFileName = header.GetFileName();
	

	bool bIsDirectory = header.IsDirectory();
	if (bIsDirectory)
	{
		int iNameLen = szFileName.GetLength();		
		if (!iNameLen || !CZipPathComponent::IsSeparator(szFileName[iNameLen-1]))
		{
			szFileName += CZipPathComponent::m_cSeparator;
			header.SetFileName(szFileName);
		}
	}

	if (szFileName.IsEmpty())
	{
		szFileName.Format(_T("file%u"), GetCount());
		header.SetFileName(szFileName);
	}

	bool bEncrypted = EncryptNextFile();

#if defined _DEBUG && !defined NOZIPTRACE
	if (bIsDirectory && bEncrypted)
		ZIPTRACE("%s(%i) : Encrypting a directory. You may want to consider clearing the password before adding a directory.\n");
#endif		
	
	bool bReplace = uReplaceIndex != ZIP_FILE_INDEX_UNSPECIFIED;

	if (iLevel < -1 || iLevel > 9)
		iLevel = -1;
	
	if (bEncrypted)
	{
		header.m_uEncryptionMethod = (BYTE)m_iEncryptionMethod;
		CreateCryptograph(m_iEncryptionMethod);
	}
	else
		header.m_uEncryptionMethod = CZipCryptograph::encNone;

	if (iLevel == 0 || bIsDirectory)
		header.m_uMethod = CZipCompressor::methodStore;
	else
		header.m_uMethod = m_uCompressionMethod;

	CreateCompressor(header.m_uMethod);	
	CZipFileHeader* pHeader = m_centralDir.AddNewFile(header, uReplaceIndex, iLevel);	

	// replace can happen only from AddNewFile and the compressed size is already known and set (the file is stored, not compressed)
	if (bReplace)
	{
		// this will be used in GetLocalSize and WriteLocal
		pHeader->PrepareFileName();
		// we use the local size, because the real does not exist yet
		ZIP_SIZE_TYPE uFileSize = pHeader->GetDataSize(true, false) + pHeader->GetLocalSize(false) + pHeader->GetDataDescriptorSize(&m_storage);
		MakeSpaceForReplace(uReplaceIndex, uFileSize, szFileName);
	}

	CurrentFile()->WriteLocal(&m_storage);

	if (m_pCryptograph)
		m_pCryptograph->InitEncode(m_pszPassword, *pHeader, m_storage);

	m_pCompressor->InitCompression(iLevel, CurrentFile(), m_pCryptograph);
		
	m_iFileOpened = compress;
	return true;
}


bool CZipArchive::ExtractFile(ZIP_INDEX_TYPE uIndex,                  
                              LPCTSTR lpszPath,             
                              bool bFullPath,              
                              LPCTSTR lpszNewName,          
                              DWORD nBufSize)
{
	
	if (!nBufSize && !lpszPath)
		return false;
	
	CZipFileHeader* pHeader = (*this)[uIndex];	
	CZipString szFileNameInZip = pHeader->GetFileName();
	CZipString szFile = PredictExtractedFileName(szFileNameInZip, lpszPath, bFullPath, lpszNewName);
	CZipActionCallback* pCallback = GetCallback(CZipActionCallback::cbExtract);
	if (pCallback)
		pCallback->Init(szFileNameInZip, szFile);
	

	if (pHeader->IsDirectory())
	{
		if (pCallback)
			pCallback->SetTotal(0); // in case of calling LeftToProcess() afterwards

		ZipPlatform::ForceDirectory(szFile);
		ZipPlatform::SetFileAttr(szFile, pHeader->GetSystemAttr());

		if (pCallback)
			pCallback->CallbackEnd();
		return true;
	}
	else
	{
		if (pCallback)
			pCallback->SetTotal(pHeader->m_uUncomprSize);

		if (!OpenFile(uIndex))
			return false;

		CZipPathComponent zpc(szFile);
		ZipPlatform::ForceDirectory(zpc.GetFilePath());
		CZipFile f(szFile, CZipFile::modeWrite | 
			CZipFile::modeCreate | CZipFile::shareDenyWrite);
		DWORD iRead;
		CZipAutoBuffer buf(nBufSize);
		int iAborted = 0;
		for(;;)
		{
			iRead = ReadFile(buf, buf.GetSize());
			if (!iRead)
			{
				if (pCallback && !pCallback->RequestLastCallback())
					iAborted = CZipException::abortedSafely;
				break;
			}
			f.Write(buf, iRead);
			if (pCallback && !pCallback->RequestCallback(iRead))
			{
				if (iRead == buf.GetSize() && ReadFile(buf, 1) != 0) // test one byte if there is something left
					iAborted = CZipException::abortedAction; 
				else
					iAborted = CZipException::abortedSafely;
				break;
			}
		}

		if (pCallback)
		{
			if (!iAborted)
			{
				bool bRet = CloseFile(f) == 1;
				pCallback->CallbackEnd();
				return bRet;
			}
			else
			{
				if (iAborted == CZipException::abortedAction)
					CloseFile(NULL, true);
				else
				{
					bool bRet;
					try
					{
						bRet = CloseFile(f) == 1;
					}
					// if any exception was thrown, then we are not successful
					// catch all exceptions to throw aborted exception only
#ifdef ZIP_ARCHIVE_MFC
					catch(CException* e)
					{
						e->Delete();
						bRet = false;
					}
#endif
					catch(...)
					{
						bRet = false;
					}
					if (!bRet)
					{
						CloseFile(NULL, true);
						iAborted = CZipException::abortedAction;
					}
				}

				pCallback->CallbackEnd();
				CZipException::Throw(iAborted, szFile);
				return false; // for the compiler
			}
		}
		else
			return CloseFile(f) == 1;		
	}	
}

bool CZipArchive::ExtractFile(ZIP_INDEX_TYPE uIndex,                  
                              CZipMemFile& mf,
							  bool bRewind,
                              DWORD nBufSize)
{
	if (!nBufSize)
		return false;
	
	CZipFileHeader* pHeader = (*this)[uIndex];
	CZipActionCallback* pCallback = GetCallback(CZipActionCallback::cbExtract);
	if (pCallback)
	{
		pCallback->Init(pHeader->GetFileName());
		pCallback->SetTotal(pHeader->m_uUncomprSize);
	}

	if (pHeader->IsDirectory() || !OpenFile(uIndex))
		return false;

	CZipAutoBuffer buf(nBufSize);
	//mf.SeekToEnd();
	ZIP_FILE_USIZE oldPos = 0;

	if (bRewind)
		oldPos = mf.GetPosition();

	DWORD iRead;
	int iAborted = 0;
	for(;;)
	{
		iRead = ReadFile(buf, buf.GetSize());
		if (!iRead)
		{
			if (pCallback && !pCallback->RequestLastCallback())
				iAborted = CZipException::abortedSafely;
			break;
		}
		mf.Write(buf, iRead);
		if (pCallback && !pCallback->RequestCallback(iRead))
		{
			if (iRead == buf.GetSize() && ReadFile(buf, 1) != 0) // test one byte if there is something left
				iAborted = CZipException::abortedAction; 
			else
				iAborted = CZipException::abortedSafely; // we did it!
			break;
		}
	}

	if (pCallback)
	{
		if (!iAborted)
		{
			bool bRet = CloseFile() == 1;
			pCallback->CallbackEnd();
			return bRet;
		}
		else
		{
			if (iAborted == CZipException::abortedAction)
				CloseFile(NULL, true);
			else
			{
				bool bRet;
				try
				{
					bRet = CloseFile() == 1;
				}
				// if any exception was thrown, then we are not successful
				// catch all exceptions to thrown aborted exception only
	#ifdef ZIP_ARCHIVE_MFC
				catch(CException* e)
				{
					e->Delete();
					bRet = false;
				}
	#endif
				catch(...)
				{
					bRet = false;
				}
				if (!bRet)
				{
					CloseFile(NULL, true);
					iAborted = CZipException::abortedAction;
				}
			}
			
			pCallback->CallbackEnd();
			CZipException::Throw(iAborted);
			return false; // for the compiler
		}
	}
	else
		return CloseFile() == 1;
}


bool CZipArchive::WriteNewFile(const void *pBuf, DWORD uSize)
{
	if (m_iFileOpened != compress)
	{
		ZIPTRACE("%s(%i) : A new file must be opened.\n");
		return false;
	}
	
	m_pCompressor->Compress(pBuf, uSize);	
	return true;
}

bool CZipArchive::CloseNewFile(bool bAfterException)
{
	if (m_iFileOpened != compress)
	{
		ZIPTRACE("%s(%i) : A new file must be opened.\n");
		return false;
	}
	    
	m_pCompressor->FinishCompression(bAfterException);
    if (bAfterException)
		m_centralDir.m_pOpenedFile = NULL;
	else
	{
		if (m_pCryptograph)
			m_pCryptograph->FinishEncode(*CurrentFile(), m_storage);
		
		m_centralDir.CloseNewFile();
	}	
	m_iFileOpened = nothing;
	ReleaseBuffer();	
	ClearCryptograph();
	if (m_bAutoFlush && !bAfterException)
		Flush();

	return true;
}

bool CZipArchive::RemoveFile(ZIP_INDEX_TYPE uIndex)
{
	CZipIndexesArray indexes;
	indexes.Add(uIndex);
	return RemoveFiles(indexes);
}

void CZipArchive::GetIndexes(const CZipStringArray &aNames, CZipIndexesArray& aIndexes)
{
	if (IsClosed())
	{
		ZIPTRACE("%s(%i) : ZipArchive is closed.\n");
		return;
	}
	ZIP_INDEX_TYPE uSize = (ZIP_INDEX_TYPE)aNames.GetSize();
	for (ZIP_INDEX_TYPE i = 0; i < uSize; i++)
		aIndexes.Add(FindFile(aNames[(ZIP_ARRAY_SIZE_TYPE)i], ffDefault, false));			
}

bool CZipArchive::RemoveFiles(const CZipStringArray &aNames)
{
	CZipIndexesArray indexes;
	GetIndexes(aNames, indexes);
	return RemoveFiles(indexes);
}

bool CZipArchive::RemoveFiles(CZipIndexesArray &aIndexes)
{
	if (IsClosed())
	{
		ZIPTRACE("%s(%i) : ZipArchive is closed.\n");
		return false;
	}
	
	if (m_storage.IsSegmented())
	{
		ZIPTRACE("%s(%i) : You cannot delete files from a segmented archive.\n");
		return false;
	}
	
	if (m_iFileOpened)
	{
		ZIPTRACE("%s(%i) : You cannot delete files if there is a file opened.\n");
		return false;
	}

	if (GetCount() == 0)
	{
		ZIPTRACE("%s(%i) : There is nothing to delete: the archive is empty.\n");
		return false;
	}
	
	CZipActionCallback* pCallback = GetCallback(CZipActionCallback::cbDeleteCnt);
	if (pCallback)
		pCallback->Init();

	ZIP_INDEX_TYPE uSize = (ZIP_INDEX_TYPE)aIndexes.GetSize();
	if (!uSize)
	{
		ZIPTRACE("%s(%i) : The indexes array is empty.\n");
		return true;
	}

	ZIP_INDEX_TYPE i;
	
	aIndexes.Sort(true);
	// remove all - that's easy so don't waste the time
	if (uSize == GetCount())
	{		
		// check that the indexes are correct
		bool allIncluded = true;
		// iterate all indexes, if all are sorted then the condition should always be true
		for (i = 0; i < uSize; i++)
			if (aIndexes[(ZIP_ARRAY_SIZE_TYPE)i] != i)
			{
				allIncluded = false;
				break;
			}

		if (allIncluded)
		{
			pCallback = GetCallback(CZipActionCallback::cbDelete);
			if (pCallback)
			{
				// do it right and sent the notification
				pCallback->Init();
				pCallback->SetTotal(uSize);
			}

			m_centralDir.RemoveFromDisk();
			m_storage.m_pFile->SetLength((ZIP_FILE_USIZE) m_storage.m_uBytesBeforeZip);
			m_centralDir.RemoveAll();
			if (m_bAutoFlush)
				Flush();
			if (pCallback)
				pCallback->CallbackEnd();
			return true;
		}
	}
	else
	{
		for (i = 0; i < uSize; i++)
			if (!m_centralDir.IsValidIndex(aIndexes[(ZIP_ARRAY_SIZE_TYPE)i]))
				return false;
	}

	CZipArray<CZipDeleteInfo> aInfo;
	
	if (pCallback)
		pCallback->SetTotal(GetCount());
	
	ZIP_INDEX_TYPE uDelIndex = 0;
	ZIP_INDEX_TYPE uMaxDelIndex = aIndexes[(ZIP_ARRAY_SIZE_TYPE)(uSize - 1)];
	i = aIndexes[0];
	// GetCount() is greater than 0 (checked before) and when it is unsigned we do not cause overflow
	ZIP_INDEX_TYPE uLastPosition = (ZIP_INDEX_TYPE)(GetCount() - 1);
	bool bAborted = false;
	if (i <= uLastPosition) 
		for(;;)
		{
			CZipFileHeader* pHeader = m_centralDir[i];
			bool bDelete;
			if (i <= uMaxDelIndex && i == aIndexes[(ZIP_ARRAY_SIZE_TYPE)uDelIndex])
			{
				uDelIndex++;
				bDelete = true;
			}
			else
				bDelete = false;
			aInfo.Add(CZipDeleteInfo(pHeader, bDelete));
			if (i == uLastPosition)
			{
				if (pCallback && !pCallback->RequestLastCallback(1))
					bAborted = true;
				break;
			}
			else
			{				
				if (pCallback && !pCallback->RequestCallback())
				{
					bAborted = true;
					break;
				}
				i++;
			}
		}

	ASSERT(uDelIndex == uSize);

	if (pCallback)
	{
		pCallback->CallbackEnd();
		if (bAborted)
			ThrowError(CZipException::abortedSafely);
	}

	uSize = (ZIP_INDEX_TYPE)aInfo.GetSize();
	if (!uSize) // it is possible
		return true;

	// they should already be sorted after reading the in CZipCentralDir::ReadHeaders and when replacing, the index is placed at the same place as the old one
	//aInfo.Sort(true); // sort by offsets (when operators uncommented in CZipDeleteInfo)
	
	// now we start deleting (not safe to break)
	pCallback = GetCallback(CZipActionCallback::cbDelete);
	if (pCallback)
		pCallback->Init();
	
	m_centralDir.RemoveFromDisk();

	ZIP_SIZE_TYPE uTotalToMoveBytes = 0, uLastOffset = (ZIP_SIZE_TYPE)m_storage.m_pFile->GetLength() - m_storage.m_uBytesBeforeZip;
	// count the number of bytes to move
	// uSize is greated than 0 (checked before) - no overflow
	i = (ZIP_INDEX_TYPE)(uSize - 1);
	for(;;)
	{
		// we alread know it is not 0
		// cannot use a decreasing loop because i is unsigned and instead negative at the end of the loop it will be maximum positive
		const CZipDeleteInfo& di = aInfo[(ZIP_ARRAY_SIZE_TYPE)i];
		if (!di.m_bDelete)
			uTotalToMoveBytes += uLastOffset - di.m_pHeader->m_uOffset;
		uLastOffset = di.m_pHeader->m_uOffset;
		if (i == 0)
			break;
		i--;
	}
	
	if (pCallback)
		pCallback->SetTotal(uTotalToMoveBytes);

	
	InitBuffer();

	ZIP_SIZE_TYPE uMoveBy = 0, uOffsetStart = 0;
	for (i = 0; i < uSize; i++)
	{
		const CZipDeleteInfo& di = aInfo[(ZIP_ARRAY_SIZE_TYPE)i];
		
		if (di.m_bDelete)
		{
			// next hole
			ZIP_SIZE_TYPE uTemp = di.m_pHeader->m_uOffset;
			m_centralDir.RemoveFile(di.m_pHeader); // first remove
			if (uOffsetStart)
			{
				// copy the files over a previous holes
				MovePackedFiles(uOffsetStart, uTemp, uMoveBy, pCallback, false, false);
				uOffsetStart = 0;  // never be at the beginning, because the first file is always to be deleted
			}
			if (i == uSize - 1)
				uTemp = ((ZIP_SIZE_TYPE)m_storage.m_pFile->GetLength() - m_storage.m_uBytesBeforeZip) - uTemp;
			else
				uTemp = aInfo[(ZIP_ARRAY_SIZE_TYPE)(i + 1)].m_pHeader->m_uOffset - uTemp;

			uMoveBy += uTemp;
			
		}
		else
		{
			if (uOffsetStart == 0) // find contiuos area to move
				uOffsetStart = di.m_pHeader->m_uOffset;
			di.m_pHeader->m_uOffset -= uMoveBy;
		}

	}
	if (uOffsetStart)
	{
		// will call the last callback, if necessary
		MovePackedFiles(uOffsetStart,
			(ZIP_SIZE_TYPE)m_storage.m_pFile->GetLength() - m_storage.m_uBytesBeforeZip,
			uMoveBy, pCallback); 
	}
	else
	{
		// call last callback (it was not called in the MovePackedFiles calls in the loop)
		if (pCallback && !pCallback->RequestLastCallback())
		{
			pCallback->CallbackEnd();
			ThrowError(CZipException::abortedAction);
		}
	}

	ReleaseBuffer();
	if (uMoveBy) // just in case
		m_storage.m_pFile->SetLength((ZIP_FILE_USIZE)(m_storage.m_pFile->GetLength() - uMoveBy));

	if (pCallback)
		pCallback->CallbackEnd();

	if (m_bAutoFlush)
		Flush();
	return true;
}



bool CZipArchive::AddNewFile(LPCTSTR lpszFilePath,
                             int iComprLevel,          
                             bool bFullPath,
							 int iSmartLevel,
                             unsigned long nBufSize)
{
	
	CZipAddNewFileInfo zanfi (lpszFilePath, bFullPath);
	zanfi.m_iComprLevel = iComprLevel;
	zanfi.m_iSmartLevel = iSmartLevel;
	zanfi.m_nBufSize = nBufSize;
	return AddNewFile(zanfi);	
}

bool CZipArchive::AddNewFile(LPCTSTR lpszFilePath,
							 LPCTSTR lpszFileNameInZip,
                             int iComprLevel,                                       
							 int iSmartLevel,
                             unsigned long nBufSize)
{
	CZipAddNewFileInfo zanfi(lpszFilePath, lpszFileNameInZip);
	zanfi.m_iComprLevel = iComprLevel;
	zanfi.m_iSmartLevel = iSmartLevel;
	zanfi.m_nBufSize = nBufSize;
	return AddNewFile(zanfi);	
}

bool CZipArchive::AddNewFile(CZipMemFile& mf,
							 LPCTSTR lpszFileNameInZip,
                             int iComprLevel,                                       
							 int iSmartLevel,
                             unsigned long nBufSize)
{
	CZipAddNewFileInfo zanfi(&mf, lpszFileNameInZip);
	zanfi.m_iComprLevel = iComprLevel;
	zanfi.m_iSmartLevel = iSmartLevel;
	zanfi.m_nBufSize = nBufSize;
	return AddNewFile(zanfi);
}

/**
	A structure for the internal use only. Clears the password if necessary and
	restores it later (also in case of an exception).
*/
struct CZipSmClrPass
{
	CZipSmClrPass()
	{
		m_pZip = NULL;
	}

	void ClearPasswordSmartly(CZipArchive* pZip)
	{
		m_pZip = pZip;
		m_szPass = pZip->GetPassword();
		if (!m_szPass.IsEmpty())
			pZip->SetPassword();
	}

	~CZipSmClrPass()
	{
		if (!m_szPass.IsEmpty())
			m_pZip->SetPassword(m_szPass);
	}
private:
	CZipString m_szPass;
	CZipArchive* m_pZip;
};

bool CZipArchive::AddNewFile(CZipAddNewFileInfo& info)
{
	// no need for ASSERT and TRACE here - it will be done by OpenNewFile
	
	if (!m_iBufferSize)
		return false;

	if (info.m_pFile)
		info.m_szFilePath = info.m_pFile->GetFilePath();
	else
	{
		CZipPathComponent::RemoveSeparators(info.m_szFilePath);
		if (info.m_szFilePath.IsEmpty())
			return false;	
	}

	bool bSegm = GetSegmMode() != 0;

	// checking the replace index
	if (!UpdateReplaceIndex(info.m_uReplaceIndex))
		return false;

	bool bReplace = info.m_uReplaceIndex != ZIP_FILE_INDEX_UNSPECIFIED;
	
	DWORD uAttr;
	time_t ttime;
	if (info.m_pFile)
	{
		uAttr = ZipPlatform::GetDefaultAttributes();
		ttime = time(NULL);
	}
	else
	{
		if (!ZipPlatform::GetFileAttr(info.m_szFilePath, uAttr))
			return false; // we don't know whether it is a file or a directory
		ZipPlatform::GetFileModTime(info.m_szFilePath, ttime);
	}
	CZipFileHeader header;
	
	if (ZipPlatform::GetSystemID() != ZipCompatibility::zcUnix)
		uAttr |= ZipCompatibility::ConvertToSystem(uAttr, ZipPlatform::GetSystemID(), ZipCompatibility::zcUnix);  // make it readable under Unix as well, since it stores its attributes in HIWORD(uAttr)
	SetFileHeaderAttr(header, uAttr);
	if (info.m_szFileNameInZip.IsEmpty())
		info.m_szFileNameInZip = PredictFileNameInZip(info.m_szFilePath, info.m_bFullPath, header.IsDirectory() ? prDir : prFile);
	header.SetFileName(info.m_szFileNameInZip);
	header.SetTime(ttime);
	bool bInternal = (info.m_iSmartLevel & zipsmInternal01) != 0;
	
	if (header.IsDirectory()) // will never be when m_pFile is not NULL, so we don't check it
	{
		ASSERT(!info.m_pFile); // should never happened
		ASSERT(!bInternal);

		CZipActionCallback* pCallback = GetCallback(CZipActionCallback::cbAdd); 

		if (pCallback)
		{
			pCallback->Init(info.m_szFileNameInZip, info.m_szFilePath);
			pCallback->SetTotal(0); // in case of calling LeftToProcess() afterwards		
		}

		// clear password for a directory
		CZipSmClrPass smcp;
		if (info.m_iSmartLevel & zipsmCPassDir)
			smcp.ClearPasswordSmartly(this);

		bool bRet = OpenNewFile(header, 0, NULL, info.m_uReplaceIndex);
		
		CloseNewFile();
		if (pCallback)
			pCallback->CallbackEnd();
		
		return bRet;		
	}
	
	CZipSmClrPass smcp;
	bool bIsCompression = info.m_iComprLevel != 0;
	bool bEff = (info.m_iSmartLevel & zipsmCheckForEff)&& bIsCompression;
	bool bCheckForZeroSized = (info.m_iSmartLevel & zipsmCPFile0) && EncryptNextFile();
	bool bCheckForSmallFiles = (info.m_iSmartLevel & zipsmNotCompSmall) && bIsCompression;
	ZIP_SIZE_TYPE uFileSize = ZIP_SIZE_TYPE(-1);
	bool bNeedTempArchive = (bEff && bSegm) || (bReplace && bIsCompression);
	if (bCheckForSmallFiles || bCheckForZeroSized || bNeedTempArchive)
	{
		
		if (info.m_pFile)
			uFileSize = (ZIP_SIZE_TYPE)info.m_pFile->GetLength();
		else
		{
			if (!ZipPlatform::GetFileSize(info.m_szFilePath, uFileSize) && bEff)
				bEff = false; // the file size is needed only when efficient in a segmented archive
		}

		if (uFileSize !=  ZIP_SIZE_TYPE(-1))
		{
			if (bCheckForZeroSized && uFileSize == 0)
				smcp.ClearPasswordSmartly(this);			
			if (bCheckForSmallFiles && uFileSize < 5)
				info.m_iComprLevel = 0;			
		}
	}
	bool bEffInMem = bEff && (info.m_iSmartLevel & zipsmMemoryFlag);
	CZipString szTempFileName;
	if (bNeedTempArchive && (bEffInMem || 
		!(szTempFileName = ZipPlatform::GetTmpFileName(
			m_szTempPath.IsEmpty() ? NULL : (LPCTSTR)m_szTempPath, uFileSize)
		).IsEmpty()))
	{
		CZipMemFile* pmf = NULL;
		CZipArchive zip;
		try
		{
			// compress first to a temporary file, if ok - copy the data, if not - add storing
			
			if (bEffInMem)
			{
				pmf = new CZipMemFile;
				zip.Open(*pmf, zipCreate);
			}
			else
				zip.Open(szTempFileName, zipCreate);
			zip.SetRootPath(m_szRootPath);
			zip.SetPassword(GetPassword());
			zip.SetEncryptionMethod(m_iEncryptionMethod);
			zip.SetSystemCompatibility(m_iArchiveSystCompatib);
			zip.SetCallback(GetCallback(CZipActionCallback::cbAdd), CZipActionCallback::cbAdd);
			// create a temporary file
			ZIP_INDEX_TYPE uTempReplaceIndex = info.m_uReplaceIndex;
			info.m_iSmartLevel = zipsmLazy;
			info.m_uReplaceIndex = ZIP_FILE_INDEX_UNSPECIFIED;
			if (!zip.AddNewFile(info))
				throw false;
			info.m_uReplaceIndex = uTempReplaceIndex;

			// this may also happen when bReplace, but not in a segmented archive
			if (bEff)
			{
				if (!zip[0]->CompressionEfficient())
				{
					info.m_iComprLevel = 0;
					info.m_iSmartLevel = zipsmInternal01;
					// compression is not efficient, store instead
					throw AddNewFile(info);
				}
			}
			zip.m_storage.Flush();
			InitBuffer();
			throw GetFromArchive(zip, 0, NULL, info.m_uReplaceIndex, true, GetCallback(CZipActionCallback::cbAddTmp));
		}
		catch (bool bRet)
		{
			zip.Close(!bRet); // that doesn't really matter how it will be closed
			if (pmf)
				delete pmf;
			if (!bEffInMem)
				ZipPlatform::RemoveFile(szTempFileName, false);
			ReleaseBuffer();
			return bRet;
		}
		catch (...)
		{
			zip.Close(true);
			if (pmf)
				delete pmf;
			if (!bEffInMem)
				ZipPlatform::RemoveFile(szTempFileName, false);
			ReleaseBuffer();
			throw;
		}
	}

	// try to open before adding
	CZipFile f;
	CZipAbstractFile *pf;
	if (info.m_pFile)
	{
		pf = info.m_pFile;
		pf->SeekToBegin();
	}
	else
	{
		// cannot be shareDenyWrite		
		// If you specify the GENERIC_READ and GENERIC_WRITE access modes along with the FILE_SHARE_READ and FILE_SHARE_WRITE sharing modes in your first call to CreateFile. If you specify the GENERIC_READ and GENERIC_WRITE access modes and the FILE_SHARE_READ sharing mode only in your second call to CreateFile, the function will fail with a sharing violation because the read-only sharing mode specified in the second call conflicts with the read/write access that has been granted in the first call.
		// Original information was here (but not any longer): http://msdn.microsoft.com/library/default.asp?url=/library/en-us/fileio/base/creating_and_opening_files.asp
		if (!f.Open(info.m_szFilePath, CZipFile::modeRead | CZipFile::shareDenyNone, false))
			return false;
		pf = &f;
	}

	ASSERT(pf);
	// call init before opening (in case of exception we have the names)
	uFileSize = (ZIP_SIZE_TYPE)pf->GetLength();
	
	// predict sizes in local header, so that zip64 can write extra header if needed
	header.m_uLocalUncomprSize = uFileSize;	
	if (!bIsCompression)
		header.m_uLocalComprSize = uFileSize;

	bool bRet;	
	if (bReplace)
	{
		ASSERT(!bIsCompression);		
		bRet = OpenNewFile(header, 0, NULL, info.m_uReplaceIndex);
	}
	else
		bRet = OpenNewFile(header, info.m_iComprLevel);
	if (!bRet)
		return false;

	// we do it here, because if in OpenNewFile is replacing 
	// then we get called cbReplace callback before and it would 
	// overwrite callback information written in pCallback->Init()
	CZipActionCallback* pCallback =  GetCallback(bInternal ? CZipActionCallback::cbAddStore : CZipActionCallback::cbAdd);

	if (pCallback)
	{
		// Init cbAdd here as well - after smart add - to avoid double initiation when
		// temporary archive is used - it would init cbAdd again
		pCallback->Init(info.m_szFileNameInZip, info.m_szFilePath);
		pCallback->SetTotal(uFileSize);
	}

	CZipAutoBuffer buf(info.m_nBufSize);
	DWORD iRead;
	int iAborted = 0;
	do
	{
		iRead = pf->Read(buf, info.m_nBufSize);
		if (iRead)
		{
			WriteNewFile(buf, iRead);
			if (pCallback && !pCallback->RequestCallback(iRead))
			{
				if (iRead == buf.GetSize() && pf->Read(buf, 1) != 0) // test one byte if there is something left
				{						
					if (!m_storage.IsSegmented() && !bReplace)
					{
						RemoveLast(true);														
						iAborted = CZipException::abortedSafely;
					}
					else
						iAborted = CZipException::abortedAction;
					// close new file with bException set to true, even if abortedSafely, 
					// because in that case we have removed the last file - there is nothing to close
					CloseNewFile(true);
				}
				else
					// temporary value - possible safe abort
					iAborted = CZipException::aborted;				
				break;
			}				
		}		
	}
	while (iRead == buf.GetSize());

	if (pCallback)
	{
		if (!iAborted && !pCallback->RequestLastCallback())	  
			// temporaty value - safe abort
			iAborted = CZipException::aborted;

		if (!iAborted)
		{
			CloseNewFile();
			pCallback->CallbackEnd();
		}
		else
		{
			// possible safe abort
			if (iAborted == CZipException::aborted)
			{
				bool bRet;
				try
				{
					bRet = CloseNewFile();
				}
#ifdef ZIP_ARCHIVE_MFC
				catch(CException* e)
				{
					e->Delete();
					bRet = false;
				}
#endif
				catch(...)
				{
					bRet = false;
				}
				if (bRet)
					iAborted = CZipException::abortedSafely;
				else
				{
					CloseNewFile(true);
					iAborted = CZipException::abortedAction;
				}
			}
			pCallback->CallbackEnd();	
			CZipException::Throw(iAborted); // throw to distinguish from other return codes
		}
	}
	else
		CloseNewFile();
	
	if (bEff)
	{
		// remove the last file and add it without the compression if needed
		if (!info.m_pFile)
			f.Close();

		buf.Release();
		if (RemoveLast())
		{
			info.m_iComprLevel = 0;
			info.m_iSmartLevel = zipsmInternal01;
			return AddNewFile(info);
		}
	}
	return true;		
}

bool CZipArchive::RemoveLast(bool bRemoveAnyway)
{
	if (GetCount() == 0)
		return false;
	ZIP_INDEX_TYPE uIndex = (ZIP_INDEX_TYPE)(GetCount() - 1);
	CZipFileHeader* pHeader = m_centralDir[uIndex];

	if (!bRemoveAnyway && pHeader->CompressionEfficient())
		return false;

	m_centralDir.RemoveLastFile(pHeader, uIndex);
	return true;
}

class CZipRootPathRestorer
{
	CZipString m_szOldRootPath;
	CZipArchive* m_pZip;
public:	
	CZipRootPathRestorer()
	{
		m_pZip = NULL;
	}
	void SetNewRootPath(CZipArchive* pZip, LPCTSTR lpszNewRoot)
	{
		m_pZip = pZip;
		m_szOldRootPath = m_pZip->GetRootPath();
		m_pZip->SetRootPath(lpszNewRoot);
	}
	~CZipRootPathRestorer()
	{
		if (m_pZip)
			m_pZip->SetRootPath(m_szOldRootPath);
	}
};

class CCalculateAddFilesEnumerator : public ZipArchiveLib::CDirEnumerator
{
	CZipActionCallback* m_pCallback;
public:		
	ZIP_FILE_USIZE m_uTotalBytes;
	ZIP_FILE_USIZE m_uTotalFiles;
	CCalculateAddFilesEnumerator(LPCTSTR lpszDirectory, bool bRecursive, CZipActionCallback* pCallback)
		:ZipArchiveLib::CDirEnumerator(lpszDirectory, bRecursive)
	{
		m_pCallback = pCallback;
		m_uTotalFiles = m_uTotalBytes = 0;
	}
protected:
	void OnEnumerationBegin()
	{
		if (m_pCallback)
			m_pCallback->Init();
	}

	bool Process(LPCTSTR, const ZipArchiveLib::CFileInfo& info)
	{
		m_uTotalFiles++;
		m_uTotalBytes += info.m_uSize;
		if (m_pCallback && !m_pCallback->RequestCallback())
			return false;
		else
			return true;
	}

	void OnEnumerationEnd(bool bResult)
	{
		if (m_pCallback)
		{
			if (bResult)
				bResult = m_pCallback->RequestLastCallback();
			m_pCallback->CallbackEnd();
			// can be false only, if the callback returns false
			if (!bResult)
				CZipException::Throw(CZipException::abortedSafely);
		}
	}
};

class CAddFilesEnumerator : public ZipArchiveLib::CDirEnumerator
{
	CZipArchive* m_pZip;
	CZipActionCallback* m_pMultiCallback;
	int m_iComprLevel;
	int m_iSmartLevel;
	unsigned long m_nBufSize;
public:
	CAddFilesEnumerator(LPCTSTR lpszDirectory, 
			bool bRecursive, 
			CZipArchive* pZip, 
			int iComprLevel, 
			int iSmartLevel, 
			unsigned long nBufSize,
			CZipActionCallback* pMultiCallback)
		:ZipArchiveLib::CDirEnumerator(lpszDirectory, bRecursive), m_pZip(pZip)
	{
		m_iComprLevel = iComprLevel;
		m_nBufSize = nBufSize;
		m_iSmartLevel = iSmartLevel;
		m_pMultiCallback = pMultiCallback;
	}
protected:
	bool Process(LPCTSTR lpszPath, const ZipArchiveLib::CFileInfo&)
	{
		bool ret = m_pZip->AddNewFile(lpszPath, m_iComprLevel, m_pZip->GetRootPath().IsEmpty() != 0, m_iSmartLevel, m_nBufSize);
		if (ret && m_pMultiCallback)
			if (!m_pMultiCallback->MultiActionsNext())
				CZipException::Throw(CZipException::abortedSafely);
		return ret;
	}
};

bool CZipArchive::AddNewFiles(LPCTSTR lpszPath,
								CFileFilter& filter,
								bool bRecursive,
								int iComprLevel,
								bool bSkipInitialPath,
								int iSmartLevel,
								unsigned long nBufSize)
{
	CZipRootPathRestorer restorer;
	if (bSkipInitialPath)
		restorer.SetNewRootPath(this, lpszPath);

	CZipActionCallback* pMultiCallback = GetCallback(CZipActionCallback::cbMultiAdd);
	if (pMultiCallback)
	{		
		// if multi callback is set, calculate total data to process
		// call cbCalculateForMulti in the meantime
		CCalculateAddFilesEnumerator calculateEnumerator(lpszPath, bRecursive, GetCallback(CZipActionCallback::cbCalculateForMulti));
		if (!calculateEnumerator.Start(filter))
			return false;
		if (pMultiCallback->m_iType != CZipActionCallback::cbMultiAdd)
			// may happen, if it is the same as calculate
			pMultiCallback->m_iType = CZipActionCallback::cbMultiAdd;
		pMultiCallback->MultiActionsInit((ZIP_SIZE_TYPE)calculateEnumerator.m_uTotalFiles, (ZIP_SIZE_TYPE)calculateEnumerator.m_uTotalBytes, CZipActionCallback::cbAdd);
	}

	try
	{
		CAddFilesEnumerator addFilesEnumerator(lpszPath, bRecursive, this, iComprLevel, iSmartLevel, nBufSize, pMultiCallback);
		bool ret = addFilesEnumerator.Start(filter);
		if (pMultiCallback)
			pMultiCallback->MultiActionsEnd();
		return ret;
	}
	catch(...)
	{
		if (pMultiCallback)
			pMultiCallback->MultiActionsEnd();
		throw;
	}
}


CZipString CZipArchive::GetArchivePath() const
{
	if (IsClosed(false))
	{
		ZIPTRACE("%s(%i) : ZipArchive is closed.\n");
		return _T("");
	}
	return m_storage.m_pFile->GetFilePath();
}

CZipString CZipArchive::GetGlobalComment() const
{
	if (IsClosed())
	{
		ZIPTRACE("%s(%i) : ZipArchive is closed.\n");
		return _T("");
	}
	CZipString temp;
	m_centralDir.GetComment(temp);
	return temp;
}

bool CZipArchive::SetGlobalComment(LPCTSTR lpszComment)
{
	if (IsClosed())
	{
		ZIPTRACE("%s(%i) : ZipArchive is closed.\n");
		return false;
	}
	if (m_storage.IsSegmented() == -1)
	{
		ZIPTRACE("%s(%i) : You cannot modify the global comment of an existing segmented archive.\n");
		return false;
	}

	m_centralDir.SetComment(lpszComment);
	if (m_bAutoFlush)
		Flush();

	return true;
}



ZIP_PART_TYPE CZipArchive::GetCurrentDisk() const 
{
	return (ZIP_PART_TYPE)(m_storage.GetCurrentDisk() + 1);
}

bool CZipArchive::SetFileComment(ZIP_INDEX_TYPE uIndex, LPCTSTR lpszComment)
{
	if (IsClosed())
	{
		ZIPTRACE("%s(%i) : ZipArchive is closed.\n");
		return false;
	}
	if (m_storage.IsSegmented() == -1)
	{
		ZIPTRACE("%s(%i) : You cannot modify the file comment in an existing segmented archive.\n");
		return false;
	}
	
	m_centralDir.SetFileComment(uIndex, lpszComment);
	if (m_bAutoFlush)
		Flush();
	return true;
}

bool CZipArchive::SetPassword(LPCTSTR lpszPassword)
{
	if (m_iFileOpened != nothing)
	{
		ZIPTRACE("%s(%i) : You cannot change the password when a file is opened.\n");
		return false; // it's important not to change the password when the file inside archive is opened
	}
	if (IsClosed())
	{
		ZIPTRACE("%s(%i) : Setting the password for a closed archive has no effect.\n");
	}
	if (lpszPassword)
		ZipCompatibility::ConvertStringToBuffer(lpszPassword, m_pszPassword, CP_ACP);
	else
		m_pszPassword.Release();
	return true;
}

bool CZipArchive::SetEncryptionMethod(int iEncryptionMethod)
{
	if (m_iFileOpened == compress)
	{
		ZIPTRACE("%s(%i) : You cannot change the encryption method when there is a file opened for compression.\n");
		return false;
	}	
	
	if (iEncryptionMethod != CZipCryptograph::encNone && !CZipCryptograph::IsEncryptionSupported(iEncryptionMethod))
		return false;
	m_iEncryptionMethod = iEncryptionMethod;
	return true;
}

bool CZipArchive::SetCompressionMethod(WORD uCompressionMethod)
{
	if (m_iFileOpened == compress)
	{
		ZIPTRACE("%s(%i) : You cannot change the compression method when there is a file opened for compression.\n");
		return false; 
	}
	if (uCompressionMethod == CZipCompressor::methodStore)
	{
		ZIPTRACE("%s(%i) : Use the compression level CZipCompressor::levelStore when compressing files instead.\n");
		return false; 
	}

	if (!CZipCompressor::IsCompressionSupported(uCompressionMethod))
	{
		ZIPTRACE("%s(%i) : The compression method is not supported.\n");
		return false; 
	}

	m_uCompressionMethod = uCompressionMethod;
	return true;
}

CZipString CZipArchive::GetPassword()const 
{
	CZipString temp;
	ZipCompatibility::ConvertBufferToString(temp, m_pszPassword, CP_ACP);
	return temp;
}

bool CZipArchive::TestFile(ZIP_INDEX_TYPE uIndex, DWORD uBufSize)
{
	if (m_storage.IsSegmented() == 1)
	{
		ZIPTRACE("%s(%i) : You cannot test a segmented archive in creation.\n");
		return false;
	}
	if (!uBufSize)
		return false;
	
	CZipFileHeader* pHeader = m_centralDir[uIndex];
	CZipActionCallback* pCallback = GetCallback(CZipActionCallback::cbTest);
	if (pCallback)
	{
		pCallback->Init(pHeader->GetFileName());
	}

	if (pHeader->IsDirectory())
	{
		if (pCallback)
			pCallback->SetTotal(0);

		// we do not test whether the password for the encrypted directory
		// is correct, since it seems to be senseless (anyway password 
		// encrypted directories should be avoided - it adds 12 bytes)
		ZIP_SIZE_TYPE uSize = pHeader->m_uComprSize;
		if ((uSize != 0 || uSize != pHeader->m_uUncomprSize)
			// different treating compressed directories
			&& !(pHeader->IsEncrypted() && uSize == 12 && !pHeader->m_uUncomprSize))
			CZipException::Throw(CZipException::dirWithSize);

		if (pCallback)
			pCallback->CallbackEnd();

		return true;
	}
	else
	{
		try
		{			
			if (pCallback)
				pCallback->SetTotal(pHeader->m_uUncomprSize);
			
			if (!OpenFile(uIndex))
				return false;
			CZipAutoBuffer buf(uBufSize);
			DWORD iRead;
			int iAborted = 0;
			for(;;)
			{	
				iRead = ReadFile(buf, buf.GetSize());
				if (!iRead)
				{
					if (pCallback && !pCallback->RequestLastCallback())
						iAborted = CZipException::abortedSafely;
					break;
				}
				if (pCallback && !pCallback->RequestCallback(iRead))
				{
					if (iRead == buf.GetSize() && ReadFile(buf, 1) != 0) // test one byte if there is something left
						iAborted = CZipException::abortedAction; 
					else
						iAborted = CZipException::abortedSafely; // we did it!
					break;
				}
			}

			if (!iAborted)
			{
				if (CloseFile() == 1)
				{
					if (pCallback)
						pCallback->CallbackEnd();
					return true;
				}
				else
					CZipException::Throw(CZipException::badZipFile);
			}
			else
			{
				if (iAborted == CZipException::abortedAction)
					CloseFile(NULL, true);
				else
				{
					bool bRet;
					try
					{
						bRet = CloseFile() == 1;
					}
					// if any exception was thrown, then we are not successful
					// catch all exceptions to thrown aborted exception only
#ifdef ZIP_ARCHIVE_MFC
					catch(CException* e)
					{
						e->Delete();
						bRet = false;
					}
#endif
					catch(...)
					{
						bRet = false;
					}
					if (!bRet)
					{
						CloseFile(NULL, true);
						iAborted = CZipException::abortedAction;
					}
				}

				pCallback->CallbackEnd();
				CZipException::Throw(iAborted);
			}
			return false; // to satisfy some compilers (some will complain)...
		}
		catch(...)
		{
			CloseFile(NULL, true);
			throw;
		}
	}
}

void CZipArchive::SetFileHeaderAttr(CZipFileHeader& header, DWORD uAttr)const
{
	header.SetSystemCompatibility(m_iArchiveSystCompatib);
	header.SetSystemAttr(uAttr);
}

void CZipArchive::EnableFindFast(bool bEnable)
{
	if (IsClosed())
	{
		ZIPTRACE("%s(%i) : Set it after opening the archive.\n");
		return;
	}
	m_centralDir.EnableFindFast(bEnable, m_bCaseSensitive);

}

bool CZipArchive::SetSystemCompatibility(int iSystemComp)
{
	if (IsClosed())
	{
		ZIPTRACE("%s(%i) : Set it after opening the archive.\n");
		return false;
	}

	if (m_iFileOpened == compress)
	{
		ZIPTRACE("%s(%i) : Set it before opening a file inside archive.\n");
		return false;
	}

	if (!ZipCompatibility::IsPlatformSupported(iSystemComp))
		return false;
	// change the name coding page, if it was not changed before
	if (m_centralDir.m_stringSettings.IsStandardNameCodePage(m_iArchiveSystCompatib))
		m_centralDir.m_stringSettings.SetDefaultNameCodePage(iSystemComp);	
	m_iArchiveSystCompatib = iSystemComp;
	
	return true;
}

void CZipArchive::SetRootPath(LPCTSTR szPath)
{
	if (IsClosed())
	{
		ZIPTRACE("%s(%i) : Set it after opening the archive.\n");
		return;
	}

	if (m_iFileOpened != nothing)
	{
		ZIPTRACE("%s(%i) : Set it before opening a file inside archive.\n");
		return;
	}

	if (szPath)
	{
		m_szRootPath = szPath;
		CZipPathComponent::RemoveSeparators(m_szRootPath);
	}
	else
		m_szRootPath.Empty();
}

CZipString CZipArchive::TrimRootPath(CZipPathComponent &zpc)const 
{
	if (m_szRootPath.IsEmpty())
		return zpc.GetFileName();
	CZipString szPath = zpc.GetFullPath();
	return RemovePathBeginning(m_szRootPath, szPath, m_pZipCompare) ? szPath : zpc.GetFileName();
}

bool CZipArchive::RemovePathBeginning(LPCTSTR lpszBeginning, CZipString& szPath, ZIPSTRINGCOMPARE pCompareFunction)
{
	CZipString szBeginning(lpszBeginning);
	CZipPathComponent::RemoveSeparators(szBeginning);
	int iRootPathLength = szBeginning.GetLength();
	if (iRootPathLength && szPath.GetLength() >= iRootPathLength &&
		(szPath.Left(iRootPathLength).*pCompareFunction)(szBeginning) == 0)
	{		
		// the beginning is the same
		if (szPath.GetLength() == iRootPathLength)
		{
			szPath.Empty();
			return true;
		}
		// is the end of m_szPathRoot only a beginning of a directory name? 
		// check for a separator
		// we know the length is larger, so we can write:
		if (CZipPathComponent::IsSeparator(szPath[iRootPathLength]))
		{			
			szPath = szPath.Mid(iRootPathLength);
			CZipPathComponent::RemoveSeparatorsLeft(szPath);
			return true;
		}
	}
	return false;
}

void CZipArchive::SetTempPath(LPCTSTR lpszPath, bool bForce)
{
	m_szTempPath = lpszPath;
	if (lpszPath && bForce)
		ZipPlatform::ForceDirectory(lpszPath);
	CZipPathComponent::RemoveSeparators(m_szTempPath);
}

CZipString CZipArchive::PredictFileNameInZip(LPCTSTR lpszFilePath, 
						 bool bFullPath, int iWhat)const 
{
	CZipString sz = lpszFilePath;
	if (sz.IsEmpty())
		return _T("");
	bool bAppend;
	switch (iWhat)
	{
	case prFile:
		bAppend = false;
		break;
	case prDir:
		bAppend = true;
		break;
	default:
		bAppend = CZipPathComponent::IsSeparator(sz[sz.GetLength() - 1]);
	}
	
	// remove for CZipPathComponent treating last name as a file even if dir
	CZipPathComponent::RemoveSeparators(sz);
	// it may be empty after removing separators, e.g.: "/"
	if (sz.IsEmpty())
		return _T("");
	CZipPathComponent zpc(sz);

	if (bFullPath)
	{
		if (m_bRemoveDriveLetter)
			sz = zpc.GetNoDrive();
	}
	else
		sz = TrimRootPath(zpc);

	if (bAppend && !sz.IsEmpty())
		CZipPathComponent::AppendSeparator(sz);
	return sz;	
}

CZipString CZipArchive::PredictExtractedFileName(LPCTSTR lpszFileNameInZip, LPCTSTR lpszPath, bool bFullPath, LPCTSTR lpszNewName)const 
{
	CZipString szFile = lpszPath;
	CZipString sz = lpszNewName ? lpszNewName : lpszFileNameInZip;
	if (sz.IsEmpty())
		return szFile;
	if (!szFile.IsEmpty())
		CZipPathComponent::AppendSeparator(szFile);
	
	
	// remove for CZipPathComponent treating last name as a file even if dir
	CZipPathComponent::RemoveSeparators(sz);
	CZipPathComponent zpc(sz);
	szFile += bFullPath ? (m_bRemoveDriveLetter ? zpc.GetNoDrive() : sz) 
						  : TrimRootPath(zpc);	
	return szFile;
}

ZIP_SIZE_TYPE CZipArchive::PredictMaximumFileSizeInArchive(LPCTSTR lpszFilePath, bool bFullPath) const
{
	DWORD attr;
	if (!ZipPlatform::GetFileAttr(lpszFilePath, attr))
		return 0;		
	CZipFileHeader fh;		
	SetFileHeaderAttr(fh, attr);
	if (!fh.IsDirectory())
		if (!ZipPlatform::GetFileSize(lpszFilePath, fh.m_uUncomprSize))
			return 0;	
	fh.SetFileName(PredictFileNameInZip(lpszFilePath, bFullPath, fh.IsDirectory() ? prDir  : prFile));
	return PredictMaximumFileSizeInArchive(fh);
}

ZIP_SIZE_TYPE CZipArchive::PredictMaximumFileSizeInArchive(CZipFileHeader& fh) const
{
	fh.m_stringSettings = m_centralDir.m_stringSettings;
	fh.m_uEncryptionMethod = EncryptNextFile() ? (BYTE)m_iEncryptionMethod : (BYTE)CZipCryptograph::encNone;
	fh.m_uMethod = CZipCompressor::methodStore;
	fh.PrepareData(CZipCompressor::levelStore, m_storage.IsSegmented() != 0);
	DWORD uLocalSize = fh.GetLocalSize(true);
	return fh.GetSize() + uLocalSize + fh.GetDataSize(true, false) + fh.GetDataDescriptorSize(&m_storage);
}

void CZipArchive::SetAutoFlush(bool bAutoFlush)
{
	if (IsClosed())
	{
		ZIPTRACE("%s(%i) : ZipArchive not yet opened.\n");
		return;
	}
	if (m_storage.IsSegmented() != 0)
	{
		ZIPTRACE("%s(%i) : Cannot set auto-flush for a segmented archive.\n");
		return;
	}
	m_bAutoFlush = bAutoFlush;
}

void CZipArchive::Flush()
{
	if (IsClosed())
	{
		ZIPTRACE("%s(%i) : ZipArchive not yet opened.\n");
		return;
	}

	if (m_storage.IsSegmented() < 0)
	{
		ZIPTRACE("%s(%i) : Cannot flush an existing segmented archive.\n");
		return;
	}
	WriteCentralDirectory();
	m_storage.FlushFile();
	if (m_storage.IsSegmented() > 0) // try to finalize a segmented archive without closing it
		m_storage.FinalizeSegm();
}


void CZipArchive::GetCentralDirInfo(CZipCentralDir::Info& info)const
{
	if (IsClosed())
	{
		ZIPTRACE("%s(%i) : ZipArchive not yet opened.\n");
		return;
		
	}
	m_centralDir.GetInfo(info);
	if (GetSegmMode() > 0)
		info.m_uLastDisk = m_storage.GetCurrentDisk();
}

void CZipArchive::FindMatches(LPCTSTR lpszPattern, CZipIndexesArray &ar, bool bFullPath)
{
	if (IsClosed())
	{
		ZIPTRACE("%s(%i) : ZipArchive is closed.\n");
		return;
	}

	//	ar.RemoveAll(); don't do this
	ZIP_INDEX_TYPE uCount = (ZIP_INDEX_TYPE)GetCount();
	CWildcard wc(lpszPattern, m_bCaseSensitive);
	for (ZIP_INDEX_TYPE i = 0; i < uCount; i++)
	{
		CZipString sz = m_centralDir[i]->GetFileName();
		if (!bFullPath)
		{
			CZipPathComponent::RemoveSeparators(sz);
			CZipPathComponent zpc(sz);			
			sz = zpc.GetFileName();
		}
		if (wc.IsMatch(sz))
			ar.Add(i);
	}
}

ZIP_INDEX_TYPE CZipArchive::WillBeDuplicated(LPCTSTR lpszFilePath, bool bFullPath, bool bFileNameOnly , int iWhat)
{
	CZipString szFile;
	if (bFileNameOnly)
	{
		CZipPathComponent zpc(lpszFilePath);
		szFile = PredictFileNameInZip(zpc.GetFileName(), false, iWhat); 
	}
	else
		szFile = PredictFileNameInZip(lpszFilePath, bFullPath, iWhat);
	return FindFile(szFile, ffDefault, bFileNameOnly);
}


bool CZipArchive::GetFromArchive(CZipArchive& zip, ZIP_INDEX_TYPE uIndex, LPCTSTR lpszNewFileName, ZIP_INDEX_TYPE uReplaceIndex, bool bKeepSystComp, CZipActionCallback* pCallback)
{
	if (this == &zip)
	{
		ZIPTRACE("%s(%i) : You cannot get files from the same archive.\n");
		return false;
	}

	if (IsClosed() || zip.IsClosed())
	{
		ZIPTRACE("%s(%i) : ZipArchive is closed.\n");
		return false;
	}
	
	if (m_iFileOpened || zip.m_iFileOpened)
	{
		ZIPTRACE("%s(%i) : You cannot get files from another archive if there is a file opened.\n");
		return false;
	}
	
	if (m_storage.IsSegmented() == -1)
	{
		ZIPTRACE("%s(%i) : You cannot add files to an existing segmented archive.\n");
		return false;
	}

	ASSERT(m_pBuffer.GetSize() > 0);

	bool bSegm = m_storage.IsSegmented() == 1;

	if (!zip.m_centralDir.IsValidIndex(uIndex))
		return false;
	// update sizes of local filename and extra field - they may differ from the ones in the central directory
	zip.m_centralDir.OpenFile(uIndex);
	// skip checking the data descriptor, we are not there yet
	zip.m_centralDir.CloseFile(true);

	CZipFileHeader originalHeader;
	// we need a copy - we are going to modify this
	if (!zip.GetFileInfo(originalHeader, uIndex))
		return false;
	if (zip.m_storage.IsSegmented() != 0 && originalHeader.m_uLocalComprSize == 0)
		originalHeader.m_uLocalComprSize = originalHeader.m_uComprSize;
	bool bConvertSystem = !bKeepSystComp && originalHeader.GetSystemCompatibility() != m_iArchiveSystCompatib;
	CZipString szFileName;
	if (lpszNewFileName != NULL)
	{
		szFileName = lpszNewFileName;
		originalHeader.SetFileName(lpszNewFileName);
	}
	else
		szFileName = originalHeader.GetFileName(); // empty

	if (bConvertSystem)
	{
		DWORD uAttr = originalHeader.GetSystemAttr();
		originalHeader.SetSystemCompatibility(m_iArchiveSystCompatib);
		originalHeader.SetSystemAttr(uAttr);
	}
	
	if (!UpdateReplaceIndex(uReplaceIndex))
		return false;

	bool bReplace = uReplaceIndex != ZIP_FILE_INDEX_UNSPECIFIED;
	if (bReplace && bSegm)
		return false;

	int iCallbackType = 0;
	if (pCallback)
		iCallbackType = pCallback->m_iType;

	// if the same callback is applied to cbReplace, then the previous information about the type will be lost
	// we restore it later
	CZipFileHeader* pHeader = m_centralDir.AddNewFile(originalHeader, uReplaceIndex, originalHeader.GetCompressionLevel(), true);
	pHeader->m_stringSettings.m_uCommentCodePage = originalHeader.m_stringSettings.m_uCommentCodePage;
	
	// prepare this here: it will be used for GetLocalSize and WriteLocal
	pHeader->PrepareFileName();

	ZIP_SIZE_TYPE uTotalToMove = originalHeader.m_uComprSize;

	if (bReplace)
		MakeSpaceForReplace(uReplaceIndex, uTotalToMove + pHeader->GetLocalSize(false) + pHeader->GetDataDescriptorSize(&m_storage), szFileName);

	if (pCallback)
	{
		// must be set before Init()
		pCallback->m_iType = iCallbackType;
		pCallback->Init(szFileName, zip.GetArchivePath());
		pCallback->SetTotal(pHeader->m_uComprSize);
	}

	// must be written as not converted
	pHeader->WriteLocal(&m_storage); 


	char* buf = (char*)m_pBuffer;

	ZIP_SIZE_TYPE uToRead;
	DWORD size_read;	
	int iAborted = 0;
	bool bBreak = false;
	if (uTotalToMove)
	{
		do
		{
			uToRead = uTotalToMove > m_pBuffer.GetSize() ? m_pBuffer.GetSize() : uTotalToMove;
			size_read = (UINT)zip.m_storage.Read(buf, (UINT)uToRead, false);
			if (!size_read)
				break;
			uTotalToMove -= size_read;
			if (uTotalToMove == 0)
				bBreak = true;
			
			m_storage.Write(buf, size_read, false);
			if (pCallback && !pCallback->RequestCallback(size_read))
			{												
				if (uTotalToMove != 0) 
				{							
					if (!bSegm && !bReplace)
					{							
						m_centralDir.RemoveLastFile();
						iAborted = CZipException::abortedSafely;
					}
					else
						iAborted = CZipException::abortedAction; 
				}
				else
					iAborted = CZipException::abortedSafely;
				break;
				
			}
		}
		while (!bBreak);

		if (pCallback)
		{
			if (!iAborted && !pCallback->RequestLastCallback())
				iAborted = CZipException::abortedSafely;
			
			if (iAborted)
			{
				// when no exception, CallbackEnd() called later
				pCallback->CallbackEnd();
				CZipException::Throw(iAborted); // throw to distinguish from other return codes					
			}
		}
	}

	m_centralDir.m_pOpenedFile = NULL;
	// copying from a not segmented to a segmented archive so add the data descriptor

	// we want write the descriptor only if everything is all right, but we don't want to flush the storage before 
	// (and we want to flush the storage before throwing an exception, if something is wrong)
	if (uTotalToMove == 0)
		// it will be written only if needed
		pHeader->WriteDataDescriptor(&m_storage);

	m_storage.Flush();
	if (uTotalToMove > 0)
		ThrowError(CZipException::badZipFile);

	if (pCallback)
		pCallback->CallbackEnd();
				
	return true;
}

bool CZipArchive::GetFromArchive(CZipArchive& zip, CZipIndexesArray &aIndexes, bool bKeepSystComp)
{
	aIndexes.Sort(true);
	ZIP_INDEX_TYPE uFiles = (ZIP_INDEX_TYPE)aIndexes.GetSize();		
	InitBuffer();
	try
	{
		for (ZIP_INDEX_TYPE i = 0; i < uFiles; i++)
		{
			ZIP_INDEX_TYPE uFileIndex = aIndexes[(ZIP_ARRAY_SIZE_TYPE)i];
			if (!GetFromArchive(zip, uFileIndex, NULL, ZIP_FILE_INDEX_UNSPECIFIED, bKeepSystComp, GetCallback(CZipActionCallback::cbGet)))
			{
				ReleaseBuffer();
				return false;
			}
		}
	}
	catch (...)
	{
		ReleaseBuffer();
		throw;
	}
	ReleaseBuffer();
	if (m_bAutoFlush)
		Flush();
	return true;
}
bool CZipArchive::RenameFile(ZIP_INDEX_TYPE uIndex, LPCTSTR lpszNewName)
{
	if (IsClosed())
	{
		ZIPTRACE("%s(%i) : ZipArchive is closed.\n");
		return false;
	}
	
	if (m_storage.IsSegmented())
	{
		ZIPTRACE("%s(%i) : You cannot rename files in a segmented archive.\n");
		return false;
	}
	
	if (m_iFileOpened)
	{
		ZIPTRACE("%s(%i) : You cannot rename a file if there is a file opened.\n");
		return false;
	}
	CZipFileHeader* pHeader = (*this)[uIndex];
	if (pHeader == NULL)
		return false;

	CZipString szNewName(lpszNewName);
	if (pHeader->IsDirectory())
		CZipPathComponent::AppendSeparator(szNewName);
	else
		CZipPathComponent::RemoveSeparators(szNewName);
	CZipString szPreviousFileName = pHeader->GetFileName();
	if (szPreviousFileName.Collate(szNewName) == 0)
		return true;
	
	// don't copy the comment code page, it already could have been set using it's own code page (set CD CP, change comment, change CD CP)
	pHeader->m_stringSettings.m_bStoreNameInExtraData = m_centralDir.m_stringSettings.m_bStoreNameInExtraData;
	pHeader->m_stringSettings.m_uNameCodePage = m_centralDir.m_stringSettings.m_uNameCodePage;
	pHeader->SetFileName(szNewName);

	m_centralDir.RemoveFromDisk(); // does m_storage.Flush();
	// read local data - it may differ from central data
	char localInfo[4];
	m_storage.Seek(pHeader->m_uOffset + 26);
	m_storage.m_pFile->Read(localInfo, 4); // read at once
	WORD uFileNameLen, uExtraFieldSize;
	CBytesWriter::ReadBytes(uFileNameLen, localInfo);
	// skip endian issues - the variable will not be used, but written back as it is
	memcpy(&uExtraFieldSize, localInfo + 2, 2);
	
	// filename buffer already cleared (GetFileName), the conversion will take place
	pHeader->PrepareFileName();
	ASSERT(pHeader->m_pszFileNameBuffer.IsAllocated());
	WORD uNewFileNameLen = (WORD)pHeader->m_pszFileNameBuffer.GetSize();
	int iDelta = uNewFileNameLen - uFileNameLen;
	int iOffset = 0;
	CZipAutoBuffer buf, *pBuf;
	
	if (iDelta != 0)
	{
		// we need to make more or less space		
		InitBuffer();
		ZIP_SIZE_TYPE uStartOffset = pHeader->m_uOffset + 30 + uFileNameLen;
		ZIP_SIZE_TYPE uFileLen = (ZIP_SIZE_TYPE)m_storage.m_pFile->GetLength();
		ZIP_SIZE_TYPE uEndOffset = uFileLen - m_storage.m_uBytesBeforeZip;
		CZipActionCallback* pCallback = GetCallback(CZipActionCallback::cbRename);
		if (pCallback)
		{
			// do it right and sent the notification
			pCallback->Init(szPreviousFileName, GetArchivePath());
			pCallback->SetTotal(uEndOffset - uStartOffset);
		}
		bool bForward = iDelta > 0;
		if (bForward)
			m_storage.m_pFile->SetLength((ZIP_FILE_USIZE)(uFileLen + iDelta)); // ensure the seek is correct

		MovePackedFiles(uStartOffset, uEndOffset, abs(iDelta), pCallback, bForward);
		if (pCallback)
			pCallback->CallbackEnd();

		if (!bForward)
			m_storage.m_pFile->SetLength((ZIP_FILE_USIZE)(uFileLen + iDelta)); // delta < 0; shrink the file

		ReleaseBuffer();	

		ZIP_INDEX_TYPE uSize = (ZIP_INDEX_TYPE)GetCount();
		for (ZIP_INDEX_TYPE i = (ZIP_INDEX_TYPE)(uIndex + 1); i < uSize; i++)
			m_centralDir[i]->m_uOffset += iDelta;
		buf.Allocate(4 + uNewFileNameLen);
		CBytesWriter::WriteBytes(buf, uNewFileNameLen);
		// the variable was not used - we write it back as it was (no endian issues)
		// to write everything at once
		memcpy(buf + 2, &uExtraFieldSize, 2);
		memcpy(buf + 4, pHeader->m_pszFileNameBuffer, uNewFileNameLen);	
		pBuf = &buf;
		iOffset = -4;
	}
	else
		pBuf = &pHeader->m_pszFileNameBuffer;
	
	m_storage.Seek(pHeader->m_uOffset + 30 + iOffset);
	m_storage.m_pFile->Write(*pBuf, pBuf->GetSize());
	m_centralDir.RebuildFindFastArray();
	if (m_bAutoFlush)
		Flush();
	
	return true;
}

bool CZipArchive::UpdateReplaceIndex(ZIP_INDEX_TYPE& uReplaceIndex)
{
	if (uReplaceIndex == ZIP_FILE_INDEX_UNSPECIFIED)
		return true;

	if (GetSegmMode() != 0)
	{
		ZIPTRACE("%s(%i) : You cannot replace files in a segmented archive.\n");		
		return false;
	}
	
	if (!m_centralDir.IsValidIndex(uReplaceIndex))
	{
		ZIPTRACE("%s(%i) : Not valid replace index.\n");
		return false;
	}
	if (uReplaceIndex == GetCount() - 1) // replacing last file in the archive
	{
		RemoveLast(true);
		uReplaceIndex = ZIP_FILE_INDEX_UNSPECIFIED;
	}
	return true;	
}

void CZipArchive::MakeSpaceForReplace(ZIP_INDEX_TYPE uReplaceIndex, ZIP_SIZE_TYPE uTotal, LPCTSTR lpszFileName)
{		
	ASSERT(uReplaceIndex < GetCount() - 1);
	// we can't use the offset from the central directory here, because the header is already replaced
	ZIP_SIZE_TYPE uReplaceStart = (ZIP_SIZE_TYPE)m_storage.m_pFile->GetPosition() - m_storage.m_uBytesBeforeZip;
	
	// find the next offset (files in the central directory may not necessarily be ordered by offset)
	ZIP_SIZE_TYPE uReplaceEnd = ZIP_SIZE_TYPE(-1);
	ZIP_INDEX_TYPE i;
	for (i = 0; (ZIP_ARRAY_SIZE_TYPE)i < m_centralDir.m_headers.GetSize(); i++)
		if (i != uReplaceIndex)
		{
			ZIP_SIZE_TYPE uOffset = m_centralDir.m_headers[(ZIP_ARRAY_SIZE_TYPE)i]->m_uOffset;
			if (uOffset > uReplaceStart && uOffset < uReplaceEnd)
				uReplaceEnd = uOffset;
		}
		
	ZIP_SIZE_TYPE uReplaceTotal = uReplaceEnd - uReplaceStart;
	if (uTotal == uReplaceTotal)
		return;

	bool bForward = uTotal > uReplaceTotal;
	ZIP_SIZE_TYPE uDelta;
	if (bForward)
		uDelta = uTotal - uReplaceTotal;
	else
		uDelta = uReplaceTotal - uTotal;
	
	
	//	InitBuffer(); don't - the calling functions will
	CZipActionCallback* pCallback = GetCallback(CZipActionCallback::cbReplace);
	ZIP_SIZE_TYPE uFileLen = (ZIP_SIZE_TYPE)m_storage.m_pFile->GetLength();
	ZIP_SIZE_TYPE uUpperLimit = uFileLen - m_storage.m_uBytesBeforeZip; // will be added in m_storage.Seek
	if (pCallback)
	{
		pCallback->Init(lpszFileName, GetArchivePath());
		pCallback->SetTotal(uUpperLimit - uReplaceEnd);
	}			
		
	if (bForward)
		m_storage.m_pFile->SetLength((ZIP_FILE_USIZE)(uFileLen + uDelta)); // ensure the seek is correct

	MovePackedFiles(uReplaceEnd, uUpperLimit, uDelta, pCallback, bForward);

	if (!bForward)
		m_storage.m_pFile->SetLength((ZIP_FILE_USIZE)(uFileLen - uDelta));

	m_storage.Seek(uReplaceStart);
	ZIP_INDEX_TYPE uSize = (ZIP_INDEX_TYPE)m_centralDir.m_headers.GetSize();
	for (i = (ZIP_INDEX_TYPE)(uReplaceIndex + 1); i < uSize; i++)
	{
		ZIP_SIZE_TYPE uOffset = m_centralDir.m_headers[(ZIP_ARRAY_SIZE_TYPE)i]->m_uOffset;
		m_centralDir.m_headers[(ZIP_ARRAY_SIZE_TYPE)i]->m_uOffset = bForward ? uOffset + uDelta : uOffset - uDelta;
	}
	if (pCallback)
		pCallback->CallbackEnd();			
}

void CZipArchive::MovePackedFiles(ZIP_SIZE_TYPE uStartOffset, ZIP_SIZE_TYPE uEndOffset, ZIP_SIZE_TYPE uMoveBy, CZipActionCallback* pCallback, bool bForward, bool bLastCall)
{
	ASSERT(m_pBuffer.GetSize() > 0);
	
	ZIP_SIZE_TYPE uTotalToMove = uEndOffset - uStartOffset;
	ZIP_SIZE_TYPE uPack = uTotalToMove > m_pBuffer.GetSize() ? m_pBuffer.GetSize() : uTotalToMove;
	char* buf = (char*)m_pBuffer;
	
	
	UINT size_read;
	bool bBreak = false;
	bool bAborted = false;
	do
	{
		if (uEndOffset - uStartOffset < uPack)
		{
			uPack = uEndOffset - uStartOffset;
			if (!uPack)
				break;
			bBreak = true;
			
		}
		ZIP_SIZE_TYPE uPosition = bForward ? uEndOffset - uPack : uStartOffset;
		
		m_storage.Seek(uPosition);
		size_read = m_storage.m_pFile->Read(buf, (UINT)uPack);
		if (!size_read)
			break;
		
		if (bForward)
			uPosition += uMoveBy;
		else
			uPosition -= uMoveBy;
		m_storage.Seek(uPosition);
		m_storage.m_pFile->Write(buf, size_read);
		if (bForward)
			uEndOffset -= size_read;
		else
			uStartOffset += size_read;
		if (pCallback && !pCallback->RequestCallback(size_read))
		{
			bAborted = true;
			break;			
		}
	}
	while (!bBreak);

	if (pCallback)
	{
		if (!bAborted && bLastCall && !pCallback->RequestLastCallback())
			bAborted = true;
		// do not call here - it will be called in the calling method
		//pCallback->CallbackEnd();
		if (bAborted)
		{
			// call here before throwing the the aborted exception
			pCallback->CallbackEnd();
			ThrowError(CZipException::abortedAction);
		}
	}
	
	if (uEndOffset != uStartOffset)
		ThrowError(CZipException::internalError);
	
}
