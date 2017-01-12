// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Moderation/Steamworks/Steamworks_ReportManager.h"
#include "Engine/Platform/Platform.h"
#include "Engine/Network/NetWebRequest.h"
#include "Generic/Helper/StringHelper.h"

#include "Engine/Network/NetManager.h"

#include "Engine/Online/OnlinePlatform.h"

#define CHEAT_REPORT_HOST		"api.zombiegrinder.com"
#define CHEAT_REPORT_PORT		80
#define CHEAT_REPORT_SSL		false
#define CHEAT_REPORT_URI		"/ICheatReporting/v001"

//#define OPT_UPLOAD_DEBUG_VERSION

#define OPT_LOG_REPORT_DEBUG

Steamworks_ReportManager::Steamworks_ReportManager()
	: m_uploading(false)
	, m_has_notification(false)
	, m_has_queue(false)
	, m_auth_ticket_created(false)
	, m_auth_ticket_is_valid(false)
	, m_callback_auth_ticket_reponse(this, &Steamworks_ReportManager::Callback_TicketValidated)
{
}

Steamworks_ReportManager::~Steamworks_ReportManager()
{
	if (m_auth_ticket_created)
	{
		DBG_LOG("Cancelling WebAPI session ticket ...");
		SteamUser()->CancelAuthTicket(m_auth_ticket_handle);
		m_auth_ticket_created = false;
	}
}

bool Steamworks_ReportManager::Init()
{
	Check_Notification();
	Check_Queue();

	return true;
}

void Steamworks_ReportManager::Tick(const FrameTime& time)
{
#ifdef OPT_UPLOAD_DEBUG_VERSION
	static bool bTestStarted = false;
	if (!bTestStarted)
	{
		CheatReport report;
		report.Category = CheatCategory::Abuse;
		report.Description = "This is a test blah asdkadfl gjsdlfkgj sldkfjglsdkfjg";
		report.Timeframe = 60;
		report.Demo.Reserve(128 * 1024, false);
		report.Reported_User = 76561197977204045;
		Post_Report(report);

		bTestStarted = true;
	}
#endif

	// Not useful on dedi server.
	if (NetManager::Get()->Is_Dedicated_Server())
	{
		return;
	}

	if (m_auth_ticket_is_valid)
	{
		for (std::vector<ReportTransaction*>::iterator iter = m_active_transactions.begin(); iter != m_active_transactions.end(); )
		{
			ReportTransaction* transaction = *iter;
		
			if (transaction->Poll())
			{
				if (transaction->Was_Success())
				{
					DBG_LOG("Transaction 0x%08x has finished with success.");
					transaction->Success();
				}
				else
				{
					DBG_LOG("Transaction 0x%08x has finished with failure.");
					transaction->Failed();
				}

				SAFE_DELETE(transaction);

				iter = m_active_transactions.erase(iter);
				continue;
			}

			iter++;
		}

		while (m_queued_transactions.size() > 0 && m_active_transactions.size() < MAX_ACTIVE_TRANSACTIONS)
		{
			ReportTransaction* transaction = *m_queued_transactions.begin();
			m_queued_transactions.erase(m_queued_transactions.begin());

			DBG_LOG("Beginning transaction 0x%08x.", transaction);
			transaction->Begin();

			m_active_transactions.push_back(transaction);
		}
	}
	else if (OnlinePlatform::Get()->Is_Online())
	{
		if (!m_auth_ticket_created)
		{
			DBG_LOG("Creating WebAPI session ticket ...");
			m_auth_ticket_handle = SteamUser()->GetAuthSessionTicket(m_auth_ticket, MAX_AUTH_TICKET_LENGTH, &m_auth_ticket_length);
			m_auth_ticket_created = true;
		}
	}
}

bool Steamworks_ReportManager::Is_Available()
{
	return OnlinePlatform::Get()->Is_Online() && m_auth_ticket_is_valid;
}

void Steamworks_ReportManager::Callback_TicketValidated(GetAuthSessionTicketResponse_t* param)
{
	if (param->m_hAuthTicket == m_auth_ticket_handle)
	{
		if (param->m_eResult == k_EResultOK)
		{
			DBG_LOG("WebAPI session ticket was validated.");
			m_auth_ticket_is_valid = true;
		}
		else
		{
			DBG_LOG("WebAPI session ticket was not valid, result is '%i'.", param->m_eResult);
		}
	}
}

std::string Steamworks_ReportManager::Get_Auth_Ticket_Hex()
{
	std::string value = "";

	for (unsigned int i = 0; i < m_auth_ticket_length; i++)
	{
		unsigned char val = m_auth_ticket[i];
		std::string v = StringHelper::Format("%02x", val);
		value += v;
	}

	return value;
}

bool Steamworks_ReportManager::Is_Uploading()
{
	return m_queued_transactions.size() > 0 || m_active_transactions.size() > 0;
}

bool Steamworks_ReportManager::Has_Notification()
{
	return m_has_notification;
}

bool Steamworks_ReportManager::Has_Queue()
{
	return m_has_queue;
}

std::vector<CheatReportCase>& Steamworks_ReportManager::Get_Queue()
{
	return m_queue;
}

std::vector<CheatReportQueueNotification>& Steamworks_ReportManager::Get_Notifications()
{
	return m_notifications;
}

bool Steamworks_ReportManager::Check_Notification()
{
	ReportCheckNotificationTransaction* upload = new ReportCheckNotificationTransaction();
	upload->Manager = this;
	m_queued_transactions.push_back(upload);
	return false;
}

bool Steamworks_ReportManager::Check_Queue()
{
	ReportCheckQueueTransaction* upload = new ReportCheckQueueTransaction();
	upload->Manager = this;
	m_queued_transactions.push_back(upload);
	return false;
}

bool Steamworks_ReportManager::Post_Report(const CheatReport& report)
{
	ReportPostReportTransaction* upload = new ReportPostReportTransaction();
	upload->Manager = this;
	upload->Report = report;
	m_queued_transactions.push_back(upload);
	return true;
}

bool Steamworks_ReportManager::Post_Judgement(const CheatReportJudgement& judgement)
{
	// Remove judgement from our queue.
	for (unsigned int i = 0; i < m_queue.size(); i++)
	{
		if (m_queue[i].Judgement_ID == judgement.Judgement_ID)
		{
			m_queue.erase(m_queue.begin() + i);
			i--;
		}
	}

	m_has_queue = (m_queue.size() > 0);

	ReportPostJudgementTransaction* upload = new ReportPostJudgementTransaction();
	upload->Manager = this;
	upload->Judgement = judgement;
	m_queued_transactions.push_back(upload);
	return true;
}

bool Steamworks_ReportManager::Download_Case(int id)
{
	for (unsigned int i = 0; i < m_queue.size(); i++)
	{
		CheatReportCase& cheatCase = m_queue[i];
		if (cheatCase.Judgement_ID == id)
		{
			if (cheatCase.Download_State == CheatDownloadState::Downloading)
			{
				return false;
			}

			cheatCase.Download_State = CheatDownloadState::Downloading;

			DownloadDemoTransaction* upload = new DownloadDemoTransaction();
			upload->Manager = this;
			upload->Judgement_ID = id;
			upload->Path = cheatCase.Demo_File_Path;
			m_queued_transactions.push_back(upload);

			return true;
		}
	}

	return false;
}

ReportTransaction::ReportTransaction()
	: bWasSuccess(false)
{
}

void ReportTransaction::Fail()
{
	bWasSuccess = true;
}

void ReportTransaction::Pass()
{
	bWasSuccess = true;
}

bool ReportTransaction::Was_Success()
{
	return bWasSuccess;
}

bool ReportTransaction::Poll()
{
	if (Request.Poll())
	{
		bWasSuccess = Request.Was_Success();

		DataBuffer buffer = Request.Get_Result();

#ifdef OPT_LOG_REPORT_DEBUG
		DBG_LOG("[Report] Recieve: %s", buffer.Buffer());
#endif

		// Parse the JSON result.
		ResultJson.Parse(buffer.Buffer());

		// If we failed, the whole transaction failed.
		if (ResultJson.HasParseError())
		{
#ifdef OPT_LOG_REPORT_DEBUG
			DBG_LOG("[Report] Parse Error: %s", buffer.Buffer());
#endif
			bWasSuccess = false;
		}

		// General failure.
		if (!ResultJson["success"].GetBool())
		{
#ifdef OPT_LOG_REPORT_DEBUG
			DBG_LOG("[Report] Failed with error: %s", ResultJson["result"]["message"].GetString());
#endif
			bWasSuccess = false;
		}

		return true;
	}
	return false;
}

void ReportPostReportTransaction::Begin()
{
	DBG_LOG("Beginning cheat report upload.");

	Request.Add_Body_Parameter("steam_id",				 StringHelper::To_String(SteamUser()->GetSteamID().ConvertToUint64()));
	Request.Add_Body_Parameter("auth_ticket",			 ReportManager::Get()->Get_Auth_Ticket_Hex());
	Request.Add_Body_Parameter("reported_user",			 StringHelper::To_String(Report.Reported_User));
	Request.Add_Body_Parameter("reported_username",		 StringHelper::To_String(Report.Reported_Username));
	Request.Add_Body_Parameter("timeframe",			(int)Report.Timeframe);
	Request.Add_Body_Parameter("description",			 Report.Description);
	Request.Add_Body_Parameter("category",			(int)Report.Category);
	Request.Add_Body_Parameter("demo",					 Report.Demo);
	Request.Post(
		CHEAT_REPORT_HOST, 
		CHEAT_REPORT_PORT, 
		CHEAT_REPORT_SSL, 
		CHEAT_REPORT_URI "/PostReport/"
	);
}

void ReportPostReportTransaction::Success()
{
	DBG_LOG("Cheat report was successfully uploaded.");
}

void ReportPostReportTransaction::Failed()
{
	DBG_LOG("Cheat report failed to be uploaded.");
}

void ReportCheckNotificationTransaction::Begin()
{
	DBG_LOG("Starting get notification request.");

	Request.Add_Body_Parameter("steam_id", StringHelper::To_String(SteamUser()->GetSteamID().ConvertToUint64()));
	Request.Add_Body_Parameter("auth_ticket", ReportManager::Get()->Get_Auth_Ticket_Hex());
	Request.Post(
		CHEAT_REPORT_HOST,
		CHEAT_REPORT_PORT,
		CHEAT_REPORT_SSL,
		CHEAT_REPORT_URI "/GetNotification/"
		);
}

void ReportCheckNotificationTransaction::Success()
{
	DBG_LOG("Get notification was successful.");

	Manager->m_notifications.clear();

	for (rapidjson::SizeType i = 0; i < ResultJson["result"]["bans"].Size(); i++)
	{
		const rapidjson::Value& value = ResultJson["result"]["bans"][i];

		CheatReportQueueNotification notification;
		notification.End_Time		= value["end_time"].GetString();
		notification.Report_Time	= value["report_time"].GetString();
		notification.Report_ID		= value["report_id"].GetInt();
		notification.Category		= (CheatCategory::Type)value["category"].GetInt();
		Manager->m_notifications.push_back(notification);

		DBG_LOG("Ban[%i] end=%s reported=%s category=%i report_id=%i", 
			i, 
			notification.End_Time.c_str(),
			notification.Report_Time.c_str(),
			notification.Category,
			notification.Report_ID);
	}

	Manager->m_has_notification = (Manager->m_notifications.size() > 0);
}

void ReportCheckNotificationTransaction::Failed()
{
	DBG_LOG("Get own cheat status failed.");
}

void ReportCheckQueueTransaction::Begin()
{
	DBG_LOG("Starting get queue request.");

	Request.Add_Body_Parameter("steam_id", StringHelper::To_String(SteamUser()->GetSteamID().ConvertToUint64()));
	Request.Add_Body_Parameter("auth_ticket", ReportManager::Get()->Get_Auth_Ticket_Hex());
	Request.Post(
		CHEAT_REPORT_HOST,
		CHEAT_REPORT_PORT,
		CHEAT_REPORT_SSL,
		CHEAT_REPORT_URI "/GetQueue/"
		);
}

void ReportCheckQueueTransaction::Success()
{
	DBG_LOG("Get cheat queue was successful.");

	Manager->m_queue.clear();

	for (rapidjson::SizeType i = 0; i < ResultJson["result"]["cases"].Size(); i++)
	{
		const rapidjson::Value& value = ResultJson["result"]["cases"][i];

		CheatReportCase notification;
		notification.Judgement_ID		= value["judgement_id"].GetInt();
		notification.Demo_File_Path		= value["demo_file_path"].GetString();
		notification.Reported_User_ID	= value["reported_user_id"].GetInt64();
		notification.Category			= (CheatCategory::Type)value["category"].GetInt();
		notification.Timeframe			= value["timeframe"].GetInt();
		notification.Reported_Username	= value["reported_username"].GetString();
		notification.Download_State		= CheatDownloadState::Idle;
		notification.Download_Progress	= 0.0f;

		Manager->m_queue.push_back(notification);

		DBG_LOG("Case[%i] judgement_id=%i demo=%s",
			i,
			notification.Judgement_ID,
			notification.Demo_File_Path.c_str());
	}

	Manager->m_has_queue = (Manager->m_queue.size() > 0);
}

void ReportCheckQueueTransaction::Failed()
{
	DBG_LOG("Get cheat queue failed.");
}

void ReportPostJudgementTransaction::Begin()
{
	DBG_LOG("Starting judgement request.");

	Request.Add_Body_Parameter("steam_id", StringHelper::To_String(SteamUser()->GetSteamID().ConvertToUint64()));
	Request.Add_Body_Parameter("auth_ticket", ReportManager::Get()->Get_Auth_Ticket_Hex());
	Request.Add_Body_Parameter("judgement_id", Judgement.Judgement_ID);
	Request.Add_Body_Parameter("result", (int)Judgement.Result);
	Request.Add_Body_Parameter("category", (int)Judgement.Category);
	Request.Post(
		CHEAT_REPORT_HOST,
		CHEAT_REPORT_PORT,
		CHEAT_REPORT_SSL,
		CHEAT_REPORT_URI "/PostJudgement/"
		);
}

void ReportPostJudgementTransaction::Success()
{
	DBG_LOG("Pass cheat judgement was successful.");
}

void ReportPostJudgementTransaction::Failed()
{
	DBG_LOG("Pass cheat judgement failed.");
}

void DownloadDemoTransaction::Begin()
{
	DBG_LOG("Starting download of '%s'.", Path.c_str());

	std::string Protocol = "http";
	std::string Host = "";
	std::string URI = "/";
	int Port = 80;
	
	Request.Crack_URL(Path, Protocol, Host, Port, URI);

	Request.Get(
		Host,
		Port,
		false,
		URI
	);
}

bool DownloadDemoTransaction::Poll()
{
	bool bResult = false;
	float Progress = 0.0f;

	if (Request.Poll())
	{
		bWasSuccess = Request.Was_Success();

		bResult = true;
		Progress = 1.0f;
	}
	else
	{
		Progress = Request.Get_Progress();
	}

	for (unsigned int i = 0; i < Manager->m_queue.size(); i++)
	{
		CheatReportCase& cheatCase = Manager->m_queue[i];
		if (cheatCase.Judgement_ID == Judgement_ID)
		{
			cheatCase.Download_State = CheatDownloadState::Downloading;
			cheatCase.Download_Progress = Progress;
		}
	}

	return bResult;
}

void DownloadDemoTransaction::Success()
{
	DBG_LOG("Successfully downloaded '%s'.", Path.c_str());

	for (unsigned int i = 0; i < Manager->m_queue.size(); i++)
	{
		CheatReportCase& cheatCase = Manager->m_queue[i];
		if (cheatCase.Judgement_ID == Judgement_ID)
		{
			cheatCase.Download_State = CheatDownloadState::Success;
			cheatCase.Download_Progress = 1.0f;
			cheatCase.Download_Data = Request.Get_Result();
		}
	}
}

void DownloadDemoTransaction::Failed()
{
	DBG_LOG("Failed to download '%s'.", Path.c_str());

	for (unsigned int i = 0; i < Manager->m_queue.size(); i++)
	{
		CheatReportCase& cheatCase = Manager->m_queue[i];
		if (cheatCase.Judgement_ID == Judgement_ID)
		{
			cheatCase.Download_State = CheatDownloadState::Failed;
		}
	}
}