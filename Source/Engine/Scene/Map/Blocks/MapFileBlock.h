// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_SCENE_MAP_MAPFILE_DECODED_BLOCK_
#define _ENGINE_SCENE_MAP_MAPFILE_DECODED_BLOCK_

#include <vector>

struct MapVersion
{
	enum Type
	{
		// Has to be first.
		Unversioned,

		Base_Version,
		Cutscenes_Added,

		// Has to be last.
		Current_Version,
	};
};

class Stream;
class MapFileBlock;

#define COMBINE_MAP_FILE_BLOCK_ID(a,b,c,d)	((d << 24) | (c << 16) | (b << 8) | (a))

#define DEFINE_MAP_BLOCK_FACTORY(classname) \
	MapFileBlockFactory<classname> _block_factory_##classname;

class MapFileBlockFactoryBase
{
private:
	static std::vector<MapFileBlockFactoryBase*> m_factories;

public:
	MapFileBlockFactoryBase();
	~MapFileBlockFactoryBase();

	virtual u32 Get_ID() = 0;
	virtual MapFileBlock* Create_Block() = 0;

	static MapFileBlock* New_Block(u32 id)
	{
		for (std::vector<MapFileBlockFactoryBase*>::iterator iter = m_factories.begin(); iter != m_factories.end(); iter++)
		{
			MapFileBlockFactoryBase* base = *iter;
			if (base->Get_ID() == id)
			{
				return base->Create_Block();
			}
		}
		return NULL;
	}
};

template <typename T>
class MapFileBlockFactory : public MapFileBlockFactoryBase
{
private:
	T m_archetype;

public:
	u32 Get_ID()
	{
		return m_archetype.Get_ID();
	}

	MapFileBlock* Create_Block()
	{
		return new T();
	}

};

class MapFileBlock 
{
public:
	virtual bool Decode(MapVersion::Type version, Stream* stream) = 0;
	virtual bool Encode(MapVersion::Type version, Stream* stream) = 0;
	virtual u32  Get_ID() = 0;

};

#endif

