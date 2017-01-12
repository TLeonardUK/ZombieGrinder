// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_SCENE_ACTORS_COMPONENTS_COMPONENT_
#define _ENGINE_SCENE_ACTORS_COMPONENTS_COMPONENT_

#include "Game/Scene/Actors/CompositeActor.h"

class CompositeActor;
class Component;

class ComponentFactoryBase
{
public:
	virtual int ID() = 0;
	virtual Component* Create() = 0;
};

template <typename T>
class ComponentFactory : public ComponentFactoryBase
{
private:
	int m_id;

public:
	ComponentFactory()
	{
		T* tmp = new T();
		m_id = tmp->Get_ID();
		SAFE_DELETE(tmp);
	}

	virtual int ID()
	{
		return m_id;
	}

	virtual Component* Create()
	{
		return new T();
	}
};

class Component
{
	MEMORY_ALLOCATOR(Component, "Scene");

protected:
	static std::vector<ComponentFactoryBase*> m_factories;

	CompositeActor* m_parent;
	bool m_active;

private:
	friend class CompositeActor;

	void			Set_Parent	(CompositeActor* actor);

public:
	Component();
	virtual ~Component();

	virtual int Get_ID() = 0;
	virtual bool Is_Used_In_Demo() = 0;

	static Component* Create_By_ID(int id)
	{
		for (std::vector<ComponentFactoryBase*>::iterator iter = m_factories.begin(); iter != m_factories.end(); iter++)
		{
			if ((*iter)->ID() == id)
			{
				return (*iter)->Create();
			}
		}
		return NULL;
	}
	
	template <typename T>
	static void Register_Component_Type()
	{
		m_factories.push_back(new ComponentFactory<T>());
	}

	virtual void Serialize_Demo(BinaryStream* stream, DemoVersion::Type version, bool bSaving, float frameDelta);

	void Deactivate();

	virtual std::string	Get_Script_Class_Name()
	{
		return "";
	}

	CompositeActor* Get_Parent	();

};

#endif

