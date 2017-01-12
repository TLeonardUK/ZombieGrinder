// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_HELPER_ENDIANHELPER_
#define _GENERIC_HELPER_ENDIANHELPER_

void Swap_Endian_Array(char* buffer, int length);

template <typename T>
void Swap_Endian(T* source)
{
	char* buffer = (char*)source;
	int length = sizeof(T);

	int half = length / 2;
	for (int i = 0; i < half; i++)
	{
		buffer[i] = buffer[length - (i + 1)];
	}
}

#endif
