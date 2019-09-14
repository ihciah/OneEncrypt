#include "ConfigLoader.h";

/* Get current file path and combine CONFIG_FILE
   dest: copy to dest in MBS
   return: error code
*/
errno_t GetConfigPath(PCWSTR configFileName, char* dest) {
	WCHAR pwd[MAX_PATH];
	DWORD length = GetModuleFileNameW(NULL, pwd, MAX_PATH);
	PathCchRemoveFileSpec(pwd, MAX_PATH);
	PathCchCombine(pwd, MAX_PATH, pwd, configFileName);

	size_t converted;
	return wcstombs_s(&converted, dest, MAX_PATH, pwd, MAX_PATH);
}

ConfigLoader::ConfigLoader(PCWSTR configFileName) {
	char path[MAX_PATH];

	GetConfigPath(configFileName, path);
	reader = std::make_unique<INIReader>(path);
}

errno_t ConfigLoader::GetEncryptBasePath(wchar_t* encryptBase) {
	auto folderPath = reader->GetString("global", "path", "WRONGPATH");
	std::cout << "[ConfigLoader] FolderPath loaded: " << folderPath << std::endl;

	size_t converted;
	MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, folderPath.c_str(), folderPath.size(), encryptBase, MAX_PATH);
}

errno_t ConfigLoader::GetKey(char* lpKey) {
	auto key = reader->GetString("global", "key", "");
	std::cout << "[ConfigLoader] Key loaded: " << key << std::endl;

	strcpy_s(lpKey, MAX_KEY, key.c_str());
}
