// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Online/Steamworks/Steamworks_OnlineMods.h"
#include "Engine/Online/Steamworks/Steamworks_OnlinePlatform.h"

#include "Engine/Platform/Platform.h"

#include "Engine/Renderer/Textures/PixelmapFactory.h"

#include "Generic/Helper/StringHelper.h"

#include "Engine/IO/BinaryStream.h"

#include <algorithm>

Steamworks_OnlineMods::Steamworks_OnlineMods()
	: m_upload_active(false)
	, m_upload_legal_required(false)
	, m_steam_favorite_list_updated(this, &Steamworks_OnlineMods::OnFavoriteListUpdate)
	, m_item_list_dirty(false)
{
}

Steamworks_OnlineMods::~Steamworks_OnlineMods()
{
	for (std::vector<Steamworks_OnlineSubscribedMod*>::iterator iter = m_mods.begin(); iter != m_mods.end(); iter++)
	{
		SAFE_DELETE(*iter);
	}
	m_mods.clear();
}

void Steamworks_OnlineMods::OnFavoriteListUpdate(UserFavoriteItemsListChanged_t * callback)
{
	DBG_LOG("[Steam Mods] Recieved favorite list update, refreshing favorites list, result=%i.", callback->m_eResult);

	if (callback->m_eResult == k_EResultOK && m_favorite_request_in_progress)
	{
		Start_Favorites_Request();
	}
}

void Steamworks_OnlineMods::Start_Favorites_Request()
{
	m_pending_favorite_list.clear();
	Request_Favorite_Page(1);
}

void Steamworks_OnlineMods::Request_Favorite_Page(int page)
{
	Steamworks_OnlinePlatform* platform = static_cast<Steamworks_OnlinePlatform*>(OnlinePlatform::Get());

	DBG_LOG("[Steam Mods] Requesting favorites page %i.", page);

	UGCQueryHandle_t handle = SteamUGC()->CreateQueryUserUGCRequest(
		SteamUser()->GetSteamID().GetAccountID(),
		k_EUserUGCList_Favorited,
		k_EUGCMatchingUGCType_Items,
		k_EUserUGCListSortOrder_VoteScoreDesc,
		platform->Get_AppID(),
		platform->Get_AppID(),
		page
	);

	m_favorite_request_call = SteamUGC()->SendQueryUGCRequest(handle);
	m_favorite_request_call_result.Set(m_favorite_request_call, this, &Steamworks_OnlineMods::On_Recieve_Favorite_Page_Recieved);

	m_favorite_request_in_progress = true;
	m_favorite_request_page = page;
}

void Steamworks_OnlineMods::On_Recieve_Favorite_Page_Recieved(SteamUGCQueryCompleted_t* param, bool bIOFailure)
{
	if (bIOFailure || param->m_eResult != k_EResultOK)
	{
		DBG_LOG("[Steam Mods] Failed to recieve favorite details due to IO failure.");
		
		m_favorite_request_in_progress = false;
		m_favorite_list = m_pending_favorite_list;

		SteamUGC()->ReleaseQueryUGCRequest(param->m_handle);

		return;
	}
	else
	{
		DBG_LOG("[Steam Mods] Recieved SteamUGCQueryCompleted_t callback for favorites.");

		bool bComplete = false;

		if (param->m_unNumResultsReturned > 0)
		{
			for (unsigned int i = 0; i < param->m_unNumResultsReturned; i++)
			{
				SteamUGCDetails_t details;
				SteamUGC()->GetQueryUGCResult(param->m_handle, i, &details);
				m_pending_favorite_list.push_back((u64)details.m_nPublishedFileId);
			}

			if (m_pending_favorite_list.size() >= param->m_unTotalMatchingResults)
			{
				bComplete = true;
			}
			else
			{
				Request_Favorite_Page(m_favorite_request_page + 1);
			}
		}
		else
		{
			bComplete = true;
		}

		if (bComplete)
		{
			DBG_LOG("[Steam Mods] Favorite list recieved, size=%i.", m_pending_favorite_list.size());

			m_favorite_request_in_progress = false;
			m_favorite_list = m_pending_favorite_list;

			for (unsigned int i = 0; i < m_pending_favorite_list.size(); i++)
			{
				Find_Or_Create_Item((PublishedFileId_t)m_pending_favorite_list[i]);
			}

			for (std::vector<Steamworks_OnlineSubscribedMod*>::iterator iter = m_mods.begin(); iter != m_mods.end(); iter++)
			{
				Steamworks_OnlineSubscribedMod* mod = *iter;
				mod->Favorited = (std::find(m_favorite_list.begin(), m_favorite_list.end(), (u64)mod->ID) != m_favorite_list.end());
			}
		}
	}

	SteamUGC()->ReleaseQueryUGCRequest(param->m_handle);
}

void Steamworks_OnlineMods::On_Mod_Details(SteamUGCRequestUGCDetailsResult_t* param, bool bIOFailure)
{
	for (std::vector<Steamworks_OnlineSubscribedMod*>::iterator iter = m_mods.begin(); iter != m_mods.end(); iter++)
	{
		Steamworks_OnlineSubscribedMod* mod = *iter;
		if (mod->ID == param->m_details.m_nPublishedFileId)
		{
			mod->DetailRequestInProgress = false;
		}
	}

	if (bIOFailure)
	{
		DBG_LOG("[Steam Mods] Failed to recieve mod details due to IO failure.");
		return;
	}
	else
	{
		DBG_LOG("[Steam Mods] Recieved SteamUGCRequestUGCDetailsResult_t callback for mod '%llu'.", param->m_details.m_nPublishedFileId);
	}

	for (std::vector<Steamworks_OnlineSubscribedMod*>::iterator iter = m_mods.begin(); iter != m_mods.end(); iter++)
	{
		Steamworks_OnlineSubscribedMod* mod = *iter;
		if (mod->ID == param->m_details.m_nPublishedFileId)
		{
			if (param->m_details.m_bBanned)
			{
				DBG_LOG("Workshop item %i is banned, unsubscribing.", param->m_details.m_nPublishedFileId);
				SteamUGC()->UnsubscribeItem(param->m_details.m_nPublishedFileId);
				continue;
			}

			uint64 sizeOnDisk = 0;
			char installFolder[255];
			uint32 timestamp = 0;

			mod->InfoAvailable = true;
			mod->Title = param->m_details.m_rgchTitle;
			mod->Description = param->m_details.m_rgchDescription;
			mod->Author = SteamFriends()->GetFriendPersonaName(param->m_details.m_ulSteamIDOwner); 
			mod->IsAuthor = (CSteamID(param->m_details.m_ulSteamIDOwner) == SteamUser()->GetSteamID());
			mod->UpVotes = param->m_details.m_unVotesUp;
			mod->DownVotes = param->m_details.m_unVotesDown;
			mod->Favorited = (std::find(m_favorite_list.begin(), m_favorite_list.end(), mod->ID) != m_favorite_list.end());
			mod->HasVoted = false;
			mod->VotedUp = false;
			StringHelper::Split(param->m_details.m_rgchTags, ',', mod->Tags);
			mod->PreviewUGCHandle = param->m_details.m_hPreviewFile;

			if (SteamUGC()->GetItemInstallInfo(mod->ID, &sizeOnDisk, installFolder, 255, &timestamp))
			{
				mod->Installed = true;
				mod->InstallDir = installFolder;
			}
			else
			{
				mod->Installed = false;
			}

			// Check for admin-tag for whitelisting status.
			for (unsigned int i = 0; i < mod->Tags.size(); i++)
			{
				if (mod->Tags[i] == "Whitelisted")
				{
					mod->IsWhitelisted = true;
					break;
				}
			}

			DBG_LOG("==============================================================================\nInstall Dir: %s\nAuthor: %s\nName: %s\nTags: %s\nVotes: %i/%i\nDescription:\n%s", 
				mod->InstallDir.c_str(),
				mod->Author.c_str(),
				mod->Title.c_str(), 
				param->m_details.m_rgchTags,
				param->m_details.m_unVotesUp,
				param->m_details.m_unVotesDown,
				mod->Description.c_str());
			DBG_LOG("==============================================================================");
		}
	}

	m_item_list_dirty = true;
}

void Steamworks_OnlineMods::On_Create_Mod(CreateItemResult_t* param, bool bIOFailure)
{
	if (param->m_eResult == k_EResultOK && !bIOFailure)
	{
		DBG_LOG("[Steam Mods] Created mod with ID '%llu'", param->m_nPublishedFileId);
		if (param->m_bUserNeedsToAcceptWorkshopLegalAgreement)
		{
			DBG_LOG("[Steam Mods] User needs to accept legal agreement!");
		}
		*m_create_mod_output = param->m_nPublishedFileId;
	}
	else
	{
		DBG_LOG("[Steam Mods] Unable to create new mod with error %i (0x%08x)", param->m_eResult, param->m_eResult);
		*m_create_mod_output = false;
	}

	*m_create_mod_finished = true;
}

void Steamworks_OnlineMods::On_Submit_Mod(SubmitItemUpdateResult_t* param, bool bIOFailure)
{
	if (param->m_eResult == k_EResultOK && !bIOFailure)
	{
		DBG_LOG("[Steam Mods] Updated mod with ID '%llu'", m_mod_id);
		if (param->m_bUserNeedsToAcceptWorkshopLegalAgreement)
		{
			DBG_LOG("[Steam Mods] User needs to accept legal agreement!");
		}

		m_upload_legal_required = param->m_bUserNeedsToAcceptWorkshopLegalAgreement;
		m_upload_active = false;
		m_upload_success = true;
	}
	else
	{
		DBG_LOG("[Steam Mods] Unable to update mod with error %i (0x%08x)", param->m_eResult, param->m_eResult);
		m_upload_active = false;
		m_upload_success = false;
	}

	SAFE_DELETE_ARRAY(m_tag_array.m_ppStrings);
}

Steamworks_OnlineSubscribedMod* Steamworks_OnlineMods::Find_Or_Create_Item(PublishedFileId_t id)
{
	for (std::vector<Steamworks_OnlineSubscribedMod*>::iterator iter = m_mods.begin(); iter != m_mods.end(); iter++)
	{
		Steamworks_OnlineSubscribedMod* item = *iter;
		if (item->ID == id)
		{
			return item;
		}
	}

	Steamworks_OnlineSubscribedMod* mod = new Steamworks_OnlineSubscribedMod();
	mod->InfoAvailable = false;
	mod->ID = id;
	mod->DetailRequestInProgress = true;
	mod->m_call = SteamUGC()->RequestUGCDetails(mod->ID, 60);
	mod->m_call_result.Set(mod->m_call, this, &Steamworks_OnlineMods::On_Mod_Details);
	mod->Preview = NULL;
	mod->PreviewTexture = NULL;
	mod->PreviewUGCHandle = NULL;
	mod->DownloadingPreview = false;
	mod->Subscribed = false;
	mod->Favorited = false;

	DBG_LOG("[Steam Mods] Found subscribed mod '%llu'.", mod->ID);

	m_mods.push_back(mod);

	m_item_list_dirty = true;

	return mod;
}

void Steamworks_OnlineMods::Get_Mod_Preview(u64 mod, Pixelmap*& Preview, Texture*& PreviewTexture)
{
	for (std::vector<Steamworks_OnlineSubscribedMod*>::iterator iter = m_mods.begin(); iter != m_mods.end(); iter++)
	{
		Steamworks_OnlineSubscribedMod* item = *iter;
		if (item->ID == mod)
		{
			if (item->DownloadingPreview)
			{
				return;
			}
			else
			{
				// Got preview, return it!
				if (item->Preview != NULL)
				{
					Preview = item->Preview;
					PreviewTexture = item->PreviewTexture;
				}
				else if (item->PreviewUGCHandle != NULL)
				{
					// Begin download.				
					DBG_LOG("[Steam Mods] Beginning download of mod preview (UGC='%llu')", item->PreviewUGCHandle);
					item->DownloadingPreview = true;
					item->m_preview_download_call = SteamRemoteStorage()->UGCDownload(item->PreviewUGCHandle, 0);
					item->m_preview_download_call_result.Set(item->m_preview_download_call, this, &Steamworks_OnlineMods::On_UGC_Download_Complete);
				}
			}
		}
	}
}

void Steamworks_OnlineMods::On_UGC_Download_Complete(RemoteStorageDownloadUGCResult_t* param, bool bIOFailure)
{
	if (param->m_eResult != k_EResultOK || bIOFailure)
	{
		DBG_LOG("[Steam Mods] Failed to download UGC='%llu', result=%i", param->m_hFile, param->m_eResult);
	}
	else
	{
		DBG_LOG("[Steam Mods] Finished downloading UGC='%llu'", param->m_hFile);

		for (std::vector<Steamworks_OnlineSubscribedMod*>::iterator iter = m_mods.begin(); iter != m_mods.end(); iter++)
		{
			Steamworks_OnlineSubscribedMod* item = *iter;
			if (item->DownloadingPreview && item->PreviewUGCHandle == param->m_hFile)
			{
				char* data = new char[param->m_nSizeInBytes];
				int32 bytes = SteamRemoteStorage()->UGCRead(param->m_hFile, data, param->m_nSizeInBytes, 0, k_EUGCRead_ContinueReadingUntilFinished);
				if (bytes == param->m_nSizeInBytes)
				{
					BinaryStream stream(data, param->m_nSizeInBytes);
					item->Preview = PixelmapFactory::Load(&stream);
					if (item->Preview != NULL)
					{
						item->PreviewTexture = Renderer::Get()->Create_Texture(item->Preview, TextureFlags::NONE);
					}
				}
				item->DownloadingPreview = false;
				SAFE_DELETE(data);
			}
		}
	}
}

bool Steamworks_OnlineMods::Initialize()
{
	PublishedFileId_t file_ids[256];
	uint32 count = SteamUGC()->GetNumSubscribedItems();

	DBG_LOG("User is subscribed to %u files.", count);

	if (count > 0)
	{
		count = SteamUGC()->GetSubscribedItems(file_ids, 256);
		for (uint32 i = 0; i < count; i++)
		{
			Steamworks_OnlineSubscribedMod* mod = Find_Or_Create_Item(file_ids[i]);
			mod->Subscribed = true;
		}

		// Wait till we have all the ugc details, we need them before we can continue.
		if (OnlinePlatform::Get()->Is_Online())
		{
			while (true)
			{
				bool allFinished = true;
				for (uint32 i = 0; i < count; i++)
				{
					if (m_mods[i]->DetailRequestInProgress)
					{
						allFinished = false;
						break;
					}
				}

				if (allFinished)
				{
					break;
				}

				SteamAPI_RunCallbacks();
				Platform::Get()->Sleep(10.0f);
			}
		}
	}

	// Request initial favorites list.
	Start_Favorites_Request();

	return true;
}

void Steamworks_OnlineMods::Tick(const FrameTime& time)
{
	if (m_upload_active)
	{
		SteamUGC()->GetItemUpdateProgress(m_update_handle, &m_upload_progress, &m_upload_progress);
	}
}

std::vector<OnlineSubscribedMod*> Steamworks_OnlineMods::Get_Subscribed_Mods()
{
	std::vector<OnlineSubscribedMod*> mods;

	for (std::vector<Steamworks_OnlineSubscribedMod*>::iterator iter = m_mods.begin(); iter != m_mods.end(); iter++)
	{
		Steamworks_OnlineSubscribedMod* item = *iter;
		if (item->DetailRequestInProgress)
		{
			continue;
		}
		if (item->Subscribed)
		{
			mods.push_back(*iter);
		}
	}

	return mods;
}

std::vector<OnlineSubscribedMod*> Steamworks_OnlineMods::Get_Favorited_Mods()
{
	std::vector<OnlineSubscribedMod*> mods;

	for (std::vector<Steamworks_OnlineSubscribedMod*>::iterator iter = m_mods.begin(); iter != m_mods.end(); iter++)
	{
		Steamworks_OnlineSubscribedMod* item = *iter;
		if (item->DetailRequestInProgress)
		{
			continue;
		}
		if (item->Favorited)
		{
			mods.push_back(*iter);
		}
	}

	return mods;
}

Steamworks_TaggedRequests* Steamworks_OnlineMods::Find_Or_Create_Tagged_Request(std::vector<std::string> tags)
{
	for (std::vector<Steamworks_TaggedRequests*>::iterator iter = m_tagged_requests.begin(); iter != m_tagged_requests.end(); iter++)
	{
		Steamworks_TaggedRequests* request = *iter;

		bool bMatches = true;
		if (request->Tags.size() != tags.size())
		{
			bMatches = false;
		}
		else
		{
			for (unsigned int i = 0; i < tags.size(); i++)
			{
				if (request->Tags[i] != tags[i])
				{
					bMatches = false;
					break;
				}
			}
		}

		if (bMatches)
		{
			return request;
		}
	}

	std::string joined_tags = "";
	for (unsigned int i = 0; i < tags.size(); i++)
	{
		if (joined_tags != "")
		{
			joined_tags += ",";
		}
		joined_tags += tags[i];
	}

	DBG_LOG("[Steam Mods] Created new tagged request using tags '%s'.", joined_tags.c_str());

	Steamworks_TaggedRequests* request = new Steamworks_TaggedRequests();
	request->Tags = tags;
	request->TotalMatching = 0;
	request->RequestInProgress = false;
	request->HasMadeRequest = false;
	request->HasGotToEnd = false;
	m_tagged_requests.push_back(request);

	// Create initial request.
	Request_Tagged_Page(request, 1);

	return request;
}

void Steamworks_OnlineMods::Page_Tagged_Items(std::vector<std::string> tags)
{
	for (std::vector<Steamworks_TaggedRequests*>::iterator iter = m_tagged_requests.begin(); iter != m_tagged_requests.end(); iter++)
	{
		Steamworks_TaggedRequests* request = *iter;

		bool bMatches = true;
		if (request->Tags.size() != tags.size())
		{
			bMatches = false;
		}
		else
		{
			for (unsigned int i = 0; i < tags.size(); i++)
			{
				if (request->Tags[i] != tags[i])
				{
					bMatches = false;
					break;
				}
			}
		}

		if (bMatches)
		{
			if (!request->RequestInProgress && !request->HasGotToEnd)
			{
				Request_Tagged_Page(request, request->LastRequestPage + 1);
			}
			return;
		}
	}
}

void Steamworks_OnlineMods::Request_Tagged_Page(Steamworks_TaggedRequests* request, int page)
{
	Steamworks_OnlinePlatform* platform = static_cast<Steamworks_OnlinePlatform*>(OnlinePlatform::Get());

	DBG_LOG("[Steam Mods] Requesting tagged requests for page %i.", page);

	UGCQueryHandle_t handle;

	handle = SteamUGC()->CreateQueryAllUGCRequest(
		k_EUGCQuery_RankedByVote, 
		k_EUGCMatchingUGCType_Items,
		platform->Get_AppID(),
		platform->Get_AppID(),
		page
	);

	for (std::vector<std::string>::iterator iter = request->Tags.begin(); iter != request->Tags.end(); iter++)
	{
		SteamUGC()->AddRequiredTag(handle, (*iter).c_str());
	}

	request->LastRequestPage = page;
	request->RequestCallHandle = handle;
	request->RequestCall = SteamUGC()->SendQueryUGCRequest(handle);
	request->RequestCallResult.Set(request->RequestCall, this, &Steamworks_OnlineMods::On_Recieve_Tagged_Page_Recieved);
	request->RequestInProgress = true;
	request->HasMadeRequest = true;
}

void Steamworks_OnlineMods::On_Recieve_Tagged_Page_Recieved(SteamUGCQueryCompleted_t* param, bool bIOFailure)
{
	Steamworks_TaggedRequests* request = NULL;

	for (std::vector<Steamworks_TaggedRequests*>::iterator iter = m_tagged_requests.begin(); iter != m_tagged_requests.end(); iter++)
	{
		Steamworks_TaggedRequests* r = *iter;
		if (r->RequestCallHandle == param->m_handle)
		{
			r->RequestInProgress = false;
			request = r;
			break;
		}
	}

	if (request == NULL)
	{
		SteamUGC()->ReleaseQueryUGCRequest(param->m_handle);
		return;
	}

	if (bIOFailure || param->m_eResult != k_EResultOK)
	{
		DBG_LOG("[Steam Mods] Failed to recieve tagged request details due to IO failure.");
		SteamUGC()->ReleaseQueryUGCRequest(param->m_handle);
		request->HasGotToEnd = true;
		return;
	}
	else
	{
		DBG_LOG("[Steam Mods] Recieved SteamUGCQueryCompleted_t callback for tagged page (%i returned).", param->m_unNumResultsReturned);

		if (param->m_unNumResultsReturned > 0)
		{
			request->TotalMatching = param->m_unTotalMatchingResults;

			for (unsigned int i = 0; i < param->m_unNumResultsReturned; i++)
			{
				SteamUGCDetails_t details;
				SteamUGC()->GetQueryUGCResult(param->m_handle, i, &details);
				Find_Or_Create_Item((PublishedFileId_t)details.m_nPublishedFileId);
			}

			request->HasGotToEnd = (param->m_unNumResultsReturned >= (uint32)request->TotalMatching);
		}
		else
		{
			request->HasGotToEnd = true;
		}
	}

	SteamUGC()->ReleaseQueryUGCRequest(param->m_handle);
	m_item_list_dirty = true;
}

std::vector<OnlineSubscribedMod*> Steamworks_OnlineMods::Get_Tagged_Items(std::vector<std::string> tags)
{
	std::vector<OnlineSubscribedMod*> mods;

	Find_Or_Create_Tagged_Request(tags);

	for (std::vector<Steamworks_OnlineSubscribedMod*>::iterator iter = m_mods.begin(); iter != m_mods.end(); iter++)
	{
		Steamworks_OnlineSubscribedMod* item = *iter;

		if (item->DetailRequestInProgress)
		{
			continue;
		}

		bool is_match = false;

		for (std::vector<std::string>::iterator tag_iter = item->Tags.begin(); tag_iter != item->Tags.end(); tag_iter++)
		{
			std::string needle = *tag_iter;
			if (std::find(tags.begin(), tags.end(), needle) != tags.end())
			{
				is_match = true;
				break;
			}
		}

		if (tags.size() == 0 || is_match)
		{
			mods.push_back(*iter);
		}
	}

	return mods;
}

bool Steamworks_OnlineMods::Get_Item_Lists_Dirty()
{
	bool bDirty = m_item_list_dirty;
	m_item_list_dirty = false;
	return bDirty;
}

void Steamworks_OnlineMods::Update_Mod()
{
	Steamworks_OnlinePlatform* platform = static_cast<Steamworks_OnlinePlatform*>(OnlinePlatform::Get());

	DBG_LOG("[Steam Mods] Updating configuration of mod '%llu'", m_mod_id);
	
	m_update_handle = SteamUGC()->StartItemUpdate(platform->Get_AppID(), m_mod_id);

	const char** string_pointers = new const char*[m_mod_tags.size()];
	for (int i = 0; i < (int)m_mod_tags.size(); i++)
	{
		string_pointers[i] = m_mod_tags[i].c_str();
	}

	m_tag_array.m_nNumStrings = m_mod_tags.size();
	m_tag_array.m_ppStrings = string_pointers;

	SteamUGC()->SetItemTitle(m_update_handle, m_mod_name.c_str());
	SteamUGC()->SetItemDescription(m_update_handle, m_mod_description.c_str());
	SteamUGC()->SetItemVisibility(m_update_handle, k_ERemoteStoragePublishedFileVisibilityPublic);
	SteamUGC()->SetItemContent(m_update_handle, m_mod_path.c_str());
	SteamUGC()->SetItemPreview(m_update_handle, m_mod_preview_path.c_str());
	SteamUGC()->SetItemTags(m_update_handle, &m_tag_array);

	m_active_ugc_call = SteamUGC()->SubmitItemUpdate(m_update_handle, m_mod_changelog.c_str());
	m_active_ugc_submit_call_result.Set(m_active_ugc_call, this, &Steamworks_OnlineMods::On_Submit_Mod);
}

void Steamworks_OnlineMods::Create_Mod(u64* output, bool* bFinished)
{
	Steamworks_OnlinePlatform* platform = static_cast<Steamworks_OnlinePlatform*>(OnlinePlatform::Get());

	DBG_LOG("[Steam Mods] Creating new steam mod!");

	m_active_ugc_call = SteamUGC()->CreateItem(platform->Get_AppID(), k_EWorkshopFileTypeCommunity);
	m_active_ugc_call_result.Set(m_active_ugc_call, this, &Steamworks_OnlineMods::On_Create_Mod);
	m_create_mod_output = output;
	m_create_mod_finished = bFinished;
}

void Steamworks_OnlineMods::Publish_Mod(u64 existing_mod_id, std::string name, std::string description, std::string mod_path, std::string preview_path, std::string changelog, std::vector<std::string> tags)
{
	Steamworks_OnlinePlatform* platform = static_cast<Steamworks_OnlinePlatform*>(OnlinePlatform::Get());

	DBG_LOG("[Steam Mods] Attempting to publish mod with steam consumer id '%llu'", existing_mod_id);

	m_mod_id = existing_mod_id;
	m_mod_name = name;
	m_mod_description = description;
	m_mod_path = mod_path;
	m_mod_preview_path = preview_path;
	m_mod_changelog = changelog;
	m_mod_tags = tags;

	m_upload_active = true;

	Update_Mod();
}

bool Steamworks_OnlineMods::Get_Mod_Update_Progress(bool& wasSuccessful, bool& legalAgreementRequired, u64& modId, u64& uploadedBytes, u64& totalBytes)
{
	wasSuccessful = m_upload_success;
	legalAgreementRequired = m_upload_legal_required;
	modId = m_mod_id;
	uploadedBytes = m_upload_progress;
	totalBytes = m_upload_total;

	return !m_upload_active;
}

bool Steamworks_OnlineMods::Request_Mod_Details(u64 id, OnlineSubscribedMod*& result)
{
	// Already exists?
	for (std::vector<Steamworks_OnlineSubscribedMod*>::iterator iter = m_mods.begin(); iter != m_mods.end(); iter++)
	{
		Steamworks_OnlineSubscribedMod* mod = *iter;
		if (mod->ID == id)
		{
			if (mod->InfoAvailable)
			{
				result = mod;
				return true;
			}
			else
			{
				return false;
			}
		}
	}

	// Request info.
	Steamworks_OnlineSubscribedMod* mod = Find_Or_Create_Item((PublishedFileId_t)id);
	mod->InfoAvailable = false;
	mod->ID = id;
	mod->m_call = SteamUGC()->RequestUGCDetails(mod->ID, 60);
	mod->m_call_result.Set(mod->m_call, this, &Steamworks_OnlineMods::On_Mod_Details);
	mod->DetailRequestInProgress = true;

	DBG_LOG("[Steam Mods] Requesting info for mod '%llu'.", mod->ID);

	m_mods.push_back(mod);

	return false;
}

void Steamworks_OnlineMods::VoteUp(u64 mod_id)
{
	SteamUGC()->SetUserItemVote((PublishedFileId_t)mod_id, true);

	for (std::vector<Steamworks_OnlineSubscribedMod*>::iterator iter = m_mods.begin(); iter != m_mods.end(); iter++)
	{
		Steamworks_OnlineSubscribedMod* mod = *iter;
		if (mod->ID == mod_id)
		{
			if (mod->HasVoted)
			{
				if (!mod->VotedUp)
				{
					mod->DownVotes--;
					mod->UpVotes++;
				}
			}
			else
			{
				mod->UpVotes++;
				mod->HasVoted = true;
			}

			mod->VotedUp = true;
		}
	}
}

void Steamworks_OnlineMods::VoteDown(u64 mod_id)
{
	SteamUGC()->SetUserItemVote((PublishedFileId_t)mod_id, false);

	for (std::vector<Steamworks_OnlineSubscribedMod*>::iterator iter = m_mods.begin(); iter != m_mods.end(); iter++)
	{
		Steamworks_OnlineSubscribedMod* mod = *iter;
		if (mod->ID == mod_id)
		{
			if (mod->HasVoted)
			{
				if (mod->VotedUp)
				{
					mod->DownVotes++;
					mod->UpVotes--;
				}
			}
			else
			{
				mod->DownVotes++;
				mod->HasVoted = true;
			}

			mod->VotedUp = false;
		}
	}
}

void Steamworks_OnlineMods::Favorite(u64 mod_id)
{
	Steamworks_OnlinePlatform* platform = static_cast<Steamworks_OnlinePlatform*>(OnlinePlatform::Get());
	SteamUGC()->AddItemToFavorites(platform->Get_AppID(), (PublishedFileId_t)mod_id);

	for (std::vector<Steamworks_OnlineSubscribedMod*>::iterator iter = m_mods.begin(); iter != m_mods.end(); iter++)
	{
		Steamworks_OnlineSubscribedMod* mod = *iter;
		if (mod->ID == mod_id)
		{
			mod->Favorited = true;
		}
	}
}

void Steamworks_OnlineMods::Unfavorite(u64 mod_id)
{
	Steamworks_OnlinePlatform* platform = static_cast<Steamworks_OnlinePlatform*>(OnlinePlatform::Get());
	SteamUGC()->RemoveItemFromFavorites(platform->Get_AppID(), (PublishedFileId_t)mod_id);

	for (std::vector<Steamworks_OnlineSubscribedMod*>::iterator iter = m_mods.begin(); iter != m_mods.end(); iter++)
	{
		Steamworks_OnlineSubscribedMod* mod = *iter;
		if (mod->ID == mod_id)
		{
			mod->Favorited = false;
		}
	}
}

void Steamworks_OnlineMods::Subscribe(u64 mod_id)
{
	SteamUGC()->SubscribeItem((PublishedFileId_t)mod_id);

	for (std::vector<Steamworks_OnlineSubscribedMod*>::iterator iter = m_mods.begin(); iter != m_mods.end(); iter++)
	{
		Steamworks_OnlineSubscribedMod* mod = *iter;
		if (mod->ID == mod_id)
		{
			mod->Subscribed = true;
		}
	}
}

void Steamworks_OnlineMods::Unsubscribe(u64 mod_id)
{
	SteamUGC()->UnsubscribeItem((PublishedFileId_t)mod_id);

	for (std::vector<Steamworks_OnlineSubscribedMod*>::iterator iter = m_mods.begin(); iter != m_mods.end(); iter++)
	{
		Steamworks_OnlineSubscribedMod* mod = *iter;
		if (mod->ID == mod_id)
		{
			mod->Subscribed = false;
		}
	}
}
