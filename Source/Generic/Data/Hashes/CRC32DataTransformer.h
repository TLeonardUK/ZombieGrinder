// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_DATA_CRC32DATATRANSFORMER_
#define _GENERIC_DATA_CRC32DATATRANSFORMER_

#include "Generic/Data/DataTransformer.h"

class CRC32DataTransformer : public DataTransformer
{
	MEMORY_ALLOCATOR(CRC32DataTransformer, "Generic");

private:
	static const u32 LOOKUP_TABLE[256];

private:
	u32 m_crc;

public:

	CRC32DataTransformer();

	// Required overrides.
	void  Reset();
	void  Add(void* data, int size);
	void* Get_Result(int& output_size);

};

#endif
