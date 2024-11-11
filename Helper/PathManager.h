#pragma once
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

// Default setting (all lowercase)
#define			DEFAULT_LOG_DIRECTORY_NAME				TEXT("log")
#define			DEFAULT_CONFIG_DIRECTORY_NAME			TEXT("config")
#define			DEFAULT_TEMP_DIRECTORY_ROOTNAME			TEXT("winsfxmaker")
#define			DEFAULT_CONFIG_FILE_NAME				TEXT("config.ini")

class CPathManager
{
public:
	CPathManager(void);
	~CPathManager(void);

	static CPathManager*	GetInstance();
	static void				DestroyInstance();

	BOOL					Initialize();
	BOOL					UnInitialize();

	void					CleanUp();

	// Getter Directories
	LPCTSTR					GetRootPath()						{			return		m_szRootPath;					}
	LPCTSTR					GetConfigPath()						{			return		m_szConfigPath;					}
	LPCTSTR					GetLogPath()						{			return		m_szLogPath;					}
	DWORD					GetLogPathBufferSize()				{			return		_countof(m_szLogPath);			}

	// Setter Directories

	// Getter Files
	LPCTSTR					GetConfigFilePath()					{			return		m_szConfigFile;					}

private:
	TCHAR					m_szRootPath[MAX_PATH];
	TCHAR					m_szLogPath[MAX_PATH];
	TCHAR					m_szConfigPath[MAX_PATH];
	TCHAR					m_szTempPath[MAX_PATH];

	TCHAR					m_szConfigFile[MAX_PATH];

	static CPathManager*	m_pManager;

public:
	// Default folder name definition:: Getter has not been implemented separately as static member.
	static LPCTSTR			m_lpszDefaultLogName;
	static LPCTSTR			m_lpszDefaultConfigName;
	static LPCTSTR			m_lpszDefaultConfigFileName;
	static LPCTSTR			m_lpszDefaultDCConfigFileName;
	static LPCTSTR			m_lpszDefaultTempPathRootName;
};