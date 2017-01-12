// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Profile/ProfileManager.h"
#include "Game/Runner/GameOptions.h"

#include "Game/Profile/ItemManager.h"

#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMBinary.h"
#include "XScript/VirtualMachine/CVMContext.h"

#include "Engine/Online/OnlinePlatform.h"
#include "Engine/Online/OnlineUser.h"
#include "Engine/Online/OnlineInventory.h"

#include "Engine/Engine/GameEngine.h"
#include "Engine/Scripts/EngineVirtualMachine.h"

#include "Game/Scripts/ScriptEventListener.h"

#include "Engine/IO/BinaryStream.h"

ProfileManager::ProfileManager()
	: m_init(false)
	, m_update_counter(-1)
	, m_update_profile_counter(0)
	, m_syncEnabled(true)
{
}

ProfileManager::~ProfileManager()
{
	for (std::vector<Profile*>::iterator iter = m_profiles.begin(); iter != m_profiles.end(); iter++)
	{
		SAFE_DELETE(*iter);
	}
	m_profiles.clear();

	SAFE_DELETE(m_level_xp);
	SAFE_DELETE(m_level_cumulative_xp);
}

void ProfileManager::Set_Sync_Enabled(bool bEnabled)
{
	m_syncEnabled = bEnabled;
}

int ProfileManager::Get_Max_Profiles()
{
#ifdef OPT_PREMIUM_ACCOUNTS
	bool is_premium = OnlinePlatform::Get()->Check_Purchased(*GameOptions::premium_account_appid);
	return is_premium ? max_profiles_premium_ : max_profiles_;
#else
	return max_profiles_premium_;
#endif
}

bool ProfileManager::Init()
{
	DBG_ASSERT(!m_init);
	DBG_LOG("Loading profile information ...");

	EngineVirtualMachine* vm = GameEngine::Get()->Get_VM();
	m_profile_class = vm->Find_Class("Profile");
	DBG_ASSERT(m_profile_class != NULL);

	CVMContextLock lock = vm->Set_Context(vm->Get_Static_Context());

	CVMLinkedSymbol* level_cap_sym = vm->Find_Variable(m_profile_class, "Level_Cap");
	CVMValue retval;

	vm->Get_Static(m_profile_class, level_cap_sym, retval);
	m_level_cap = retval.int_value;

	level_cap_sym = vm->Find_Variable(m_profile_class, "Skill_Reroll_Cost");
	vm->Get_Static(m_profile_class, level_cap_sym, retval);
	m_skill_reroll_cost = retval.int_value;

	level_cap_sym = vm->Find_Variable(m_profile_class, "Skill_Points_Per_Level");
	vm->Get_Static(m_profile_class, level_cap_sym, retval);
	m_skill_points_per_level = retval.int_value;

	m_level_xp = new int[m_level_cap + 1];
	m_level_cumulative_xp = new int[m_level_cap + 1];

//	DBG_LOG("[XP Table]");

	CVMLinkedSymbol* level_curve_sym = vm->Find_Function(m_profile_class, "XP_Curve", 1, "int");
	int last_value = 0;
	for (int i = 0; i <= m_level_cap; i++)
	{
		CVMValue param_value(i);
		CVMValue instance_value(0);
		vm->Push_Parameter(param_value);
		vm->Invoke(level_curve_sym, instance_value);

		CVMValue result;
		vm->Get_Return_Value(result);

		if (i == 0)
			last_value = result.int_value;

		m_level_xp[i] = result.int_value - last_value;

		last_value = result.int_value;

//		DBG_LOG("[%i] %i", i + 1, m_level_xp[i]);
	}

	last_value = 0;
	for (int i = 0; i <= m_level_cap; i++)
	{
		m_level_cumulative_xp[i] = last_value + m_level_xp[i];
		last_value = m_level_cumulative_xp[i];
	//	DBG_LOG("[%i] %i", i + 1, m_level_cumulative_xp[i]);
	}

	m_init = true;
	return true;
}

void ProfileManager::Tick(const FrameTime& time)
{
	// Tick logged in profiles.
	for (int i = 0; i < MAX_LOCAL_PLAYERS; i++)
	{
		OnlineUser* user = OnlinePlatform::Get()->Get_Local_User_By_Index(i);
		if (user != NULL)
		{
			int idx = user->Get_Profile_Index();
			if (idx >= 0)
			{			
				Profile* profile = Get_Profile(idx);
				profile->Tick(time);
				
#ifndef MASTER_BUILD
				if (Input::Get()->Was_Pressed(InputBindings::Keyboard_PageUp))
				{
					profile->Level_Up();
				}
#endif
			}
		}
	}

	if (m_syncEnabled)
	{
		if (OnlineInventory::Try_Get() != NULL)
		{
			if (OnlineInventory::Get()->HasInventory())
			{
				int counter = OnlineInventory::Get()->GetUpdateCounter();
				if ((counter != m_update_counter || m_profiles.size() != m_update_profile_counter) && m_profiles.size() > 0)
				{
					Sync_Profiles();
					m_update_counter = counter;
					m_update_profile_counter = m_profiles.size();
				}
			}
		}
	}
}

struct ProfileSyncItem
{
	u64 id;
	Item* item;
};

void ProfileManager::Sync_Profiles()
{
	for (std::vector<Profile*>::iterator iter = m_profiles.begin(); iter != m_profiles.end(); iter++)
	{
		Profile* profile = *iter;
		Sync_Profile_To_Inventory(profile);
	}
}

void ProfileManager::Check_Unlocks()
{
	for (std::vector<Profile*>::iterator iter = m_profiles.begin(); iter != m_profiles.end(); iter++)
	{
		Profile* profile = *iter;
		profile->Check_Unlocks();
	}
}

void ProfileManager::Sync_Profile_To_Inventory(Profile* profile)
{
	// DEBUG DEBUG DEBUG DEBUG
	//return;
	// DEBUG DEBUG DEBUG DEBUG

	CVirtualMachine* vm = GameEngine::Get()->Get_VM();
	CVMContextLock lock = vm->Set_Context(vm->Get_Static_Context());

	std::vector<OnlineInventoryItem> items = OnlineInventory::Get()->GetItems();

	std::vector<ProfileSyncItem> profile_items;

	CVMObject* profile_item_array = profile->Item_Array.Get().Get();
	for (int i = 0; i < profile_item_array->Slot_Count(); i++)
	{
		Item* existing_item = reinterpret_cast<Item*>(profile_item_array->Get_Slot(i).object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
		ProfileSyncItem sync_item;
		sync_item.item = existing_item;
		sync_item.id = StringHelper::To_U64(existing_item->inventory_drop_id);
		profile_items.push_back(sync_item);
	}

	for (std::vector<OnlineInventoryItem>::iterator iter = items.begin(); iter != items.end(); iter++)
	{
		OnlineInventoryItem& new_item = *iter;
		bool bIsNew = true;

		for (std::vector<ProfileSyncItem>::iterator iter2 = profile_items.begin(); iter2 != profile_items.end(); iter2++)
		{
			ProfileSyncItem& existing_item = *iter2;

			if (existing_item.item->was_inventory_drop && 
				existing_item.id == new_item.unique_id64)
			{
				bIsNew = false;
				break;
			}
		}

		// Add new item to list.
		if (bIsNew)
		{
			ItemArchetype* archetype = ItemManager::Get()->Find_Archetype_By_InventoryId(new_item.type_id);
			if (archetype)
			{
				Item* item = profile->Add_Item(archetype);
				item->was_inventory_drop = true;
				item->inventory_drop_id = new_item.unique_id;
				item->inventory_original_drop_id = new_item.original_unique_id;
				item->indestructable = true;

				DBG_LOG("[Profile Manager] Item %s added.", item->inventory_drop_id.c_str());
			}
			else
			{
				DBG_LOG("[Profile Manager] ERROR: Failed to load inventory item with type-id %i, unique-id %s, archetype could not be found..", new_item.type_id, new_item.original_unique_id.c_str());			
			}
		}
	}

	for (std::vector<ProfileSyncItem>::iterator iter2 = profile_items.begin(); iter2 != profile_items.end(); iter2++)
	{
		ProfileSyncItem& existing_item = *iter2;
		bool bWasDeleted = true;

		if (!existing_item.item->was_inventory_drop)
		{
			continue;
		}

		for (std::vector<OnlineInventoryItem>::iterator iter = items.begin(); iter != items.end(); iter++)
		{
			OnlineInventoryItem& new_item = *iter;
			
			if (new_item.unique_id64 == existing_item.id)
			{
				bWasDeleted = false;
				break;
			}
		}

		// Item was deleted.
		if (bWasDeleted)
		{
			DBG_LOG("[Profile Manager] Item %llu deleted.", existing_item.id);

			profile->Remove_Item(existing_item.item);
		}
	}
}

int	ProfileManager::Get_Level_Cap()
{
	return m_level_cap;
}

int	ProfileManager::Get_Skill_Points_Per_Level()
{
	return m_skill_points_per_level;
}

int	ProfileManager::Get_Skill_Reroll_Cost()
{
	return m_skill_reroll_cost;
}

int	ProfileManager::Get_Level_XP(int level)
{
	if (level >= 0 && level <= m_level_cap)
	{
		return m_level_xp[level];
	}
	else
	{
		return 0;
	}
}

int	ProfileManager::Get_Level_Cumulative_XP(int level)
{
	if (level >= 0 && level <= m_level_cap)
	{
		return m_level_cumulative_xp[level];
	}
	else
	{
		return 0;
	}
}

int	ProfileManager::Get_Profile_Count()
{
	return m_profiles.size();
}

Profile* ProfileManager::Get_Profile(int index)
{
	return m_profiles.at(index);
}

int ProfileManager::Create_Profile()
{
	EngineVirtualMachine* vm = GameEngine::Get()->Get_VM();
	
	CVMContextLock lock = vm->Set_Context(vm->Get_Static_Context());

	Profile* profile = new Profile();
	profile->Script_Object = vm->New_Object(m_profile_class, true, profile);
	vm->Set_Default_State(profile->Script_Object);

	ScriptEventListener::Fire_On_Character_Created();

	m_profiles.push_back(profile);

	profile->Check_Unlocks();
	profile->Item_Unlock_Events.clear();
	profile->Skill_Unlock_Events.clear();

	return Get_Profile_Count() - 1;
}

void ProfileManager::Delete_Profile(int index)
{
	Profile* profile = m_profiles.at(index);
	SAFE_DELETE(profile);
	m_profiles.erase(m_profiles.begin() + index);
}

bool ProfileManager::Serialize(BinaryStream* stream)
{
	DBG_ASSERT(m_init);

	EngineVirtualMachine* vm = GameEngine::Get()->Get_VM();
	CVMContextLock lock = vm->Set_Context(vm->Get_Static_Context());

	stream->Write<u32>(m_profiles.size());

	for (std::vector<Profile*>::iterator iter = m_profiles.begin(); iter != m_profiles.end(); iter++)
	{
		Profile* profile = *iter;
		
		int version = 0;

		BinaryStream profile_stream;
		profile->Script_Object.Get().Get()->Serialize(&profile_stream, CVMObjectSerializeFlags::Full, &version);

		stream->Write<int>(version);
		stream->Write<int>(profile_stream.Length());
		stream->WriteBuffer(profile_stream.Data(), 0, profile_stream.Length());
	}

	return true;
}

bool ProfileManager::Deserialize(BinaryStream* stream)
{
	EngineVirtualMachine* vm = GameEngine::Get()->Get_VM();
	CVMContextLock lock = vm->Set_Context(vm->Get_Static_Context());

	for (std::vector<Profile*>::iterator iter = m_profiles.begin(); iter != m_profiles.end(); iter++)
	{
		SAFE_DELETE(*iter);
	}
	m_profiles.clear();

	u32 profile_count = stream->Read<u32>();
	for (u32 i = 0; i < profile_count; i++)
	{
		Profile* profile = new Profile();
		profile->Script_Object = vm->New_Object(m_profile_class, false, profile);
		vm->Set_Default_State(profile->Script_Object);
		
		int version = stream->Read<int>();
		int length = stream->Read<int>();
		int data_start = stream->Position();

		bool found = false;

		//BinaryStream profile_stream(stream->Data() + stream->Position(), length);
		vm->Get_Static_Context()->MetaData = profile;
		profile->Script_Object.Get().Get()->Deserialize(stream, CVMObjectSerializeFlags::Full, version); //&profile_stream, CVMObjectSerializeFlags::Full, version);

		//DBG_LOG("PROFILE SIZE=%i", length);

		// Skip excess data.
		stream->Seek(data_start + length); 

		profile->Sanitize();

		m_profiles.push_back(profile);	
	}

	return true;
}

DataBuffer ProfileManager::Profile_To_Buffer(Profile* profile)
{
	EngineVirtualMachine* vm = GameEngine::Get()->Get_VM();
	CVMContextLock lock = vm->Set_Context(vm->Get_Static_Context());

	int version = 0;

	BinaryStream profile_stream;
	profile->Script_Object.Get().Get()->Serialize(&profile_stream, CVMObjectSerializeFlags::Full, &version);

	BinaryStream stream;
	stream.Write<int>(version);
	stream.WriteBuffer(profile_stream.Data(), 0, profile_stream.Length());

	return DataBuffer(stream.Data(), stream.Length());
}

Profile* ProfileManager::Buffer_To_Profile(DataBuffer buffer)
{
	EngineVirtualMachine* vm = GameEngine::Get()->Get_VM();
	CVMContextLock lock = vm->Set_Context(vm->Get_Static_Context());

	// new profile!
	Profile* profile = new Profile();
	profile->Script_Object = vm->New_Object(m_profile_class, false, profile);
	vm->Set_Default_State(profile->Script_Object);
		
	// Read header.
	BinaryStream stream(buffer.Buffer(), buffer.Size());
	int version = stream.Read<int>();

	// Deserialize the profile data.
	vm->Get_Static_Context()->MetaData = profile;
	profile->Script_Object.Get().Get()->Deserialize(&stream, CVMObjectSerializeFlags::Full, version);

	return profile;
}