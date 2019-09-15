// Copyright 2019 ihciah <ihciah@gmail.com>
//
// Licensed under the GNU Affero General Public License, Version 3.0
// (the "License"); you may not use this file except in compliance with the
// License.
// You may obtain a copy of the License at
//
//     https://www.gnu.org/licenses/agpl-3.0.html
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "FileHook.h"
#include <fstream>
#include <locale>
#include <codecvt>

__declspec(dllimport) FileHook *fileHook;

/*
    Read configure file, load key and encryptBasePath
*/
FileHook::FileHook() {
    logger = std::make_unique<Logger>(L"log.txt");
    logger << "[FileHook Boot] " << "Start load config: " << CONFIG_FILE << "\n"; // DEBUG

    ConfigLoader configLoader(CONFIG_FILE);
    char key[MAX_KEY + 1];
    configLoader.GetEncryptBasePath(encryptBasePath);
    configLoader.GetKey(key);
    logger << "[FileHook Boot ConfigLoader] " << "EncrpytBasePath: " << encryptBasePath << "; key: " << key << "\n"; // DEBUG

    int ret = crypto_pwhash(masterKey, crypto_stream_xchacha20_KEYBYTES, key, strlen(key), ZEROSALT, OPSLIMIT, MEMLIMIT, ALG);
    logger << "[FileHook Boot ConfigLoader] Calculate password hash " << (ret == 0 ? "succeed" : "failed") << "\n"; // DEBUG
}

/*
    Add handle->shared_ptr<Encryptor> to hash map
*/
std::shared_ptr<Encryptor> FileHook::AddHandleEncryptor(const HANDLE h, const std::shared_ptr<Encryptor> e) {
    std::shared_lock lock(mutex_);
    encryptorMap_[h] = e;
}

/*
    Remove handle->shared_ptr<Encryptor> from hash map if exists
*/
void FileHook::RemoveHandleEncryptor(const HANDLE& h) {
    std::unique_lock lock(mutex_);
    encryptorMap_.erase(h);
}

/*
    Get shared_ptr<Encryptor> from hash map if exists, otherwise nullptr
*/
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

BOOL WINAPI fakeSetFilePointer(HANDLE hFile, LONG lDistanceToMove, PLONG lpDistanceToMoveHigh, DWORD dwMoveMethod) {
    return fileHook->FakeSetFilePointer(hFile, lDistanceToMove, lpDistanceToMoveHigh, dwMoveMethod);
}

BOOL WINAPI fakeSetFilePointerEx(HANDLE hFile, LARGE_INTEGER liDistanceToMove, PLARGE_INTEGER lpNewFilePointer, DWORD dwMoveMethod) {
    return fileHook->FakeSetFilePointerEx(hFile, liDistanceToMove, lpNewFilePointer, dwMoveMethod);
}

DWORD WINAPI fakeGetFileSize(HANDLE hFile, LPDWORD lpFileSizeHigh) {
    return fileHook->FakeGetFileSize(hFile, lpFileSizeHigh);
}

BOOL WINAPI fakeGetFileSizeEx(HANDLE hFile, PLARGE_INTEGER lpFileSize) {
    return fileHook->FakeGetFileSizeEx(hFile, lpFileSize);
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
    realSetFilePointer = (SETFILEPOINTER)DetourFindFunction("Kernel32.dll", "SetFilePointer");
    realSetFilePointerEx = (SETFILEPOINTEREX)DetourFindFunction("Kernel32.dll", "SetFilePointerEx");
    realGetFileSize = (GETFILESIZE)DetourFindFunction("Kernel32.dll", "GetFileSize");
    realGetFileSizeEx = (GETFILESIZEEX)DetourFindFunction("Kernel32.dll", "GetFileSizeEx");
    HookFunction(realReadFile, fakeReadFile);
    HookFunction(realReadFileEx, fakeReadFileEx);
    HookFunction(realReadFileScatter, fakeReadFileScatter);
    HookFunction(realCreateFileW, fakeCreateFileW);
    HookFunction(realCloseHandle, fakeCloseHandle);
    HookFunction(realWriteFile, fakeWriteFile);
    HookFunction(realSetFilePointer, fakeSetFilePointer);
    HookFunction(realSetFilePointerEx, fakeSetFilePointerEx);
    HookFunction(realGetFileSize, fakeGetFileSize);
    HookFunction(realGetFileSizeEx, fakeGetFileSizeEx);
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
    UnhookFunction(realSetFilePointer, fakeSetFilePointer);
    UnhookFunction(realSetFilePointerEx, fakeSetFilePointerEx);
    UnhookFunction(realGetFileSize, fakeGetFileSize);
    UnhookFunction(realGetFileSizeEx, fakeGetFileSizeEx);
    DetourTransactionCommit();
}
