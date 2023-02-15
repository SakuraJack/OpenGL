#pragma once
#include <string>
#include <memory>
using namespace std;

class CreateDump
{
public:
	CreateDump();
	~CreateDump(void);
	static CreateDump* Instance();
	static long __stdcall UnhandleExceptionFilter(_EXCEPTION_POINTERS* ExceptionInfo);
	void DeclareDumpFile(string dumFileName = "");

private:
	static string m_strDumpFile;
	static shared_ptr<CreateDump*> m_cdmpInstance;
};

