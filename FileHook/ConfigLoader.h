#pragma once
#include <memory>
#include <Windows.h>
#include <pathcch.h>
#include "common.h"
#include "inih/cpp/INIReader.h"

#pragma comment(lib, "Pathcch.lib")

class ConfigLoader {
private:
	std::unique_ptr<INIReader> reader;
public:
	ConfigLoader(PCWSTR configFileName);
	errno_t GetEncryptBase(wchar_t* encryptBase);
	errno_t GetKey(char* key);
};