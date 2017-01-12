// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Game/TutorialManager.h"
#include "Game/Online/StatisticsManager.h"
#include "Game/Profile/ProfileManager.h"

#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMBinary.h"

#include "Engine/Engine/GameEngine.h"
#include "Engine/Scripts/EngineVirtualMachine.h"

#include "Engine/Engine/EngineOptions.h"

#include "Engine/Online/OnlinePlatform.h"
#include "Engine/Online/OnlineUser.h"

#include "Game/Runner/Game.h"

TutorialManager::TutorialManager()
	: m_init(false)
	, m_first_load(false)
{
}

TutorialManager::~TutorialManager()
{
}

bool TutorialManager::Init()
{	
	DBG_ASSERT(!m_init);
	DBG_LOG("Loading tutorials ...");

	EngineVirtualMachine* vm = GameEngine::Get()->Get_VM();
	CVMLinkedSymbol* base_symbol = vm->Find_Class("BaseTutorial");
	DBG_ASSERT(base_symbol != NULL);

	m_is_complete_field = vm->Find_Variable(base_symbol, "IsComplete");
	DBG_ASSERT(m_is_complete_field != NULL);

	std::vector<CVMLinkedSymbol*> extended_classes = vm->Find_Derived_Classes(base_symbol);

	m_tutorials.resize(extended_classes.size());

	CVMContextLock lock = vm->Set_Context(vm->Get_Static_Context());

	int index = 0;
	for (std::vector<CVMLinkedSymbol*>::iterator iter = extended_classes.begin(); iter != extended_classes.end(); iter++, index++)
	{
		CVMLinkedSymbol* achievement_class = *iter;
		DBG_LOG("Loading Tutorial Class: %s", achievement_class->symbol->name);

		Tutorial& achievement = m_tutorials.at(index);
		achievement.id = achievement_class->name_hash;
		achievement.completed = false;

		achievement.script_object = vm->New_Object(achievement_class, true, &achievement);
		vm->Set_Default_State(achievement.script_object);

		achievement.event_listener = new ScriptEventListener(vm->Get_Static_Context(), achievement.script_object);
	}

	m_init = true;
	return true;
}

void TutorialManager::Tick(const FrameTime& time)
{
	if (!m_init)
		return;

	EngineVirtualMachine* vm = GameEngine::Get()->Get_VM();
	CVMContextLock lock = vm->Set_Context(vm->Get_Static_Context());

	for (std::vector<Tutorial>::iterator iter = m_tutorials.begin(); iter != m_tutorials.end(); iter++)
	{
		Tutorial& tutorial = *iter;

		if (!tutorial.completed)
		{
			// Grab completed state.
			CVMValue output;
			vm->Get_Field(m_is_complete_field, tutorial.script_object, output);
			tutorial.completed = (output.int_value != 0);

			if (tutorial.completed)
			{
				DBG_LOG("Tutorial '%s' is now complete and will not be shown again.", tutorial.script_object.Get().Get()->Get_Symbol()->symbol->name);
 				SAFE_DELETE(tutorial.event_listener);
			}
		}
	}
}

bool TutorialManager::Serialize(BinaryStream* stream)
{
	DBG_ASSERT(m_init);

	EngineVirtualMachine* vm = GameEngine::Get()->Get_VM();
	CVMContextLock lock = vm->Set_Context(vm->Get_Static_Context());

	stream->Write<u32>(m_tutorials.size());

	for (std::vector<Tutorial>::iterator iter = m_tutorials.begin(); iter != m_tutorials.end(); iter++)
	{
		Tutorial& achievement = *iter;
		int version = 0;

		BinaryStream achievement_stream;
		achievement.script_object.Get().Get()->Serialize(&achievement_stream, CVMObjectSerializeFlags::Full, &version);

		stream->Write<int>(achievement.id);
		stream->Write<int>(version);
		stream->Write<int>(achievement_stream.Length());
		stream->WriteBuffer(achievement_stream.Data(), 0, achievement_stream.Length());
	}

	return true;
}

bool TutorialManager::Deserialize(BinaryStream* stream)
{
	EngineVirtualMachine* vm = GameEngine::Get()->Get_VM();
	CVMContextLock lock = vm->Set_Context(vm->Get_Static_Context());

	if (!m_first_load)
	{
		u32 count = stream->Read<u32>();
		for (u32 i = 0; i < count; i++)
		{
			int id = stream->Read<int>();
			int version = stream->Read<int>();
			int length = stream->Read<int>();

			bool found = false;
			
			if (!(*EngineOptions::reset_tutorials))
			{
				for (std::vector<Tutorial>::iterator iter = m_tutorials.begin(); iter != m_tutorials.end(); iter++)
				{
					Tutorial& achievement = *iter;
					if (achievement.id == id)
					{
						achievement.script_object.Get().Get()->Deserialize(stream, CVMObjectSerializeFlags::Full, version);

						// Grab completed state.
						CVMValue output;
						vm->Get_Field(m_is_complete_field, achievement.script_object, output);
						achievement.completed = (output.int_value != 0);

						DBG_LOG("Tutorial State: ID=%s Completed=%i", achievement.script_object.Get().Get()->Get_Symbol()->symbol->name, achievement.completed);
						if (achievement.completed)
						{
							DBG_LOG("Tutorial '%s' is now complete and will not be shown again.", achievement.script_object.Get().Get()->Get_Symbol()->symbol->name);
							SAFE_DELETE(achievement.event_listener);
						}

						found = true;
						break;
					}
				}
			}

			if (found == false)
			{
				DBG_LOG("Could not deserialize tutorial '%i'. Could not be found. Has it been removed?", id);
				stream->Seek(stream->Position() + length); // Skip over unreadable data.
			}
		}
	}

	m_first_load = true;

	return true;
}