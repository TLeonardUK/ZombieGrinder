// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_TUTORIAL_MANAGER_
#define _GAME_TUTORIAL_MANAGER_

#include "Engine/IO/BinaryStream.h"
#include "Engine/Engine/FrameTime.h"

#include "XScript/VirtualMachine/CVMObject.h"

#include "Game/Scripts/ScriptEventListener.h"

#include "Generic/Patterns/Singleton.h"

struct AtlasFrame;

struct Tutorial
{
public:
	int						id;
	bool					completed;
	ScriptEventListener*	event_listener;
	CVMGCRoot				script_object;
};

class TutorialManager : public Singleton<TutorialManager>
{
	MEMORY_ALLOCATOR(TutorialManager, "Game");

private:
	bool m_init;
	bool m_first_load;

	std::vector<Tutorial> m_tutorials;

	CVMLinkedSymbol* m_is_complete_field;

public:
	TutorialManager();
	~TutorialManager();

	bool Init();

	bool Serialize(BinaryStream* stream);
	bool Deserialize(BinaryStream* stream);

	void Tick(const FrameTime& time);

};

#endif

