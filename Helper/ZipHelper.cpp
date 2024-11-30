#include "stdafx.h"
//#include "CompressLib.h"
#include "ZipHelper.h"
#include <shlwapi.h>
#include "FileFinderHelper.h"

const long	MAX_VOLUME_SIZE_BYTES	= 100000000;
const int	ZIP_COMPRESS_LEVEL		= 9;
/////////////////////////////////////////////////////////////////////////////
// CZip

CZipHelper::CZipHelper()
{
}

CZipHelper::~CZipHelper()
{
}

BOOL CZipHelper::AddCompressTarget(CString strTargetPath)
{
	if(IsTargetExist(strTargetPath))
		return TRUE;

	m_arrTarget.Add(strTargetPath);

	return TRUE;
}

BOOL CZipHelper::Compress(CString strOutputName)
{
	long lVolumeSize = MAX_VOLUME_SIZE_BYTES;
	int nLevel = ZIP_COMPRESS_LEVEL;

	TCHAR szOldCurrentDir[2048] = { 0 };
	GetCurrentDirectory(2048, szOldCurrentDir);

	strOutputName.MakeLower();
	if(strOutputName.Find(_T(".zip")) < 0)
	{
		strOutputName += _T(".zip");
	}

	try
	{
		m_zip.Open((LPCTSTR)strOutputName, CZipArchive::zipCreate, lVolumeSize);
	}
	catch(CException* pEx)
	{
		pEx->Delete();
		return FALSE;
	}

	int nCount = (int)m_arrTarget.GetSize();
	if(nCount < 0)
	{
		m_zip.CloseFile();
		return FALSE;
	}

	try
	{
		for(int i = 0; i < nCount; i++)
		{
			CString strPath = m_arrTarget.GetAt(i);
			//m_zip.AddNewFile((LPCTSTR)strPath, nLevel);
			if(::PathIsDirectory((LPCTSTR)strPath))
			{
				CFileFinder finder;
				finder.m_fileTypeToSearch.push_back(_T("*.*"));
				int nCount = finder.Search(strPath, TRUE);

				if(strPath.Right(1) != _T("\\"))
				{
					strPath += _T("\\");
				}

				::SetCurrentDirectory((LPCTSTR)strPath);

				for(int j = 0; j < (int)finder.m_fileList.size(); j++)
				{
					CString strFileName(finder.m_fileList[j].fullpath);
					strFileName.Replace(strPath, _T(""));
					m_zip.AddNewFile(strFileName, nCount);
				}
			}
			else
			{
				TCHAR szDrive[12];
				TCHAR szDirectory[MAX_PATH];
				TCHAR szPath[MAX_PATH];
				_tsplitpath_s((LPCTSTR)strPath, szDrive, sizeof(szDrive)/sizeof(TCHAR), szDirectory, sizeof(szDirectory)/sizeof(TCHAR), NULL, 0, NULL, 0);

				_tcscpy_s(szPath, sizeof(szPath)/sizeof(TCHAR), szDrive);
				_tcscat_s(szPath, sizeof(szPath)/sizeof(TCHAR), szDirectory);

				::SetCurrentDirectory(szPath);
				strPath.Replace(szPath, _T(""));				
				m_zip.AddNewFile(strPath, nLevel);				
			}
		}
	}
	catch(CException* pEx2)
	{
		m_zip.Close();
		pEx2->Delete();
		return FALSE;
	}

	m_arrTarget.RemoveAll();
	m_zip.Close();
	
	SetCurrentDirectory(szOldCurrentDir);

	return TRUE;
}

BOOL CZipHelper::Decompress(CString strDecompressTarget, CString strDestPath)
{
	if(!::PathIsDirectory((LPCTSTR)strDestPath))
	{
		::CreateDirectory((LPCTSTR)strDestPath, NULL);
	}

	if(strDestPath.Right(1) != _T("\\"))
	{
		strDestPath += _T("\\");
	}

	m_zip.Open((LPCTSTR)strDecompressTarget);//, CZipArchive::zipOpen);
	
	CZipCentralDir::Info info;
	m_zip.GetCentralDirInfo(info);
	int nCount = info.m_uEntriesNumber;
	for (int i = 0; i < nCount ; i++)
	{
		try
		{
			if(!m_zip.ExtractFile((WORD)i, (LPCTSTR)strDestPath))
			{
				break;
			}
		}
		catch(CException* pEx)
		{
			pEx->Delete();
		}
	}

	m_zip.Close();
	return TRUE;
}

BOOL CZipHelper::IsTargetExist(LPCTSTR lpszTarget)
{
	int nCount = (int)m_arrTarget.GetSize();
	for(int i = 0; i < nCount; i++)
	{
		CString strTemp = m_arrTarget.GetAt(i);
		if(!strTemp.CompareNoCase(lpszTarget))
		{
			return TRUE;
		}
	}

	return FALSE;
}
