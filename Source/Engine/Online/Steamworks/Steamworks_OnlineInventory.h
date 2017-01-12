// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_ONLINE_STEAMWORKS_INVENTORY_
#define _ENGINE_ONLINE_STEAMWORKS_INVENTORY_

#include "Generic/Patterns/Singleton.h"
#include "Engine/Online/OnlineInventory.h"

#include "Engine/Engine/FrameTime.h"

#include "public/steam/steam_api.h"
#include "public/steam/steam_gameserver.h"

#include <string>
#include <vector>

class Steamworks_OnlineInventory : public OnlineInventory
{
	MEMORY_ALLOCATOR(Steamworks_OnlineInventory, "Network");

private:
	SteamInventoryResult_t m_promo_request_result;
	SteamInventoryResult_t m_drop_trigger_result;
	SteamInventoryResult_t m_full_request_result;
	std::vector<SteamInventoryResult_t> m_pending_give_results;
	std::vector<SteamInventoryResult_t> m_pending_consume_results;
	std::vector<SteamInventoryResult_t> m_pending_results;

	std::vector<OnlineInventoryItem> m_items;
	std::vector<OnlineInventoryItem> m_dropped_items;

	STEAM_CALLBACK(Steamworks_OnlineInventory, OnSteamInventoryResult, SteamInventoryResultReady_t, m_steam_inventory_result);
	STEAM_CALLBACK(Steamworks_OnlineInventory, OnSteamInventoryFullUpdate, SteamInventoryFullUpdate_t, m_steam_inventory_full_update);

	void UpdateItems(SteamInventoryResult_t id, bool bFullUpdate);

	void CompleteRequest(SteamInventoryResult_t id);

	int m_update_counter;
	int m_pending_update_counter;
	bool m_has_inventory;
	bool m_last_online_state;

public:	
	Steamworks_OnlineInventory();
	virtual ~Steamworks_OnlineInventory();

	virtual bool CanAccess();

	virtual bool Initialize();

	virtual void Tick(const FrameTime& time);

	virtual void Generate();
	virtual void CheckForDrops(int item_id);

	virtual std::vector<OnlineInventoryItem> GetItems();
	virtual std::vector<OnlineInventoryItem> GetDroppedItems();

	virtual void GiveSelf(int id);

	virtual bool HasInventory();

	virtual int GetUpdateCounter();

	virtual void RemoveItem(u64 id);

};

#endif

