// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_PROFILE_MANAGER_
#define _GAME_PROFILE_MANAGER_

#include "Generic/Patterns/Singleton.h"
#include "Game/Profile/Profile.h"

#include "Engine/Engine/FrameTime.h"

#include <vector>

class BinaryStream;
struct Profile;
struct CVMLinkedSymbol;

class ProfileManager : public Singleton<ProfileManager>
{
	MEMORY_ALLOCATOR(ProfileManager, "Game");

public:
	enum
	{
		max_profiles_ = 4,
		max_profiles_premium_ = 16
	};

private:
	bool m_init;
	CVMLinkedSymbol* m_profile_class;
	std::vector<Profile*> m_profiles;

	int m_skill_points_per_level;
	int m_skill_reroll_cost;
	int m_level_cap;

	int* m_level_xp;
	int* m_level_cumulative_xp;

	int m_update_counter;
	unsigned int m_update_profile_counter;

	bool m_syncEnabled;

public:

	ProfileManager();
	~ProfileManager();

	static int Get_Max_Profiles();

	bool		Init();
	
	void		Tick(const FrameTime& time);

	void		Set_Sync_Enabled(bool bEnabled);

	void		Sync_Profiles();
	void		Sync_Profile_To_Inventory(Profile* profile);

	void		Check_Unlocks();

	int			Get_Level_Cap();
	int			Get_Skill_Points_Per_Level();
	int			Get_Skill_Reroll_Cost();
	int			Get_Level_XP(int level);
	int			Get_Level_Cumulative_XP(int level);

	int			Get_Profile_Count();
	Profile*	Get_Profile(int index);
	int			Create_Profile();
	void		Delete_Profile(int index);

	bool		Serialize(BinaryStream* stream);
	bool		Deserialize(BinaryStream* stream);

	DataBuffer	Profile_To_Buffer(Profile* profile);
	Profile*	Buffer_To_Profile(DataBuffer buffer);

};

#endif

