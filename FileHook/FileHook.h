#pragma once
#include "common.h"
#include <detours.h>
#include <detver.h>
#include <wchar.h>
#include <unordered_map>
#include <functional>
#include <utility>
#include <string>
#include <shared_mutex>
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

using READFILE = BOOL(WINAPI *) (HANDLE, LPVOID, DWORD, LPDWORD, LPOVERLAPPED);
using READFILEEX = BOOL(WINAPI *)(HANDLE, LPVOID, DWORD, LPOVERLAPPED, LPOVERLAPPED_COMPLETION_ROUTINE);
using READFILESCATTER = BOOL(WINAPI *)(HANDLE, FILE_SEGMENT_ELEMENT[], DWORD, LPDWORD, LPOVERLAPPED);
using WRITEFILE = BOOL(WINAPI *)(HANDLE, LPCVOID, DWORD, LPDWORD, LPOVERLAPPED);
using CREATEFILEW = HANDLE(WINAPI *)(LPCWSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);
using CLOSEHANDLE = BOOL(WINAPI *)(HANDLE);

class FileHook {
private:
	mutable std::shared_timed_mutex mutex_;
	std::unordered_map<HANDLE, std::shared_ptr<Encryptor>> encryptorMap_;
	std::allocator<unsigned char> allocator;
	wchar_t encryptBase[MAX_PATH];
	unsigned char masterKey[crypto_stream_xchacha20_KEYBYTES];

	std::shared_ptr<Encryptor> AddHandle(const HANDLE, const std::shared_ptr<Encryptor>);
	void RemoveHandle(const HANDLE&);
	std::shared_ptr<Encryptor> GetHandleEncryptor(const HANDLE&);

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
