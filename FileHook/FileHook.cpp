#include "FileHook.h"

BOOL WINAPI FileHook::FakeReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped) {
	BOOL ret = realReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
	std::unordered_map<HANDLE, Encryptor>::iterator it;
	if ((it = encryptorMap.find(hFile)) == encryptorMap.end())
		return ret;
	// DEBUG
	logger << "[FileHook][ReadFile] Will encrypt\n";
	it->second.EncryptBuffer(static_cast<unsigned char*>(lpBuffer), *lpNumberOfBytesRead);
	return ret;
}

BOOL WINAPI FileHook::FakeReadFileEx(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPOVERLAPPED lpOverlapped, LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine) {
	// DEBUG
	logger << "[FileHook][ReadFileEx] ReadFileEx API called but not implemented!\n";
	BOOL ret = realReadFileEx(hFile, lpBuffer, nNumberOfBytesToRead, lpOverlapped, lpCompletionRoutine);
	return ret;
}

BOOL WINAPI FileHook::FakeReadFileScatter(HANDLE hFile, FILE_SEGMENT_ELEMENT aSegmentArray[], DWORD nNumberOfBytesToRead, LPDWORD lpReserved, LPOVERLAPPED lpOverlapped) {
	// DEBUG
	logger << "[FileHook][ReadFileScatter]ReadFileScatter API called but not implemented!\n";
	BOOL ret = realReadFileScatter(hFile, aSegmentArray, nNumberOfBytesToRead, lpReserved, lpOverlapped);
	return ret;
}

BOOL WINAPI FileHook::FakeWriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped)
{
	std::unordered_map<HANDLE, Encryptor>::iterator it;
	if ((it = encryptorMap.find(hFile)) == encryptorMap.end())
		return realWriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
	// DEBUG
	logger << "[FileHook][WriteFile] Will decrypt\n";
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
	logger << "[FileHook][CreateFileW] CreateFileW API called!\n";
	HANDLE ret = realCreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	// DEBUG
	logger << "[FileHook][CreateFileW] FileName: " << lpFileName << "\n";

	wchar_t buffer[MAX_PATH];
	wchar_t relativeBuffer[MAX_PATH];

	if (ret == INVALID_HANDLE_VALUE) {
		// When API fails
		logger << "[FileHook][CreateFileW] API failed\n";
		return ret;
	}
	
	if (wcscmp(lpFileName, L"CONIN$") == 0 || wcscmp(lpFileName, L"CONOUT$") == 0)
		return ret;

	LPCWSTR path = lpFileName;
	if (dwCreationDisposition == OPEN_EXISTING && PathIsDirectoryW(path)) {
		// Open a directory
		logger << "[FileHook][CreateFileW] File is a directory: " << path << "\n";
		return ret;
	}

	if (PathIsRelativeW(lpFileName)) {
		GetFullPathNameW(lpFileName, MAX_PATH, buffer, nullptr);
		path = buffer;
	}
	logger << encryptBase << "; " << path << "\n";
	
	if (wcslen(path) >= wcslen(encryptBase) && wcsncmp(encryptBase, path, wcslen(encryptBase)) == 0 && PathIsPrefixW(encryptBase, path)) {
		BOOL result = PathRelativePathToW(relativeBuffer, encryptBase, FILE_ATTRIBUTE_DIRECTORY, path, FILE_ATTRIBUTE_NORMAL);
		if (!result) {
			logger << "[FileHook][CreateFileW] File" << path << " is not within EncryptBase\n";
			return ret;
		}

		encryptorMap[ret] = Encryptor(masterKey, relativeBuffer);
		logger << "[FileHook][CreateFileW] Add handler to encryptorMap: " << relativeBuffer << "\n";
	}
	return ret;
}

BOOL WINAPI FileHook::FakeCloseHandle(HANDLE hObject) {
	// DEBUG
	logger << "[FileHook][CloseHandle] CloseHandle API called!\n";
	BOOL ret = realCloseHandle(hObject);
	encryptorMap.erase(hObject);
	return ret;
}
