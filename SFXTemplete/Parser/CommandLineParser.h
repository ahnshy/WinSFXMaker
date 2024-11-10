#pragma once
class CCommandLineParser
{
public:
	CCommandLineParser();
	~CCommandLineParser();

	void				SetEmpty();
	INT32				Parse(LPTSTR lpCmdLine);

private:
	CString				m_strCmdLine;
};

