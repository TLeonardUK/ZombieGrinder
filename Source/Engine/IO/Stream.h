// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_STREAM_
#define _ENGINE_STREAM_

#include <string>

#include "Generic/Types/DataBuffer.h"
#include "Generic/Types/Color.h"
#include "Generic/Types/Rect2D.h"
#include "Generic/Math/Math.h"

struct StreamMode
{
	enum Type
	{
		Read		= 0x1, // Grant read access.
		Write		= 0x2, // Grant write access.
		Truncate	= 0x4, // Truncate file if it does exist, otherwise append.
		Create		= 0x8, // Create file if it dosen't already exist.
	};
};

class Stream
{
	MEMORY_ALLOCATOR(Stream, "IO");

private:
	char* m_buffer;
	int   m_buffer_size;

public:
	
	// Constructor!
	Stream();
	virtual ~Stream();

	// Members that have to be overidden.
	virtual bool			IsEOF		() = 0;
	virtual unsigned int	Position	() = 0;
	virtual void			Seek		(unsigned int offset) = 0;
	virtual unsigned int	Length		() = 0;
	virtual void			WriteBuffer	(const char* buffer, int offset, int length) = 0;
	virtual void			ReadBuffer	(char* buffer, int offset, int length) = 0;
	virtual void			Close		() = 0;
	virtual void			Flush		() = 0;

	unsigned int			Bytes_Remaining();

	// Special read/write methods.
	void WriteLine	(const char* str);
	void WriteString(const char* str);
	void WriteNullTerminatedString(const char* str);

	const char*	ReadLine();
	const char*	ReadString(int length);
	const char* ReadNullTerminatedString();

	void CopyTo(Stream* other)
	{
		const int buffer_size = 2048;
		char buffer[buffer_size];

		while (Bytes_Remaining() > 0)
		{
			int bytes = Min((int)Bytes_Remaining(), buffer_size);			
			ReadBuffer(buffer, 0, bytes);
			other->WriteBuffer(buffer, 0, bytes);
		}
	}

	template <typename T>
	void Serialize(bool bSaving, T& output)
	{
		if (bSaving)
		{
			Write<T>(output);
		}
		else
		{
			output = Read<T>();
		}
	}

	// ----------------------------------------------------------------------------
	//	Writing
	// ----------------------------------------------------------------------------
	template<typename T>
	void WriteBlob(T value)
	{
		// No! Do not try and read/write as blobs! I know what your thinking :(.
		// Endianness will get fucked when we port if you do :P.
		value.Serialize(this);
		
		//WriteBuffer(reinterpret_cast<const char*>(&value), 0, sizeof(T)):
	}
	
	template <typename T>
	void Write(T val)
	{
		WritePrimitive(val);
	}

	void WritePrimitive(std::string val)
	{
		Write<unsigned int>(val.size());
		WriteBuffer(val.c_str(), 0, val.size());
	}

	void WritePrimitive(DataBuffer val)
	{
		Write<unsigned int>(val.Size());
		WriteBuffer(val.Buffer(), 0, val.Size());
	}
	
	void WritePrimitive(Rect2D val)
	{
		Write<float>(val.X);
		Write<float>(val.Y);
		Write<float>(val.Width);
		Write<float>(val.Height);
	}

	void WritePrimitive(Color val)
	{
		Write<unsigned char>(val.R);
		Write<unsigned char>(val.G);
		Write<unsigned char>(val.B);
		Write<unsigned char>(val.A);
	}

	void WritePrimitive(int val)
	{
	#ifdef PLATFORM_LITTLE_ENDIAN
		WriteBuffer(reinterpret_cast<char*>(&val), 0, sizeof(int));
	#else
		Swap_Endian_Array(&val, sizeof(int));
		WriteBuffer(reinterpret_cast<char*>(&val), 0, sizeof(int));
	#endif
	}

	void WritePrimitive(unsigned int val)
	{
	#ifdef PLATFORM_LITTLE_ENDIAN
		WriteBuffer(reinterpret_cast<char*>(&val), 0, sizeof(unsigned int));
	#else
		Swap_Endian_Array(&val, sizeof(unsigned int));
		WriteBuffer(reinterpret_cast<char*>(&val), 0, sizeof(unsigned int));
	#endif
	}
	
	void WritePrimitive(u64 val)
	{
	#ifdef PLATFORM_LITTLE_ENDIAN
		WriteBuffer(reinterpret_cast<char*>(&val), 0, sizeof(u64));
	#else
		Swap_Endian_Array(&val, sizeof(u64));
		WriteBuffer(reinterpret_cast<char*>(&val), 0, sizeof(u64));
	#endif
	}

	void WritePrimitive(short val)
	{
	#ifdef PLATFORM_LITTLE_ENDIAN
		WriteBuffer(reinterpret_cast<char*>(&val), 0, sizeof(short));
	#else
		Swap_Endian_Array(&val, sizeof(short));
		WriteBuffer(reinterpret_cast<char*>(&val), 0, sizeof(short));
	#endif
	}
	
	void WritePrimitive(unsigned short val)
	{	
	#ifdef PLATFORM_LITTLE_ENDIAN
		WriteBuffer(reinterpret_cast<char*>(&val), 0, sizeof(unsigned short));
	#else
		Swap_Endian_Array(&val, sizeof(unsigned short));
		WriteBuffer(reinterpret_cast<char*>(&val), 0, sizeof(unsigned short));
	#endif
	}
	
	void WritePrimitive(char val)
	{
		WriteBuffer(reinterpret_cast<char*>(&val), 0, sizeof(char));
	}
	
	void WritePrimitive(bool val)
	{
		WriteBuffer(reinterpret_cast<char*>(&val), 0, sizeof(bool));
	}
	
	void WritePrimitive(unsigned char val)
	{
		WriteBuffer(reinterpret_cast<char*>(&val), 0, sizeof(unsigned char));
	}
	
	void WritePrimitive(float val)
	{
	#ifdef PLATFORM_LITTLE_ENDIAN
		WriteBuffer(reinterpret_cast<char*>(&val), 0, sizeof(float));
	#else
		Swap_Endian_Array(&val, sizeof(float));
		WriteBuffer(reinterpret_cast<char*>(&val), 0, sizeof(float));
	#endif
	}
	
	void WritePrimitive(double val)
	{
	#ifdef PLATFORM_LITTLE_ENDIAN
		WriteBuffer(reinterpret_cast<char*>(&val), 0, sizeof(double));
	#else
		Swap_Endian_Array(&val, sizeof(double));
		WriteBuffer(reinterpret_cast<char*>(&val), 0, sizeof(double));
	#endif
	}
	
	// ----------------------------------------------------------------------------
	//	Reading
	// ----------------------------------------------------------------------------
	template<typename T>
	T ReadBlob()
	{
		// No! Do not try and read/write as blobs! I know what your thinking :(.
		// Endianness will get fucked when we port if you do :P.
		T value;
		value.Deserialize(this);
		return value;
	}

	template<typename T>
	T Read()
	{
		T val;
		ReadPrimitive(&val);
		return val;
	}
	
	void ReadPrimitive(std::string* result)
	{
		unsigned int size = Read<unsigned int>();

		char* buffer = new char[size];
		ReadBuffer(buffer, 0, size);

		std::string string = std::string(buffer, size);

		SAFE_DELETE_ARRAY(buffer);

		*result = string;
	}

	void ReadPrimitive(DataBuffer* result)
	{
		unsigned int size = Read<unsigned int>();

		DataBuffer buffer;
		buffer.Set(NULL, size);

		ReadBuffer(buffer.Buffer(), 0, size);

		*result = buffer;
	}

	void ReadPrimitive(Rect2D* result)
	{
		Rect2D buffer;
		buffer.X = Read<float>();
		buffer.Y = Read<float>();
		buffer.Width = Read<float>();
		buffer.Height = Read<float>();
		*result = buffer;
	}

	void ReadPrimitive(Color* result)
	{
		Color buffer;
		buffer.R = Read<unsigned char>();
		buffer.G = Read<unsigned char>();
		buffer.B = Read<unsigned char>();
		buffer.A = Read<unsigned char>();
		*result = buffer;
	}

	void ReadPrimitive(int* result)
	{
		char buffer[sizeof(int)];
		ReadBuffer(buffer, 0, sizeof(int));
	#ifdef PLATFORM_BIG_ENDIAN
		Swap_Endian_Array(&buffer, sizeof(int));
	#endif
		*result = *reinterpret_cast<int*>(buffer);
	}
	
	void ReadPrimitive(unsigned int* result)
	{
		char buffer[sizeof(unsigned int)];
		ReadBuffer(buffer, 0, sizeof(unsigned int));
	#ifdef PLATFORM_BIG_ENDIAN
		Swap_Endian_Array(&buffer, sizeof(unsigned int));
	#endif
		*result = *reinterpret_cast<unsigned int*>(buffer);
	}
	
	void ReadPrimitive(u64* result)
	{
		char buffer[sizeof(u64)];
		ReadBuffer(buffer, 0, sizeof(u64));
	#ifdef PLATFORM_BIG_ENDIAN
		Swap_Endian_Array(&buffer, sizeof(u64));
	#endif
		*result = *reinterpret_cast<u64*>(buffer);
	}

	void ReadPrimitive(short* result)
	{
		char buffer[sizeof(short)];
		ReadBuffer(buffer, 0, sizeof(short));
	#ifdef PLATFORM_BIG_ENDIAN
		Swap_Endian_Array(&buffer, sizeof(short));
	#endif
		*result = *reinterpret_cast<short*>(buffer);
	}
	
	void ReadPrimitive(unsigned short* result)
	{
		char buffer[sizeof(unsigned short)];
		ReadBuffer(buffer, 0, sizeof(unsigned short));
	#ifdef PLATFORM_BIG_ENDIAN
		Swap_Endian_Array(&buffer, sizeof(unsigned short));
	#endif
		*result = *reinterpret_cast<unsigned short*>(buffer);
	}
	
	void ReadPrimitive(char* result)
	{
		char buffer[sizeof(char)];
		ReadBuffer(buffer, 0, sizeof(char));
		*result = *reinterpret_cast<char*>(buffer);
	}

	void ReadPrimitive(bool* result)
	{
		char buffer[sizeof(bool)];
		ReadBuffer(buffer, 0, sizeof(bool));
	#ifdef PLATFORM_BIG_ENDIAN
		Swap_Endian_Array(&buffer, sizeof(bool));
	#endif
		*result = *reinterpret_cast<bool*>(buffer);
	}
	
	void ReadPrimitive(unsigned char* result)
	{
		char buffer[sizeof(unsigned char)];
		ReadBuffer(buffer, 0, sizeof(unsigned char));
		*result = *reinterpret_cast<unsigned char*>(buffer);
	}
	
	void ReadPrimitive(float* result)
	{
		char buffer[sizeof(float)];
		ReadBuffer(buffer, 0, sizeof(float));
	#ifdef PLATFORM_BIG_ENDIAN
		Swap_Endian_Array(&buffer, sizeof(float));
	#endif
		*result = *reinterpret_cast<float*>(buffer);
	}
	
	void ReadPrimitive(double* result)
	{
		char buffer[sizeof(double)];
		ReadBuffer(buffer, 0, sizeof(double));
	#ifdef PLATFORM_BIG_ENDIAN
		Swap_Endian_Array(&buffer, sizeof(double));
	#endif
		*result = *reinterpret_cast<double*>(buffer);
	}

};

#endif

