// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_UI_ELEMENTS_UISPLITCONTAINERITEM_
#define _ENGINE_UI_ELEMENTS_UISPLITCONTAINERITEM_

#include "Engine/UI/UIElement.h"
#include "Engine/UI/UIFrame.h"
#include "Engine/Renderer/Text/Font.h"
#include "Engine/Renderer/Text/FontRenderer.h"

#include "Engine/Renderer/Atlases/AtlasRenderer.h"

class AtlasHandle;
class UIManager;
struct AtlasFrame;
class UISplitContainer;

class UISplitContainerItem : public UIElement 
{
	MEMORY_ALLOCATOR(UISplitContainerItem, "UI");

protected:	
	friend class UILayout;
	friend class UISplitContainer;

	float	m_size;
	bool	m_fixed;
	bool	m_collapsed;

public:
	UISplitContainerItem();
	~UISplitContainerItem();
	
	void Refresh();
	void After_Refresh();

	void Tick(const FrameTime& time, UIManager* manager, UIScene* scene);
	void Draw(const FrameTime& time, UIManager* manager, UIScene* scene);

	float Get_Size();
	void Set_Size(float size);

	bool Get_Collapsed();
	void Set_Collapsed(bool value);

};

#endif

