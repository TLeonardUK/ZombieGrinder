// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Scene/Actors/Components/Component.h"

#include "Engine/IO/BinaryStream.h"

std::vector<ComponentFactoryBase*> Component::m_factories;

Component::Component()
	: m_parent(NULL)
	, m_active(true)
{
}

Component::~Component()
{
}

void Component::Set_Parent(CompositeActor* actor)
{	
	m_parent = actor;
}

CompositeActor* Component::Get_Parent()
{
	return m_parent;
}

void Component::Deactivate()
{
	m_active = false;
}

void Component::Serialize_Demo(BinaryStream* stream, DemoVersion::Type version, bool bSaving, float frameDelta)
{
	// TODO!
}