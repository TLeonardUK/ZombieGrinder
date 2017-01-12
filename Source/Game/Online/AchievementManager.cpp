// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Online/AchievementManager.h"
#include "Game/Online/StatisticsManager.h"

#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMBinary.h"

#include "Engine/Engine/GameEngine.h"
#include "Engine/Scripts/EngineVirtualMachine.h"

#include "Engine/Engine/EngineOptions.h"

#include "Engine/Online/OnlineAchievements.h"

#include "Engine/Localise/Locale.h"

#include "Game/Runner/Game.h"

#define LOG_ACHIEVEMENT_PROGRESS

AchievementManager::AchievementManager()
	: m_init(false)
	, m_first_load(false)
{
}

AchievementManager::~AchievementManager()
{
	for (std::vector<Achievement>::iterator iter = m_achievements.begin(); iter != m_achievements.end(); iter++)
	{
		Achievement& achievement = *iter;
		SAFE_DELETE(achievement.event_listener);
	}
}

int AchievementManager::Get_Achievement_Count()
{
	return m_achievements.size();
}

Achievement* AchievementManager::Get_Achievement(int index)
{
	return &m_achievements.at(index);
}

bool AchievementManager::Init()
{	
	DBG_ASSERT(!m_init);
	DBG_LOG("Loading achievements ...");

	EngineVirtualMachine* vm = GameEngine::Get()->Get_VM();
	CVMLinkedSymbol* base_symbol = vm->Find_Class("Achievement");
	DBG_ASSERT(base_symbol != NULL);

	std::vector<CVMLinkedSymbol*> extended_classes = vm->Find_Derived_Classes(base_symbol);

	m_achievements.resize(extended_classes.size());

	CVMContextLock lock = vm->Set_Context(vm->Get_Static_Context());

	int index = 0;
	for (std::vector<CVMLinkedSymbol*>::iterator iter = extended_classes.begin(); iter != extended_classes.end(); iter++, index++)
	{
		CVMLinkedSymbol* achievement_class = *iter;
		DBG_LOG("Loading Achievement Class: %s", achievement_class->symbol->name);

		Achievement& achievement = m_achievements.at(index);
		achievement.last_state_change_counter = -1;
		achievement.last_progress = 0;
		achievement.unlocked = 0;
		achievement.track_stat_type = NULL;
		achievement.use_stat_progress = false;

		achievement.script_object = vm->New_Object(achievement_class, true, &achievement);
		vm->Set_Default_State(achievement.script_object);

		achievement.event_listener = new ScriptEventListener(vm->Get_Static_Context(), achievement.script_object);
	}

	m_init = true;
	return true;
}

void AchievementManager::Tick(const FrameTime& time)
{
	if (!m_init)
		return;

	if (!StatisticsManager::Get()->Is_Init())
		return;

	// Restricted mode prevents achievements.
	if (Game::Get()->In_Restricted_Mode())
	{
		return;
	}

	EngineVirtualMachine* vm = GameEngine::Get()->Get_VM();
	CVMContextLock lock = vm->Set_Context(vm->Get_Static_Context());

	for (std::vector<Achievement>::iterator iter = m_achievements.begin(); iter != m_achievements.end(); iter++)
	{
		Achievement& achievement = *iter;

		if (achievement.unlocked == true)
		{
			achievement.progress = achievement.max_progress;
			continue;
		}

		// Track statistic.
		if (achievement.track_stat_type != NULL)
		{
			float stat_progress = StatisticsManager::Get()->Get_Statistic(achievement.track_stat_type)->value;
			
			if (achievement.use_stat_progress == true || 
				stat_progress > achievement.max_progress)
			{
				achievement.progress = stat_progress;
			}
		}
		
		// Unlocked?
		if (achievement.progress >= achievement.max_progress)
		{
			DBG_LOG("Unlocked achievement '%s'.", achievement.name.c_str());
			achievement.unlocked = true;
			achievement.progress = achievement.max_progress;

			Game::Get()->Queue_Save();
			
		//	OnlineAchievements::Get()->Set_Progress(achievement.id.c_str(), achievement.max_progress, achievement.max_progress);
			OnlineAchievements::Get()->Unlock(achievement.id.c_str());
		}

		// Log progress.
		// Note: We don't need to do this any longer as progress based achievements
		//		 are attached to statistics now.
		/*else
		{
			if (achievement.progress != achievement.last_progress)
			{
#ifdef LOG_ACHIEVEMENT_PROGRESS
				DBG_LOG("Achievement Progress: %f / %f.", achievement.progress, achievement.max_progress);
#endif

				if (achievement.max_progress != 1)
					OnlineAchievements::Get()->Set_Progress(achievement.id.c_str(), achievement.progress, achievement.max_progress);
			}
			achievement.last_progress = achievement.progress;
		}*/
	}
}

bool AchievementManager::Serialize(BinaryStream* stream)
{
	DBG_ASSERT(m_init);
	
	EngineVirtualMachine* vm = GameEngine::Get()->Get_VM();
	CVMContextLock lock = vm->Set_Context(vm->Get_Static_Context());

	stream->Write<u32>(m_achievements.size());
	
	for (std::vector<Achievement>::iterator iter = m_achievements.begin(); iter != m_achievements.end(); iter++)
	{
		Achievement& achievement = *iter;
		int version = 0;

		BinaryStream achievement_stream;
		achievement.script_object.Get().Get()->Serialize(&achievement_stream, CVMObjectSerializeFlags::Full, &version);

		stream->WriteNullTerminatedString(achievement.id.c_str());
		stream->Write<int>(version);
		stream->Write<int>(achievement_stream.Length());
		stream->WriteBuffer(achievement_stream.Data(), 0, achievement_stream.Length());
	}

	return true;
}

bool AchievementManager::Deserialize(BinaryStream* stream)
{
	EngineVirtualMachine* vm = GameEngine::Get()->Get_VM();
	CVMContextLock lock = vm->Set_Context(vm->Get_Static_Context());

	bool bIgnore = false;

	if (m_first_load == false && *EngineOptions::reset_achievements)
	{		
		for (std::vector<Achievement>::iterator iter = m_achievements.begin(); iter != m_achievements.end(); iter++)
		{
			Achievement& achievement = *iter;
			OnlineAchievements::Get()->Reset(achievement.id.c_str());
			bIgnore = true;
		}
	}


	u32 count = stream->Read<u32>();
	for (u32 i = 0; i < count; i++)
	{
		std::string id = stream->ReadNullTerminatedString();
		int version = stream->Read<int>();
		int length = stream->Read<int>();

		bool found = false;

		if (!bIgnore)
		{
			for (std::vector<Achievement>::iterator iter = m_achievements.begin(); iter != m_achievements.end(); iter++)
			{
				Achievement& achievement = *iter;
				if (achievement.id == id)
				{
					achievement.script_object.Get().Get()->Deserialize(stream, CVMObjectSerializeFlags::Full, version);

					//					DBG_LOG("Achievement: name=%s unlocked=%i", S(achievement.name.c_str()), achievement.unlocked);

					found = true;
					break;
				}
			}
		}

		if (found == false)
		{
			DBG_LOG("Could not deserialize achievement '%s'. Could not be found. Has it been removed?", id.c_str());
			stream->Seek(stream->Position() + length); // Skip over unreadable data.
		}
	}

	m_first_load = true;

	return true;
}