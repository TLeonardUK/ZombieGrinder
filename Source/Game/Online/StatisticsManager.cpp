// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Online/StatisticsManager.h"
#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMBinary.h"

#include "Engine/Engine/GameEngine.h"
#include "Engine/Scripts/EngineVirtualMachine.h"

#include "Engine/Online/OnlineAchievements.h"

#include "Engine/Engine/EngineOptions.h"

#include "Game/Runner/Game.h"

StatisticsManager::StatisticsManager()
	: m_init(false)
	, m_first_load(false)
	, m_aggregate_update_timeout(0.0f)
{
}

StatisticsManager::~StatisticsManager()
{
	for (std::vector<Statistic>::iterator iter = m_statistics.begin(); iter != m_statistics.end(); iter++)
	{
		Statistic& stat = *iter;
		SAFE_DELETE(stat.event_listener);
	}
}

bool StatisticsManager::Init()
{	
	DBG_ASSERT(!m_init);
	DBG_LOG("Loading statistics ...");

	EngineVirtualMachine* vm = GameEngine::Get()->Get_VM();
	CVMLinkedSymbol* base_symbol = vm->Find_Class("Statistic");
	DBG_ASSERT(base_symbol != NULL);

	std::vector<CVMLinkedSymbol*> extended_classes = vm->Find_Derived_Classes(base_symbol);

	m_statistics.resize(extended_classes.size());

	CVMContextLock lock = vm->Set_Context(vm->Get_Static_Context());

	int index = 0;
	for (std::vector<CVMLinkedSymbol*>::iterator iter = extended_classes.begin(); iter != extended_classes.end(); iter++, index++)
	{
		CVMLinkedSymbol* statistic_class = *iter;
		DBG_LOG("Loading Statistic Class: %s", statistic_class->symbol->name);

		Statistic& stat = m_statistics.at(index);
		stat.last_state_change_counter = -1;
		stat.last_progress = 0;
		stat.value = 0;
		stat.last_value = 0;
		stat.aggregated_value = 0;
		stat.mirror_stat_type = NULL;
		stat.type = StatisticDataType::Int;
		stat.script_object = vm->New_Object(statistic_class, true, &stat);
		vm->Set_Default_State(stat.script_object);		
		
		stat.event_listener = new ScriptEventListener(vm->Get_Static_Context(), stat.script_object);
	}

	m_init = true;
	return true;
}

void StatisticsManager::Tick(const FrameTime& time)
{
	if (!m_init)
		return;

	// Restricted mode prevents achievements.
	if (Game::Get()->In_Restricted_Mode())
	{
		return;
	}

	EngineVirtualMachine* vm = GameEngine::Get()->Get_VM();
	CVMContextLock lock = vm->Set_Context(vm->Get_Static_Context());

	m_aggregate_update_timeout -= time.Get_Frame_Time();
	bool update_aggregated = (m_aggregate_update_timeout <= 0);

	if (update_aggregated)
	{
		m_aggregate_update_timeout = aggregate_update_interval;
	}

	for (std::vector<Statistic>::iterator iter = m_statistics.begin(); iter != m_statistics.end(); iter++)
	{
		Statistic& stat = *iter;

		// Mirror values.
		if (stat.mirror_stat_type != NULL)
		{
			Statistic* mirror = Get_Statistic(stat.mirror_stat_type);
			if (mirror != NULL)
			{
				stat.value = mirror->value;
			}
		}

		if (stat.aggregated == true)
		{
			if (update_aggregated)
			{
				// Get the aggregated value.
				if (stat.type == StatisticDataType::Int)
				{
					int value = 0;
					OnlineAchievements::Get()->Get_Global_Stat(stat.id.c_str(), value, stat.aggregation_period);
					stat.aggregated_value = (float)value;
				}
				else
				{
					OnlineAchievements::Get()->Get_Global_Stat(stat.id.c_str(), stat.aggregated_value, stat.aggregation_period);
				}
			}

			// Update global statistic?
			if (stat.value != stat.last_value)
			{
			//	DBG_LOG("Aggregated statistic '%s' changed.", stat.id.c_str());

				if (stat.type == StatisticDataType::Int)
				{
					OnlineAchievements::Get()->Set_Global_Stat(stat.id.c_str(), (int)stat.value);
				}
				else
				{
					OnlineAchievements::Get()->Set_Global_Stat(stat.id.c_str(), stat.value);
				}

				stat.last_value = stat.value;
			}
		}
		else
		{
			if (stat.type == StatisticDataType::Int)
			{
				OnlineAchievements::Get()->Set_Stat(stat.id.c_str(), (int)stat.value);
			}
			else
			{
				OnlineAchievements::Get()->Set_Stat(stat.id.c_str(), stat.value);
			}
		}
	}
}

Statistic* StatisticsManager::Get_Statistic(CVMLinkedSymbol* type)
{
	for (std::vector<Statistic>::iterator iter = m_statistics.begin(); iter != m_statistics.end(); iter++)
	{
		Statistic& stat = *iter;
		if (stat.script_object.Get().Get()->Get_Symbol() == type)
		{
			return &stat;
		}
	}
	return NULL;
}

std::vector<Statistic*> StatisticsManager::Get_Stats()
{
	std::vector<Statistic*> list;
	for (std::vector<Statistic>::iterator iter = m_statistics.begin(); iter != m_statistics.end(); iter++)
	{
		Statistic& stat = *iter;
		list.push_back(&stat);
	}
	return list;
}

bool StatisticsManager::Serialize(BinaryStream* stream)
{
	DBG_ASSERT(m_init);
	
	EngineVirtualMachine* vm = GameEngine::Get()->Get_VM();
	CVMContextLock lock = vm->Set_Context(vm->Get_Static_Context());

	stream->Write<u32>(m_statistics.size());
	
	for (std::vector<Statistic>::iterator iter = m_statistics.begin(); iter != m_statistics.end(); iter++)
	{
		Statistic& stat = *iter;
		int version = 0;

		BinaryStream stat_stream;
		stat.script_object.Get().Get()->Serialize(&stat_stream, CVMObjectSerializeFlags::Full, &version);

		stream->WriteNullTerminatedString(stat.id.c_str());
		stream->Write<int>(version);
		stream->Write<int>(stat_stream.Length());
		stream->WriteBuffer(stat_stream.Data(), 0, stat_stream.Length());
	}

	return true;
}

bool StatisticsManager::Deserialize(BinaryStream* stream)
{
	EngineVirtualMachine* vm = GameEngine::Get()->Get_VM();
	CVMContextLock lock = vm->Set_Context(vm->Get_Static_Context());

	bool bIgnore = false;

	if (m_first_load == false && *EngineOptions::reset_stats)
	{		
		for (std::vector<Statistic>::iterator iter = m_statistics.begin(); iter != m_statistics.end(); iter++)
		{
			Statistic& stat = *iter;
			stat.value = 0;
		}
		bIgnore = true;
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
			for (std::vector<Statistic>::iterator iter = m_statistics.begin(); iter != m_statistics.end(); iter++)
			{
				Statistic& achievement = *iter;
				if (achievement.id == id)
				{
					achievement.script_object.Get().Get()->Deserialize(stream, CVMObjectSerializeFlags::Full, version);
					achievement.last_value = achievement.value;
					found = true;
					break;
				}
			}
		}

		if (found == false)
		{
			DBG_LOG("Could not deserialize stat '%s'. Could not be found. Has it been removed?", id.c_str());
			stream->Seek(stream->Position() + length); // Skip over unreadable data.
		}
	}

	m_first_load = true;

	return true;
}

// Attempts to sync with first-party statistic values.
void StatisticsManager::Sync()
{
	DBG_LOG("Attempt to sync statistics with first-party values.");

	for (std::vector<Statistic>::iterator iter = m_statistics.begin(); iter != m_statistics.end(); iter++)
	{
		Statistic& stat = *iter;
		if (stat.aggregated)
		{
			continue;
		}

		if (stat.type == StatisticDataType::Int)
		{
			int value = 0;
			OnlineAchievements::Get()->Get_Stat(stat.id.c_str(), value);

			if (stat.value < value)
			{
				DBG_LOG("Statistic '%s' has a lower value than first-party state (%i / %i), overwriting.", stat.name.c_str(), (int)stat.value, value);
				stat.value = (float)value;
			}
		}
		else
		{
			float value = 0.0f;
			OnlineAchievements::Get()->Get_Stat(stat.id.c_str(), value);

			if (stat.value < value)
			{
				DBG_LOG("Statistic '%s' has a lower value than first-party state (%f / %f), overwriting.", stat.name.c_str(), stat.value, value);
				stat.value = value;
			}
		}

		stat.last_value = stat.value;
	}
}