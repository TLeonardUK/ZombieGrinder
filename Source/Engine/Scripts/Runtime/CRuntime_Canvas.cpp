// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Scripts/Runtime/CRuntime_Canvas.h"
#include "XScript/VirtualMachine/CVMBindingHelper.h"
#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMContext.h"
#include "XScript/VirtualMachine/CVMBinary.h"
#include "Engine/Resources/ResourceFactory.h"
#include "Engine/Renderer/Canvas.h"
#include "Engine/Display/GfxDisplay.h"
#include "Engine/Engine/GameEngine.h"
#include "Engine/UI/UIManager.h"

CVMObjectHandle CRuntime_Canvas::Get_Bounds(CVirtualMachine* vm, CVMValue self)
{
	Canvas* canvas = reinterpret_cast<Canvas*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));

	CVMLinkedSymbol* ret_symbol = vm->Get_Symbol_Table_Entry(vm->Get_Active_Function()->symbol->method_data->return_data_type_class_index);

	CVMObjectHandle result = vm->New_Object(ret_symbol, false);
	CVMObject* obj_result = result.Get();
	obj_result->Resize(4);
	obj_result->Get_Slot(0).float_value = canvas->Get_Bounds().X;
	obj_result->Get_Slot(1).float_value = canvas->Get_Bounds().Y;
	obj_result->Get_Slot(2).float_value = canvas->Get_Bounds().Width;
	obj_result->Get_Slot(3).float_value = canvas->Get_Bounds().Height;

	return result;
}

CVMObjectHandle CRuntime_Canvas::Get_Display_Bounds(CVirtualMachine* vm, CVMValue self)
{
	Canvas* canvas = reinterpret_cast<Canvas*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));

	CVMLinkedSymbol* ret_symbol = vm->Get_Symbol_Table_Entry(vm->Get_Active_Function()->symbol->method_data->return_data_type_class_index);

	CVMObjectHandle result = vm->New_Object(ret_symbol, false);
	CVMObject* obj_result = result.Get();
	obj_result->Resize(4);
	obj_result->Get_Slot(0).float_value = 0.0f;
	obj_result->Get_Slot(1).float_value = 0.0f;
	obj_result->Get_Slot(2).float_value = (float)GfxDisplay::Get()->Get_Width();
	obj_result->Get_Slot(3).float_value = (float)GfxDisplay::Get()->Get_Height();

	return result;
}
void CRuntime_Canvas::Set_Bounds(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value)
{
	Canvas* canvas = reinterpret_cast<Canvas*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	canvas->Set_Bounds(
		Rect2D(
			value.Get()->Get_Slot(0).float_value,
			value.Get()->Get_Slot(1).float_value,
			value.Get()->Get_Slot(2).float_value,
			value.Get()->Get_Slot(3).float_value
		)
	);
}

CVMObjectHandle CRuntime_Canvas::Get_Viewport(CVirtualMachine* vm, CVMValue self)
{
	Canvas* canvas = reinterpret_cast<Canvas*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));

	CVMLinkedSymbol* ret_symbol = vm->Get_Symbol_Table_Entry(vm->Get_Active_Function()->symbol->method_data->return_data_type_class_index);

	CVMObjectHandle result = vm->New_Object(ret_symbol, false);
	CVMObject* obj_result = result.Get();
	obj_result->Resize(4);
	obj_result->Get_Slot(0).float_value = canvas->Get_Viewport().X;
	obj_result->Get_Slot(1).float_value = canvas->Get_Viewport().Y;
	obj_result->Get_Slot(2).float_value = canvas->Get_Viewport().Width;
	obj_result->Get_Slot(3).float_value = canvas->Get_Viewport().Height;

	return result;
}

void CRuntime_Canvas::Set_Viewport(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value)
{
	Canvas* canvas = reinterpret_cast<Canvas*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	canvas->Set_Viewport(
		Rect2D(
			value.Get()->Get_Slot(0).float_value,
			value.Get()->Get_Slot(1).float_value,
			value.Get()->Get_Slot(2).float_value,
			value.Get()->Get_Slot(3).float_value
		)
	);
}

CVMObjectHandle CRuntime_Canvas::Get_Color(CVirtualMachine* vm, CVMValue self)
{
	Canvas* canvas = reinterpret_cast<Canvas*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));

	CVMLinkedSymbol* ret_symbol = vm->Get_Symbol_Table_Entry(vm->Get_Active_Function()->symbol->method_data->return_data_type_class_index);

	CVMObjectHandle result = vm->New_Object(ret_symbol, false);
	CVMObject* obj_result = result.Get();
	obj_result->Resize(4);
	obj_result->Get_Slot(0).float_value = canvas->Get_Color().R;
	obj_result->Get_Slot(1).float_value = canvas->Get_Color().G;
	obj_result->Get_Slot(2).float_value = canvas->Get_Color().B;
	obj_result->Get_Slot(3).float_value = canvas->Get_Color().A;

	return result;
}

void CRuntime_Canvas::Set_Color(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value)
{
	Canvas* canvas = reinterpret_cast<Canvas*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	canvas->Set_Color(
		Color(
			value.Get()->Get_Slot(0).float_value,
			value.Get()->Get_Slot(1).float_value,
			value.Get()->Get_Slot(2).float_value,
			value.Get()->Get_Slot(3).float_value
		)
	);
}

CVMObjectHandle CRuntime_Canvas::Get_Global_Color(CVirtualMachine* vm, CVMValue self)
{
	Canvas* canvas = reinterpret_cast<Canvas*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));

	CVMLinkedSymbol* ret_symbol = vm->Get_Symbol_Table_Entry(vm->Get_Active_Function()->symbol->method_data->return_data_type_class_index);

	CVMObjectHandle result = vm->New_Object(ret_symbol, false);
	CVMObject* obj_result = result.Get();
	obj_result->Resize(4);
	obj_result->Get_Slot(0).float_value = canvas->Get_Global_Color().R;
	obj_result->Get_Slot(1).float_value = canvas->Get_Global_Color().G;
	obj_result->Get_Slot(2).float_value = canvas->Get_Global_Color().B;
	obj_result->Get_Slot(3).float_value = canvas->Get_Global_Color().A;

	return result;
}

void CRuntime_Canvas::Set_Global_Color(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value)
{
	Canvas* canvas = reinterpret_cast<Canvas*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	canvas->Set_Global_Color(
		Color(
			value.Get()->Get_Slot(0).float_value,
			value.Get()->Get_Slot(1).float_value,
			value.Get()->Get_Slot(2).float_value,
			value.Get()->Get_Slot(3).float_value
			)
		);
}

CVMObjectHandle CRuntime_Canvas::Get_Scale(CVirtualMachine* vm, CVMValue self)
{
	Canvas* canvas = reinterpret_cast<Canvas*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));

	CVMLinkedSymbol* ret_symbol = vm->Get_Symbol_Table_Entry(vm->Get_Active_Function()->symbol->method_data->return_data_type_class_index);

	CVMObjectHandle result = vm->New_Object(ret_symbol, false);
	CVMObject* obj_result = result.Get();
	obj_result->Resize(2);
	obj_result->Get_Slot(0).float_value = canvas->Get_Scale().X;
	obj_result->Get_Slot(1).float_value = canvas->Get_Scale().Y;

	return result;
}

float CRuntime_Canvas::Get_UI_Scale(CVirtualMachine* vm, CVMValue self)
{
	return GameEngine::Get()->Get_UIManager()->Get_UI_Scale().X;
}

void CRuntime_Canvas::Set_Scale(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value)
{
	Canvas* canvas = reinterpret_cast<Canvas*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	canvas->Set_Scale(
		Vector2(
			value.Get()->Get_Slot(0).float_value,
			value.Get()->Get_Slot(1).float_value
		)
	);
}

float CRuntime_Canvas::Get_Rotation(CVirtualMachine* vm, CVMValue self)
{
	Canvas* canvas = reinterpret_cast<Canvas*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return canvas->Get_Rotation();
}

void CRuntime_Canvas::Set_Rotation(CVirtualMachine* vm, CVMValue self, float value)
{
	Canvas* canvas = reinterpret_cast<Canvas*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	canvas->Set_Rotation(value);
}

int CRuntime_Canvas::Get_Blend(CVirtualMachine* vm, CVMValue self)
{
	Canvas* canvas = reinterpret_cast<Canvas*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return canvas->Get_Blend();
}

void CRuntime_Canvas::Set_Blend(CVirtualMachine* vm, CVMValue self, int value)
{
	Canvas* canvas = reinterpret_cast<Canvas*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	canvas->Set_Blend((RendererOption::Type)value);
}

int CRuntime_Canvas::Get_Font_Shadowed(CVirtualMachine* vm, CVMValue self)
{
	Canvas* canvas = reinterpret_cast<Canvas*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return canvas->Get_Font_Shadowed();
}

void CRuntime_Canvas::Set_Font_Shadowed(CVirtualMachine* vm, CVMValue self, int value)
{
	Canvas* canvas = reinterpret_cast<Canvas*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	canvas->Set_Font_Shadowed(!!value);
}

CVMString CRuntime_Canvas::Get_Font(CVirtualMachine* vm, CVMValue self)
{
	Canvas* canvas = reinterpret_cast<Canvas*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	FontHandle* font = canvas->Get_Font();
	return font == NULL ? "" : font->Get()->Get_Name().c_str();
}

void CRuntime_Canvas::Set_Font(CVirtualMachine* vm, CVMValue self, CVMString value)
{
	Canvas* canvas = reinterpret_cast<Canvas*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	canvas->Set_Font(ResourceFactory::Get()->Get_Font(value.C_Str()));
}

void CRuntime_Canvas::Draw_Frame(CVirtualMachine* vm, CVMValue self, CVMString frame_name, CVMObjectHandle point, int flip_h, int flip_v)
{
	Canvas* canvas = reinterpret_cast<Canvas*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	canvas->Draw_Frame(
		frame_name.C_Str(), 
		Vector2(point.Get()->Get_Slot(0).float_value, point.Get()->Get_Slot(1).float_value),
		flip_h != 0,
		flip_v != 0
	);
}

void CRuntime_Canvas::Draw_Animation(CVirtualMachine* vm, CVMValue self, CVMString frame_name, CVMObjectHandle bounds, int flip_h, int flip_v)
{
	Canvas* canvas = reinterpret_cast<Canvas*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	canvas->Draw_Animation(
		frame_name.C_Str(), 
		Rect2D(bounds.Get()->Get_Slot(0).float_value, 
				bounds.Get()->Get_Slot(1).float_value, 
				bounds.Get()->Get_Slot(2).float_value, 
				bounds.Get()->Get_Slot(3).float_value),
		flip_h != 0,
		flip_v != 0
		);
}

void CRuntime_Canvas::Draw_Frame_Rect(CVirtualMachine* vm, CVMValue self, CVMString frame_name, CVMObjectHandle bounds, int flip_h, int flip_v)
{
	Canvas* canvas = reinterpret_cast<Canvas*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	canvas->Draw_Frame(
		frame_name.C_Str(), 
		Rect2D(bounds.Get()->Get_Slot(0).float_value, 
			   bounds.Get()->Get_Slot(1).float_value, 
			   bounds.Get()->Get_Slot(2).float_value, 
			   bounds.Get()->Get_Slot(3).float_value),
		flip_h != 0,
		flip_v != 0
	);
}

void CRuntime_Canvas::Draw_Frame_Box(CVirtualMachine* vm, CVMValue self, CVMString frame_name, CVMObjectHandle bounds, float scale)
{
	Canvas* canvas = reinterpret_cast<Canvas*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	canvas->Draw_Frame_Box(frame_name.C_Str(),
		Rect2D(bounds.Get()->Get_Slot(0).float_value, 
			   bounds.Get()->Get_Slot(1).float_value, 
			   bounds.Get()->Get_Slot(2).float_value, 
			   bounds.Get()->Get_Slot(3).float_value),
			   scale);
}

void CRuntime_Canvas::Draw_Rect(CVirtualMachine* vm, CVMValue self, CVMObjectHandle bounds)
{
	Canvas* canvas = reinterpret_cast<Canvas*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	canvas->Draw_Rect(Rect2D(
		bounds.Get()->Get_Slot(0).float_value,
		bounds.Get()->Get_Slot(1).float_value,
		bounds.Get()->Get_Slot(2).float_value,
		bounds.Get()->Get_Slot(3).float_value));
}

void CRuntime_Canvas::Draw_Hollow_Rect(CVirtualMachine* vm, CVMValue self, CVMObjectHandle bounds, float line_width)
{
	Canvas* canvas = reinterpret_cast<Canvas*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	canvas->Draw_Hollow_Rect(Rect2D(
		bounds.Get()->Get_Slot(0).float_value,
		bounds.Get()->Get_Slot(1).float_value,
		bounds.Get()->Get_Slot(2).float_value,
		bounds.Get()->Get_Slot(3).float_value));
}

void CRuntime_Canvas::Draw_Oval(CVirtualMachine* vm, CVMValue self, CVMObjectHandle bounds)
{
	Canvas* canvas = reinterpret_cast<Canvas*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	canvas->Draw_Oval(Rect2D(
		bounds.Get()->Get_Slot(0).float_value,
		bounds.Get()->Get_Slot(1).float_value,
		bounds.Get()->Get_Slot(2).float_value,
		bounds.Get()->Get_Slot(3).float_value));
}

void CRuntime_Canvas::Draw_Line(CVirtualMachine* vm, CVMValue self, CVMObjectHandle startp, CVMObjectHandle endp, float line_width)
{
	Canvas* canvas = reinterpret_cast<Canvas*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	canvas->Draw_Line(
		Vector2
		(
			startp.Get()->Get_Slot(0).float_value,
			startp.Get()->Get_Slot(1).float_value
		),
		Vector2
		(
			endp.Get()->Get_Slot(0).float_value,
			endp.Get()->Get_Slot(1).float_value
		),
		line_width
	);
}

void CRuntime_Canvas::Draw_Text(CVirtualMachine* vm, CVMValue self, CVMString text, CVMObjectHandle bounds, float font_height, int h_align, int v_align)
{
	Canvas* canvas = reinterpret_cast<Canvas*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	canvas->Draw_Text(
		text.C_Str(),
		Rect2D(bounds.Get()->Get_Slot(0).float_value, 
			   bounds.Get()->Get_Slot(1).float_value, 
			   bounds.Get()->Get_Slot(2).float_value, 
			   bounds.Get()->Get_Slot(3).float_value),
		font_height,
		(TextAlignment::Type)h_align,
		(TextAlignment::Type)v_align
	);
}

void CRuntime_Canvas::Draw_Text_Point(CVirtualMachine* vm, CVMValue self, CVMString text, CVMObjectHandle point, float font_height)
{
	Canvas* canvas = reinterpret_cast<Canvas*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	canvas->Draw_Text(
		text.C_Str(),
		Vector2(point.Get()->Get_Slot(0).float_value, 
			    point.Get()->Get_Slot(1).float_value),
		font_height
	);
}

void CRuntime_Canvas::Draw_Markup_Text(CVirtualMachine* vm, CVMValue self, CVMString text, CVMObjectHandle bounds, float font_height, int h_align, int v_align)
{
	Canvas* canvas = reinterpret_cast<Canvas*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	canvas->Draw_Markup_Text(
		text.C_Str(),
		Rect2D(bounds.Get()->Get_Slot(0).float_value, 
			   bounds.Get()->Get_Slot(1).float_value, 
			   bounds.Get()->Get_Slot(2).float_value, 
			   bounds.Get()->Get_Slot(3).float_value),
		font_height,
		(TextAlignment::Type)h_align,
		(TextAlignment::Type)v_align
	);
}

void CRuntime_Canvas::Draw_Markup_Text_Point(CVirtualMachine* vm, CVMValue self, CVMString text, CVMObjectHandle point, float font_height)
{
	Canvas* canvas = reinterpret_cast<Canvas*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	canvas->Draw_Markup_Text(
		text.C_Str(),
		Vector2(point.Get()->Get_Slot(0).float_value, 
			    point.Get()->Get_Slot(1).float_value),
		font_height
	);
}

CVMString CRuntime_Canvas::Word_Wrap(CVirtualMachine* vm, CVMValue self, CVMString text, CVMObjectHandle bounds, float font_height)
{
	Canvas* canvas = reinterpret_cast<Canvas*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return canvas->Word_Wrap(
		text.C_Str(), 
		Rect2D(bounds.Get()->Get_Slot(0).float_value, 
			   bounds.Get()->Get_Slot(1).float_value, 
			   bounds.Get()->Get_Slot(2).float_value, 
			   bounds.Get()->Get_Slot(3).float_value),
		font_height).c_str();
}

CVMObjectHandle CRuntime_Canvas::Calculate_Text_Size(CVirtualMachine* vm, CVMValue self, CVMString text, float font_height)
{
	Canvas* canvas = reinterpret_cast<Canvas*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));

	CVMLinkedSymbol* ret_symbol = vm->Get_Symbol_Table_Entry(vm->Get_Active_Function()->symbol->method_data->return_data_type_class_index);

	Vector2 size = canvas->Calculate_Text_Size(text.C_Str(), font_height);

	CVMObjectHandle result = vm->New_Object(ret_symbol, false);
	CVMObject* obj_result = result.Get();
	obj_result->Resize(2);
	obj_result->Get_Slot(0).float_value = size.X;
	obj_result->Get_Slot(1).float_value = size.Y;

	return result;
}

CVMString CRuntime_Canvas::Word_Wrap_Markup(CVirtualMachine* vm, CVMValue self, CVMString text, CVMObjectHandle bounds, float font_height)
{
	Canvas* canvas = reinterpret_cast<Canvas*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return canvas->Word_Wrap_Markup(
		text.C_Str(), 
		Rect2D(bounds.Get()->Get_Slot(0).float_value, 
			   bounds.Get()->Get_Slot(1).float_value, 
			   bounds.Get()->Get_Slot(2).float_value, 
			   bounds.Get()->Get_Slot(3).float_value),
		font_height).c_str();
}

CVMObjectHandle CRuntime_Canvas::Calculate_Markup_Text_Size(CVirtualMachine* vm, CVMValue self, CVMString text, float font_height)
{
	Canvas* canvas = reinterpret_cast<Canvas*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));

	CVMLinkedSymbol* ret_symbol = vm->Get_Symbol_Table_Entry(vm->Get_Active_Function()->symbol->method_data->return_data_type_class_index);

	Vector2 size = canvas->Calculate_Markup_Text_Size(text.C_Str(), font_height);

	CVMObjectHandle result = vm->New_Object(ret_symbol, false);
	CVMObject* obj_result = result.Get();
	obj_result->Resize(2);
	obj_result->Get_Slot(0).float_value = size.X;
	obj_result->Get_Slot(1).float_value = size.Y;

	return result;
}

CVMString CRuntime_Canvas::Escape_Markup(CVirtualMachine* vm, CVMValue self, CVMString text)
{
	Canvas* canvas = reinterpret_cast<Canvas*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return canvas->Escape_Markup(text.C_Str()).c_str();
}

CVMObjectHandle CRuntime_Canvas::Get_Frame_Size(CVirtualMachine* vm, CVMString name)
{
	AtlasFrame* frame = ResourceFactory::Get()->Get_Atlas_Frame(name.C_Str());

	CVMLinkedSymbol* ret_symbol = vm->Get_Symbol_Table_Entry(vm->Get_Active_Function()->symbol->method_data->return_data_type_class_index);

	CVMObjectHandle result = vm->New_Object(ret_symbol, false);
	CVMObject* obj_result = result.Get();
	obj_result->Resize(2);
	obj_result->Get_Slot(0).float_value = frame->Rect.Width;
	obj_result->Get_Slot(1).float_value = frame->Rect.Height;

	return result;
}

CVMObjectHandle CRuntime_Canvas::World_BBox_To_Screen(CVirtualMachine* vm, CVMValue self, CVMObjectHandle bbox)
{
	Canvas* canvas = reinterpret_cast<Canvas*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));

	Vector4 vec = bbox.Get()->To_Vec4();
	Rect2D world_bb = Rect2D(vec.X, vec.Y, vec.Z, vec.W);
	Rect2D screen_bbox = canvas->World_To_Screen(world_bb);
	return vm->Create_Vec4(screen_bbox.X, screen_bbox.Y, screen_bbox.Width, screen_bbox.Height);
}

CVMObjectHandle CRuntime_Canvas::Screen_BBox_To_World(CVirtualMachine* vm, CVMValue self, CVMObjectHandle bbox)
{
	Canvas* canvas = reinterpret_cast<Canvas*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));

	Vector4 vec = bbox.Get()->To_Vec4();
	Rect2D world_bb = Rect2D(vec.X, vec.Y, vec.Z, vec.W);
	Rect2D screen_bbox = canvas->Screen_To_World(world_bb);
	return vm->Create_Vec4(screen_bbox.X, screen_bbox.Y, screen_bbox.Width, screen_bbox.Height);
}

int CRuntime_Canvas::Get_Animation_Frame_Count(CVirtualMachine* vm, CVMString anim_name)
{
	AtlasAnimation* anim = ResourceFactory::Get()->Get_Atlas_Animation(anim_name.C_Str());
	return anim == NULL ? 0 : anim->Frames.size();
}

CVMString CRuntime_Canvas::Get_Animation_Frame(CVirtualMachine* vm, CVMString anim_name, int frame)
{
	AtlasAnimation* anim = ResourceFactory::Get()->Get_Atlas_Animation(anim_name.C_Str());
	return anim == NULL ? "" : anim->Frames.at(frame)->Name.c_str();
}

void CRuntime_Canvas::Bind(CVirtualMachine* vm)
{
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>("Canvas", "Get_Bounds", &Get_Bounds);
	vm->Get_Bindings()->Bind_Method<void,CVMObjectHandle>("Canvas", "Set_Bounds", &Set_Bounds);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>("Canvas", "Get_Display_Bounds", &Get_Display_Bounds);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>("Canvas", "Get_Viewport", &Get_Viewport);
	vm->Get_Bindings()->Bind_Method<void,CVMObjectHandle>("Canvas", "Set_Viewport", &Set_Viewport);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>("Canvas", "Get_Color", &Get_Color);
	vm->Get_Bindings()->Bind_Method<void,CVMObjectHandle>("Canvas", "Set_Color", &Set_Color);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>("Canvas", "Get_Global_Color", &Get_Global_Color);
	vm->Get_Bindings()->Bind_Method<void, CVMObjectHandle>("Canvas", "Set_Global_Color", &Set_Global_Color);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>("Canvas", "Get_Scale", &Get_Scale);
	vm->Get_Bindings()->Bind_Method<void,CVMObjectHandle>("Canvas", "Set_Scale", &Set_Scale);
	vm->Get_Bindings()->Bind_Method<float>("Canvas", "Get_UI_Scale", &Get_UI_Scale);
	vm->Get_Bindings()->Bind_Method<float>("Canvas", "Get_Rotation", &Get_Rotation);
	vm->Get_Bindings()->Bind_Method<void,float>("Canvas", "Set_Rotation", &Set_Rotation);
	vm->Get_Bindings()->Bind_Method<int>("Canvas", "Get_Blend", &Get_Blend);
	vm->Get_Bindings()->Bind_Method<void,int>("Canvas", "Set_Blend", &Set_Blend);
	vm->Get_Bindings()->Bind_Method<int>("Canvas", "Get_Font_Shadowed", &Get_Font_Shadowed);
	vm->Get_Bindings()->Bind_Method<void,int>("Canvas", "Set_Font_Shadowed", &Set_Font_Shadowed);
	vm->Get_Bindings()->Bind_Method<CVMString>("Canvas", "Get_Font", &Get_Font);
	vm->Get_Bindings()->Bind_Method<void,CVMString>("Canvas", "Set_Font", &Set_Font);
	vm->Get_Bindings()->Bind_Method<void,CVMString,CVMObjectHandle,int,int>("Canvas", "Draw_Frame", &Draw_Frame);
	vm->Get_Bindings()->Bind_Method<void,CVMString,CVMObjectHandle,int,int>("Canvas", "Draw_Animation", &Draw_Animation);
	vm->Get_Bindings()->Bind_Method<void,CVMString,CVMObjectHandle,int,int>("Canvas", "Draw_Frame_Rect", &Draw_Frame_Rect);
	vm->Get_Bindings()->Bind_Method<void,CVMString,CVMObjectHandle, float>("Canvas", "Draw_Frame_Box", &Draw_Frame_Box);
	vm->Get_Bindings()->Bind_Method<void,CVMObjectHandle>("Canvas", "Draw_Rect", &Draw_Rect);
	vm->Get_Bindings()->Bind_Method<void,CVMObjectHandle,float>("Canvas", "Draw_Hollow_Rect", &Draw_Hollow_Rect);
	vm->Get_Bindings()->Bind_Method<void,CVMObjectHandle>("Canvas", "Draw_Oval", &Draw_Oval);
	vm->Get_Bindings()->Bind_Method<void,CVMObjectHandle,CVMObjectHandle,float>("Canvas", "Draw_Line", &Draw_Line);
	vm->Get_Bindings()->Bind_Method<void,CVMString,CVMObjectHandle,float,int,int>("Canvas", "Draw_Text", &Draw_Text);
	vm->Get_Bindings()->Bind_Method<void,CVMString,CVMObjectHandle,float>("Canvas", "Draw_Text_Point", &Draw_Text_Point);
	vm->Get_Bindings()->Bind_Method<void,CVMString,CVMObjectHandle,float,int,int>("Canvas", "Draw_Markup_Text", &Draw_Markup_Text);
	vm->Get_Bindings()->Bind_Method<void,CVMString,CVMObjectHandle,float>("Canvas", "Draw_Markup_Text_Point", &Draw_Markup_Text_Point);
	vm->Get_Bindings()->Bind_Method<CVMString,CVMString,CVMObjectHandle,float>("Canvas", "Word_Wrap", &Word_Wrap);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle,CVMString,float>("Canvas", "Calculate_Text_Size", &Calculate_Text_Size);
	vm->Get_Bindings()->Bind_Method<CVMString,CVMString,CVMObjectHandle,float>("Canvas", "Word_Wrap_Markup", &Word_Wrap_Markup);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle,CVMString,float>("Canvas", "Calculate_Markup_Text_Size", &Calculate_Markup_Text_Size);
	vm->Get_Bindings()->Bind_Method<CVMString,CVMString>("Canvas", "Escape_Markup", &Escape_Markup);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle,CVMObjectHandle>("Canvas", "World_BBox_To_Screen", &World_BBox_To_Screen);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle, CVMObjectHandle>("Canvas", "Screen_BBox_To_World", &Screen_BBox_To_World);
	vm->Get_Bindings()->Bind_Function<CVMObjectHandle,CVMString>("Canvas", "Get_Frame_Size", &Get_Frame_Size);
	vm->Get_Bindings()->Bind_Function<int,CVMString>("Canvas", "Get_Animation_Frame_Count", &Get_Animation_Frame_Count);
	vm->Get_Bindings()->Bind_Function<CVMString,CVMString,int>("Canvas", "Get_Animation_Frame", &Get_Animation_Frame);
}
