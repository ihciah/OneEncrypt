#include "FileHook.h"

__declspec(dllimport) FileHook *fileHook;

const unsigned char ZEROSALT[crypto_pwhash_SALTBYTES] = { 0 };
const auto OPSLIMIT = crypto_pwhash_OPSLIMIT_MIN;
const auto MEMLIMIT = crypto_pwhash_MEMLIMIT_MIN;
const auto ALG = crypto_pwhash_ALG_DEFAULT;

static void ReportError(const TCHAR* errorMsg)
{
	MessageBox(NULL, errorMsg, NULL, MB_OK | MB_ICONERROR);
}

errno_t GetConfigPath(char* dest)
{
	WCHAR pwd[MAX_PATH];
	DWORD length = GetModuleFileName(NULL, pwd, MAX_PATH);
	PathCchRemoveFileSpec(pwd, MAX_PATH);
	PathCchCombine(pwd, MAX_PATH, pwd, L"config.ini");

	size_t converted;
	return wcstombs_s(&converted, dest, MAX_PATH, pwd, MAX_PATH);
}

// Read configure file
FileHook::FileHook() {
	DetourRestoreAfterWith();
	realWriteFile = (WRITEFILE)DetourFindFunction("Kernel32.dll", "WriteFile");

	print("Trying to get current");
	char path[MAX_PATH];
	GetConfigPath(path);

	INIReader reader(path);
	auto key = reader.GetString("global", "key", "");
	print("loaded key:");
	println(key.c_str());

	int ret = crypto_pwhash(masterKey, crypto_stream_xchacha20_KEYBYTES, key.c_str(), key.size(), ZEROSALT, OPSLIMIT, MEMLIMIT, ALG);
	print(ret == 0 ? "pwhash succ" : "pwhash fail"); // DEBUG
}

void FileHook::print(const char s[]) {
	DWORD written_b;
	HANDLE outH = GetStdHandle(STD_OUTPUT_HANDLE);
	if (!outH) {
		ReportError(TEXT("No standard handles associated with this app."));
	}
	else if (outH == INVALID_HANDLE_VALUE) {
		TCHAR errMsg[100];
		wsprintf(errMsg, TEXT("GetStdHandle() failed with error code %lu"), GetLastError());
		ReportError(errMsg);
	}
	else {
		if (!realWriteFile(outH, s, strlen(s), &written_b, NULL))
		{
			TCHAR errMsg[100];
			wsprintf(errMsg, TEXT("WriteFile() failed with error code %lu"), GetLastError());
			ReportError(errMsg);
		}
	}
}

void FileHook::print(const LPCWSTR s) {
	DWORD written_b;
	HANDLE outH = GetStdHandle(STD_OUTPUT_HANDLE);
	if (!outH) {
		ReportError(TEXT("No standard handles associated with this app."));
	}
	else if (outH == INVALID_HANDLE_VALUE) {
		TCHAR errMsg[100];
		wsprintf(errMsg, TEXT("GetStdHandle() failed with error code %lu"), GetLastError());
		ReportError(errMsg);
	}
	else {
		if (!realWriteFile(outH, s, wcslen(s) * sizeof(WCHAR), &written_b, NULL))
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

void FileHook::println(const LPCWSTR s) {
	print(s);
	print("\n");
}

BOOL WINAPI fakeReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped) {
	return fileHook->FakeReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
}

BOOL WINAPI fakeReadFileEx(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPOVERLAPPED lpOverlapped, LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine) {
	return fileHook->FakeReadFileEx(hFile, lpBuffer, nNumberOfBytesToRead, lpOverlapped, lpCompletionRoutine);
}

BOOL WINAPI fakeReadFileScatter(HANDLE hFile, FILE_SEGMENT_ELEMENT aSegmentArray[], DWORD nNumberOfBytesToRead, LPDWORD lpReserved, LPOVERLAPPED lpOverlapped) {
	return fileHook->FakeReadFileScatter(hFile, aSegmentArray, nNumberOfBytesToRead, lpReserved, lpOverlapped);
}

BOOL WINAPI fakeWriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped) {
	return fileHook->FakeWriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
}

HANDLE WINAPI fakeCreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile) {
	return fileHook->FakeCreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

BOOL WINAPI fakeCloseHandle(HANDLE hObject) {
	return fileHook->FakeCloseHandle(hObject);
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
