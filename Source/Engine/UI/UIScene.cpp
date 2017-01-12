// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/UI/UIScene.h"
#include "Engine/UI/UIManager.h"
#include "Engine/UI/UIElement.h"
#include "Engine/UI/Layouts/UILayout.h"
#include "Engine/UI/Layouts/UILayoutHandle.h"
#include "Engine/UI/Layouts/UILayoutFactory.h"
#include "Engine/Engine/GameEngine.h"
#include "Engine/Renderer/Atlases/AtlasHandle.h"
#include "Engine/Resources/ResourceFactory.h"

UIScene::UIScene()
	: m_layout(NULL)
	, m_layout_last_version(-1)
	, m_layout_last(NULL)
	, m_scale(1, 1)
	, m_render_lower_scenes_override_setting(false)
	, m_render_lower_scenes_override(false)
{
}

UIScene::~UIScene()
{
	Clear_Elements();
}

void UIScene::Clear_Elements()
{
	for (std::vector<UIElement*>::iterator iter = m_elements.begin(); iter != m_elements.end(); iter++)
	{
		delete *iter;
	}
	m_elements.clear();
}

void UIScene::Enter(UIManager* manager)
{
}

void UIScene::Exit(UIManager* manager)
{
}

bool UIScene::Should_Render_Lower_Scenes()
{
	return false;
}

bool UIScene::Should_Render_Lower_Scenes_Background()
{
	return true;
}

bool UIScene::Can_Accept_Invite()
{
	return true;
}

bool UIScene::Should_Fade_Cursor()
{
	return true;
}

void UIScene::Set_Render_Lower_Scenes_Override(bool should_override, bool override_setting)
{
	m_render_lower_scenes_override = should_override;
	m_render_lower_scenes_override_setting = override_setting;
}

bool UIScene::Get_Should_Render_Lower_Scenes()
{
	if (m_render_lower_scenes_override == true &&
		m_render_lower_scenes_override_setting == false)
	{
		return false;
	}
	else
	{
		return Should_Render_Lower_Scenes();
	}
}

bool UIScene::Get_Should_Render_Lower_Scenes_Background()
{
	if (m_render_lower_scenes_override == true &&
		m_render_lower_scenes_override_setting == false)
	{
		return false;
	}
	else
	{
		return Should_Render_Lower_Scenes_Background();
	}
}

bool UIScene::Should_Tick_When_Not_Top()
{
	return false;
}

bool UIScene::Should_Display_Cursor()
{
	return false;
}

bool UIScene::Should_Display_Focus_Cursor()
{
	return true;
}

bool UIScene::Is_Focusable()
{
	return false;
}

UIScene* UIScene::Get_Background(UIManager* manager)
{
	return NULL;
}

void UIScene::Set_Scale(Vector2 scale)
{
	m_scale = scale;
}

Vector2 UIScene::Get_Scale()
{
	return m_scale;
}

void UIScene::Reset_Element_Offsets()
{
	for (std::vector<UIElement*>::iterator iter = m_elements.begin(); iter != m_elements.end(); iter++)
	{
		UIElement* element = *iter;
		element->Set_Offset(Vector2(0, 0));
	}
	Refresh_Elements();
}

UIElement* UIScene::Find_Element_By_Position(UIManager* manager, Vector2 position, int scene_index, bool only_mouse_hot, bool focusable_only)
{
	// Check all elements.
	for (std::vector<UIElement*>::iterator iter = m_elements.begin(); iter != m_elements.end(); iter++)
	{
		UIElement* element = *iter;
		UIElement* hit = element->Find_Element_By_Position(manager, position, only_mouse_hot, focusable_only);
		if (hit != NULL)
		{
			return hit;
		}
	}

	// Check lower scenes?
	/*if (Get_Should_Render_Lower_Scenes() && scene_index > 0)
	{
		UIScene* scene = manager->Get_Scene(scene_index - 1);
		return scene->Find_Element_By_Position(manager, position, scene_index - 1, only_mouse_hot, focusable_only);
	}*/

	return NULL;
}

void UIScene::Set_Element_Value(const char* name, std::string& value)
{
	Set_Element_Value(name, value.c_str());
}

void UIScene::Set_Element_Value(const char* name, const char* value)
{
	int hash = StringHelper::Hash(name);
	for (std::vector<UIElement*>::iterator iter = m_elements.begin(); iter != m_elements.end(); iter++)
	{
		UIElement* element = *iter;
		if (element->Get_Name_Hash() == hash)
		{
			element->Set_Value(value);
			return;
		}
	}
}

void UIScene::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	Pre_Draw(time, manager, scene_index);
	Post_Draw(time, manager, scene_index);
}

void UIScene::Pre_Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	// Render lower scenes?
	if (Get_Should_Render_Lower_Scenes() && scene_index > 0)
	{
		UIScene* scene = manager->Get_Scene(scene_index - 1);
		scene->Draw(time, manager, scene_index - 1);
	}

	// Render all elements.
	for (std::vector<UIElement*>::iterator iter = m_elements.begin(); iter != m_elements.end(); iter++)
	{
		UIElement* element = *iter;
		if (element->Get_Visible())
			element->Draw(time, manager, this);
	}
}

void UIScene::Post_Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	for (std::vector<UIElement*>::iterator iter = m_elements.begin(); iter != m_elements.end(); iter++)
	{
		UIElement* element = *iter;
		if (element->Get_Visible())
			element->Post_Draw(time, manager, this);
	}
}


void UIScene::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	// Has the scene reloaded?
	if (m_layout)
	{
		if (m_layout->Get_Version() != m_layout_last_version)
		{
			Refresh(manager);
		}
	}

	// Render lower scenes?
	/*
	if (Get_Should_Render_Lower_Scenes() && scene_index > 0)
	{
		int index = scene_index;
		while (index > 0)
		{
			UIScene* scene = manager->Get_Scene(scene_index - 1);
			if (scene->Should_Tick_When_Not_Top())
			{
				scene->Tick(time, manager, scene_index - 1);
				break;
			}
			index--;
		}
	}
	*/

	if (scene_index > 0)
	{
		int index = scene_index;
		while (index > 0)
		{
			UIScene* scene = manager->Get_Scene(index - 1);
			if (scene->Should_Tick_When_Not_Top())
			{
				scene->Tick(time, manager, index - 1);
				break;
			}
			index--;
		}
	}

	// Tick all elements.
	for (unsigned int i = 0; i < m_elements.size(); i++)
	{
		m_elements[i]->Tick(time, manager, this);
	}
}

void UIScene::Refresh(UIManager* manager)
{
	Instantiate_Layout();
}

void UIScene::Refresh_Elements()
{
	for (std::vector<UIElement*>::iterator iter = m_elements.begin(); iter != m_elements.end(); iter++)
	{
		UIElement* element = *iter;
		element->Refresh();
	}
	for (std::vector<UIElement*>::iterator iter = m_elements.begin(); iter != m_elements.end(); iter++)
	{
		UIElement* element = *iter;
		element->After_Refresh();
	}
}

void UIScene::Instantiate_Layout()
{
	if (m_layout_last_version != m_layout->Get_Version() ||
		m_layout != m_layout_last)
	{
		Clear_Elements();

		// Instantiate elements and add to scene.
		std::vector<UIElement*> elements = m_layout->Get()->Instantiate(this);
		for (std::vector<UIElement*>::iterator iter = elements.begin(); iter != elements.end(); iter++)
		{
			m_elements.push_back(*iter);
		}
	}
	
	Refresh_Elements();

	m_layout_last_version = m_layout->Get_Version();
	m_layout_last = m_layout;
}

void UIScene::Set_Layout(UILayoutHandle* layout)
{
	DBG_ASSERT_STR(layout != NULL, "Attempt to set screen layout of '%s' to NULL.", Get_Name());

	if (m_layout != layout)
	{
		m_layout = layout;
		Refresh(GameEngine::Get()->Get_UIManager());
	}
	else
	{
		m_layout = layout;
	}
}

void UIScene::Set_Layout(const char* path)
{
	Set_Layout(ResourceFactory::Get()->Get_Layout(path));
}

std::vector<UIElement*>& UIScene::Get_Elements()
{
	return m_elements;
}

void UIScene::Dispatch_Event(UIManager* manager, UIEvent e)
{
	manager->Queue_Event(e, this);
}

void UIScene::Internal_Dispatch_Event(UIManager* manager, UIEvent e)
{
	if (e.Source != NULL)
	{
		DBG_LOG("UI Event '%s' was fired by element '%s'.", e.Name, e.Source->Get_Name().c_str());
	}
	else
	{
		DBG_LOG("UI Event '%s' was fired by scene '%s'.", e.Name, e.Scene->Get_Name());	
	}
	Recieve_Event(manager, e);
}

void UIScene::Recieve_Event(UIManager* manager, UIEvent e)
{
}
