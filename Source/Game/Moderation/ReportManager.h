// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_MODERATION_REPORTMANAGER_
#define _GAME_MODERATION_REPORTMANAGER_

#include "Generic/Patterns/Singleton.h"

#include "Engine/Engine/FrameTime.h"

#include "Generic/Types/DataBuffer.h"

#include <vector>
#include <string>

struct CheatCategory
{
	enum Type
	{
		// This is a bitfield enum.
		None	= 0,
		Exploit	= 1,
		Cheat	= 2,
		Abuse	= 4,
		Grief	= 8,
	};
};

struct CheatDownloadState
{
	enum Type
	{
		Idle			= 0,
		Downloading		= 1,
		Success			= 2,
		Failed			= 3
	};
};

struct CheatJudgementResult
{
	enum Type
	{
		Guilty,
		Inconclusive,
		Innocent,
	};
};

struct CheatReport
{
	std::string			Description;
	int					Timeframe;
	CheatCategory::Type	Category;
	DataBuffer			Demo;
	u64					Reported_User;
	std::string			Reported_Username;
};

struct CheatReportJudgement
{
	int							Judgement_ID;
	CheatJudgementResult::Type	Result;
	CheatCategory::Type			Category;
};

struct CheatReportCase
{
	int							Judgement_ID;
	std::string					Demo_File_Path;
	u64							Reported_User_ID;
	CheatCategory::Type			Category;
	int							Timeframe;
	std::string					Reported_Username;

	CheatDownloadState::Type	Download_State;
	float						Download_Progress;
	DataBuffer					Download_Data;
};

struct CheatReportQueueNotification
{
	int					Report_ID;
	std::string			End_Time;	
	std::string			Report_Time;
	CheatCategory::Type	Category;
};

class ReportManager : public Singleton<ReportManager>
{
	MEMORY_ALLOCATOR(ReportManager, "Network");

private:

public:

public:
	virtual ~ReportManager() {}

	static ReportManager* Create();

	// Connects to online platform.
	virtual bool Init() = 0;

	// Tick tock tick tock
	virtual void Tick(const FrameTime& time) = 0;

	// ----------------------------------------------------------------------------
	// Reporting interface.
	// ----------------------------------------------------------------------------

	virtual bool Is_Uploading() = 0;

	virtual bool Is_Available() = 0;
	virtual std::string Get_Auth_Ticket_Hex() = 0;

	virtual bool Has_Notification() = 0;
	virtual bool Has_Queue() = 0;
	virtual std::vector<CheatReportCase>& Get_Queue() = 0;
	virtual std::vector<CheatReportQueueNotification>& Get_Notifications() = 0;

	virtual bool Check_Notification() = 0;
	virtual bool Check_Queue() = 0;
	virtual bool Post_Report(const CheatReport& report) = 0;
	virtual bool Post_Judgement(const CheatReportJudgement& judgement) = 0;

	virtual bool Download_Case(int id) = 0;

};

#endif

