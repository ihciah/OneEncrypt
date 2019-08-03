#include "FileHook.h"
#include <iostream>
#include <functional>

BOOL WINAPI fakeReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped) {
	__declspec(dllimport) FileHook *fileHook;
	return fileHook->FakeReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
}

BOOL WINAPI fakeReadFileEx(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPOVERLAPPED lpOverlapped, LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine) {
	__declspec(dllimport) FileHook *fileHook;
	return fileHook->FakeReadFileEx(hFile, lpBuffer, nNumberOfBytesToRead, lpOverlapped, lpCompletionRoutine);
}

BOOL WINAPI fakeReadFileScatter(HANDLE hFile, FILE_SEGMENT_ELEMENT aSegmentArray[], DWORD nNumberOfBytesToRead, LPDWORD lpReserved, LPOVERLAPPED lpOverlapped) {
	__declspec(dllimport) FileHook *fileHook;
	return fileHook->FakeReadFileScatter(hFile, aSegmentArray, nNumberOfBytesToRead, lpReserved, lpOverlapped);
}


void FileHook::hookReadFile() {
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	realReadFile = (READFILE)DetourFindFunction("Kernel32.dll", "ReadFile");
	HookFunction(realReadFile, fakeReadFile);
	DetourTransactionCommit();
	std::cout << "ReadFile hooked" << std::endl;
}

void FileHook::unhookReadFile() {
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	UnhookFunction(realReadFile, fakeReadFile);
	DetourTransactionCommit();
}

BOOL WINAPI FileHook::FakeReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped) {
	std::cout << "ReadFile API called!" << std::endl;
	BOOL ret = realReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
	for (unsigned int i = 0; i < *lpNumberOfBytesRead; i++) {
		*((char*)lpBuffer + i) = (*((char*)lpBuffer + i)) ^ 0x02;
	}
	return ret;
}

BOOL WINAPI FileHook::FakeReadFileEx(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPOVERLAPPED lpOverlapped, LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine) {
	std::cout << "ReadFileEx API called!" << std::endl;
	BOOL ret = realReadFileEx(hFile, lpBuffer, nNumberOfBytesToRead, lpOverlapped, lpCompletionRoutine);
	return ret;
}

BOOL WINAPI FileHook::FakeReadFileScatter(HANDLE hFile, FILE_SEGMENT_ELEMENT aSegmentArray[], DWORD nNumberOfBytesToRead, LPDWORD lpReserved, LPOVERLAPPED lpOverlapped) {
	std::cout << "ReadFileScatter API called!" << std::endl;
	BOOL ret = realReadFileScatter(hFile, aSegmentArray, nNumberOfBytesToRead, lpReserved, lpOverlapped);
	return ret;
}
