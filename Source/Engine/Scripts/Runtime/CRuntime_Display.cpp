// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "XScript/VirtualMachine/CVMBindingHelper.h"
#include "Engine/Scripts/Runtime/CRuntime_Display.h"
#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMContext.h"
#include "XScript/VirtualMachine/CVMBinary.h"

#include "Engine/Engine/EngineOptions.h"

#include "Engine/Display/GfxDisplay.h"

CVMObjectHandle CRuntime_Display::Get_Resolution(CVirtualMachine* vm)
{	
	CVMLinkedSymbol* vec2_symbol = vm->Get_Symbol_Table_Entry(vm->Get_Active_Function()->symbol->method_data->return_data_type_class_index);

	CVMObjectHandle result = vm->New_Object(vec2_symbol, false);
	CVMObject* obj_result = result.Get();
	obj_result->Resize(2);
	obj_result->Get_Slot(0).float_value = (float)GfxDisplay::Get()->Get_Width();
	obj_result->Get_Slot(1).float_value = (float)GfxDisplay::Get()->Get_Height();

	return result;
}

float CRuntime_Display::Get_Aspect_Ratio(CVirtualMachine* vm)
{	
	return (float)GfxDisplay::Get()->Get_Width() / (float)GfxDisplay::Get()->Get_Height();
}

CVMObjectHandle CRuntime_Display::Get_Game_Resolution(CVirtualMachine* vm)
{
	CVMLinkedSymbol* vec2_symbol = vm->Get_Symbol_Table_Entry(vm->Get_Active_Function()->symbol->method_data->return_data_type_class_index);

	CVMObjectHandle result = vm->New_Object(vec2_symbol, false);
	CVMObject* obj_result = result.Get();
	obj_result->Resize(2);
	obj_result->Get_Slot(0).float_value = (float)*EngineOptions::render_game_width;
	obj_result->Get_Slot(1).float_value = (float)*EngineOptions::render_game_height;

	return result;
}

void CRuntime_Display::Bind(CVirtualMachine* vm)
{
	vm->Get_Bindings()->Bind_Function<CVMObjectHandle>("Display", "Get_Resolution", &Get_Resolution);
	vm->Get_Bindings()->Bind_Function<CVMObjectHandle>("Display", "Get_Game_Resolution", &Get_Game_Resolution);
	vm->Get_Bindings()->Bind_Function<float>("Display", "Get_Aspect_Ratio", &Get_Aspect_Ratio);
}
