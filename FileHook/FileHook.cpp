#include "FileHook.h"

BOOL WINAPI FileHook::FakeReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped) {
	println("ReadFile API called!");
	if (lpOverlapped == nullptr) println("noOverlapped"); else println("withOverlapped"); // DEBUG
	BOOL ret = realReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
	std::unordered_map<HANDLE, Encryptor>::iterator it;
	if ((it = encryptorMap.find(hFile)) == encryptorMap.end())
		return ret;
	println("will encrypt");
	it->second.EncryptBuffer(static_cast<unsigned char*>(lpBuffer), *lpNumberOfBytesRead);
	return ret;
}

BOOL WINAPI FileHook::FakeReadFileEx(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPOVERLAPPED lpOverlapped, LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine) {
	println("[Not Impl]ReadFileEx API called!");
	BOOL ret = realReadFileEx(hFile, lpBuffer, nNumberOfBytesToRead, lpOverlapped, lpCompletionRoutine);
	return ret;
}

BOOL WINAPI FileHook::FakeReadFileScatter(HANDLE hFile, FILE_SEGMENT_ELEMENT aSegmentArray[], DWORD nNumberOfBytesToRead, LPDWORD lpReserved, LPOVERLAPPED lpOverlapped) {
	println("[Not Impl]ReadFileScatter API called!");
	BOOL ret = realReadFileScatter(hFile, aSegmentArray, nNumberOfBytesToRead, lpReserved, lpOverlapped);
	return ret;
}

BOOL WINAPI FileHook::FakeWriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped)
{
	println("WriteFile API called!");
	std::unordered_map<HANDLE, Encryptor>::iterator it;
	if ((it = encryptorMap.find(hFile)) == encryptorMap.end())
		return realWriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
	println("will decrypt");
	unsigned char *buffer = allocator.allocate(nNumberOfBytesToWrite);
	// TODO: the copy may unnecessary
	memcpy_s(buffer, nNumberOfBytesToWrite, lpBuffer, nNumberOfBytesToWrite);
	it->second.EncryptBuffer(buffer, nNumberOfBytesToWrite);
	BOOL ret = realWriteFile(hFile, buffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
	// e.seek(lpNumberOfBytesWritten - nNumberOfBytesToWrite);
	return ret;
}

HANDLE WINAPI FileHook::FakeCreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
	print("CreateFileW API called: ");
	HANDLE ret = realCreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	println(lpFileName);
	if (wcsstr(lpFileName, L"testout.txt") != nullptr)
		encryptorMap[ret] = Encryptor(masterKey, lpFileName);
	return ret;
}

BOOL WINAPI FileHook::FakeCloseHandle(HANDLE hObject) {
	println("CloseHandle API called!");
	BOOL ret = realCloseHandle(hObject);
	encryptorMap.erase(hObject);
	return ret;
}
