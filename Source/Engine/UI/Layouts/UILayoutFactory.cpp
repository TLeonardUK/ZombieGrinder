// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/UI/Layouts/UILayoutFactory.h"
#include "Engine/UI/Layouts/UILayout.h"
#include "Engine/UI/Layouts/UILayoutHandle.h"

#include "Generic/Helper/StringHelper.h"
ThreadSafeHashTable<UILayoutHandle*, int>	UILayoutFactory::m_loaded_layouts;

UILayoutFactory::UILayoutFactory()
{
}

UILayoutFactory::~UILayoutFactory()
{
}

UILayoutHandle* UILayoutFactory::Load(const char* url)
{
	int url_hash = StringHelper::Hash(url);

	// Texture already loaded?
	if (m_loaded_layouts.Contains(url_hash))
	{
		DBG_LOG("Loaded layout from cache: %s", url);
		return m_loaded_layouts.Get(url_hash);
	}

	// Try and load texture!
	UILayout* layout = Load_Without_Handle(url);
	if (layout != NULL)
	{		
		UILayoutHandle* handle = new UILayoutHandle(url, layout);

		m_loaded_layouts.Set(url_hash, handle);
		
		DBG_LOG("Loaded UI layout: %s", url);

		return handle;
	}
	else
	{
		DBG_LOG("Failed to load UI layout: %s", url);
	}

	return NULL;
}

UILayout* UILayoutFactory::Load_Without_Handle(const char* url)
{
	return UILayout::Load(url);
}

void UILayoutFactory::Dispose()
{
	for (ThreadSafeHashTable<UILayoutHandle*, int>::Iterator iter = m_loaded_layouts.Begin(); iter != m_loaded_layouts.End(); iter++)
	{
		UILayoutHandle* handle = *iter;
		SAFE_DELETE(handle);
	}
	m_loaded_layouts.Clear();
}
