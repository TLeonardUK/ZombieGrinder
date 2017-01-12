// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_RUNTIME_AUDIO_
#define _GAME_RUNTIME_AUDIO_

#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMValue.h"

class CRuntime_Audio
{
public:
	static CVMObjectHandle Play_Spatial(CVirtualMachine* vm, CVMString resource, CVMObjectHandle position, int start_paused);
	static CVMObjectHandle Play_Global(CVirtualMachine* vm, CVMString resource, int start_paused);
	static void Play_BGM(CVirtualMachine* vm, CVMString resource);
	static void Push_BGM(CVirtualMachine* vm, CVMString resource);
	static void Pop_BGM(CVirtualMachine* vm);

	static void Set_Listener_Position(CVirtualMachine* vm, int listener_index, CVMObjectHandle position);

	static float Get_BGM_Volume_Multiplier(CVirtualMachine* vm);
	static void Set_BGM_Volume_Multiplier(CVirtualMachine* vm, float vol);

	static void Bind(CVirtualMachine* machine);
};

#endif
