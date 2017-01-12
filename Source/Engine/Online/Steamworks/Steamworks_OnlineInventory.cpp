// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Online/Steamworks/Steamworks_OnlinePlatform.h"
#include "Engine/Online/Steamworks/Steamworks_OnlineMatching.h"
#include "Engine/Online/Steamworks/Steamworks_OnlineUser.h"
#include "Engine/Online/Steamworks/Steamworks_OnlineClient.h"
#include "Engine/Online/Steamworks/Steamworks_OnlineInventory.h"

#include "Engine/Network/NetConnection.h"

#include "Engine/Platform/Platform.h"

#include "Engine/Input/Input.h"

#include "Engine/Localise/Locale.h"

#include "Engine/Network/NetManager.h"
#include "Engine/Network/NetClient.h"
#include "Engine/Network/NetServer.h"
#include "Engine/Network/Packets/EnginePackets.h"

#include "Engine/Engine/EngineOptions.h"

#include "Engine/Engine/GameEngine.h"
#include "Engine/Engine/GameRunner.h"

#include "Engine/Version.h"

Steamworks_OnlineInventory::Steamworks_OnlineInventory()
	: m_steam_inventory_result(this, &Steamworks_OnlineInventory::OnSteamInventoryResult)
	, m_steam_inventory_full_update(this, &Steamworks_OnlineInventory::OnSteamInventoryFullUpdate)
	, m_update_counter(-1)
	, m_has_inventory(false)
	, m_last_online_state(false)
	, m_pending_update_counter(-1)
{
	m_promo_request_result = k_SteamInventoryResultInvalid;
	m_drop_trigger_result = k_SteamInventoryResultInvalid;
	m_full_request_result = k_SteamInventoryResultInvalid; 
} 

Steamworks_OnlineInventory::~Steamworks_OnlineInventory()
{
}

bool Steamworks_OnlineInventory::CanAccess()
{
	return ((m_has_inventory || m_full_request_result != k_SteamInventoryResultInvalid) && SteamUser()->BLoggedOn());
}

bool Steamworks_OnlineInventory::Initialize()
{
	DBG_LOG("Setting up online inventory.");

	SteamInventory()->GrantPromoItems(&m_promo_request_result);
	SteamInventory()->GetAllItems(&m_full_request_result);

	m_pending_results.push_back(m_full_request_result);
	m_pending_results.push_back(m_promo_request_result);

	return true;
}

void Steamworks_OnlineInventory::Tick(const FrameTime& time)
{
	// Come online? Need to refresh inventory.
	if (m_full_request_result == k_SteamInventoryResultInvalid)
	{
		if (m_last_online_state != SteamUser()->BLoggedOn())
		{
			SteamInventory()->GetAllItems(&m_full_request_result);
			m_pending_results.push_back(m_full_request_result);
			m_last_online_state = SteamUser()->BLoggedOn();
		}
	}

	if (m_pending_results.size() <= 0)
	{
		m_update_counter = m_pending_update_counter;
	}
}

void Steamworks_OnlineInventory::Generate()
{
	if (!GameEngine::Get()->Get_Runner()->In_Restricted_Mode())
	{
		SteamInventory()->SendItemDropHeartbeat();
	}
}

void Steamworks_OnlineInventory::CheckForDrops(int item_id)
{
	if (!GameEngine::Get()->Get_Runner()->In_Restricted_Mode())
	{
		SteamInventory()->TriggerItemDrop(&m_drop_trigger_result, item_id);
		m_pending_results.push_back(m_drop_trigger_result);
	}
}

std::vector<OnlineInventoryItem> Steamworks_OnlineInventory::GetItems()
{
	return m_items;
}

std::vector<OnlineInventoryItem> Steamworks_OnlineInventory::GetDroppedItems()
{
	std::vector<OnlineInventoryItem> items = m_dropped_items;
	m_dropped_items.clear();
	return items;
}

bool Steamworks_OnlineInventory::HasInventory()
{
	return m_has_inventory;
}

void Steamworks_OnlineInventory::RemoveItem(u64 id)
{
	DBG_LOG("[Steam Inventory] Removing item: %llu", id);

	SteamInventoryResult_t result;
	SteamInventory()->ConsumeItem(&result, id, 1);
	m_pending_consume_results.push_back(result);
	m_pending_results.push_back(result);
}

void Steamworks_OnlineInventory::GiveSelf(int id)
{
	DBG_LOG("[Steam Inventory] Giving self: %i", id);

	std::vector<SteamItemDef_t> newItems;
	newItems.push_back(id);

	SteamInventoryResult_t result;
	SteamInventory()->GenerateItems(&result, newItems.data(), NULL, (uint32)newItems.size());
	m_pending_give_results.push_back(result);
	m_pending_results.push_back(result);
}

void Steamworks_OnlineInventory::UpdateItems(SteamInventoryResult_t id, bool bFullUpdate)
{
	bool bHasResult = false;
	std::vector<SteamItemDetails_t> items;
	uint32 count = 0;

	if (SteamInventory()->GetResultItems(id, NULL, &count))
	{
		items.resize(count);
		bHasResult = SteamInventory()->GetResultItems(id, items.data(), &count);
	}

	if (bHasResult)
	{
		m_dropped_items.clear();

		for (std::vector<SteamItemDetails_t>::iterator iter = items.begin(); iter != items.end(); iter++)
		{
			SteamItemDetails_t& new_item = *iter;
			bool bIsNew = true;

			for (std::vector<OnlineInventoryItem>::iterator iter2 = m_items.begin(); iter2 != m_items.end(); )
			{
				OnlineInventoryItem& existing_item = *iter2;
				bool bWasDeleted = false;

				// Update pre-existing item?
				if (existing_item.unique_id64 == new_item.m_itemId)
				{
					// Removed?
					if (new_item.m_unFlags & k_ESteamItemRemoved)
					{
						bWasDeleted = true;
					}	

					bIsNew = false;
				}

				if (bWasDeleted)
				{
					DBG_LOG("[Steam Inventory] Item %llu deleted.", new_item.m_itemId);
					iter2 = m_items.erase(iter2);
				}
				else
				{
					iter2++;
				}
			}

			// Add new item to list.
			if (bIsNew && new_item.m_itemId != 0 && (new_item.m_unFlags & k_ESteamItemRemoved) == 0)
			{
				DBG_LOG("[Steam Inventory] Item %llu added.", new_item.m_itemId);

				OnlineInventoryItem add_item;
				add_item.type_id = new_item.m_iDefinition;
				add_item.unique_id64 = new_item.m_itemId;
				add_item.unique_id = StringHelper::To_String(add_item.unique_id64);

				// TODO: This shit needs fixing immediately. Stats will not transfer with trades currently
				//		 we need to use originalitemid, but steam doesn't expose it yet.
				add_item.original_unique_id64 = add_item.unique_id64;
				add_item.original_unique_id = add_item.unique_id;

				if (id == m_drop_trigger_result)
				{
					m_dropped_items.push_back(add_item);
				}

				m_items.push_back(add_item);
			}
		}
	}

	m_pending_update_counter++;
}

void Steamworks_OnlineInventory::OnSteamInventoryResult(SteamInventoryResultReady_t *callback)
{
	DBG_LOG("[Steam Inventory] Recieved call result, result=%i.", callback->m_result);

	// Process result.
	if (callback->m_result == k_EResultOK &&
		SteamInventory()->CheckResultSteamID( callback->m_handle, SteamUser()->GetSteamID() ))
	{
		DBG_LOG("[Steam Inventory] Operation was a success.");
		UpdateItems(callback->m_handle, false);
	}

	CompleteRequest(callback->m_handle);
}

void Steamworks_OnlineInventory::OnSteamInventoryFullUpdate(SteamInventoryFullUpdate_t *callback)
{
	DBG_LOG("[Steam Inventory] Recieved full inventory update.");
	UpdateItems(callback->m_handle, true);

	m_has_inventory = true;

	CompleteRequest(callback->m_handle);
}

void Steamworks_OnlineInventory::CompleteRequest(SteamInventoryResult_t id)
{
	// Clear out any pending handles.
	if (id == m_promo_request_result)
	{
		m_promo_request_result = -1;
	}
	else if (id == m_drop_trigger_result)
	{
		m_drop_trigger_result = -1;
	}
	else if (id == m_full_request_result)
	{
		m_full_request_result = k_SteamInventoryResultInvalid;
	}

	std::vector<SteamInventoryResult_t>::iterator iter = std::find(m_pending_give_results.begin(), m_pending_give_results.end(), id);
	if (iter != m_pending_give_results.end())
	{
		m_pending_give_results.erase(iter);
	}

	std::vector<SteamInventoryResult_t>::iterator iter2 = std::find(m_pending_consume_results.begin(), m_pending_consume_results.end(), id);
	if (iter2 != m_pending_consume_results.end())
	{
		m_pending_consume_results.erase(iter2);
	}

	std::vector<SteamInventoryResult_t>::iterator iter3 = std::find(m_pending_results.begin(), m_pending_results.end(), id);
	if (iter3 != m_pending_results.end())
	{
		m_pending_results.erase(iter3);
	}

	// We're not hanging on the the result after processing it.
	SteamInventory()->DestroyResult(id);
}

int Steamworks_OnlineInventory::GetUpdateCounter()
{
	return m_update_counter;
}