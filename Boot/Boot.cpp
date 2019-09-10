#include <iostream>
#include <Windows.h>
#include <filesystem>
#include <string.h>
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

using namespace std;

errno_t Inject(LPSTR path);
errno_t Inject(LPWSTR path);
errno_t GetFilePath(LPSTR dest);
errno_t GetFilePath(LPWSTR dest);

int main(int argc, char* argv[])
{
	char buffer[MAX_PATH];
	if (GetFilePath(buffer) == 0 && Inject(buffer) == 0) {
		MessageBoxW(NULL, L"[BootLoader] Loader run successfully!", L"Loader Exit", MB_OK);
		return 0;
	}
	MessageBoxW(NULL, L"[BootLoader] Loader run fail!", L"Loader Exit", MB_OK);
	return 1;
}

errno_t GetFilePath(LPSTR dest)
{
	if (dest == nullptr) return 1;
	DWORD length = GetModuleFileNameA(NULL, dest, MAX_PATH);
	PathRemoveFileSpecA(dest);
	return 0;
}

errno_t GetFilePath(LPWSTR dest)
{
	if (dest == nullptr) return 1;
	DWORD length = GetModuleFileNameW(NULL, dest, MAX_PATH);
	PathRemoveFileSpecW(dest);
	return 0;
}

errno_t Inject(LPSTR path)
{
	if (path == nullptr) return 1;
	char ExecPath[MAX_PATH], DLLPath[MAX_PATH];
	PathCombineA(ExecPath, path, "DriveMock.exe");
	PathCombineA(DLLPath, path, "FileHook.dll");

	cout << "[BootLoader] Exec path: " << ExecPath << endl;
	cout << "[BootLoader] DLL path: " << DLLPath << endl;

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));
	BOOL bCreateProccess = NULL;
	bCreateProccess = CreateProcessA(
		ExecPath,
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
	return 0;
}

errno_t Inject(LPWSTR path)
{
	if (path == nullptr) return 1;
	wchar_t ExecPath[MAX_PATH], DLLPath[MAX_PATH];
	PathCombineW(ExecPath, path, L"DriveMock.exe");
	PathCombineW(DLLPath, path, L"FileHook.dll");

	wcout << "[BootLoader] Exec path: " << ExecPath << endl;
	wcout << "[BootLoader] DLL path: " << DLLPath << endl;

	STARTUPINFOW si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));
	BOOL bCreateProccess = NULL;
	bCreateProccess = CreateProcessW(
		ExecPath,
		NULL,
		NULL,
		NULL,
		FALSE,
		CREATE_SUSPENDED,
		NULL,
		NULL,
		&si,
		&pi);

	void *page = VirtualAllocEx(pi.hProcess, NULL, MAX_PATH * 2, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	WriteProcessMemory(pi.hProcess, page, DLLPath, (wcslen(DLLPath) + 1) * sizeof(wchar_t), NULL);

	HANDLE hThread = CreateRemoteThread(pi.hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)LoadLibraryW, page, 0, NULL);

	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);

	ResumeThread(pi.hThread);

	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
	VirtualFreeEx(pi.hProcess, page, MAX_PATH * 2, MEM_RELEASE);
	return 0;
}
