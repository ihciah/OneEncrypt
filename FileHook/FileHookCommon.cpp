#include "FileHook.h"
#include <fstream>
#include <locale>
#include <codecvt>

__declspec(dllimport) FileHook *fileHook;

// Read configure file
FileHook::FileHook() {
	DetourRestoreAfterWith();

	logger = std::make_unique<Logger>(L"log.txt");
	logger << "[FileHook Boot] " << "Start load config: " << CONFIG_FILE << "\n"; // DEBUG

	ConfigLoader configLoader(CONFIG_FILE);
	char key[MAX_KEY + 1];
	configLoader.GetEncryptBase(encryptBase);
	configLoader.GetKey(key);
	logger << "[FileHook Boot ConfigLoader] " << "EncrpytBase: " << encryptBase << "; key: " << key << "\n"; // DEBUG

	int ret = crypto_pwhash(masterKey, crypto_stream_xchacha20_KEYBYTES, key, strlen(key), ZEROSALT, OPSLIMIT, MEMLIMIT, ALG);
	logger << "[FileHook Boot ConfigLoader] " << (ret == 0 ? "Pwhash succ" : "Pwhash fail") << "\n"; // DEBUG
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
	// DEBUG
	logger << "[FileHook Boot] Read hooked\n";
}

void FileHook::hookWrite() {
	DetourRestoreAfterWith();
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	realWriteFile = (WRITEFILE)DetourFindFunction("Kernel32.dll", "WriteFile");
	HookFunction(realWriteFile, fakeWriteFile);
	DetourTransactionCommit();
	// DEBUG
	logger << "[FileHook Boot] Write hooked\n";
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
