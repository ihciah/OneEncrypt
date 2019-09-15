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

#include "Encryptor.h"

using namespace std;

/*
    Encryptor for given nonce
*/
Encryptor::Encryptor(const unsigned char *k, const unsigned char *n) {
    key = k;
    memcpy_s(nonce, crypto_stream_xchacha20_NONCEBYTES, n, crypto_stream_xchacha20_NONCEBYTES);
}

/*
    Encryptor for random nonce
*/
Encryptor::Encryptor(const unsigned char *k) {
    key = k;
    randombytes_buf(nonce, crypto_stream_xchacha20_NONCEBYTES);
}

/*
    Return nonce pointer
*/
const unsigned char* Encryptor::GetNonce()
{
    return nonce;
}

/*
    Set cursor to given value
*/
void Encryptor::SetCursor(uint64_t cursor)
{
    this->cursor = cursor;
}

/*
    Add a movement to cursor
*/
void Encryptor::MoveCursor(int64_t movement)
{
    this->cursor += movement;
}

/*
    Encrypt data within the same block.
    c: cipher text output
    m: plain text message
*/
void Encryptor::EncryptBlock(unsigned char *c, const unsigned char *m, uint64_t mlen, uint64_t blockId, uint64_t startPos) {
    if (startPos == 0) {
        crypto_stream_xchacha20_xor_ic(c, m, mlen, nonce, blockId, key);
        return;
    }

    unsigned char buffer[BLOCKSIZE];
    memset(buffer, 0, BLOCKSIZE);

    memcpy_s(buffer + startPos, BLOCKSIZE - startPos, m, mlen);
    crypto_stream_xchacha20_xor_ic(buffer, buffer, BLOCKSIZE, nonce, blockId, key);
    memcpy_s(c, BLOCKSIZE - startPos, buffer + startPos, mlen);

    //crypto_stream_xchacha20_xor_ic(buffer, buffer, BLOCKSIZE, nonce, blockId, key);
    //for (unsigned int offset = 0; offset < mlen; offset++) {
    //    c[offset] = m[offset] ^ buffer[startPos + offset];
    //}
}

/*
    Encrypt a specific message with arbitrary length.
    c: cipher text output
    m: plain text message
*/
void Encryptor::EncryptBuffer(unsigned char *c, const unsigned char *m, uint64_t mlen) {
    uint64_t offset = 0;
    while (mlen > 0) {
        uint64_t blockId = cursor / BLOCKSIZE;
        uint64_t blockPos = cursor % BLOCKSIZE;

        uint64_t blockLen = BLOCKSIZE - blockPos;
        uint64_t contentLen = min(blockLen, mlen);
        EncryptBlock(c + offset, m + offset, contentLen, blockId, blockPos);

        mlen -= contentLen;
        cursor += contentLen;
        offset += contentLen;
    }
}
