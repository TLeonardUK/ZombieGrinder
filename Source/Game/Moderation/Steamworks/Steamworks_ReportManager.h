// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_MODERATION_STEAMWORKS_REPORTMANAGER_
#define _GAME_MODERATION_STEAMWORKS_REPORTMANAGER_

#include "Game/Moderation/ReportManager.h"

#include "Generic/Patterns/Singleton.h"

#include "Engine/Engine/FrameTime.h"

#include "Engine/Network/NetWebRequest.h"

#include "public/steam/steam_api.h"
#include "public/steam/steam_gameserver.h"

#include "Generic/ThirdParty/RapidJSON/include/rapidjson/document.h"
#include "Generic/ThirdParty/RapidJSON/include/rapidjson/writer.h"
#include "Generic/ThirdParty/RapidJSON/include/rapidjson/stringbuffer.h"

class Steamworks_ReportManager;

class ReportTransaction
{
public:
	bool			bWasSuccess;
	NetWebRequest	Request;

	rapidjson::Document ResultJson;

	Steamworks_ReportManager* Manager;

public:
	ReportTransaction();

	virtual bool Poll();
	virtual bool Was_Success();
	virtual void Fail();
	virtual void Pass();

	virtual void Begin() = 0;
	virtual void Success() = 0;
	virtual void Failed() = 0;
};

class ReportCheckNotificationTransaction : public ReportTransaction
{
public:

public:
	virtual void Begin();
	virtual void Success();
	virtual void Failed();
};

class ReportCheckQueueTransaction : public ReportTransaction
{
public:

public:
	virtual void Begin();
	virtual void Success();
	virtual void Failed();
};

class ReportPostJudgementTransaction : public ReportTransaction
{
public:
	CheatReportJudgement Judgement;

public:
	virtual void Begin();
	virtual void Success();
	virtual void Failed();
};

class ReportPostReportTransaction : public ReportTransaction
{
public:
	CheatReport	Report;

public:
	virtual void Begin();
	virtual void Success();
	virtual void Failed();
};

class DownloadDemoTransaction : public ReportTransaction
{
public:
	int Judgement_ID;
	std::string Path;

public:
	virtual void Begin();
	virtual void Success();
	virtual void Failed();
	virtual bool Poll();
};

class Steamworks_ReportManager : public ReportManager
{
	MEMORY_ALLOCATOR(Steamworks_ReportManager, "Network");

public:
	friend class ReportCheckNotificationTransaction;
	friend class ReportCheckQueueTransaction;

	bool m_has_notification;
	bool m_has_queue;

	std::vector<CheatReportCase> m_queue;
	std::vector<CheatReportQueueNotification> m_notifications;

private:
	bool m_uploading;

	std::vector<ReportTransaction*> m_queued_transactions;
	std::vector<ReportTransaction*> m_active_transactions;

	enum
	{
		MAX_ACTIVE_TRANSACTIONS = 1,
		MAX_AUTH_TICKET_LENGTH = 32 * 1024
	};

	unsigned char m_auth_ticket[MAX_AUTH_TICKET_LENGTH];
	uint32 m_auth_ticket_length;
	HAuthTicket m_auth_ticket_handle;
	bool m_auth_ticket_created;
	bool m_auth_ticket_is_valid;

	STEAM_CALLBACK(Steamworks_ReportManager, Callback_TicketValidated, GetAuthSessionTicketResponse_t, m_callback_auth_ticket_reponse);

public:

	Steamworks_ReportManager();
	virtual ~Steamworks_ReportManager();

	// Checks if the report manager is available.
	bool Is_Available();
	std::string Get_Auth_Ticket_Hex();

	// Connects to online platform.
	virtual bool Init();

	// Tick tock tick tock
	virtual void Tick(const FrameTime& time);

	// ----------------------------------------------------------------------------
	// Reporting interface.
	// ----------------------------------------------------------------------------

	virtual bool Is_Uploading();

	virtual bool Has_Notification();
	virtual bool Has_Queue();
	virtual std::vector<CheatReportCase>& Get_Queue();
	virtual std::vector<CheatReportQueueNotification>& Get_Notifications();

	virtual bool Check_Notification();
	virtual bool Check_Queue();
	virtual bool Post_Report(const CheatReport& report);
	virtual bool Post_Judgement(const CheatReportJudgement& judgement);

	virtual bool Download_Case(int id);

};

#endif

