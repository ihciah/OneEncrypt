#include <iostream>
#include <Windows.h>
#include <filesystem>
#include <string.h>

using namespace std;
void Inject();

char buffer[MAX_PATH];
int main(int argc, char* argv[])
{
	GetCurrentDirectory(MAX_PATH, buffer);
	Inject();
}

void Inject()
{
	LPCSTR ExePath = buffer[MAX_PATH] + "DriveMock.exe";
	LPCSTR DLLPath = buffer[MAX_PATH] + "FileHook.dll";

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));
	BOOL bCreateProccess = NULL;
	bCreateProccess = CreateProcess(
		ExePath,
		NULL,
		NULL,
		NULL,
		FALSE,
		CREATE_SUSPENDED,
		NULL,
		NULL,
		&si,
		&pi);

	void *page = VirtualAllocEx(pi.hProcess, NULL, MAX_PATH, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	WriteProcessMemory(pi.hProcess, page, DLLPath, strlen(DLLPath) + 1, NULL);

	HANDLE hThread = CreateRemoteThread(pi.hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, page, 0, NULL);

	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);

	ResumeThread(pi.hThread);

	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
	VirtualFreeEx(pi.hProcess, page, MAX_PATH, MEM_RELEASE);
	MessageBoxW(NULL, L"Loader Exit!", L"Loader Exit", MB_OK);
}
