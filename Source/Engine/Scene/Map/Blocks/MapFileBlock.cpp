// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Scene/Map/Blocks/MapFileBlock.h"

#include <algorithm>

std::vector<MapFileBlockFactoryBase*> MapFileBlockFactoryBase::m_factories;

MapFileBlockFactoryBase::MapFileBlockFactoryBase()
{
	m_factories.push_back(this);
}

MapFileBlockFactoryBase::~MapFileBlockFactoryBase()
{
	m_factories.erase(std::find(m_factories.begin(), m_factories.end(), this));
}

