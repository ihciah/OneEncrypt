#pragma once
#include <string>
#include <algorithm>
#include <Windows.h>
#define SODIUM_STATIC 1
#include "libsodium/include/sodium.h"
#include "libsodium/include/sodium/crypto_stream_xchacha20.h"

const uint64_t BLOCKSIZE = 1024;

class Encryptor
{
private:
	const unsigned char *key;
	unsigned char nonce[crypto_stream_xchacha20_NONCEBYTES];
	long cursor = 0;

public:
	Encryptor() {};
	Encryptor(const unsigned char *k, const unsigned char *n);
	Encryptor(const unsigned char *k, LPCWSTR fileName);

	void EncryptBlock(unsigned char *m, uint64_t mlen, uint64_t blockId, uint64_t startPos);
	void EncryptBuffer(unsigned char *m, uint64_t mlen);
};

