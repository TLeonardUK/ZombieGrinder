// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Scripts/Runtime/CRuntime_Post_Process_FX.h"
#include "XScript/VirtualMachine/CVMBindingHelper.h"
#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMContext.h"
#include "XScript/VirtualMachine/CVMBinary.h"
#include "Game/Scene/PostProcess/GamePostProcessManager.h"
#include "Engine/Resources/ResourceFactory.h"
#include "Engine/Resources/Compiled/Shaders/CompiledShader.h"
#include "Engine/Renderer/Shaders/ShaderProgram.h"
#include "Engine/Renderer/Shaders/ShaderProgramHandle.h"

int CRuntime_Post_Process_FX::Get_Active(CVirtualMachine* vm, CVMValue self)
{
	PostProcessEffect* fx = reinterpret_cast<PostProcessEffect*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return fx->active;
}

void CRuntime_Post_Process_FX::Set_Active(CVirtualMachine* vm, CVMValue self, int value)
{
	PostProcessEffect* fx = reinterpret_cast<PostProcessEffect*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	fx->active = !!value;
}

int CRuntime_Post_Process_FX::Get_Camera_Index(CVirtualMachine* vm, CVMValue self)
{
	PostProcessEffect* fx = reinterpret_cast<PostProcessEffect*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return fx->camera_index;
}

CVMString CRuntime_Post_Process_FX::Get_Shader(CVirtualMachine* vm, CVMValue self)
{
	PostProcessEffect* fx = reinterpret_cast<PostProcessEffect*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return fx->shader.c_str();
}

void CRuntime_Post_Process_FX::Set_Shader(CVirtualMachine* vm, CVMValue self, CVMString value)
{
	PostProcessEffect* fx = reinterpret_cast<PostProcessEffect*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	fx->shader = value.C_Str();
}

void CRuntime_Post_Process_FX::Set_Shader_Uniform_Float(CVirtualMachine* vm, CVMValue self, CVMString name, float value)
{
	PostProcessEffect* fx = reinterpret_cast<PostProcessEffect*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	if (fx->resolved_shader != NULL && fx->resolved_shader->Shader_Program->Get() != NULL)
	{
		fx->resolved_shader->Shader_Program->Get()->Bind_Float(name.C_Str(), value);
	}
}

void CRuntime_Post_Process_FX::Set_Shader_Uniform_Vec3(CVirtualMachine* vm, CVMValue self, CVMString name, CVMObjectHandle value)
{
	PostProcessEffect* fx = reinterpret_cast<PostProcessEffect*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	if (fx->resolved_shader != NULL && fx->resolved_shader->Shader_Program->Get() != NULL)
	{
		fx->resolved_shader->Shader_Program->Get()->Bind_Vector(name.C_Str(), value.Get()->To_Vec3());
	}
}

CVMObjectHandle CRuntime_Post_Process_FX::Get_Instance(CVirtualMachine* vm, CVMObjectHandle type_of, int camera_index)
{
	CVMLinkedSymbol* class_symbol = reinterpret_cast<CVMLinkedSymbol*>(type_of.Get()->Get_Meta_Data());	
	PostProcessEffect* fx = GamePostProcessManager::Get()->Get_Instance(class_symbol, camera_index);
	return fx == NULL ? NULL : fx->script_object.Get();
}

void CRuntime_Post_Process_FX::Bind(CVirtualMachine* vm)
{
	vm->Get_Bindings()->Bind_Method<int>("Post_Process_FX", "Get_Active", &Get_Active);
	vm->Get_Bindings()->Bind_Method<void,int>("Post_Process_FX", "Set_Active", &Set_Active);
	vm->Get_Bindings()->Bind_Method<CVMString>("Post_Process_FX", "Get_Shader", &Get_Shader);
	vm->Get_Bindings()->Bind_Method<int>("Post_Process_FX", "Get_Camera_Index", &Get_Camera_Index);
	vm->Get_Bindings()->Bind_Method<void,CVMString>("Post_Process_FX", "Set_Shader", &Set_Shader);
	vm->Get_Bindings()->Bind_Method<void,CVMString,float>("Post_Process_FX", "Set_Shader_Uniform_Float", &Set_Shader_Uniform_Float);
	vm->Get_Bindings()->Bind_Method<void,CVMString,CVMObjectHandle>("Post_Process_FX", "Set_Shader_Uniform_Vec3", &Set_Shader_Uniform_Vec3);

	vm->Get_Bindings()->Bind_Function<CVMObjectHandle,CVMObjectHandle,int>("Post_Process_FX", "Get_Instance", &Get_Instance);
}
