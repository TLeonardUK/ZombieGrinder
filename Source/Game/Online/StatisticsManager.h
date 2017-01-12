// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_ONLINE_STATISTICS_MANAGER_
#define _GAME_ONLINE_STATISTICS_MANAGER_

#include "Engine/IO/BinaryStream.h"
#include "Engine/Engine/FrameTime.h"

#include "XScript/VirtualMachine/CVMObject.h"

#include "Game/Scripts/ScriptEventListener.h"

#include "Generic/Patterns/Singleton.h"

struct AtlasFrame;

struct StatisticDataType
{
	enum Type
	{
		Int,
		Float
	};
};

struct Statistic
{
public:
	std::string			id;
	std::string			name;
	std::string			description;
	std::string			category;
	bool				aggregated;
	int					aggregation_period;
	bool				display;
	float				value;
	float				last_value;
	float				aggregated_value;

	StatisticDataType::Type	type;

	CVMLinkedSymbol*	mirror_stat_type;

	int					last_progress;
	int					last_state_change_counter;
	ScriptEventListener*	event_listener;
	CVMGCRoot		script_object;

};

struct StatisticsManager : public Singleton<StatisticsManager>
{
	MEMORY_ALLOCATOR(StatisticsManager, "Game");

private:
	bool m_init;
	bool m_first_load;
	
	std::vector<Statistic> m_statistics;

	float m_aggregate_update_timeout;

	enum
	{
		aggregate_update_interval = 5 * 60 * 1000
	};

public:
	StatisticsManager();
	~StatisticsManager();

	INLINE bool Is_Init()
	{
		return m_init;
	}

	bool Init();

	void Sync();

	bool Serialize(BinaryStream* stream);
	bool Deserialize(BinaryStream* stream);

	void Tick(const FrameTime& time);

	Statistic* Get_Statistic(CVMLinkedSymbol* type);

	std::vector<Statistic*> Get_Stats();

};

#endif

