// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_ONLINE_DLC_MANAGER_
#define _GAME_ONLINE_DLC_MANAGER_

#include "Engine/IO/BinaryStream.h"
#include "Engine/Engine/FrameTime.h"

#include "XScript/VirtualMachine/CVMObject.h"

#include "Game/Scripts/ScriptEventListener.h"

#include "Generic/Patterns/Singleton.h"

struct AtlasFrame;

struct DLC
{
public:
	int						id;
	std::string				name;
	std::string				description;
	bool					purchased;

	CVMGCRoot			script_object;
};

class DLCManager : public Singleton<DLCManager>
{
	MEMORY_ALLOCATOR(DLCManager, "Game");

private:
	bool m_init;
	bool m_first_load;

	std::vector<DLC> m_dlc;

public:
	DLCManager();
	~DLCManager();

	bool Init();

	int Get_DLC_Count();
	DLC* Get_DLC(int index);

	bool Serialize(BinaryStream* stream);
	bool Deserialize(BinaryStream* stream);

	void Tick(const FrameTime& time);

};

#endif

