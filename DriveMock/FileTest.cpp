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

#include "FileTest.h"

using namespace std;

// Copy file in stream
void FileTest::ReadWriteTest(std::string srcPath, std::string dstPath) {
    std::ifstream input(srcPath, std::ios::binary);
    std::ofstream output(dstPath, std::ios::binary);

    if (!input.is_open() || !output.is_open()) {
        cout << "Unable to open file to read or write." << endl;
        return;
    }

    auto src = std::istreambuf_iterator<char>(input);
    auto srcEnd = std::istreambuf_iterator<char>();
    auto dst = std::ostreambuf_iterator<char>(output);

    std::copy(src, srcEnd, dst);

    input.close();
    output.close();
}

// Copy file in multiple fread and fwrite
// However, manually fread does not mean ReadFile call
// So if testing multiple read and write, use a little big file
void FileTest::ReadWriteTestMultipleTimes(std::string srcPath, std::string dstPath) {
    FILE *ptr, *wptr;
    const unsigned int BUFFER_SIZE = 2048;
    unsigned char buffer[BUFFER_SIZE];

    auto errRead = fopen_s(&ptr, srcPath.c_str(), "rb");
    auto errWrite = fopen_s(&wptr, dstPath.c_str(), "wb");

    if (errRead != 0 || errWrite != 0) {
        cout << "Unable to open file to read or write." << endl;
        return;
    }

    while (true) {
        int x = fread_s(buffer, sizeof(buffer), 1, sizeof(buffer), ptr);
        if (x <= 0) break;
        cout << x << endl;
        fwrite(buffer, x, 1, wptr);
    }

    fclose(ptr);
    fclose(wptr);
}
