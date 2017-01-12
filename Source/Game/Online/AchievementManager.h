// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_ONLINE_ACHIEVEMENT_MANAGER_
#define _GAME_ONLINE_ACHIEVEMENT_MANAGER_

#include "Engine/IO/BinaryStream.h"
#include "Engine/Engine/FrameTime.h"

#include "XScript/VirtualMachine/CVMObject.h"

#include "Game/Scripts/ScriptEventListener.h"

#include "Generic/Patterns/Singleton.h"

struct AtlasFrame;

struct Achievement
{
public:
	std::string			id;
	std::string			name;
	std::string			description;
	float				progress;
	float				max_progress;
	std::string			locked_frame_name;
	AtlasFrame*			locked_frame;
	std::string			unlocked_frame_name;
	AtlasFrame*			unlocked_frame;
	bool				unlocked;

	CVMLinkedSymbol*	track_stat_type;
	bool				use_stat_progress;

	float				last_progress;
	int					last_state_change_counter;
	ScriptEventListener* event_listener;
	CVMGCRoot			script_object;

};

class AchievementManager : public Singleton<AchievementManager>
{
	MEMORY_ALLOCATOR(AchievementManager, "Game");

private:
	bool m_init;
	bool m_first_load;

	std::vector<Achievement> m_achievements;

public:
	AchievementManager();
	~AchievementManager();

	bool Init();

	int Get_Achievement_Count();
	Achievement* Get_Achievement(int index);

	bool Serialize(BinaryStream* stream);
	bool Deserialize(BinaryStream* stream);

	void Tick(const FrameTime& time);

};

#endif

