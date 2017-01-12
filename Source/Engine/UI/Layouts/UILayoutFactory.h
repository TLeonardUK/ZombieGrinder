// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_UI_LAYOUT_UILAYOUTFACTORY_
#define _ENGINE_UI_LAYOUT_UILAYOUTFACTORY_

#include "Engine/Engine/FrameTime.h"
#include "Generic/Types/ThreadSafeHashTable.h"

class UIManager;
class UIScene;
class UILayout;
class UILayoutHandle;

class UILayoutFactory
{
	MEMORY_ALLOCATOR(UILayoutFactory, "UI");

protected:
	static ThreadSafeHashTable<UILayoutHandle*, int>	m_loaded_layouts;

	UILayoutFactory();
	~UILayoutFactory();

public:

	// Disposal.
	static void Dispose();
		
	// Static methods.
	static UILayoutHandle* Load				  (const char* url);
	static UILayout*	   Load_Without_Handle(const char* url);

};

#endif

