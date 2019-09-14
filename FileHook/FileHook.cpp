#include "FileHook.h"

BOOL WINAPI FileHook::FakeReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped) {
    auto ret = realReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);

    if (auto enc = GetHandleEncryptor(hFile); enc != nullptr) {
        logger << "[FileHook][ReadFile] Will encrypt\n";
        enc->EncryptBuffer(static_cast<unsigned char*>(lpBuffer), *lpNumberOfBytesRead);
    }
    return ret;
}

BOOL WINAPI FileHook::FakeReadFileEx(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPOVERLAPPED lpOverlapped, LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine) {
    // DEBUG
    logger << "[FileHook][ReadFileEx] ReadFileEx API called but not implemented!\n";
    auto ret = realReadFileEx(hFile, lpBuffer, nNumberOfBytesToRead, lpOverlapped, lpCompletionRoutine);
    return ret;
}

BOOL WINAPI FileHook::FakeReadFileScatter(HANDLE hFile, FILE_SEGMENT_ELEMENT aSegmentArray[], DWORD nNumberOfBytesToRead, LPDWORD lpReserved, LPOVERLAPPED lpOverlapped) {
    // DEBUG
    logger << "[FileHook][ReadFileScatter]ReadFileScatter API called but not implemented!\n";
    auto ret = realReadFileScatter(hFile, aSegmentArray, nNumberOfBytesToRead, lpReserved, lpOverlapped);
    return ret;
}

/*
    Alloc a buffer and
*/
BOOL WINAPI FileHook::FakeWriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped)
{
    auto enc = GetHandleEncryptor(hFile);
    if (enc == nullptr)
        return realWriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);

    logger << "[FileHook][WriteFile] Will decrypt\n";
    unsigned char *buffer = allocator.allocate(nNumberOfBytesToWrite);
    memcpy_s(buffer, nNumberOfBytesToWrite, lpBuffer, nNumberOfBytesToWrite);
    enc->EncryptBuffer(buffer, nNumberOfBytesToWrite);
    auto ret = realWriteFile(hFile, buffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
    enc->MoveCursor(*lpNumberOfBytesWritten - nNumberOfBytesToWrite);
    allocator.deallocate(buffer, nNumberOfBytesToWrite);
    return ret;
}

/*
    Filter request and create encryptor with cursor moving to correct position, then call AddHandleEncryptor
*/
HANDLE WINAPI FileHook::FakeCreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
    logger << "[FileHook][CreateFileW] CreateFileW API called!\n";
    auto ret = realCreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
    auto err = GetLastError();
    logger << "[FileHook][CreateFileW] FileName: " << lpFileName << "\n";

    if (ret == INVALID_HANDLE_VALUE) {
        // Early return: System call failed
        logger << "[FileHook][CreateFileW] API failed\n";
        return ret;
    }

    if (wcscmp(lpFileName, L"CONIN$") == 0 || wcscmp(lpFileName, L"CONOUT$") == 0) {
        // Early return: The file is stdin or stdout
        return ret;
    }

    if (dwCreationDisposition == OPEN_EXISTING && PathIsDirectoryW(lpFileName)) {
        // Early return: The file opened is a directory
        logger << "[FileHook][CreateFileW] File is a directory: " << lpFileName << "\n";
        return ret;
    }

    LPCWSTR fullPath = lpFileName;
    wchar_t buffer[MAX_PATH];
    wchar_t relativePathBuffer[MAX_PATH];

    if (PathIsRelativeW(lpFileName)) {
        GetFullPathNameW(lpFileName, MAX_PATH, buffer, nullptr);
        fullPath = buffer;
    }

    // Compare paths using native APIs first to reduce system call
    if (wcslen(fullPath) < wcslen(encryptBasePath) || wcsncmp(encryptBasePath, fullPath, wcslen(encryptBasePath)) != 0 || !PathIsPrefixW(encryptBasePath, fullPath)) {
        // Early return: The file is not within encryptedBasePath
        return ret;
    }

    if (!PathRelativePathToW(relativePathBuffer, encryptBasePath, FILE_ATTRIBUTE_DIRECTORY, fullPath, FILE_ATTRIBUTE_NORMAL)) {
        // Early return: The file is not within encryptedBasePath
        logger << "[FileHook][CreateFileW] File " << fullPath << " is not within EncryptBase\n";
        return ret;
    }

    // Filters finished
    logger << "[FileHook][CreateFileW] File " << fullPath << " will be encrypted\n";

    std::shared_ptr<Encryptor> encryptor;
    auto newNonce = [&](std::shared_ptr<Encryptor> &encryptor) {
        // TODO: Maybe a write loop is needed to ensure nonce is written?
        encryptor = std::make_shared<Encryptor>(masterKey);
        DWORD bytesWritten;
        if (!realWriteFile(ret, encryptor->GetNonce(), NONCE_LEN, &bytesWritten, nullptr) || bytesWritten != NONCE_LEN) {
            logger << "[FileHook][CreateFileW][newNonce] Nonce written error.\n";
            return INVALID_HANDLE_VALUE;
        }
        return ret;
    };

    auto existedNonce = [&](std::shared_ptr<Encryptor> &encryptor) {
        // TODO: Maybe a read loop is needed to ensure nonce is read?
        unsigned char nonceBuffer[NONCE_LEN];
        DWORD bytesRead;
        if (!realReadFile(ret, nonceBuffer, NONCE_LEN, &bytesRead, nullptr) || bytesRead != NONCE_LEN) {
            logger << "[FileHook][CreateFileW][newNonce] Nonce read error.\n";
            return INVALID_HANDLE_VALUE;
        }
        encryptor = std::make_shared<Encryptor>(masterKey, nonceBuffer);
        return ret;
    };

    switch (dwCreationDisposition) {
    case CREATE_NEW:
    case CREATE_ALWAYS:
    case TRUNCATE_EXISTING:
        logger << "[FileHook][CreateFileW] Nonce will be generated and written.\n";
        if (newNonce(encryptor) == INVALID_HANDLE_VALUE) {
            return INVALID_HANDLE_VALUE;
        }
        break;
    case OPEN_EXISTING:
        logger << "[FileHook][CreateFileW] Nonce will be read from file.\n";
        if (existedNonce(encryptor) == INVALID_HANDLE_VALUE) {
            return INVALID_HANDLE_VALUE;
        }
        break;
    case OPEN_ALWAYS:
        switch (err) {
        case ERROR_ALREADY_EXISTS:
            logger << "[FileHook][CreateFileW] Nonce will be read from file.\n";
            if (existedNonce(encryptor) != INVALID_HANDLE_VALUE) {
                break;
            }
        case NOERROR:
            logger << "[FileHook][CreateFileW] Nonce will be generated and written.\n";
            if (newNonce(encryptor) != INVALID_HANDLE_VALUE) {
                break;
            }
        default:
            logger << "[FileHook][CreateFileW] Other error.\n";
            return ret;
        }
        break;
    }
    AddHandleEncryptor(ret, encryptor);
    logger << "[FileHook][CreateFileW] Add handler to encryptorMap.\n";
    return ret;
}

BOOL WINAPI FileHook::FakeCloseHandle(HANDLE hObject) {
    // DEBUG
    logger << "[FileHook][CloseHandle] CloseHandle API called!\n";
    auto ret = realCloseHandle(hObject);
    RemoveHandleEncryptor(hObject);
    return ret;
}

DWORD WINAPI FileHook::FakeSetFilePointer(HANDLE hFile, LONG lDistanceToMove, PLONG lpDistanceToMoveHigh, DWORD dwMoveMethod) {
    if (lpDistanceToMoveHigh != nullptr) {
        // TODO: support file pointer movement more than 4G
        logger << "TODO unsupported yet\n";
        return realSetFilePointer(hFile, lDistanceToMove, lpDistanceToMoveHigh, dwMoveMethod);
    }

    DWORD ret;
    switch (dwMoveMethod){
    case FILE_BEGIN:
        ret = realSetFilePointer(hFile, lDistanceToMove + NONCE_LEN, lpDistanceToMoveHigh, dwMoveMethod);
        if (ret == INVALID_SET_FILE_POINTER && GetLastError() != NOERROR) {
            // Early return: System call failed
            return ret;
        }
        GetHandleEncryptor(hFile)->SetCursor(lDistanceToMove);
        return ret;
    case FILE_CURRENT:
        ret = realSetFilePointer(hFile, lDistanceToMove, lpDistanceToMoveHigh, dwMoveMethod);
        if (ret == INVALID_SET_FILE_POINTER && GetLastError() != NOERROR) {
            // Early return: System call failed
            return ret;
        }
        GetHandleEncryptor(hFile)->MoveCursor(lDistanceToMove);
        return ret;
    case FILE_END:
        // Move pointer to file begin first
        auto fileBeginPointer = realSetFilePointer(hFile, 0, nullptr, FILE_BEGIN);
        if (fileBeginPointer == INVALID_SET_FILE_POINTER && GetLastError() != NOERROR) {
            // Early return: System call failed
            return fileBeginPointer;
        }

        // Then move pointer to file end
        ret = realSetFilePointer(hFile, lDistanceToMove, lpDistanceToMoveHigh, dwMoveMethod);
        if (ret == INVALID_SET_FILE_POINTER && GetLastError() != NOERROR) {
            // Early return: System call failed
            return ret;
        }
        // Calculate cursor and set it
        GetHandleEncryptor(hFile)->SetCursor(ret - fileBeginPointer + 12);
        return ret;
    }
    logger << "Unexpected error occurred.\n";
    return INVALID_SET_FILE_POINTER;
}
