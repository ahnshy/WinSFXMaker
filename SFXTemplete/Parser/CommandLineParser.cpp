#include "stdafx.h"
#include "CommandLineParser.h"

CCommandLineParser::CCommandLineParser()
{
	SetEmpty();
}


CCommandLineParser::~CCommandLineParser()
{
}

void CCommandLineParser::SetEmpty()
{
	m_strCmdLine.Empty();
}

INT32 CCommandLineParser::Parse(LPTSTR lpCmdLine)
{
	if (lpCmdLine == NULL || lpCmdLine[0] == NULL)
		return 0;

	m_strCmdLine = lpCmdLine;

	{
		TCHAR seps[] = _T(" ");
		TCHAR* pToken = NULL;
		TCHAR* p = NULL;
		pToken = _tcstok(m_strCmdLine.GetBuffer(m_strCmdLine.GetLength() + sizeof(TCHAR)), seps); // establish first token
		while (pToken != NULL)
		{
			do
			{
				if (((p = _tcsstr(wcsupr(pToken), _T("/w"))) != NULL) || ((p = _tcsstr(wcsupr(pToken), _T("/W"))) != NULL))
				{

					//m_strPath = _tcstok(NULL, _T("\""));
					//PathAddBackslash(m_strPath.GetBuffer(512));
					//m_strPath.ReleaseBuffer();
					break;
				}

				if (((p = _tcsstr(wcsupr(pToken), _T("/p"))) != NULL) || ((p = _tcsstr(wcsupr(pToken), _T("/P"))) != NULL))
				{
					//m_strPatientID = _tcstok(NULL, _T("\""));
					break;
				}

				if (((p = _tcsstr(wcsupr(pToken), _T("/n"))) != NULL) || ((p = _tcsstr(wcsupr(pToken), _T("/N"))) != NULL))
				{
					//m_strName = _tcstok(NULL, _T("\""));
					break;
				}

				if (((p = _tcsstr(wcsupr(pToken), _T("/r"))) != NULL) || ((p = _tcsstr(wcsupr(pToken), _T("/R"))) != NULL))
				{
					//m_strRecordID = _tcstok(NULL, _T("\""));
					break;
				}

			} while (0);
			pToken = _tcstok(NULL, seps);
		}
	}

	//if (m_strPath.IsEmpty() || m_strPatientID.IsEmpty() || m_strName.IsEmpty() || m_strRecordID.IsEmpty())
	//	return 0;

	m_strCmdLine.ReleaseBuffer();
	return 1;
}