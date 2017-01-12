// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/UI/Elements/UISplitContainerItem.h"
#include "Engine/Renderer/Atlases/Atlas.h"
#include "Engine/Renderer/Atlases/AtlasHandle.h"
#include "Engine/Renderer/Atlases/AtlasRenderer.h"
#include "Engine/UI/UIFrame.h"
#include "Engine/UI/UIManager.h"
#include "Engine/Engine/GameEngine.h"
#include "Engine/Input/Input.h"
#include "Engine/Platform/Platform.h"
#include "Engine/Localise/Locale.h"

UISplitContainerItem::UISplitContainerItem()
	: m_size(0)
	, m_fixed(false)
	, m_collapsed(false)
{
}

UISplitContainerItem::~UISplitContainerItem()
{
}

float UISplitContainerItem::Get_Size()
{
	return m_size;
}

void UISplitContainerItem::Set_Size(float size)
{
	m_size = size;
}

bool UISplitContainerItem::Get_Collapsed()
{
	return m_collapsed;
}

void UISplitContainerItem::Set_Collapsed(bool value)
{
	m_collapsed = value;
}

void UISplitContainerItem::Refresh()
{
	// Get children to calculate their screen box.
	UIElement::Refresh();
}

void UISplitContainerItem::After_Refresh()
{
	// After-refresh children.
	UIElement::After_Refresh();
}

void UISplitContainerItem::Tick(const FrameTime& time, UIManager* manager, UIScene* scene)
{
	if (m_collapsed == true)
	{
		return;
	}

	// Update children.
	UIElement::Tick(time, manager, scene);
}

void UISplitContainerItem::Draw(const FrameTime& time, UIManager* manager, UIScene* scene)
{
	if (m_collapsed == true)
	{
		return;
	}

	// Draw children.
	UIElement::Draw(time, manager, scene);
}
