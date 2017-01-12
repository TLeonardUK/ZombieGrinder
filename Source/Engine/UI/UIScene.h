// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_UI_UISCENE_
#define _ENGINE_UI_UISCENE_

#include "Engine/Engine/FrameTime.h"

#include "Generic/Types/LinkedList.h"
#include "Generic/Types/Vector2.h"

#include "Engine/UI/UIEvent.h"
#include "Engine/UI/UIElement.h"

class UIManager;
class UIElement;
class UILayout;
class UILayoutHandle;

class UIScene
{
	MEMORY_ALLOCATOR(UIScene, "UI");

private:
	std::vector<UIElement*>	m_elements;
	UILayoutHandle*			m_layout;
	UILayoutHandle*			m_layout_last;
	int						m_layout_last_version;
	int						m_layout_last_atlas_version;
	Vector2					m_scale;

	bool					m_render_lower_scenes_override;
	bool					m_render_lower_scenes_override_setting;

	// You really should not touch these 2 unless you really know what your up to.
protected:
	void Clear_Elements();
	void Instantiate_Layout();
	
protected:
	void Set_Layout(UILayoutHandle* layout);
	void Set_Layout(const char* path);
	
	void Set_Element_Value(const char* name, std::string& value);
	void Set_Element_Value(const char* name, const char* value);

public:
	UIScene();
	virtual ~UIScene();
	
	template<typename T>
	T Find_Element(const char* name)
	{
		int hash = StringHelper::Hash(name);
		for (std::vector<UIElement*>::iterator iter = m_elements.begin(); iter != m_elements.end(); iter++)
		{
			UIElement* element = (*iter)->Find_Element<T>(hash);
			if (element != NULL)
			{
				return static_cast<T>(element);
			}
		}	
		return NULL;
	}

	void Set_Scale(Vector2 scale);
	Vector2 Get_Scale();

	bool Should_Fade_Cursor();
	
	std::vector<UIElement*>& Get_Elements();

	void Reset_Element_Offsets();

	void Set_Render_Lower_Scenes_Override(bool should_override, bool override_setting);

	bool Get_Should_Render_Lower_Scenes();
	bool Get_Should_Render_Lower_Scenes_Background();

	virtual bool Can_Accept_Invite();

	virtual const char* Get_Name() = 0;
	virtual bool Should_Render_Lower_Scenes();
	virtual bool Should_Tick_When_Not_Top();
	virtual bool Should_Display_Cursor();
	virtual bool Should_Display_Focus_Cursor();
	virtual bool Should_Render_Lower_Scenes_Background();
	virtual bool Is_Focusable();
	virtual UIScene* Get_Background(UIManager* manager);

	virtual void Enter(UIManager* manager);
	virtual void Exit(UIManager* manager);

	virtual void Refresh(UIManager* manager);
	virtual void Refresh_Elements();

	virtual void Tick(const FrameTime& time, UIManager* manager, int scene_index);
	virtual void Draw(const FrameTime& time, UIManager* manager, int scene_index);
	virtual void Pre_Draw(const FrameTime& time, UIManager* manager, int scene_index);
	virtual void Post_Draw(const FrameTime& time, UIManager* manager, int scene_index);
	
	void Dispatch_Event(UIManager* manager, UIEvent e);
	void Internal_Dispatch_Event(UIManager* manager, UIEvent e);
	virtual void Recieve_Event(UIManager* manager, UIEvent e);
	
	UIElement* Find_Element_By_Position(UIManager* manager, Vector2 position, int scene_index, bool only_mouse_hot, bool focusable_only);

};

#endif

