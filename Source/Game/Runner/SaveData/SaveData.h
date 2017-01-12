// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_SAVEDATA_
#define _GAME_SAVEDATA_

#include "Engine/IO/BinaryStream.h"

#include "Generic/Types/DataBuffer.h"

class SaveData
{
private:
	static bool Serialize_SaveData(BinaryStream* stream);
	static bool Deserialize_SaveData(BinaryStream* stream);

public:
	static bool Serialize(DataBuffer& buffer);
	static bool Deserialize(DataBuffer& buffer);

	static void Enable_Save_Data();
};

#endif

