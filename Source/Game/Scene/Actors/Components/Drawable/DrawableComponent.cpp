// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Scene/Actors/Components/Drawable/DrawableComponent.h"
#include "Game/Scene/Actors/ScriptedActor.h"
#include "Game/Scene/GameScene.h"
#include "Game/Runner/Game.h"
#include "Engine/Renderer/PrimitiveRenderer.h"

#include "Engine/Engine/GameEngine.h"
#include "Engine/Scene/Scene.h"

DrawableComponent::DrawableComponent()
{
}

DrawableComponent::~DrawableComponent()
{
	Scene* scene = GameEngine::Get()->Get_Scene();
	if (scene)
	{
		scene->Remove_Drawable(this);
	}
}

float DrawableComponent::Get_Draw_Depth()
{
	return m_parent->Get_Draw_Depth();
}

bool DrawableComponent::Is_Visible_From(Rect2D viewport)
{
	Rect2D bb = m_parent->Get_World_Bounding_Box();
	
	// Inflate rectangle by a small amount (100% on each side) to account for possible rotation or incorrectly sized BB's.
	bb = bb.Inflate(bb.Width * 3, bb.Height * 3);
	
	return viewport.Intersects(bb);
}

void DrawableComponent::Pre_Draw()
{

}

void DrawableComponent::Draw_Debug(const FrameTime& time, RenderPipeline* pipeline)
{
	if (!m_active)
	{
		return;
	}

	ScriptedActor* scripted_parent = dynamic_cast<ScriptedActor*>(m_parent);
	if (scripted_parent != NULL)
	{
		if (scripted_parent->Get_Layer() != RenderPipeline::Get()->Get_Debug_Layer())
		{
			return;
		}
	}

	PrimitiveRenderer pr;

	if (m_parent->Get_Selected())
	{
		pr.Draw_Wireframe_Quad(m_parent->Get_World_Bounding_Box(), 0.0f, Color(51, 255, 153, 255), 1.0f);
	}
	else
	{
		pr.Draw_Wireframe_Quad(m_parent->Get_World_Bounding_Box(), 0.0f, Color(51, 153, 255, 255), 1.0f);
	}

	if (scripted_parent != NULL)
	{
		std::string link = scripted_parent->Get_Link();
		if (link != "")
		{
			std::vector<std::string> link_values;
			StringHelper::Split(link.c_str(), ',', link_values);

			Vector2 bb = m_parent->Get_World_Bounding_Box().Center();
			Vector3 center = Vector3(bb.X, bb.Y, 0.0f);

			std::vector<Actor*> actors = Game::Get()->Get_Game_Scene()->Get_Actors();

			for (std::vector<Actor*>::iterator iter = actors.begin(); iter != actors.end(); iter++)
			{
				ScriptedActor* actor = dynamic_cast<ScriptedActor*>(*iter);
				if (actor != NULL)
				{
					std::vector<std::string> tag_values;
					StringHelper::Split(actor->Get_Tag().c_str(), ',', tag_values);

					bool bFound = false;

					for (std::vector<std::string>::iterator tag_iter = tag_values.begin(); tag_iter != tag_values.end(); tag_iter++)
					{
						if (std::find(link_values.begin(), link_values.end(), *tag_iter) != link_values.end())
						{
							bFound = true;
							break;
						}
					}

					if (bFound)
					{
						Vector2 b_bb = actor->Get_World_Bounding_Box().Center();
						Vector3 b_center = Vector3(b_bb.X, b_bb.Y, 0.0f);

						pr.Draw_Line(center, b_center, 2.0f, Color::Blue);
					}
				}		
			}
		}
	}
}