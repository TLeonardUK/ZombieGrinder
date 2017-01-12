// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_ONLINE_INVENTORY_
#define _ENGINE_ONLINE_INVENTORY_

#include "Generic/Patterns/Singleton.h"

#include "Engine/Engine/FrameTime.h"

#include <string>
#include <vector>

struct OnlineInventoryItem
{
	std::string unique_id;
	u64 unique_id64;
	std::string original_unique_id;
	u64 original_unique_id64;
	int type_id;
};

class OnlineInventory : public Singleton<OnlineInventory>
{
	MEMORY_ALLOCATOR(OnlineInventory, "Network");

private:

public:	
	virtual ~OnlineInventory() {} 

	static OnlineInventory* Create();

	virtual bool Initialize() = 0;

	virtual void Tick(const FrameTime& time) = 0;

	virtual void Generate() = 0;
	virtual void CheckForDrops(int item_id) = 0;

	virtual std::vector<OnlineInventoryItem> GetItems() = 0;
	virtual std::vector<OnlineInventoryItem> GetDroppedItems() = 0;

	virtual void GiveSelf(int id) = 0;

	virtual bool CanAccess() = 0;

	virtual bool HasInventory() = 0;

	virtual void RemoveItem(u64 id) = 0;

	virtual int GetUpdateCounter() = 0;

};

#endif

