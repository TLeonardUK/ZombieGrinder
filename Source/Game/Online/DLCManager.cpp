// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Online/DLCManager.h"
#include "Game/Online/StatisticsManager.h"
#include "Game/Profile/ProfileManager.h"

#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMBinary.h"

#include "Engine/Engine/GameEngine.h"
#include "Engine/Scripts/EngineVirtualMachine.h"

#include "Engine/Engine/EngineOptions.h"

#include "Engine/Online/OnlinePlatform.h"
#include "Engine/Online/OnlineUser.h"

#include "Game/Runner/Game.h"

DLCManager::DLCManager()
	: m_init(false)
	, m_first_load(false)
{
}

DLCManager::~DLCManager()
{
}

int DLCManager::Get_DLC_Count()
{
	return m_dlc.size();
}

DLC* DLCManager::Get_DLC(int index)
{
	return &m_dlc.at(index);
}

bool DLCManager::Init()
{	
	DBG_ASSERT(!m_init);
	DBG_LOG("Loading dlc ...");

	EngineVirtualMachine* vm = GameEngine::Get()->Get_VM();
	CVMLinkedSymbol* base_symbol = vm->Find_Class("DLC");
	DBG_ASSERT(base_symbol != NULL);

	std::vector<CVMLinkedSymbol*> extended_classes = vm->Find_Derived_Classes(base_symbol);

	m_dlc.resize(extended_classes.size());

	CVMContextLock lock = vm->Set_Context(vm->Get_Static_Context());

	int index = 0;
	for (std::vector<CVMLinkedSymbol*>::iterator iter = extended_classes.begin(); iter != extended_classes.end(); iter++, index++)
	{
		CVMLinkedSymbol* achievement_class = *iter;
		DBG_LOG("Loading DLC Class: %s", achievement_class->symbol->name);

		DLC& achievement = m_dlc.at(index);
		achievement.purchased = 0;

		achievement.script_object = vm->New_Object(achievement_class, true, &achievement);
		vm->Set_Default_State(achievement.script_object);
	}

	m_init = true;
	return true;
}

void DLCManager::Tick(const FrameTime& time)
{
	if (!m_init)
		return;

	ProfileManager* profile_manager = ProfileManager::Get();

	// Check unlock state. If unlocked since last call, apply to all profiles.
	for (std::vector<DLC>::iterator iter = m_dlc.begin(); iter != m_dlc.end(); iter++)
	{
		DLC& achievement = *iter;
		
		// Check unlock state.
		if (!achievement.purchased)
		{
			//DBG_LOG("Checking DLC state '%s' (%i).", achievement.name.c_str(), achievement.id);
			achievement.purchased = OnlinePlatform::Get()->Check_Purchased(achievement.id);
			if (achievement.purchased)
			{
				DBG_LOG("DLC '%s' (%i) has been purchased.", achievement.name.c_str(), achievement.id);
			}
		}

		if (achievement.purchased)
		{
			// Apply to profiles that do not have it applied to it.
			for (int i = 0; i < profile_manager->Get_Profile_Count(); i++)
			{
				Profile* profile = profile_manager->Get_Profile(i);
				if (!profile->Has_DLC(&achievement))
				{
					profile->Apply_DLC(&achievement);
				}
			}
		}
	}
}

bool DLCManager::Serialize(BinaryStream* stream)
{
	DBG_ASSERT(m_init);
	
	EngineVirtualMachine* vm = GameEngine::Get()->Get_VM();
	CVMContextLock lock = vm->Set_Context(vm->Get_Static_Context());

	stream->Write<u32>(m_dlc.size());
	
	for (std::vector<DLC>::iterator iter = m_dlc.begin(); iter != m_dlc.end(); iter++)
	{
		DLC& achievement = *iter;
		int version = 0;

		BinaryStream achievement_stream;
		achievement.script_object.Get().Get()->Serialize(&achievement_stream, CVMObjectSerializeFlags::Full, &version);

		stream->Write<int>(achievement.id);
		stream->Write<int>(version);
		stream->Write<int>(achievement_stream.Length());
		stream->WriteBuffer(achievement_stream.Data(), 0, achievement_stream.Length());
	}

	return true;
}

bool DLCManager::Deserialize(BinaryStream* stream)
{
	EngineVirtualMachine* vm = GameEngine::Get()->Get_VM();
	CVMContextLock lock = vm->Set_Context(vm->Get_Static_Context());

	if (!m_first_load)
	{
		u32 count = stream->Read<u32>();
		for (u32 i = 0; i < count; i++)
		{
			int id = stream->Read<int>();
			int version = stream->Read<int>();
			int length = stream->Read<int>();

			bool found = false;

			for (std::vector<DLC>::iterator iter = m_dlc.begin(); iter != m_dlc.end(); iter++)
			{
				DLC& achievement = *iter;
				if (achievement.id == id)
				{
					achievement.script_object.Get().Get()->Deserialize(stream, CVMObjectSerializeFlags::Full, version);
					found = true;
					break;
				}
			}

			if (found == false)
			{
				DBG_LOG("Could not deserialize dlc '%i'. Could not be found. Has it been removed?", id);
				stream->Seek(stream->Position() + length); // Skip over unreadable data.
			}
		}
	}

	m_first_load = true;

	return true;
}