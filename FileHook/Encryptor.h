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

