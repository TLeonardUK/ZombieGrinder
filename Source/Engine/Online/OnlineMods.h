// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_ONLINE_MODS_
#define _ENGINE_ONLINE_MODS_

#include "Generic/Patterns/Singleton.h"

#include "Engine/Engine/FrameTime.h"

#include "Engine/Renderer/Textures/PixelmapFactory.h"
#include "Engine/Renderer/Textures/TextureHandle.h"
#include "Engine/Renderer/Renderer.h"

#include <string>
#include <vector>

class Pixelmap;

class OnlineSubscribedMod
{
public:
	u64							ID;
	std::string					InstallDir;
	std::string					Title;
	std::string					Description;
	std::string					Author;
	bool						IsAuthor;
	bool						Installed;
	bool						InfoAvailable;
	bool						Loaded;
	int							UpVotes;
	int							DownVotes;
	bool						Favorited;
	bool						Subscribed;

	bool						IsWhitelisted;

	bool						HasVoted;
	bool						VotedUp;

	std::vector<std::string>	Tags;

	OnlineSubscribedMod()
		: InfoAvailable(false)
		, IsAuthor(false)
		, Loaded(false)
		, UpVotes(0)
		, DownVotes(0)
		, Favorited(false)
		, Subscribed(false)
		, HasVoted(false)
		, VotedUp(false)
		, IsWhitelisted(false)
	{
	}
};

class OnlineMods : public Singleton<OnlineMods>
{
	MEMORY_ALLOCATOR(OnlineMods, "Network");

private:

public:

public:
	virtual ~OnlineMods() {} 

	static OnlineMods* Create();

	// Connects to online platform.
	virtual bool Initialize() = 0;

	// Tick tock tick tock
	virtual void Tick(const FrameTime& time) = 0;

	// Get all installed mods.
	virtual std::vector<OnlineSubscribedMod*> Get_Subscribed_Mods() = 0;
	virtual std::vector<OnlineSubscribedMod*> Get_Favorited_Mods() = 0;
	virtual std::vector<OnlineSubscribedMod*> Get_Tagged_Items(std::vector<std::string> tags) = 0;
	virtual void Page_Tagged_Items(std::vector<std::string> tags) = 0;
	virtual bool Get_Item_Lists_Dirty() = 0;
	virtual void Get_Mod_Preview(u64 mod, Pixelmap*& Preview, Texture*& PreviewTexture) = 0;

	// Create new mod!
	virtual void Create_Mod(u64* output, bool* bFinished) = 0;
	virtual void Publish_Mod(u64 existing_mod_id, std::string name, std::string description, std::string mod_path, std::string preview_path, std::string changelog, std::vector<std::string> tags) = 0;
	virtual bool Get_Mod_Update_Progress(bool& wasSuccessful, bool& legalAgreementRequired, u64& modId, u64& uploadedBytes, u64& totalBytes) = 0;

	// Mod information.
	virtual bool Request_Mod_Details(u64 id, OnlineSubscribedMod*& result) = 0;

	// Mod management.
	virtual void VoteUp(u64 mod_id) = 0;
	virtual void VoteDown(u64 mod_id) = 0;
	virtual void Favorite(u64 mod_id) = 0;
	virtual void Unfavorite(u64 mod_id) = 0;
	virtual void Subscribe(u64 mod_id) = 0;
	virtual void Unsubscribe(u64 mod_id) = 0;

};

#endif

