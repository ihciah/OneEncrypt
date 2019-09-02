#pragma once
#include "common.h"
#include <detours.h>
#include <detver.h>
#include <wchar.h>
#include <unordered_map>
#include <functional>
#include <utility>
#include <string>
#include "Encryptor.h"
#include "ConfigLoader.h"
#include "Logger.h"

template<typename T>
void HookFunction(T &fn_real, _In_ PVOID fn_mine)
{
	DetourAttach(&(PVOID&)fn_real, fn_mine);
}

template<typename T>
void UnhookFunction(T &fn_real, _In_ PVOID fn_mine)
{
	DetourDetach(&(PVOID&)fn_real, fn_mine);
}

typedef BOOL(WINAPI *READFILE) (HANDLE, LPVOID, DWORD, LPDWORD, LPOVERLAPPED);
typedef BOOL(WINAPI *READFILEEX)(HANDLE, LPVOID, DWORD, LPOVERLAPPED, LPOVERLAPPED_COMPLETION_ROUTINE);
typedef BOOL(WINAPI *READFILESCATTER)(HANDLE, FILE_SEGMENT_ELEMENT[], DWORD, LPDWORD, LPOVERLAPPED);
typedef BOOL(WINAPI *WRITEFILE)(HANDLE, LPCVOID, DWORD, LPDWORD, LPOVERLAPPED);
typedef HANDLE(WINAPI *CREATEFILEW)(LPCWSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);
typedef BOOL(WINAPI *CLOSEHANDLE)(HANDLE);

class FileHook {
private:
	std::unordered_map<HANDLE, Encryptor> encryptorMap;
	std::allocator<unsigned char> allocator;
	wchar_t encryptBase[MAX_PATH];
	unsigned char masterKey[crypto_stream_xchacha20_KEYBYTES];

public:
	FileHook();
	std::unique_ptr<Logger> logger;

	READFILE realReadFile = nullptr;
	READFILEEX realReadFileEx = nullptr;
	READFILESCATTER realReadFileScatter = nullptr;
	WRITEFILE realWriteFile = nullptr;
	CREATEFILEW realCreateFileW = nullptr;
	CLOSEHANDLE realCloseHandle = nullptr;

	void hookRead();
	void unhookRead();
	void hookWrite();
	void unhookWrite();
	BOOL WINAPI FakeReadFile(HANDLE, LPVOID, DWORD, LPDWORD, LPOVERLAPPED);
	BOOL WINAPI FakeReadFileEx(HANDLE, LPVOID, DWORD, LPOVERLAPPED, LPOVERLAPPED_COMPLETION_ROUTINE);
	BOOL WINAPI FakeReadFileScatter(HANDLE, FILE_SEGMENT_ELEMENT[], DWORD, LPDWORD, LPOVERLAPPED);
	BOOL WINAPI FakeWriteFile(HANDLE, LPCVOID, DWORD, LPDWORD, LPOVERLAPPED);
	HANDLE WINAPI FakeCreateFileW(LPCWSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);
	BOOL WINAPI FakeCloseHandle(HANDLE);
};
