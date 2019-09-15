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
