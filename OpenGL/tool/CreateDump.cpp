#include "CreateDump.h"
#include <Windows.h>
#include <DbgHelp.h>
#include <iostream>
#pragma comment(lib, "dbghelp.lib")

std::shared_ptr<CreateDump*> CreateDump::m_cdmpInstance = make_shared<CreateDump*>();
string CreateDump::m_strDumpFile = "";

CreateDump::CreateDump()
{

}

CreateDump::~CreateDump(void)
{

}

long CreateDump::UnhandleExceptionFilter(_EXCEPTION_POINTERS* ExceptionInfo)
{
	HANDLE hFile = CreateFile(m_strDumpFile.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE) {
		MINIDUMP_EXCEPTION_INFORMATION ExInfo;
		ExInfo.ThreadId = ::GetCurrentThreadId();
		ExInfo.ExceptionPointers = ExceptionInfo;
		ExInfo.ClientPointers = FALSE;

		BOOL bOK = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &ExInfo, NULL, NULL);
		std::cout << bOK << std::endl;
		if (!bOK) {
			DWORD dw = GetLastError();
			return EXCEPTION_CONTINUE_SEARCH;
		}
		else {
			return EXCEPTION_EXECUTE_HANDLER;
		}
	}
	else {
		return EXCEPTION_CONTINUE_SEARCH;
	}
}

void CreateDump::DeclareDumpFile(string dmpFileName)
{
	SYSTEMTIME syt;
	GetLocalTime(&syt);
	char szTime[MAX_PATH];
	sprintf_s(szTime, MAX_PATH, "[%04d-%02d-%02dT%02d-%02d-%02d]", syt.wYear, syt.wMonth, syt.wDay, syt.wHour, syt.wMinute, syt.wSecond);
	m_strDumpFile = dmpFileName + string(szTime);
	m_strDumpFile += string(".dmp");
	std::cout << m_strDumpFile << std::endl;
	SetUnhandledExceptionFilter(UnhandleExceptionFilter);
}

CreateDump* CreateDump::Instance()
{
	if (!(*m_cdmpInstance)) {
		m_cdmpInstance = make_shared<CreateDump*>(new CreateDump);
	}
	return *m_cdmpInstance;
}