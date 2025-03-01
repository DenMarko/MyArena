#include "CMain.h"
#define _STDINT // ~.~
#include "client/windows/handler/exception_handler.h"

#define my_jmp_buf jmp_buf
#define my_setjmp(x) setjmp(x)
#define my_longjmp longjmp

void LogFatal(const wchar_t *szMsg)
{
	FILE *f = nullptr;
	time_t t;
	wchar_t header[128];

	fopen_s(&f, "fatal.log", "at");
	if (!f)
	{
		return;
	}

	t = time(nullptr);
	tm pTm;
	localtime_s(&pTm, &t);
	wcsftime(header, sizeof header, L"%m/%d/%Y - %H:%M:%S", &pTm);
	
	wchar_t buffer[1024];
	Utilite::Format(buffer, sizeof buffer, L"%ls %ls", header, szMsg);

	fwprintf(f, L"%ls", buffer);
	OutputDebugStringW(buffer);

	fclose(f);
}

using namespace google_breakpad;

void *vectoredHandler = NULL;
ExceptionHandler *g_Handle = nullptr;

LONG CALLBACK BreakpadVectoredHandler(_In_ PEXCEPTION_POINTERS ExceptionInfo)
{
	switch (ExceptionInfo->ExceptionRecord->ExceptionCode)
	{
	case EXCEPTION_ACCESS_VIOLATION:
	case EXCEPTION_INVALID_HANDLE:
	case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
	case EXCEPTION_DATATYPE_MISALIGNMENT:
	case EXCEPTION_ILLEGAL_INSTRUCTION:
	case EXCEPTION_INT_DIVIDE_BY_ZERO:
	case EXCEPTION_STACK_OVERFLOW:
	case 0xC0000409: // STATUS_STACK_BUFFER_OVERRUN
	case 0xC0000374: // STATUS_HEAP_CORRUPTION
		break;
	case 0: // Valve use this for Sys_Error.
		if ((ExceptionInfo->ExceptionRecord->ExceptionFlags & EXCEPTION_NONCONTINUABLE) == 0)
			return EXCEPTION_CONTINUE_SEARCH;
		break;
	default:
		return EXCEPTION_CONTINUE_SEARCH;
	}

	if (g_Handle->WriteMinidumpForException(ExceptionInfo))
	{
		// Stop the handler thread from deadlocking us.
		delete g_Handle;

		// Stop Valve's handler being called.
		ExceptionInfo->ExceptionRecord->ExceptionCode = EXCEPTION_BREAKPOINT;

		return EXCEPTION_EXECUTE_HANDLER;
	} else {
		return EXCEPTION_CONTINUE_SEARCH;
	}
}


static bool dumpCallBack(const wchar_t* dump_path, const wchar_t *minidump_id, void *context, EXCEPTION_POINTERS* ex_info, MDRawAssertionInfo* assertion, bool succeeded)
{
	if (!succeeded)
	{
		printf("Failed to write minidump to: %ls\\%ls.dmp\n", dump_path, minidump_id);
		return succeeded;
	}
	printf("Wrote minidump to: %ls\\%ls.dmp\n", dump_path, minidump_id);

	return succeeded;
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR pCommandLine, int nCommandShow)
{
	int result = -1;

	g_Handle = new ExceptionHandler(L"dumps", NULL, dumpCallBack, NULL, ExceptionHandler::HANDLER_ALL, 
									static_cast<MINIDUMP_TYPE>(MiniDumpWithUnloadedModules | MiniDumpWithFullMemoryInfo | MiniDumpWithThreadInfo | MiniDumpWithCodeSegs), 
									static_cast<const wchar_t *>(NULL), NULL);

	vectoredHandler = AddVectoredExceptionHandler(0, BreakpadVectoredHandler);

	try
	{
		SpaceMain::CMain app(hInst, 1280, 800);
		auto result = app.Loop();
	}
	catch( const CBaseException& e )
	{
		const std::wstring eMsg = e.GetFullMessage() + L"\nException caught at Windows message loop.\n";
		const std::wstring eType = e.GetExceptionType() + L": " + eMsg;

		LogFatal(eType.c_str());
	}
	catch (const std::exception& e)
	{
		std::string whatStr(e.what());

		const std::wstring eMsg = L"Unhandled STL Exception: " + std::wstring(whatStr.begin(), whatStr.end()) + L"\nException caught at Windows message loop.\n";
		LogFatal(eMsg.c_str());
	}
	catch( ... )
	{
		LogFatal(L"Unhandled Non-STL Exception:\nException caught at Windows message loop.\n");
	}

	if(vectoredHandler)
		RemoveVectoredExceptionHandler(vectoredHandler);

	delete g_Handle;
	return result;
}

