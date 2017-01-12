// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_STEAMWORKS_ONLINE_MODS_
#define _ENGINE_STEAMWORKS_ONLINE_MODS_

#include "Engine/Online/OnlineMods.h"

#include "public/steam/steam_api.h"

class Steamworks_OnlineMods;

class Steamworks_OnlineSubscribedMod : public OnlineSubscribedMod
{
public:
	SteamAPICall_t m_call;
	CCallResult<Steamworks_OnlineMods, SteamUGCRequestUGCDetailsResult_t> m_call_result;

	SteamAPICall_t m_preview_download_call;
	CCallResult<Steamworks_OnlineMods, RemoteStorageDownloadUGCResult_t> m_preview_download_call_result;

	UGCHandle_t	PreviewUGCHandle;

	bool		DownloadingPreview;

	Pixelmap*	Preview;
	Texture*	PreviewTexture;

	bool DetailRequestInProgress;

};

class Steamworks_TaggedRequests
{
public:
	std::vector<std::string> Tags;
	int TotalMatching;
	bool HasGotToEnd;

	CCallResult<Steamworks_OnlineMods, SteamUGCQueryCompleted_t> RequestCallResult;
	SteamAPICall_t RequestCall;
	UGCHandle_t RequestCallHandle;

	int LastRequestPage;

	bool HasMadeRequest;
	bool RequestInProgress;

};

class Steamworks_OnlineMods : public OnlineMods
{
	MEMORY_ALLOCATOR(Steamworks_OnlineMods, "Network");

private:
	std::vector<Steamworks_OnlineSubscribedMod*> m_mods;

	u64 m_mod_id;
	std::string m_mod_name;
	std::string m_mod_description;
	std::string m_mod_path;
	std::string m_mod_preview_path;
	std::vector<std::string> m_mod_tags;
	std::string m_mod_changelog;

	SteamParamStringArray_t m_tag_array;

	CCallResult<Steamworks_OnlineMods, CreateItemResult_t> m_active_ugc_call_result;
	CCallResult<Steamworks_OnlineMods, SubmitItemUpdateResult_t> m_active_ugc_submit_call_result;
	SteamAPICall_t m_active_ugc_call;

	std::vector<Steamworks_TaggedRequests*> m_tagged_requests;

	UGCUpdateHandle_t m_update_handle;

	bool m_upload_active;
	bool m_upload_success;
	bool m_upload_legal_required;

	bool m_item_list_dirty;

	u64 m_upload_total;
	u64 m_upload_progress;

	u64* m_create_mod_output;
	bool* m_create_mod_finished;

	std::vector<u64> m_favorite_list;
	std::vector<u64> m_pending_favorite_list;
	bool m_favorite_request_in_progress;
	int m_favorite_request_page;
	CCallResult<Steamworks_OnlineMods, SteamUGCQueryCompleted_t> m_favorite_request_call_result;
	SteamAPICall_t m_favorite_request_call;

	STEAM_CALLBACK(Steamworks_OnlineMods, OnFavoriteListUpdate, UserFavoriteItemsListChanged_t, m_steam_favorite_list_updated);

protected:
	void On_Mod_Details(SteamUGCRequestUGCDetailsResult_t* param, bool bIOFailure);
	void On_Create_Mod(CreateItemResult_t* param, bool bIOFailure);
	void On_Submit_Mod(SubmitItemUpdateResult_t* param, bool bIOFailure);
	void Update_Mod();

	void On_Recieve_Favorite_Page_Recieved(SteamUGCQueryCompleted_t* param, bool bIOFailure);
	void Start_Favorites_Request();
	void Request_Favorite_Page(int page);

	Steamworks_OnlineSubscribedMod* Find_Or_Create_Item(PublishedFileId_t id);

	void On_UGC_Download_Complete(RemoteStorageDownloadUGCResult_t* param, bool bIOFailure);

	void Request_Tagged_Page(Steamworks_TaggedRequests* request, int page);
	void On_Recieve_Tagged_Page_Recieved(SteamUGCQueryCompleted_t* param, bool bIOFailure);
	Steamworks_TaggedRequests* Find_Or_Create_Tagged_Request(std::vector<std::string> tags);

public:
	Steamworks_OnlineMods();
	~Steamworks_OnlineMods();

	bool Initialize();

	void Tick(const FrameTime& time);

	std::vector<OnlineSubscribedMod*> Get_Subscribed_Mods();
	std::vector<OnlineSubscribedMod*> Get_Favorited_Mods();
	std::vector<OnlineSubscribedMod*> Get_Tagged_Items(std::vector<std::string> tags);
	void Page_Tagged_Items(std::vector<std::string> tags);
	bool Get_Item_Lists_Dirty();

	void Get_Mod_Preview(u64 mod, Pixelmap*& Preview, Texture*& PreviewTexture);

	void Create_Mod(u64* output, bool* bFinished);
	void Publish_Mod(u64 existing_mod_id, std::string name, std::string description, std::string mod_path, std::string preview_path, std::string changelog, std::vector<std::string> tags);
	bool Get_Mod_Update_Progress(bool& wasSuccessful, bool& legalAgreementRequired, u64& modId, u64& uploadedBytes, u64& totalBytes);
	bool Request_Mod_Details(u64 id, OnlineSubscribedMod*& result);

	void VoteUp(u64 mod_id);
	void VoteDown(u64 mod_id);
	void Favorite(u64 mod_id);
	void Unfavorite(u64 mod_id);
	void Subscribe(u64 mod_id);
	void Unsubscribe(u64 mod_id);

};

#endif

