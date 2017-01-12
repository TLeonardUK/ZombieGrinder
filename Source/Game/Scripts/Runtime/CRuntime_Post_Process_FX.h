// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_RUNTIME_POST_PROCESS_FX_
#define _GAME_RUNTIME_POST_PROCESS_FX_

#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMValue.h"

class CRuntime_Post_Process_FX
{
public:
	static int Get_Active(CVirtualMachine* vm, CVMValue self);
	static void Set_Active(CVirtualMachine* vm, CVMValue self, int value);
	static CVMString Get_Shader(CVirtualMachine* vm, CVMValue self);
	static void Set_Shader(CVirtualMachine* vm, CVMValue self, CVMString value);
	static void Set_Shader_Uniform_Float(CVirtualMachine* vm, CVMValue self, CVMString name, float value);
	static void Set_Shader_Uniform_Vec3(CVirtualMachine* vm, CVMValue self, CVMString name, CVMObjectHandle value);
	static int Get_Camera_Index(CVirtualMachine* vm, CVMValue self);

	static CVMObjectHandle Get_Instance(CVirtualMachine* vm, CVMObjectHandle type_of, int camera_index);

	static void Bind(CVirtualMachine* machine);
};

#endif
