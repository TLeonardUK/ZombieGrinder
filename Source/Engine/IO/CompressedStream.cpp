// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/IO/CompressedStream.h"

/*
#include "zlib/zlib.h"

CompressedStream::CompressedStream(bool compress)
	: m_length(0)
	, m_position(0)
	, m_compress(compress)
{
}

CompressedStream::~CompressedStream()
{
}

bool CompressedStream::IsEOF()
{
	return m_position >= m_length;
}

unsigned int CompressedStream::Position()
{
	return m_position;
}

void CompressedStream::Seek(unsigned int offset)
{
	DBG_ASSERT(false);
}

unsigned int CompressedStream::Length()
{
	return m_length;
}

void CompressedStream::WriteBuffer(const char* buffer, int offset, int length)
{
}

void CompressedStream::ReadBuffer(char* buffer, int offset, int length)
{
}

void CompressedStream::Close()
{
	// Nothing to do.
}

void CompressedStream::Flush()
{
	// Nothing to do.
}
*/