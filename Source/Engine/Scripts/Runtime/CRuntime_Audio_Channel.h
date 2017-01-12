// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_RUNTIME_AUDIO_CHANNEL_
#define _GAME_RUNTIME_AUDIO_CHANNEL_

#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMValue.h"

class CRuntime_Audio_Channel
{
public:
	static CVMObjectHandle Get_Position(CVirtualMachine* vm, CVMValue self);
	static void Set_Position(CVirtualMachine* vm, CVMValue self, CVMObjectHandle val);
	static float Get_Progress(CVirtualMachine* vm, CVMValue self);
	static void Set_Progress(CVirtualMachine* vm, CVMValue self, float val);
	static float Get_Pan(CVirtualMachine* vm, CVMValue self);
	static void Set_Pan(CVirtualMachine* vm, CVMValue self, float val);
	static float Get_Rate(CVirtualMachine* vm, CVMValue self);
	static void Set_Rate(CVirtualMachine* vm, CVMValue self, float val);
	static float Get_Volume(CVirtualMachine* vm, CVMValue self);
	static void Set_Volume(CVirtualMachine* vm, CVMValue self, float val);
	static int Is_Paused(CVirtualMachine* vm, CVMValue self);
	static int Is_Playing(CVirtualMachine* vm, CVMValue self);
	static int Is_Muted(CVirtualMachine* vm, CVMValue self);
	static void Stop(CVirtualMachine* vm, CVMValue self);
	static void Pause(CVirtualMachine* vm, CVMValue self);
	static void Resume(CVirtualMachine* vm, CVMValue self);
	static void Mute(CVirtualMachine* vm, CVMValue self);
	static void Unmute(CVirtualMachine* vm, CVMValue self);

	static void Bind(CVirtualMachine* machine);
};

#endif
