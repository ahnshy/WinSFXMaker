/////////////////////////////////////////////////////////////////////////////
// FileFinderHelper.cpp
//
//        Author:
// Creation date:Thursday, June 14, 2001
//   Description:
//
/////////////////////////////////////////////////////////////////////////////


#include "StdAfx.h"
#include "FileFinderHelper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////
// Constructor

CFileFinderHelper::CFileFinderHelper()
{
}

CFileFinderHelper::CFileFinderHelper(const CFileFinderHelper& rhs)
{
	UNREFERENCED_PARAMETER(rhs);

	m_fileTypeToSearch.clear();
}

///////////////////////////////
// Destructor

CFileFinderHelper::~CFileFinderHelper()
{
}

///////////////////////////////
// Overloaded assignment

CFileFinderHelper& CFileFinderHelper::operator=(const CFileFinderHelper& rhs)
{
	if(this != &rhs){
	}
	return *this;
}

int CFileFinderHelper::Search(	CString				inStartPath,
							bool				inWithSubdir)
{
	ASSERT(inStartPath.GetLength() != 0);

	m_fileList.clear();

	ASSERT(m_fileTypeToSearch.size() != 0);

	m_withSubdir = inWithSubdir;
	
	FindFile(inStartPath);

	return (int)m_fileList.size();
}

void CFileFinderHelper::FindFile(CString inFullPath)
{
	CString fileFound;
	CFileFind fileFinder;

	CString searchFile;
	searchFile.Format(_T("%s\\*.*"), inFullPath);

	BOOL bLastFile = fileFinder.FindFile(searchFile);
		
	while (bLastFile != 0)
	{
		bLastFile = fileFinder.FindNextFile();

		if(fileFinder.IsDirectory() && m_withSubdir)
		{
			if(!fileFinder.IsDots())
			{
				fileFound = fileFinder.GetFilePath();
				FindFile(fileFound);
			}
		}
		else
		{
			FileInfo fileInfo;

			fileInfo.fullpath = fileFinder.GetFilePath();
			fileInfo.filename = fileFinder.GetFileName();
			fileInfo.path = fileFinder.GetRoot();
			fileFinder.GetCreationTime(fileInfo.creationTime);
			fileFinder.GetLastWriteTime(fileInfo.lastWriteTime);

			m_fileList.push_back(fileInfo);
		}
	}

	fileFinder.Close();
}
