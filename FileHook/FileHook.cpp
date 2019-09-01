#include "FileHook.h"
#include <iostream>

BOOL WINAPI FileHook::FakeReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped) {
	// DEBUG
	std::cout << "[FileHook][ReadFile] ReadFile API called!";
	if (lpOverlapped == nullptr) 
		std::cout << "noOverlapped" << std::endl; 
	else 
		std::cout << "withOverlapped" << std::endl;
	BOOL ret = realReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
	std::unordered_map<HANDLE, Encryptor>::iterator it;
	if ((it = encryptorMap.find(hFile)) == encryptorMap.end())
		return ret;
	// DEBUG
	std::cout << "[FileHook][ReadFile] Will encrypt" << std::endl;
	it->second.EncryptBuffer(static_cast<unsigned char*>(lpBuffer), *lpNumberOfBytesRead);
	return ret;
}

BOOL WINAPI FileHook::FakeReadFileEx(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPOVERLAPPED lpOverlapped, LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine) {
	// DEBUG
	std::cout << "[FileHook][ReadFileEx] ReadFileEx API called but not implemented!" << std::endl;
	BOOL ret = realReadFileEx(hFile, lpBuffer, nNumberOfBytesToRead, lpOverlapped, lpCompletionRoutine);
	return ret;
}

BOOL WINAPI FileHook::FakeReadFileScatter(HANDLE hFile, FILE_SEGMENT_ELEMENT aSegmentArray[], DWORD nNumberOfBytesToRead, LPDWORD lpReserved, LPOVERLAPPED lpOverlapped) {
	// DEBUG
	std::cout << "[FileHook][ReadFileScatter]ReadFileScatter API called but not implemented!" << std::endl;
	BOOL ret = realReadFileScatter(hFile, aSegmentArray, nNumberOfBytesToRead, lpReserved, lpOverlapped);
	return ret;
}

BOOL WINAPI FileHook::FakeWriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped)
{
	// DEBUG
	println("[FileHook][WriteFile] WriteFile API called!");
	std::unordered_map<HANDLE, Encryptor>::iterator it;
	if ((it = encryptorMap.find(hFile)) == encryptorMap.end())
		return realWriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
	// DEBUG
	std::cout << "[FileHook][WriteFile] Will decrypt" << std::endl;
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
	// DEBUG
	println("[FileHook][CreateFileW] CreateFileW API called!");
	HANDLE ret = realCreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	// DEBUG
	print("[FileHook][CreateFileW] FileName: ");
	println(lpFileName);

	wchar_t buffer[MAX_PATH];
	wchar_t relativeBuffer[MAX_PATH];

	if (ret == INVALID_HANDLE_VALUE) {
		// When API fails
		return ret;
	}
	
	if (wcscmp(lpFileName, L"CONIN$") == 0 || wcscmp(lpFileName, L"CONOUT$") == 0)
		// STDIN and STDOUT
		return ret;

	LPCWSTR path = lpFileName;
	if (dwCreationDisposition == OPEN_EXISTING && PathFileExistsW(path)) {
		// Maybe open a directory
		return ret;
	}

	if (PathIsRelativeW(lpFileName)) {
		GetFullPathNameW(lpFileName, MAX_PATH, buffer, nullptr);
		path = buffer;
	}
	if (PathIsPrefixW(encryptBase, path)) {
		BOOL result = PathRelativePathToW(relativeBuffer, encryptBase, FILE_ATTRIBUTE_DIRECTORY, path, FILE_ATTRIBUTE_NORMAL);
		if (!result) return ret;

		encryptorMap[ret] = Encryptor(masterKey, relativeBuffer);
	}
	return ret;
}

BOOL WINAPI FileHook::FakeCloseHandle(HANDLE hObject) {
	// DEBUG
	println("[FileHook][CloseHandle] CloseHandle API called!");
	BOOL ret = realCloseHandle(hObject);
	encryptorMap.erase(hObject);
	return ret;
}
