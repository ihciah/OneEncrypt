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

#include "Logger.h"

using namespace std;

Logger::Logger(string filename) {
    ws = make_unique<wofstream>(wofstream(filename));
    locale utf8_locale(ws->getloc(), new codecvt_utf8<wchar_t>);
    ws->imbue(utf8_locale);
    *ws << L"[FileHook Boot Logger]Logger started." << endl;
}

Logger::Logger(wstring filename) {
    ws = make_unique<wofstream>(wofstream(filename));
    locale utf8_locale(ws->getloc(), new codecvt_utf8<wchar_t>);
    ws->imbue(utf8_locale);
    *ws << L"[FileHook Boot Logger]Logger started." << endl;
}

Logger::Logger(char* filename) {
    Logger(string(filename));
}

Logger::Logger(wchar_t* filename) {
    Logger(wstring(filename));
}

Logger::~Logger() {
    ws->close();
    ws.~unique_ptr();
}

void Logger::Print(const wstring logString) {
    *ws << logString;
    ws->flush();
}

void Logger::Print(const string logString) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    *ws << converter.from_bytes(logString);
    ws->flush();
}

std::unique_ptr<Logger>& operator<<(std::unique_ptr<Logger>& out, const string& s) {
    out->Print(s);
    return out;
}

std::unique_ptr<Logger>& operator<<(std::unique_ptr<Logger>& out, const wstring& s) {
    out->Print(s);
    return out;
}

std::unique_ptr<Logger>& operator<<(std::unique_ptr<Logger>& out, const char* s) {
    out->Print(string(s));
    return out;
}

std::unique_ptr<Logger>& operator<<(std::unique_ptr<Logger>& out, const wchar_t* s) {
    out->Print(wstring(s));
    return out;
}
