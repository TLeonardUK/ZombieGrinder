// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "XScript/VirtualMachine/CVMBindingHelper.h"
#include "Engine/Scripts/Runtime/CRuntime_Audio.h"
#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMContext.h"
#include "XScript/VirtualMachine/CVMBinary.h"
#include "Engine/Audio/AudioRenderer.h"
#include "Engine/Audio/Sounds/Sound.h"
#include "Engine/Audio/Sounds/SoundHandle.h"
#include "Engine/Resources/ResourceFactory.h"

#include "Generic/Math/Random.h"

CVMObjectHandle CRuntime_Audio::Play_Spatial(CVirtualMachine* vm, CVMString resource, CVMObjectHandle position, int start_paused)
{
	CVMLinkedSymbol* ret_symbol = vm->Get_Symbol_Table_Entry(vm->Get_Active_Function()->symbol->method_data->return_data_type_class_index);
	
	// Have we been pased a selection of audio? In which case we need to choose a random one.
	if (strstr(resource.C_Str(), ",") != NULL)
	{
		std::vector<std::string> segments;
		StringHelper::Split(resource.C_Str(), ',', segments);

		resource = segments[Random::Static_Next(0, segments.size())].c_str();
	}

	SoundHandle* res = ResourceFactory::Get()->Get_Sound(resource.C_Str());
	if (res == NULL || res->Get() == NULL)
	{
		DBG_LOG("Attempt to play unknown sound effect '%s'.", resource.C_Str());
		return NULL;
	}

	Vector3 pos;
	pos.X = position.Get()->Get_Slot(0).float_value;
	pos.Y = position.Get()->Get_Slot(1).float_value;
	pos.Z = position.Get()->Get_Slot(2).float_value;

	TrackedAudioHandle handle = AudioRenderer::Get()->Play_Tracked(res, !!start_paused, pos);

	CVMObjectHandle result = vm->New_Object(ret_symbol, false);
	CVMObject* obj_result = result.Get();
	obj_result->Resize(2);
	obj_result->Get_Slot(0).int_value = handle.Get_Index();
	obj_result->Get_Slot(1).int_value = handle.Get_Slot();

	return result;
}

CVMObjectHandle CRuntime_Audio::Play_Global(CVirtualMachine* vm, CVMString resource, int start_paused)
{
	CVMLinkedSymbol* ret_symbol = vm->Get_Symbol_Table_Entry(vm->Get_Active_Function()->symbol->method_data->return_data_type_class_index);

	// Have we been pased a selection of audio? In which case we need to choose a random one.
	if (strstr(resource.C_Str(), ",") != NULL)
	{
		std::vector<std::string> segments;
		StringHelper::Split(resource.C_Str(), ',', segments);

		resource = segments[Random::Static_Next(0, segments.size())].c_str();
	}

	SoundHandle* res = ResourceFactory::Get()->Get_Sound(resource.C_Str());
	if (res == NULL || res->Get() == NULL)
	{
		DBG_LOG("Attempt to play unknown sound effect '%s'.", resource.C_Str());
		return NULL;
	}

	TrackedAudioHandle handle = AudioRenderer::Get()->Play_Tracked(res, !!start_paused);

	CVMObjectHandle result = vm->New_Object(ret_symbol, false);
	CVMObject* obj_result = result.Get();
	obj_result->Resize(2);
	obj_result->Get_Slot(0).int_value = handle.Get_Index();
	obj_result->Get_Slot(1).int_value = handle.Get_Slot();

	return result;
}

void CRuntime_Audio::Play_BGM(CVirtualMachine* vm, CVMString resource)
{
	// Have we been pased a selection of audio? In which case we need to choose a random one.
	if (strstr(resource.C_Str(), ",") != NULL)
	{
		std::vector<std::string> segments;
		StringHelper::Split(resource.C_Str(), ',', segments);

		resource = segments[Random::Static_Next(0, segments.size())].c_str();
	}

	SoundHandle* res = ResourceFactory::Get()->Get_Sound(resource.C_Str());
	if (res == NULL || res->Get() == NULL)
	{
		DBG_LOG("Attempt to play unknown bgm'%s'.", resource.C_Str());
		return;
	}

	if (AudioRenderer::Get()->Get_BGM_Stack_Count() > 0)
	{
		AudioRenderer::Get()->Pop_BGM();
	}

	DBG_LOG("Script pushing BGM '%s'.", resource.C_Str());
	AudioRenderer::Get()->Push_BGM(res);
}

void CRuntime_Audio::Push_BGM(CVirtualMachine* vm, CVMString resource)
{
	// Have we been pased a selection of audio? In which case we need to choose a random one.
	if (strstr(resource.C_Str(), ",") != NULL)
	{
		std::vector<std::string> segments;
		StringHelper::Split(resource.C_Str(), ',', segments);

		resource = segments[Random::Static_Next(0, segments.size())].c_str();
	}

	SoundHandle* res = ResourceFactory::Get()->Get_Sound(resource.C_Str());
	if (res == NULL || res->Get() == NULL)
	{
		DBG_LOG("Attempt to play unknown bgm'%s'.", resource.C_Str());
		return;
	}

	DBG_LOG("Script pushing BGM '%s'.", resource.C_Str());
	AudioRenderer::Get()->Push_BGM(res);
}

void CRuntime_Audio::Pop_BGM(CVirtualMachine* vm)
{
	DBG_LOG("Script poping BGM.");
	AudioRenderer::Get()->Pop_BGM();
}

void CRuntime_Audio::Set_Listener_Position(CVirtualMachine* vm, int listener_index, CVMObjectHandle position)
{
	AudioRenderer::Get()->Set_Listener_Position(listener_index, position.Get()->To_Vec3());
}

float CRuntime_Audio::Get_BGM_Volume_Multiplier(CVirtualMachine* vm)
{
	return AudioRenderer::Get()->Get_BGM_Volume_Multiplier();
}

void CRuntime_Audio::Set_BGM_Volume_Multiplier(CVirtualMachine* vm, float val)
{
	AudioRenderer::Get()->Set_BGM_Volume_Multiplier(val);
}

void CRuntime_Audio::Bind(CVirtualMachine* vm)
{
	vm->Get_Bindings()->Bind_Function<CVMObjectHandle,CVMString,CVMObjectHandle,int>("Audio", "Play_Spatial", &Play_Spatial);
	vm->Get_Bindings()->Bind_Function<CVMObjectHandle,CVMString,int>("Audio", "Play_Global", &Play_Global);

	vm->Get_Bindings()->Bind_Function<void,int,CVMObjectHandle>("Audio", "Set_Listener_Position", &Set_Listener_Position);

	vm->Get_Bindings()->Bind_Function<void,CVMString>("Audio", "Play_BGM", &Play_BGM);
	vm->Get_Bindings()->Bind_Function<void,CVMString>("Audio", "Push_BGM", &Push_BGM);
	vm->Get_Bindings()->Bind_Function<void>("Audio", "Pop_BGM", &Pop_BGM);

	vm->Get_Bindings()->Bind_Function<float>("Audio", "Get_BGM_Volume_Multiplier", &Get_BGM_Volume_Multiplier);
	vm->Get_Bindings()->Bind_Function<void,float>("Audio", "Set_BGM_Volume_Multiplier", &Set_BGM_Volume_Multiplier);
}
