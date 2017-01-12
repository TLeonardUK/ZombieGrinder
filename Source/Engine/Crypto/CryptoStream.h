// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_CRYPTO_STREAM_
#define _ENGINE_CRYPTO_STREAM_

#include "Engine/IO/Stream.h"

#include "cryptocpp/rsa.h"
#include "cryptocpp/osrng.h"

#include <vector>

struct CryptoStreamMode
{
	enum Type
	{
		Validate,		// Validate the given signature is correct for the written data.
		Sign			// Generate a signature for the written data.
	};
};

struct CryptoKey
{
private:
	friend class CryptoStream;

	std::vector<unsigned char> m_key;

public:
	CryptoKey();
	CryptoKey(unsigned char* key, int key_size);

	void Save(const char* file);
	void Load(const char* file);

	bool SameAs(CryptoKey& other);

	static CryptoKey Empty;
	static void GeneratePair(CryptoKey& public_key, CryptoKey& private_key);

};

class CryptoStream : public Stream
{
	MEMORY_ALLOCATOR(CryptoStream, "IO");

private:
	CryptoStreamMode::Type m_mode;
	CryptoKey m_key;

	CryptoPP::RSA::PrivateKey m_private_key;
	CryptoPP::RSA::PublicKey m_public_key;

	CryptoPP::RSASS<CryptoPP::PKCS1v15, CryptoPP::SHA>::Signer   m_signer;
	CryptoPP::RSASS<CryptoPP::PKCS1v15, CryptoPP::SHA>::Verifier m_verifier;

	CryptoPP::PK_MessageAccumulator* m_accumulator;

	CryptoPP::AutoSeededRandomPool m_rand_pool;

public:
	enum
	{
		SIGNATURE_LENGTH = 128,
	};

	CryptoStream(CryptoStreamMode::Type type, CryptoKey& key);
	~CryptoStream();

	bool IsEOF();
	unsigned int Position();
	void Seek(unsigned int offset);
	unsigned int Length();
	void WriteBuffer(const char* buffer, int offset, int length);
	void ReadBuffer(char* buffer, int offset, int length);
	void Close();
	void Flush();

	void GetSignature(char* signature, int signature_length);
	bool CheckSignature(const char* signature, int signature_length);

};

#endif

