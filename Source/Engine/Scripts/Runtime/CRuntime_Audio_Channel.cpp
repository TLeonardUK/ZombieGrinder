// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Scripts/Runtime/CRuntime_Audio_Channel.h"
#include "XScript/VirtualMachine/CVMBindingHelper.h"
#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMContext.h"
#include "XScript/VirtualMachine/CVMBinary.h"

#include "Engine/Audio/AudioRenderer.h"

CVMObjectHandle CRuntime_Audio_Channel::Get_Position(CVirtualMachine* vm, CVMValue self)
{
	SoundChannel* channel = AudioRenderer::Get()->Get_Tracked_Audio_Handle(self.object_value.GetNullCheck(vm)->Get_Slot(0).int_value, self.object_value.GetNullCheck(vm)->Get_Slot(1).int_value).Get_Channel();
	if (channel != NULL)
	{
		return vm->Create_Vec3(channel->Get_Spatial_Position());
	}
	else
	{
		return vm->Create_Vec3(0.0f, 0.0f, 0.0f);
	}
}

void CRuntime_Audio_Channel::Set_Position(CVirtualMachine* vm, CVMValue self, CVMObjectHandle val)
{
	SoundChannel* channel = AudioRenderer::Get()->Get_Tracked_Audio_Handle(self.object_value.GetNullCheck(vm)->Get_Slot(0).int_value, self.object_value.GetNullCheck(vm)->Get_Slot(1).int_value).Get_Channel();
	if (channel != NULL)
	{
		channel->Set_Spatial_Position(val.Get()->To_Vec3());
	}
}

float CRuntime_Audio_Channel::Get_Progress(CVirtualMachine* vm, CVMValue self)
{
	SoundChannel* channel = AudioRenderer::Get()->Get_Tracked_Audio_Handle(self.object_value.GetNullCheck(vm)->Get_Slot(0).int_value, self.object_value.GetNullCheck(vm)->Get_Slot(1).int_value).Get_Channel();
	if (channel != NULL)
	{
		return channel->Get_Position();
	}
	else
	{
		return 0.0f;
	}
}

void CRuntime_Audio_Channel::Set_Progress(CVirtualMachine* vm, CVMValue self, float val)
{
	SoundChannel* channel = AudioRenderer::Get()->Get_Tracked_Audio_Handle(self.object_value.GetNullCheck(vm)->Get_Slot(0).int_value, self.object_value.GetNullCheck(vm)->Get_Slot(1).int_value).Get_Channel();
	if (channel != NULL)
	{
		channel->Set_Position(val);
	}
}

float CRuntime_Audio_Channel::Get_Pan(CVirtualMachine* vm, CVMValue self)
{
	SoundChannel* channel = AudioRenderer::Get()->Get_Tracked_Audio_Handle(self.object_value.GetNullCheck(vm)->Get_Slot(0).int_value, self.object_value.GetNullCheck(vm)->Get_Slot(1).int_value).Get_Channel();
	if (channel != NULL)
	{
		return channel->Get_Pan();
	}
	else
	{
		return 0.0f;
	}
}

void CRuntime_Audio_Channel::Set_Pan(CVirtualMachine* vm, CVMValue self, float val)
{
	SoundChannel* channel = AudioRenderer::Get()->Get_Tracked_Audio_Handle(self.object_value.GetNullCheck(vm)->Get_Slot(0).int_value, self.object_value.GetNullCheck(vm)->Get_Slot(1).int_value).Get_Channel();
	if (channel != NULL)
	{
		channel->Set_Pan(val);
	}
}

float CRuntime_Audio_Channel::Get_Rate(CVirtualMachine* vm, CVMValue self)
{
	SoundChannel* channel = AudioRenderer::Get()->Get_Tracked_Audio_Handle(self.object_value.GetNullCheck(vm)->Get_Slot(0).int_value, self.object_value.GetNullCheck(vm)->Get_Slot(1).int_value).Get_Channel();
	if (channel != NULL)
	{
		return channel->Get_Rate();
	}
	else
	{
		return 0.0f;
	}
}

void CRuntime_Audio_Channel::Set_Rate(CVirtualMachine* vm, CVMValue self, float val)
{
	SoundChannel* channel = AudioRenderer::Get()->Get_Tracked_Audio_Handle(self.object_value.GetNullCheck(vm)->Get_Slot(0).int_value, self.object_value.GetNullCheck(vm)->Get_Slot(1).int_value).Get_Channel();
	if (channel != NULL)
	{
		channel->Set_Rate(val);
	}
}

float CRuntime_Audio_Channel::Get_Volume(CVirtualMachine* vm, CVMValue self)
{
	SoundChannel* channel = AudioRenderer::Get()->Get_Tracked_Audio_Handle(self.object_value.GetNullCheck(vm)->Get_Slot(0).int_value, self.object_value.GetNullCheck(vm)->Get_Slot(1).int_value).Get_Channel();
	if (channel != NULL)
	{
		return channel->Get_Volume();
	}
	else
	{
		return 1.0f;
	}
}

void CRuntime_Audio_Channel::Set_Volume(CVirtualMachine* vm, CVMValue self, float val)
{
	SoundChannel* channel = AudioRenderer::Get()->Get_Tracked_Audio_Handle(self.object_value.GetNullCheck(vm)->Get_Slot(0).int_value, self.object_value.GetNullCheck(vm)->Get_Slot(1).int_value).Get_Channel();
	if (channel != NULL)
	{
		channel->Set_Volume(val);
	}
}

int CRuntime_Audio_Channel::Is_Paused(CVirtualMachine* vm, CVMValue self)
{
	SoundChannel* channel = AudioRenderer::Get()->Get_Tracked_Audio_Handle(self.object_value.GetNullCheck(vm)->Get_Slot(0).int_value, self.object_value.GetNullCheck(vm)->Get_Slot(1).int_value).Get_Channel();
	if (channel != NULL)
	{
		return channel->Is_Paused();
	}
	else
	{
		return false;
	}
}

int CRuntime_Audio_Channel::Is_Playing(CVirtualMachine* vm, CVMValue self)
{
	SoundChannel* channel = AudioRenderer::Get()->Get_Tracked_Audio_Handle(self.object_value.GetNullCheck(vm)->Get_Slot(0).int_value, self.object_value.GetNullCheck(vm)->Get_Slot(1).int_value).Get_Channel();
	if (channel != NULL)
	{
		return channel->Is_Playing();
	}
	else
	{
		return false;
	}
}

int CRuntime_Audio_Channel::Is_Muted(CVirtualMachine* vm, CVMValue self)
{
	SoundChannel* channel = AudioRenderer::Get()->Get_Tracked_Audio_Handle(self.object_value.GetNullCheck(vm)->Get_Slot(0).int_value, self.object_value.GetNullCheck(vm)->Get_Slot(1).int_value).Get_Channel();
	if (channel != NULL)
	{
		return channel->Is_Muted();
	}
	else
	{
		return false;
	}
}

void CRuntime_Audio_Channel::Stop(CVirtualMachine* vm, CVMValue self)
{
	TrackedAudioHandle handle = AudioRenderer::Get()->Get_Tracked_Audio_Handle(self.object_value.GetNullCheck(vm)->Get_Slot(0).int_value, self.object_value.GetNullCheck(vm)->Get_Slot(1).int_value);
	handle.Dispose();
}

void CRuntime_Audio_Channel::Pause(CVirtualMachine* vm, CVMValue self)
{
	SoundChannel* channel = AudioRenderer::Get()->Get_Tracked_Audio_Handle(self.object_value.GetNullCheck(vm)->Get_Slot(0).int_value, self.object_value.GetNullCheck(vm)->Get_Slot(1).int_value).Get_Channel();
	if (channel != NULL)
	{
		return channel->Pause();
	}
}

void CRuntime_Audio_Channel::Resume(CVirtualMachine* vm, CVMValue self)
{
	SoundChannel* channel = AudioRenderer::Get()->Get_Tracked_Audio_Handle(self.object_value.GetNullCheck(vm)->Get_Slot(0).int_value, self.object_value.GetNullCheck(vm)->Get_Slot(1).int_value).Get_Channel();
	if (channel != NULL)
	{
		return channel->Resume();
	}
}

void CRuntime_Audio_Channel::Mute(CVirtualMachine* vm, CVMValue self)
{
	SoundChannel* channel = AudioRenderer::Get()->Get_Tracked_Audio_Handle(self.object_value.GetNullCheck(vm)->Get_Slot(0).int_value, self.object_value.GetNullCheck(vm)->Get_Slot(1).int_value).Get_Channel();
	if (channel != NULL)
	{
		return channel->Mute();
	}
}

void CRuntime_Audio_Channel::Unmute(CVirtualMachine* vm, CVMValue self)
{
	SoundChannel* channel = AudioRenderer::Get()->Get_Tracked_Audio_Handle(self.object_value.GetNullCheck(vm)->Get_Slot(0).int_value, self.object_value.GetNullCheck(vm)->Get_Slot(1).int_value).Get_Channel();
	if (channel != NULL)
	{
		return channel->Unmute();
	}
}

void CRuntime_Audio_Channel::Bind(CVirtualMachine* vm)
{
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>("Audio_Channel", "Get_Position", &Get_Position);
	vm->Get_Bindings()->Bind_Method<void,CVMObjectHandle>("Audio_Channel", "Set_Position", &Set_Position);
	vm->Get_Bindings()->Bind_Method<float>("Audio_Channel", "Get_Progress", &Get_Progress);
	vm->Get_Bindings()->Bind_Method<void,float>("Audio_Channel", "Set_Progress", &Set_Progress);
	vm->Get_Bindings()->Bind_Method<float>("Audio_Channel", "Get_Pan", &Get_Pan);
	vm->Get_Bindings()->Bind_Method<void,float>("Audio_Channel", "Set_Pan", &Set_Pan);
	vm->Get_Bindings()->Bind_Method<float>("Audio_Channel", "Get_Rate", &Get_Rate);
	vm->Get_Bindings()->Bind_Method<void,float>("Audio_Channel", "Set_Rate", &Set_Rate);
	vm->Get_Bindings()->Bind_Method<float>("Audio_Channel", "Get_Volume", &Get_Volume);
	vm->Get_Bindings()->Bind_Method<void,float>("Audio_Channel", "Set_Volume", &Set_Volume);
	vm->Get_Bindings()->Bind_Method<int>("Audio_Channel", "Is_Paused", &Is_Paused);
	vm->Get_Bindings()->Bind_Method<int>("Audio_Channel", "Is_Playing", &Is_Playing);
	vm->Get_Bindings()->Bind_Method<int>("Audio_Channel", "Is_Muted", &Is_Muted);
	vm->Get_Bindings()->Bind_Method<void>("Audio_Channel", "Stop", &Stop);
	vm->Get_Bindings()->Bind_Method<void>("Audio_Channel", "Pause", &Pause);
	vm->Get_Bindings()->Bind_Method<void>("Audio_Channel", "Resume", &Resume);
	vm->Get_Bindings()->Bind_Method<void>("Audio_Channel", "Mute", &Mute);
	vm->Get_Bindings()->Bind_Method<void>("Audio_Channel", "Unmute", &Unmute);
}
