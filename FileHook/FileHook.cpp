#include "FileHook.h"

static void ReportError(const TCHAR* errorMsg)
{
	MessageBox(NULL, errorMsg, NULL, MB_OK | MB_ICONERROR);
}

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

FileHook::FileHook() {
	DetourRestoreAfterWith();
	realWriteFile = (WRITEFILE)DetourFindFunction("Kernel32.dll", "WriteFile");
}

void FileHook::print(const char s[]) {
	DWORD written_b;
	HANDLE outH = GetStdHandle(STD_OUTPUT_HANDLE);
	if (!outH) {
		ReportError(TEXT("No standard handles associated with this app."));
	} else if (outH == INVALID_HANDLE_VALUE) {
		TCHAR errMsg[100];
		wsprintf(errMsg, TEXT("GetStdHandle() failed with error code %lu"), GetLastError());
		ReportError(errMsg);
	} else {
		if (!realWriteFile(outH, s, strlen(s), &written_b, NULL))
		{
			TCHAR errMsg[100];
			wsprintf(errMsg, TEXT("WriteFile() failed with error code %lu"), GetLastError());
			ReportError(errMsg);
		}
	}
}

void FileHook::println(const char s[]) {
	print(s);
	print("\n");
}


void FileHook::hookRead() {
	DetourRestoreAfterWith();
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	realReadFile = (READFILE)DetourFindFunction("Kernel32.dll", "ReadFile");
	realReadFileEx = (READFILEEX)DetourFindFunction("Kernel32.dll", "ReadFileEx");
	realReadFileScatter = (READFILESCATTER)DetourFindFunction("Kernel32.dll", "ReadFileScatter");
	realCreateFileW = (CREATEFILEW)DetourFindFunction("Kernel32.dll", "CreateFileW");
	realCloseHandle = (CLOSEHANDLE)DetourFindFunction("Kernel32.dll", "CloseHandle");
	HookFunction(realReadFile, fakeReadFile);
	HookFunction(realReadFileEx, fakeReadFileEx);
	HookFunction(realReadFileScatter, fakeReadFileScatter);
	HookFunction(realCreateFileW, fakeCreateFileW);
	HookFunction(realCloseHandle, fakeCloseHandle);
	DetourTransactionCommit();
	println("Read hooked");
}

void FileHook::hookWrite() {
	DetourRestoreAfterWith();
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	realWriteFile = (WRITEFILE)DetourFindFunction("Kernel32.dll", "WriteFile");
	HookFunction(realWriteFile, fakeWriteFile);
	DetourTransactionCommit();
	println("Write hooked");
}

void FileHook::unhookRead() {
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	UnhookFunction(realReadFile, fakeReadFile);
	UnhookFunction(realReadFileEx, fakeReadFileEx);
	UnhookFunction(realReadFileScatter, fakeReadFileScatter);
	UnhookFunction(realCreateFileW, fakeCreateFileW);
	UnhookFunction(realCloseHandle, fakeCloseHandle);
	DetourTransactionCommit();
}

void FileHook::unhookWrite() {
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	UnhookFunction(realWriteFile, fakeWriteFile);
	DetourTransactionCommit();
}

BOOL WINAPI FileHook::FakeReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped) {
	println("ReadFile API called!");
	BOOL ret = realReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
	std::unordered_map<HANDLE, Encryptor>::iterator it;
	if ((it = encryptorMap.find(hFile)) == encryptorMap.end())
		return ret;
	println("will encrypt");
	it->second.EncryptBuffer(static_cast<unsigned char*>(lpBuffer), *lpNumberOfBytesRead);
	return ret;
}

BOOL WINAPI FileHook::FakeReadFileEx(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPOVERLAPPED lpOverlapped, LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine) {
	println("ReadFileEx API called!");
	BOOL ret = realReadFileEx(hFile, lpBuffer, nNumberOfBytesToRead, lpOverlapped, lpCompletionRoutine);
	return ret;
}

BOOL WINAPI FileHook::FakeReadFileScatter(HANDLE hFile, FILE_SEGMENT_ELEMENT aSegmentArray[], DWORD nNumberOfBytesToRead, LPDWORD lpReserved, LPOVERLAPPED lpOverlapped) {
	println("ReadFileScatter API called!");
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
	println("CreateFileW API called!");
	HANDLE ret = realCreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
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


