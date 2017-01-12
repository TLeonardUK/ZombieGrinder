// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Crypto/CryptoStream.h"
#include "Engine/IO/StreamFactory.h"

CryptoKey CryptoKey::Empty = CryptoKey();

CryptoKey::CryptoKey()
{
}

CryptoKey::CryptoKey(unsigned char* key, int key_size)
{
	for (int i = 0; i < key_size; i++)
	{
		m_key.push_back(key[i]);
	}
}

void CryptoKey::GeneratePair(CryptoKey& public_key, CryptoKey& private_key)
{
	CryptoPP::AutoSeededRandomPool rnd;

	// Generate pub/prv key.
	CryptoPP::InvertibleRSAFunction params;
	params.GenerateRandomWithKeySize(rnd, 1024);

	CryptoPP::RSA::PrivateKey rsaPrivate(params);
	CryptoPP::RSA::PublicKey rsaPublic(params);

	// Store private.
	CryptoPP::ByteQueue private_queue;
	rsaPrivate.Save(private_queue);
	int private_key_len = (int)private_queue.CurrentSize();

	private_key.m_key.clear();
	for (int i = 0; i < private_key_len; i++)
	{
		byte outByte;
		private_queue.Get(outByte);
		private_key.m_key.push_back(outByte);
	}

	// Store public!
	CryptoPP::ByteQueue public_queue;
	rsaPublic.Save(public_queue);
	int public_key_len = (int)public_queue.CurrentSize();

	public_key.m_key.clear();
	for (int i = 0; i < public_key_len; i++)
	{
		byte outByte;
		public_queue.Get(outByte);
		public_key.m_key.push_back(outByte);
	}
}

void CryptoKey::Save(const char* file)
{
	Stream* stream = StreamFactory::Open(file, StreamMode::Write);
	for (unsigned int i = 0; i < m_key.size(); i++)
	{
		stream->Write<unsigned char>(m_key[i]);
	}
	SAFE_DELETE(stream);
}

void CryptoKey::Load(const char* file)
{
	Stream* stream = StreamFactory::Open(file, StreamMode::Read);
	int num = stream->Length();

	m_key.clear();
	for (int i = 0; i < num; i++)
	{
		m_key.push_back(stream->Read<unsigned char>());
	}

	SAFE_DELETE(stream);
}

bool CryptoKey::SameAs(CryptoKey& other)
{
	if (other.m_key.size() != m_key.size())
	{
		return false;
	}

	for (unsigned int i = 0; i < m_key.size(); i++)
	{
		if (other.m_key[i] != m_key[i])
		{
			return false;
		}
	}

	return true;
}

CryptoStream::CryptoStream(CryptoStreamMode::Type type, CryptoKey& key)
	: m_mode(type)
	, m_key(key)
{
	if (m_mode == CryptoStreamMode::Sign)
	{
		CryptoPP::ByteQueue bytes;
		bytes.Put(m_key.m_key.data(), m_key.m_key.size());		

		m_signer = CryptoPP::RSASS<CryptoPP::PKCS1v15, CryptoPP::SHA>::Signer(bytes);
		m_accumulator = m_signer.NewSignatureAccumulator(m_rand_pool);
	}
	else if (m_mode == CryptoStreamMode::Validate)
	{
		CryptoPP::ByteQueue bytes;
		bytes.Put(m_key.m_key.data(), m_key.m_key.size());

		m_verifier = CryptoPP::RSASS<CryptoPP::PKCS1v15, CryptoPP::SHA>::Verifier(bytes);
		m_accumulator = m_verifier.NewVerificationAccumulator();
	}
	else
	{
		DBG_ASSERT(false);
	}
}

CryptoStream::~CryptoStream()
{
}

bool CryptoStream::IsEOF()
{
	DBG_ASSERT_STR(false, "Not Implemented");
	return false;
}

unsigned int CryptoStream::Position()
{
	DBG_ASSERT_STR(false, "Not Implemented");
	return 0;
}

void CryptoStream::Seek(unsigned int offset)
{
	DBG_ASSERT_STR(false, "Not Implemented");
}

unsigned int CryptoStream::Length()
{
	DBG_ASSERT_STR(false, "Not Implemented");
	return 0;
}

void CryptoStream::WriteBuffer(const char* buffer, int offset, int length)
{
	m_accumulator->Update(reinterpret_cast<const byte*>(buffer + offset), (size_t)length);
}

void CryptoStream::ReadBuffer(char* buffer, int offset, int length)
{
	DBG_ASSERT_STR(false, "Not Implemented");
}

void CryptoStream::Close()
{
	// Nothing to do.
}

void CryptoStream::Flush()
{
	// Nothing to do.
}

void CryptoStream::GetSignature(char* signature, int signature_length)
{
	DBG_ASSERT(SIGNATURE_LENGTH == signature_length);
	DBG_ASSERT_STR(SIGNATURE_LENGTH == m_signer.MaxSignatureLength(), "Signature length of %i does not match expected size.", m_signer.MaxSignatureLength());
	unsigned int sign_len = m_signer.Sign(m_rand_pool, m_accumulator, reinterpret_cast<byte*>(signature));
	m_accumulator = NULL;
}

bool CryptoStream::CheckSignature(const char* signature, int signature_length)
{
	DBG_ASSERT(SIGNATURE_LENGTH == signature_length);
	m_verifier.InputSignature(*m_accumulator, reinterpret_cast<const byte*>(signature), signature_length);

	CryptoPP::PK_MessageAccumulator* original = m_accumulator;
	m_accumulator = NULL;
	return m_verifier.Verify(original);
}