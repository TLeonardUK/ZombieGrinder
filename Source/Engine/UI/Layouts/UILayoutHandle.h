// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_UI_LAYOUT_UILAYOUTHANDLE_
#define _ENGINE_UI_LAYOUT_UILAYOUTHANDLE_

#include "Engine/Engine/FrameTime.h"

#include "Engine/Resources/Reloadable.h"

#include <string>

class UILayoutFactory;
class UILayout;

class UILayoutHandle : public Reloadable
{
	MEMORY_ALLOCATOR(UILayoutHandle, "UI");

private:

	UILayout*			m_layout;
	std::string			m_url;

protected:
	
	friend class UILayoutFactory;

	// Only texture factory should be able to modify these!
	UILayoutHandle(const char* url, UILayout* layout);
	~UILayoutHandle();
	
	void Reload();

public:

	// Get/Set
	UILayout* Get();
	std::string Get_URL();

};

#endif

