#include "FileHook.h"

__declspec(dllexport) FileHook *fileHook;

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		fileHook = new FileHook();
		fileHook->hookRead();
		fileHook->hookWrite();
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		fileHook->unhookRead();
		fileHook->unhookWrite();
		free(fileHook);
		break;
	}
	return TRUE;
}

