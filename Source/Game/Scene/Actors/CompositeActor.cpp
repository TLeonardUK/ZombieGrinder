// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Scene/Actors/CompositeActor.h"

#include "Game/Scene/Actors/Components/Drawable/DrawableComponent.h"
#include "Game/Scene/Actors/Components/Tickable/TickableComponent.h"

#include "Game/Scene/GameScene.h"
#include "Game/Runner/Game.h"

#include "Engine/IO/BinaryStream.h"

CompositeActor::CompositeActor()
{
	memset(m_components, 0, sizeof(Component*) * MAX_COMPOSITE_ACTOR_COMPONENTS);
}

CompositeActor::~CompositeActor()
{
	for (int i = 0; i < MAX_COMPOSITE_ACTOR_COMPONENTS; i++)
	{
		if (m_components[i] != NULL)
		{
			SAFE_DELETE(m_components[i]);
			m_components[i] = NULL;
		}
	}
}

void CompositeActor::Serialize_Demo(BinaryStream* stream, DemoVersion::Type version, bool bSaving, float frameDelta)
{
	Actor::Serialize_Demo(stream, version, bSaving, frameDelta);

	GameScene* scene = Game::Get()->Get_Game_Scene();

	for (int i = 0; i < MAX_COMPOSITE_ACTOR_COMPONENTS; i++)
	{
		bool bExists = (m_components[i] != NULL && m_components[i]->Is_Used_In_Demo());
		stream->Serialize(bSaving, bExists);

		int componentId = 0;

		if (bExists)
		{
			if (bSaving)
			{
				componentId = m_components[i]->Get_ID();
				DBG_ASSERT(componentId != 0);
			}

			stream->Serialize(bSaving, componentId);
		}

		if (!bSaving)
		{
			if (bExists)
			{
				if (m_components[i] == NULL || m_components[i]->Get_ID() != componentId)
				{
					if (m_components[i] != NULL)
					{
						SAFE_DELETE(m_components[i]);
						m_components[i] = NULL;
					}

					m_components[i] = Component::Create_By_ID(componentId);
					if (m_components[i])
					{
						IDrawable* drawable = dynamic_cast<IDrawable*>(m_components[i]);
						if (drawable)
						{
							scene->Add_Drawable(drawable);
						}
						Tickable* tickable = dynamic_cast<Tickable*>(m_components[i]);
						if (tickable)
						{
							scene->Add_Tickable(tickable);
						}
					
						m_components[i]->Set_Parent(this);
					}
				}
			}
			else
			{
				if (m_components[i] != NULL)
				{
					SAFE_DELETE(m_components[i]);
					m_components[i] = NULL;
				}
			}
		}

		if (bExists && m_components[i])
		{
			m_components[i]->Serialize_Demo(stream, version, bSaving, frameDelta);
		}
	}
}

void CompositeActor::Deactivate_Components()
{
	for (int i = 0; i < MAX_COMPOSITE_ACTOR_COMPONENTS; i++)
	{
		if (m_components[i] != NULL)
		{
			m_components[i]->Deactivate();
		}
	}
}

void CompositeActor::Add_Component(Component* component)
{
	for (int i = 0; i < MAX_COMPOSITE_ACTOR_COMPONENTS; i++)
	{
		if (m_components[i] == NULL)
		{
			m_components[i] = component;
			component->Set_Parent(this);
			return;
		}
	}
	DBG_ASSERT_STR(false, "Ran out of component space.");
}

void CompositeActor::Remove_Component(Component* component)
{
	for (int i = 0; i < MAX_COMPOSITE_ACTOR_COMPONENTS; i++)
	{
		if (m_components[i] == component)
		{
			m_components[i] = NULL;
			component->Set_Parent(NULL);
		}
	}
}

