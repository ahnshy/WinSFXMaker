/**
	@file		PathManager.cpp
	@class		CPathManager
	@date		2024/11/12
	@author		HoSeong,Ahn (ahnshy@gmail.com)
	@brief		This is an implementation unit with route information.
	[Directory Tree Map]
	%Root%
		¦¦CertStudio File Detector.exe
		¦¦\Log		:: Log File
		¦¦\Config	:: Config File
		@notice		1. Singleton structure. Always take Instance and call Getter to use it from start to finish.
				2. You must explicitly call Destroy Instance() at the end of the program so that no MemoryLeak occurs.
				3. All path variables are not followed by \. Please receive them as Getter and use \ at the end.

	[Config File]
	%Root%
	¦¦\Config\config.ini
*/
#include "../stdafx.h"
#include "PathManager.h"

// static member initialize
CPathManager*	CPathManager::m_pManager			= NULL;
LPCTSTR	CPathManager::m_lpszDefaultLogName			= DEFAULT_LOG_DIRECTORY_NAME;
LPCTSTR	CPathManager::m_lpszDefaultConfigName		= DEFAULT_CONFIG_DIRECTORY_NAME;
LPCTSTR	CPathManager::m_lpszDefaultTempPathName		= DEFAULT_TEMP_DIRECTORY_NAME;

// File
LPCTSTR	CPathManager::m_lpszDefaultConfigFileName	= DEFAULT_CONFIG_FILE_NAME;

/////////////////////////////////////////////////////////////////////////////
// CPathManager

CPathManager::CPathManager(void)
{
	CleanUp();
}

CPathManager::~CPathManager(void)
{
	UnInitialize();
}

BOOL CPathManager::Initialize()
{	
	GetModuleFileName(NULL, m_szRootPath, _countof(m_szRootPath));
	PathRemoveFileSpec(m_szRootPath);
	PathRemoveBackslash(m_szRootPath);

	// Directory
	// Set LogPath
	_tcscpy_s(m_szLogPath, _countof(m_szLogPath), m_szRootPath);
	PathAddBackslash(m_szLogPath);
	_tcscat_s(m_szLogPath, _countof(m_szLogPath), m_lpszDefaultLogName);
	_tmkdir(m_szLogPath);

	// Set ConfigPath
	_tcscpy_s(m_szConfigPath, _countof(m_szConfigPath), m_szRootPath);
	PathAddBackslash(m_szConfigPath);
	_tcscat_s(m_szConfigPath, _countof(m_szConfigPath), m_lpszDefaultConfigName);
	_tmkdir(m_szConfigPath);

	// File
	// Set ConfigFilePath
	_tcscpy_s(m_szConfigFile, _countof(m_szConfigFile), m_szConfigPath);
	PathAddBackslash(m_szConfigFile);
	_tcscat_s(m_szConfigFile, _countof(m_szConfigFile), m_lpszDefaultConfigFileName);

	// Set TempPath
	_tcscpy_s(m_szTempPath, _countof(m_szTempPath), m_szRootPath);
	PathAddBackslash(m_szTempPath);
	_tcscat_s(m_szTempPath, _countof(m_szTempPath), m_lpszDefaultTempPathName);
	_tmkdir(m_szTempPath);

	return TRUE;
}

BOOL CPathManager::UnInitialize()
{
	CleanUp();
	return TRUE;
}

void CPathManager::CleanUp()
{
	m_szRootPath[0]		= NULL;
	m_szLogPath[0]		= NULL;
	m_szConfigPath[0]	= NULL;
	m_szTempPath[0]		= NULL;
}

CPathManager* CPathManager::GetInstance()
{
	if (m_pManager)
		return m_pManager;

	m_pManager = new CPathManager;
	ASSERT(m_pManager);
	return m_pManager;
}

void CPathManager::DestroyInstance()
{
	if (!m_pManager)
		return;

	delete m_pManager;
	m_pManager = NULL;
}