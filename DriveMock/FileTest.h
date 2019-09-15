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
#include <fstream>
#include <iostream>
#include <string>

class FileTest
{
private:
    std::string targetFile;
public:
    FileTest() :targetFile("testout.txt") {}
    void ReadWriteTest(std::string srcPath, std::string dstPath);
    void ReadWriteTestMultipleTimes(std::string srcPath, std::string dstPath);
};

