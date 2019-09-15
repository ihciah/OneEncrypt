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

#pragma once
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#include <iostream>
#include <fstream>
#include <locale>
#include <codecvt>

class Logger {
private:
    std::unique_ptr<std::wofstream> ws;
public:
    Logger(std::string);
    Logger(std::wstring);
    Logger(char*);
    Logger(wchar_t*);
    ~Logger();
    void Print(const std::wstring);
    void Print(const std::string);
};

std::unique_ptr<Logger>& operator<<(std::unique_ptr<Logger>& out, const std::string& s);
std::unique_ptr<Logger>& operator<<(std::unique_ptr<Logger>& out, const std::wstring& s);
std::unique_ptr<Logger>& operator<<(std::unique_ptr<Logger>& out, const char* s);
std::unique_ptr<Logger>& operator<<(std::unique_ptr<Logger>& out, const wchar_t* s);
