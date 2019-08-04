#include "FileHook.h"
#include <iostream>

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

BOOL WINAPI fakeWriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped) {
	__declspec(dllimport) FileHook *fileHook;
	return fileHook->FakeWriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
}

HANDLE WINAPI fakeCreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile) {
	__declspec(dllimport) FileHook *fileHook;
	return fileHook->FakeCreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

BOOL WINAPI fakeCloseHandle(HANDLE hObject) {
	__declspec(dllimport) FileHook *fileHook;
	return fileHook->FakeCloseHandle(hObject);
}


void FileHook::hookReadFile() {
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	realReadFile = (READFILE)DetourFindFunction("Kernel32.dll", "ReadFile");
	realReadFileEx = (READFILEEX)DetourFindFunction("Kernel32.dll", "ReadFileEx");
	realReadFileScatter = (READFILESCATTER)DetourFindFunction("Kernel32.dll", "ReadFileScatter");
	realWriteFile = (WRITEFILE)DetourFindFunction("Kernel32.dll", "WriteFile");
	realCreateFileW = (CREATEFILEW)DetourFindFunction("Kernel32.dll", "CreateFileW");
	realCloseHandle = (CLOSEHANDLE)DetourFindFunction("Kernel32.dll", "CloseHandle");
	HookFunction(realReadFile, fakeReadFile);
	HookFunction(realReadFileEx, fakeReadFileEx);
	HookFunction(realReadFileScatter, fakeReadFileScatter);
	//HookFunction(realWriteFile, fakeWriteFile);
	HookFunction(realCreateFileW, fakeCreateFileW);
	HookFunction(realCloseHandle, fakeCloseHandle);
	DetourTransactionCommit();
	std::cout << "ReadFile hooked" << std::endl;
}

void FileHook::unhookReadFile() {
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	UnhookFunction(realReadFile, fakeReadFile);
	UnhookFunction(realReadFileEx, fakeReadFileEx);
	UnhookFunction(realReadFileScatter, fakeReadFileScatter);
	UnhookFunction(realWriteFile, fakeWriteFile);
	UnhookFunction(realCreateFileW, fakeCreateFileW);
	UnhookFunction(realCloseHandle, fakeCloseHandle);
	DetourTransactionCommit();
}

BOOL WINAPI FileHook::FakeReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped) {
	std::cout << "ReadFile API called!" << std::endl;
	BOOL ret = realReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
	Encryptor *e = &encryptorMap[hFile];
	if (e != nullptr)
		e->EncryptBuffer(static_cast<unsigned char*>(lpBuffer), *lpNumberOfBytesRead);
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

BOOL WINAPI FileHook::FakeWriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped)
{
	std::cout << "WriteFile API called!" << std::endl;
	Encryptor *e = &encryptorMap[hFile];
	if (e == nullptr)
		return realWriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
	unsigned char *buffer = allocator.allocate(nNumberOfBytesToWrite);
	// TODO: the copy may unnecessary
	memcpy_s(buffer, nNumberOfBytesToWrite, lpBuffer, nNumberOfBytesToWrite);
	e->EncryptBuffer(buffer, nNumberOfBytesToWrite);
	BOOL ret = realWriteFile(hFile, buffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
	// e.seek(lpNumberOfBytesWritten - nNumberOfBytesToWrite);
	return ret;
}

HANDLE WINAPI FileHook::FakeCreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
	std::cout << "CreateFileW API called!" << std::endl;
	HANDLE ret = realCreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	encryptorMap[ret] = Encryptor(masterKey, lpFileName);
	return ret;
}

BOOL WINAPI FileHook::FakeCloseHandle(HANDLE hObject) {
	std::cout << "CloseHandle API called!" << std::endl;
	BOOL ret = realCloseHandle(hObject);
	encryptorMap.erase(hObject);
	return ret;
}


