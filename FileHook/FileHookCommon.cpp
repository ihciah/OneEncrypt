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
    configLoader.GetEncryptBasePath(encryptBasePath);
    configLoader.GetKey(key);
    logger << "[FileHook Boot ConfigLoader] " << "EncrpytBasePath: " << encryptBasePath << "; key: " << key << "\n"; // DEBUG

    int ret = crypto_pwhash(masterKey, crypto_stream_xchacha20_KEYBYTES, key, strlen(key), ZEROSALT, OPSLIMIT, MEMLIMIT, ALG);
    logger << "[FileHook Boot ConfigLoader] Calculate pwhash " << (ret == 0 ? "succeed" : "failed") << "\n"; // DEBUG
}

std::shared_ptr<Encryptor> FileHook::AddHandleEncryptor(const HANDLE h, const std::shared_ptr<Encryptor> e) {
    std::shared_lock lock(mutex_);
    encryptorMap_[h] = e;
}

void FileHook::RemoveHandleEncryptor(const HANDLE& h) {
    std::unique_lock lock(mutex_);
    encryptorMap_.erase(h);
}

std::shared_ptr<Encryptor> FileHook::GetHandleEncryptor(const HANDLE& h) {
    std::shared_lock lock(mutex_);
    if (auto it = encryptorMap_.find(h); it != encryptorMap_.end()) {
        logger << "[FileHook] GetHandleEncryptor invoke finished\n";
        return it->second;
    }
    return nullptr;
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

void FileHook::Hook() {
    DetourRestoreAfterWith();
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    realReadFile = (READFILE)DetourFindFunction("Kernel32.dll", "ReadFile");
    realReadFileEx = (READFILEEX)DetourFindFunction("Kernel32.dll", "ReadFileEx");
    realReadFileScatter = (READFILESCATTER)DetourFindFunction("Kernel32.dll", "ReadFileScatter");
    realCreateFileW = (CREATEFILEW)DetourFindFunction("Kernel32.dll", "CreateFileW");
    realCloseHandle = (CLOSEHANDLE)DetourFindFunction("Kernel32.dll", "CloseHandle");
    realWriteFile = (WRITEFILE)DetourFindFunction("Kernel32.dll", "WriteFile");
    HookFunction(realReadFile, fakeReadFile);
    HookFunction(realReadFileEx, fakeReadFileEx);
    HookFunction(realReadFileScatter, fakeReadFileScatter);
    HookFunction(realCreateFileW, fakeCreateFileW);
    HookFunction(realCloseHandle, fakeCloseHandle);
    HookFunction(realWriteFile, fakeWriteFile);
    DetourTransactionCommit();

    logger << "[FileHook Boot] Hooked\n";
}

void FileHook::Unhook() {
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    UnhookFunction(realReadFile, fakeReadFile);
    UnhookFunction(realReadFileEx, fakeReadFileEx);
    UnhookFunction(realReadFileScatter, fakeReadFileScatter);
    UnhookFunction(realCreateFileW, fakeCreateFileW);
    UnhookFunction(realCloseHandle, fakeCloseHandle);
    UnhookFunction(realWriteFile, fakeWriteFile);
    DetourTransactionCommit();
}
