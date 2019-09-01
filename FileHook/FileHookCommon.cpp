#include "FileHook.h"
#include <iostream>
#include <fstream>
#include <locale>
#include <codecvt>

__declspec(dllimport) FileHook *fileHook;

static void ReportError(const TCHAR* errorMsg)
{
	MessageBox(NULL, errorMsg, NULL, MB_OK | MB_ICONERROR);
}

// Read configure file
FileHook::FileHook() {
	DetourRestoreAfterWith();
	realWriteFile = (WRITEFILE)DetourFindFunction("Kernel32.dll", "WriteFile");

	ConfigLoader configLoader(CONFIG_FILE);
	char key[MAX_KEY + 1];
	configLoader.GetEncryptBase(encryptBase);
	configLoader.GetKey(key);

	//// DEBUG
	//std::wofstream f("log.txt");
	//std::locale utf8_locale(f.getloc(), new std::codecvt_utf8<wchar_t>);
	//f.imbue(utf8_locale);
	//f << encryptBase;
	//f.close();

	int ret = crypto_pwhash(masterKey, crypto_stream_xchacha20_KEYBYTES, key, strlen(key), ZEROSALT, OPSLIMIT, MEMLIMIT, ALG);
	std::cout << "[FileHook] " << (ret == 0 ? "Pwhash succ" : "Pwhash fail") << std::endl; // DEBUG
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

void FileHook::println(const char s[]) {
	print(s);
	print("\n");
}

class chs_codecvt : public std::codecvt_byname<wchar_t, char, std::mbstate_t> {
public:
	chs_codecvt() : codecvt_byname("chs") { }
};

void FileHook::println(LPCWSTR w) {
	std::wstring_convert<chs_codecvt> converter;
	std::string string = converter.to_bytes(w);
	println(string.c_str());
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
	std::cout << "[FileHook] Read hooked" << std::endl;
}

void FileHook::hookWrite() {
	DetourRestoreAfterWith();
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	realWriteFile = (WRITEFILE)DetourFindFunction("Kernel32.dll", "WriteFile");
	HookFunction(realWriteFile, fakeWriteFile);
	DetourTransactionCommit();
	// DEBUG
	std::cout << "[FileHook] Write hooked" << std::endl;
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
