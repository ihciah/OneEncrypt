#include "Encryptor.h"

using namespace std;

Encryptor::Encryptor(const unsigned char *k, const unsigned char *n) {
	key = k;
	memcpy_s(nonce, crypto_stream_xchacha20_NONCEBYTES, n, crypto_stream_xchacha20_NONCEBYTES);
}

// Temp solution: hash(fileName) -> nonce
// v1.0: use all zero
Encryptor::Encryptor(const unsigned char * k, LPCWSTR fileName) {
	key = k;
	// Attention: fileName cannot be used to store in memory directly! Copy it.
	// TODO: read nonce from key.storage. Current nonce is all zero.
	memset(nonce, 0, crypto_stream_xchacha20_NONCEBYTES);
}

/*
	Encrypt data within the same block.
*/
void Encryptor::EncryptBlock(unsigned char *m, uint64_t mlen, uint64_t blockId, uint64_t startPos) {
	if (startPos == 0) {
		crypto_stream_xchacha20_xor_ic(m, m, mlen, nonce, blockId, key);
		return;
	}

	unsigned char buffer[BLOCKSIZE];
	memset(buffer, 0, BLOCKSIZE);
	crypto_stream_xchacha20_xor_ic(buffer, buffer, BLOCKSIZE, nonce, blockId, key);
	for (unsigned int offset = 0; offset < mlen; offset++) {
		m[offset] ^= buffer[startPos + offset];
	}
}

/*
	Encrypt a specific buffer with arbitrary length.
*/
void Encryptor::EncryptBuffer(unsigned char *m, uint64_t mlen) {
	uint64_t blockId = cursor / BLOCKSIZE;
	uint64_t blockPos = cursor % BLOCKSIZE;

	while (mlen > 0) {
		uint64_t blockLen = BLOCKSIZE - blockPos;
		uint64_t contentLen = min(blockLen, mlen);
		EncryptBlock(m, contentLen, blockId, blockPos);

		blockPos = 0; blockId++;
		mlen -= contentLen;
		cursor += contentLen;
	}
}
