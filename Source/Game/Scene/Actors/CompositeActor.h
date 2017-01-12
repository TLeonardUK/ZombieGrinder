// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_SCENE_ACTORS_STATIC_ACTOR_
#define _ENGINE_SCENE_ACTORS_STATIC_ACTOR_

#include "Engine/Scene/Actor.h"
#include "Engine/Scene/Tickable.h"
#include "Engine/Renderer/Drawable.h"

#include "Game/Scene/Actors/Components/Component.h"

// A composite actor is an actor made up of multiple different
// component parts. It does not directly have any drawing/ticking
// functionality, instead that functionality comes from its components.

// It mainly serves as a place to store information that needs to be shared
// between different components (position/rotation/etc).

#define MAX_COMPOSITE_ACTOR_COMPONENTS 16

class Component;

class CompositeActor : public Actor
{
	MEMORY_ALLOCATOR(CompositeActor, "Scene");

protected:
	Component* m_components[MAX_COMPOSITE_ACTOR_COMPONENTS];
	bool m_selected;

public:
	CompositeActor();
	virtual ~CompositeActor();

	void Set_Selected(bool value)
	{
		m_selected = value;
	}

	bool Get_Selected()
	{
		return m_selected;
	}

	virtual void Serialize_Demo(BinaryStream* stream, DemoVersion::Type version, bool bSaving, float frameDelta);

	void Add_Component		(Component* component);
	void Remove_Component	(Component* component);

	void Deactivate_Components();

	template<typename T> 
	T Get_Component()
	{
		for (int i = 0; i < MAX_COMPOSITE_ACTOR_COMPONENTS; i++)
		{
			T cast = dynamic_cast<T>(m_components[i]);
			if (cast != NULL)
			{
				return cast;
			}
		}
		return NULL;
	}

	int Get_Component_Count()
	{
		int count = 0;
		for (int i = 0; i < MAX_COMPOSITE_ACTOR_COMPONENTS; i++)
		{
			if (m_components[i] != NULL)
			{
				count++;
			}
		}
		return count;
	}

	Component* Get_Component_Index(int find_index)
	{
		int index = 0;
		for (int i = 0; i < MAX_COMPOSITE_ACTOR_COMPONENTS; i++)
		{
			if (m_components[i] != NULL)
			{
				if (index == find_index)
				{
					return m_components[i];
				}
				index++;
			}
		}
		return NULL;
	}

};

#endif

