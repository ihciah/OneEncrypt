#pragma once
#include <Windows.h>
#include <detours.h>
#include <detver.h>

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

class FileHook {
public:
	READFILE realReadFile = NULL;
	READFILEEX realReadFileEx = NULL;
	READFILESCATTER realReadFileScatter = NULL;

	void hookReadFile();
	void unhookReadFile();
	BOOL WINAPI FakeReadFile(HANDLE, LPVOID, DWORD, LPDWORD, LPOVERLAPPED);
	BOOL WINAPI FakeReadFileEx(HANDLE, LPVOID, DWORD, LPOVERLAPPED, LPOVERLAPPED_COMPLETION_ROUTINE);
	BOOL WINAPI FakeReadFileScatter(HANDLE, FILE_SEGMENT_ELEMENT[], DWORD, LPDWORD, LPOVERLAPPED);
};
