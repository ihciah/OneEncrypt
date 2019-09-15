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
#define SODIUM_STATIC 1
#include <memory>
#include <iostream>
#include <Windows.h>
#include <shlwapi.h>
#include "libsodium/include/sodium.h"
#include "libsodium/include/sodium/crypto_stream_xchacha20.h"

#pragma comment(lib, "Shlwapi.lib")

const PCWSTR CONFIG_FILE = L"config.ini";
const int MAX_KEY = 256;

const unsigned char ZEROSALT[crypto_pwhash_SALTBYTES] = { 0 };
const auto OPSLIMIT = crypto_pwhash_OPSLIMIT_MIN;
const auto MEMLIMIT = crypto_pwhash_MEMLIMIT_MIN;
const auto ALG = crypto_pwhash_ALG_DEFAULT;
const auto NONCE_LEN = crypto_stream_xchacha20_NONCEBYTES;
