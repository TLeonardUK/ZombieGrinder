// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/UI/Layouts/UILayoutFactory.h"
#include "Engine/UI/Layouts/UILayoutHandle.h"
#include "Engine/UI/Layouts/UILayout.h"

UILayoutHandle::UILayoutHandle(const char* url, UILayout* layout)
	: m_url(url)
	, m_layout(layout)
{
	Add_Reload_Trigger_File(url);
}

UILayoutHandle::~UILayoutHandle()
{
	SAFE_DELETE(m_layout);
}
	
void UILayoutHandle::Reload()
{
	UILayout* new_layout = UILayoutFactory::Load_Without_Handle(m_url.c_str());
	if (new_layout != NULL)
	{
		SAFE_DELETE(m_layout);			
		m_layout = new_layout;

		DBG_LOG("Reloaded changed layout: %s", m_url.c_str());
	}
}

UILayout* UILayoutHandle::Get()
{
	return m_layout;
}

std::string UILayoutHandle::Get_URL()
{
	return m_url;
}