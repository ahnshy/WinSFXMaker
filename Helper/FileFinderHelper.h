/////////////////////////////////////////////////////////////////////////////
// FileFinderHelper.h
//
//        Author:
// Creation date:Thursday, June 14, 2001
//   Description:
//
/////////////////////////////////////////////////////////////////////////////

#ifndef _CFILEFINDER_H_INCLUDED_
#define _CFILEFINDER_H_INCLUDED_
#include <iostream>

using namespace std;
#include <vector>


struct FileInfo
{
	CString		fullpath;
	CString		filename;
	CString		path;
	CTime		creationTime;
	CTime		lastWriteTime;
};

/////////////////////////////////////////////////////////////////////////////
// CTestClass

class CFileFinderHelper
{
	CString		m_startPath;
	CString		m_fileMask;
	bool		m_withSubdir;
	int			m_subdirLevel;

// Operations
public:
	CFileFinderHelper();
	~CFileFinderHelper();

	int Search(	CString				inStartPath, 
				bool				inWithSubdir = false);


// Attributes
public:

	vector<FileInfo>	m_fileList;
	vector<CString>		m_fileTypeToSearch;
	
// Operations
protected:

	void FindFile(CString inFullPath);

// Attributes
protected:

// Operations
private:

	// Copy constructor
	CFileFinderHelper(const CFileFinderHelper& rhs);

	// Overloaded assignment
	CFileFinderHelper& operator=(const CFileFinderHelper& rhs);

// Attributes
private:

};


#endif // _CFILEFINDER_H_INCLUDED_
