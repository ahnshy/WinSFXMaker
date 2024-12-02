#include "../stdafx.h"
//#include "CompressLib.h"
#include "ZipHelper.h"
#include <shlwapi.h>

#pragma comment(lib, "..\\lib\\ZipArchive.lib")

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
			return TRUE;
	}

	return FALSE;
}
