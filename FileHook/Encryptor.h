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
#include "common.h"
#include <string>
#include <algorithm>

const uint64_t BLOCKSIZE = 1024;

class Encryptor
{
private:
    const unsigned char *key;
    unsigned char nonce[NONCE_LEN];
    uint64_t cursor = 0;

    Encryptor(const Encryptor&);
    void EncryptBlock(unsigned char *c, const unsigned char *m, uint64_t mlen, uint64_t blockId, uint64_t startPos);

public:
    Encryptor() {};
    Encryptor(const unsigned char *k);
    Encryptor(const unsigned char *k, const unsigned char *n);

    const unsigned char* GetNonce();
    void SetCursor(uint64_t);
    void MoveCursor(int64_t);
    void EncryptBuffer(unsigned char *c, const unsigned char *m, uint64_t mlen);
    void EncryptBuffer(unsigned char *cm, uint64_t mlen) { EncryptBuffer(cm, cm, mlen); }
};

