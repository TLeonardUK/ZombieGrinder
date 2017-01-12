// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_ONLINE_USER_
#define _ENGINE_ONLINE_USER_

#include "Generic/Patterns/Singleton.h"

#include "Engine/Input/InputSource.h"

#include "Engine/GeoIP/GeoIPManager.h"

#include <string>
#include <vector>

class Texture;

class OnlineUser 
{
	MEMORY_ALLOCATOR(OnlineUser, "Network");

private:
	int m_id;

public:
	OnlineUser();

	// Numeric identifier of user.
	int Get_ID();

	// Tick user.
	virtual void Tick(const FrameTime& time) = 0;

	// Gets the players username.
	virtual std::string Get_Username() = 0;
	virtual u64 Get_User_ID() = 0;

	// Local player support.
	virtual int Get_Local_Player_Index() = 0;
	virtual void Set_Local_Player_Index(int index) = 0;
	virtual InputSource Get_Input_Source() = 0;
	virtual void Set_Input_Source(InputSource source) = 0;
	virtual int Get_Profile_Index() = 0;
	virtual void Set_Profile_Index(int index) = 0;
	virtual bool Is_Local() = 0;
	virtual bool Is_Guest() = 0;
#ifdef OPT_PREMIUM_ACCOUNTS
	virtual bool Get_Premium() = 0;
	virtual void Set_Premium(bool value) = 0;
#endif
	virtual bool Is_Developer() = 0;

	// GeoIP data.
	virtual GeoIPResult Get_GeoIP_Result() = 0;
	virtual void Set_Short_Country_Name(const char* name) = 0;

	// Guest information.
	virtual int Get_Guest_Count() = 0;

	// Avatar information.
	virtual Texture* Get_Avatar_Texture() = 0;

	// Voice chat information.
	virtual void Add_Voice_Data(const char* data, int data_size, int sample_rate, float output_volume) = 0;
	virtual bool Is_Talking() = 0;
	virtual void Mark_As_Talking() = 0;

};

#endif

