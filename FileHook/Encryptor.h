#pragma once
#include "common.h"
#include <string>
#include <algorithm>

const uint64_t BLOCKSIZE = 1024;

class Encryptor
{
private:
	const unsigned char *key;
	unsigned char nonce[crypto_stream_xchacha20_NONCEBYTES];
	uint64_t cursor = 0;

public:
	Encryptor() {};
	Encryptor(const unsigned char *k, const unsigned char *n);
	Encryptor(const unsigned char *k, LPCWSTR fileName);

	void EncryptBlock(unsigned char *m, uint64_t mlen, uint64_t blockId, uint64_t startPos);
	void EncryptBuffer(unsigned char *m, uint64_t mlen);
};

