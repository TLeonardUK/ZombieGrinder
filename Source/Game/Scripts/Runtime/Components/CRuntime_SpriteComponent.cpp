// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Scripts/Runtime/Components/CRuntime_SpriteComponent.h"
#include "Game/Scene/Actors/ScriptedActor.h"
#include "Game/Scene/Actors/Components/Drawable/SpriteComponent.h"
#include "XScript/VirtualMachine/CVMBindingHelper.h"
#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMContext.h"
#include "XScript/VirtualMachine/CVMBinary.h"
#include "Engine/Resources/ResourceFactory.h"
#include "Engine/Renderer/Atlases/Atlas.h"
#include "Engine/Renderer/Atlases/AtlasHandle.h"

#include "Engine/Engine/GameEngine.h"
#include "Engine/Scene/Scene.h"

CVMObjectHandle CRuntime_SpriteComponent::Create(CVirtualMachine* vm, CVMValue self)
{
	Scene* scene = GameEngine::Get()->Get_Scene();

	ScriptedActor* actor = reinterpret_cast<ScriptedActor*>(vm->Get_Active_Context()->MetaData);

	SpriteComponent* component = new SpriteComponent();
	actor->Add_Component(component);
	scene->Add_Drawable(component);

	CVMObject* obj = self.object_value.Get();
	obj->Set_Meta_Data(component);

	return obj;
}

void CRuntime_SpriteComponent::Set_Visible(CVirtualMachine* vm, CVMValue self, int visible)
{
	SpriteComponent* component = reinterpret_cast<SpriteComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	component->Set_Visible(visible != 0);
}

int CRuntime_SpriteComponent::Get_Visible(CVirtualMachine* vm, CVMValue self)
{
	SpriteComponent* component = reinterpret_cast<SpriteComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return component->Get_Visible();
}

void CRuntime_SpriteComponent::Set_Editor_Visible(CVirtualMachine* vm, CVMValue self, int visible)
{
	SpriteComponent* component = reinterpret_cast<SpriteComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	component->Set_Editor_Visible(visible != 0);
}

int CRuntime_SpriteComponent::Get_Editor_Visible(CVirtualMachine* vm, CVMValue self)
{
	SpriteComponent* component = reinterpret_cast<SpriteComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return component->Get_Editor_Visible();
}

void CRuntime_SpriteComponent::Set_Scale(CVirtualMachine* vm, CVMValue self, int visible)
{
	SpriteComponent* component = reinterpret_cast<SpriteComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	component->Set_Scale(visible != 0);
}

int CRuntime_SpriteComponent::Get_Scale(CVirtualMachine* vm, CVMValue self)
{
	SpriteComponent* component = reinterpret_cast<SpriteComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return component->Get_Scale();
}

void CRuntime_SpriteComponent::Set_Frame(CVirtualMachine* vm, CVMValue self, CVMString frame)
{
	SpriteComponent* component = reinterpret_cast<SpriteComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return component->Set_Frame(ResourceFactory::Get()->Get_Atlas_Frame(frame.C_Str()));
}

CVMString CRuntime_SpriteComponent::Get_Frame(CVirtualMachine* vm, CVMValue self)
{
	SpriteComponent* component = reinterpret_cast<SpriteComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	AtlasFrame* frame = component->Get_Frame();
	return frame != NULL ? frame->Name.c_str() : "";
}

void CRuntime_SpriteComponent::Set_Animation(CVirtualMachine* vm, CVMValue self, CVMString frame)
{
	SpriteComponent* component = reinterpret_cast<SpriteComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	AtlasAnimation* anim = ResourceFactory::Get()->Get_Atlas_Animation(frame.C_Str());
	component->Set_Animation(anim);
}

CVMString CRuntime_SpriteComponent::Get_Animation(CVirtualMachine* vm, CVMValue self)
{
	SpriteComponent* component = reinterpret_cast<SpriteComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	AtlasAnimation* frame = component->Get_Animation();
	return frame != NULL ? frame->Name.c_str() : "";
}

void CRuntime_SpriteComponent::Set_Animation_Speed(CVirtualMachine* vm, CVMValue self, float frame)
{
	SpriteComponent* component = reinterpret_cast<SpriteComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	component->Set_Animation_Speed(frame);
}

float CRuntime_SpriteComponent::Get_Animation_Speed(CVirtualMachine* vm, CVMValue self)
{
	SpriteComponent* component = reinterpret_cast<SpriteComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return component->Get_Animation_Speed();
}

void CRuntime_SpriteComponent::Set_Flip_Horizontal(CVirtualMachine* vm, CVMValue self, int flip)
{
	SpriteComponent* component = reinterpret_cast<SpriteComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	bool flip_h, flip_v;
	component->Get_Flip(flip_h, flip_v);
	component->Set_Flip(flip != 0, flip_v);
}

int CRuntime_SpriteComponent::Get_Flip_Horizontal(CVirtualMachine* vm, CVMValue self)
{
	SpriteComponent* component = reinterpret_cast<SpriteComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	bool flip_h, flip_v;
	component->Get_Flip(flip_h, flip_v);
	return flip_h ? 1 : 0;
}

void CRuntime_SpriteComponent::Set_Flip_Vertical(CVirtualMachine* vm, CVMValue self, int flip)
{
	SpriteComponent* component = reinterpret_cast<SpriteComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	bool flip_h, flip_v;
	component->Get_Flip(flip_h, flip_v);
	component->Set_Flip(flip_h, flip != 0);
}

int CRuntime_SpriteComponent::Get_Flip_Vertical(CVirtualMachine* vm, CVMValue self)
{	
	SpriteComponent* component = reinterpret_cast<SpriteComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	bool flip_h, flip_v;
	component->Get_Flip(flip_h, flip_v);
	return flip_v ? 1 : 0;
}

void CRuntime_SpriteComponent::Set_Sort_By_Transparency(CVirtualMachine* vm, CVMValue self, int flip)
{
	SpriteComponent* component = reinterpret_cast<SpriteComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	component->Set_Sort_By_Transparency(!!flip);
}

int CRuntime_SpriteComponent::Get_Sort_By_Transparency(CVirtualMachine* vm, CVMValue self)
{	
	SpriteComponent* component = reinterpret_cast<SpriteComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return component->Get_Sort_By_Transparency() ? 1 : 0;
}

void CRuntime_SpriteComponent::Set_Allow_Static(CVirtualMachine* vm, CVMValue self, int flip)
{
	SpriteComponent* component = reinterpret_cast<SpriteComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	component->Set_Allow_Static(!!flip);
}

int CRuntime_SpriteComponent::Get_Allow_Static(CVirtualMachine* vm, CVMValue self)
{	
	SpriteComponent* component = reinterpret_cast<SpriteComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return component->Get_Allow_Static() ? 1 : 0;
}

void CRuntime_SpriteComponent::Set_Object_Mask(CVirtualMachine* vm, CVMValue self, CVMObjectHandle color)
{
	SpriteComponent* component = reinterpret_cast<SpriteComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));

	CVMObject* obj = color.Get();
	component->Set_Object_Mask(obj->To_Vec4());
}

CVMObjectHandle CRuntime_SpriteComponent::Get_Object_Mask(CVirtualMachine* vm, CVMValue self)
{
	SpriteComponent* component = reinterpret_cast<SpriteComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	Vector4 color = component->Get_Object_Mask();

	CVMLinkedSymbol* vec4_class = vm->Get_Symbol_Table_Entry(vm->Get_Active_Function()->symbol->method_data->return_data_type_class_index);

	CVMObjectHandle result = vm->New_Object(vec4_class, false);
	result.Get()->Resize(4);
	result.Get()->Get_Slot(0).float_value = color.X;// / 255.0f;
	result.Get()->Get_Slot(1).float_value = color.Y;// / 255.0f;
	result.Get()->Get_Slot(2).float_value = color.Z;// / 255.0f;
	result.Get()->Get_Slot(3).float_value = color.W;// / 255.0f;

	return result;
}

void CRuntime_SpriteComponent::Set_Color(CVirtualMachine* vm, CVMValue self, CVMObjectHandle color)
{
	SpriteComponent* component = reinterpret_cast<SpriteComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));

	CVMObject* obj = color.Get();
	
	Color c;
	c.R = (u8)obj->Get_Slot(0).float_value;// * 255.0f);
	c.G = (u8)obj->Get_Slot(1).float_value;// * 255.0f);
	c.B = (u8)obj->Get_Slot(2).float_value;// * 255.0f);
	c.A = (u8)obj->Get_Slot(3).float_value;// * 255.0f);

	component->Set_Color(c);
}

CVMObjectHandle CRuntime_SpriteComponent::Get_Color(CVirtualMachine* vm, CVMValue self)
{
	SpriteComponent* component = reinterpret_cast<SpriteComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	Color color = component->Get_Color();

	CVMLinkedSymbol* vec4_class = vm->Get_Symbol_Table_Entry(vm->Get_Active_Function()->symbol->method_data->return_data_type_class_index);

	CVMObjectHandle result = vm->New_Object(vec4_class, false);
	result.Get()->Resize(4);
	result.Get()->Get_Slot(0).float_value = color.R;// / 255.0f;
	result.Get()->Get_Slot(1).float_value = color.G;// / 255.0f;
	result.Get()->Get_Slot(2).float_value = color.B;// / 255.0f;
	result.Get()->Get_Slot(3).float_value = color.A;// / 255.0f;

	return result;
}

void CRuntime_SpriteComponent::Set_Color_Scalar(CVirtualMachine* vm, CVMValue self, CVMObjectHandle color)
{
	SpriteComponent* component = reinterpret_cast<SpriteComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));

	CVMObject* obj = color.Get();

	Color c;
	c.R = (u8)obj->Get_Slot(0).float_value;// * 255.0f);
	c.G = (u8)obj->Get_Slot(1).float_value;// * 255.0f);
	c.B = (u8)obj->Get_Slot(2).float_value;// * 255.0f);
	c.A = (u8)obj->Get_Slot(3).float_value;// * 255.0f);

	component->Set_Color_Scalar(c);
}

CVMObjectHandle CRuntime_SpriteComponent::Get_Color_Scalar(CVirtualMachine* vm, CVMValue self)
{
	SpriteComponent* component = reinterpret_cast<SpriteComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	Color color = component->Get_Color_Scalar();

	CVMLinkedSymbol* vec4_class = vm->Get_Symbol_Table_Entry(vm->Get_Active_Function()->symbol->method_data->return_data_type_class_index);

	CVMObjectHandle result = vm->New_Object(vec4_class, false);
	result.Get()->Resize(4);
	result.Get()->Get_Slot(0).float_value = color.R;// / 255.0f;
	result.Get()->Get_Slot(1).float_value = color.G;// / 255.0f;
	result.Get()->Get_Slot(2).float_value = color.B;// / 255.0f;
	result.Get()->Get_Slot(3).float_value = color.A;// / 255.0f;

	return result;
}

void CRuntime_SpriteComponent::Set_Area(CVirtualMachine* vm, CVMValue self, CVMObjectHandle color)
{
	SpriteComponent* component = reinterpret_cast<SpriteComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));

	CVMObject* obj = color.Get();

	Rect2D c;
	c.X = obj->Get_Slot(0).float_value;// * 255.0f);
	c.Y = obj->Get_Slot(1).float_value;// * 255.0f);
	c.Width = obj->Get_Slot(2).float_value;// * 255.0f);
	c.Height = obj->Get_Slot(3).float_value;// * 255.0f);

	component->Set_Area(c);
}

CVMObjectHandle CRuntime_SpriteComponent::Get_Area(CVirtualMachine* vm, CVMValue self)
{
	SpriteComponent* component = reinterpret_cast<SpriteComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	Rect2D color = component->Get_Area();

	CVMLinkedSymbol* vec4_class = vm->Get_Symbol_Table_Entry(vm->Get_Active_Function()->symbol->method_data->return_data_type_class_index);

	CVMObjectHandle result = vm->New_Object(vec4_class, false);
	result.Get()->Resize(4);
	result.Get()->Get_Slot(0).float_value = color.X;// / 255.0f;
	result.Get()->Get_Slot(1).float_value = color.Y;// / 255.0f;
	result.Get()->Get_Slot(2).float_value = color.Width;// / 255.0f;
	result.Get()->Get_Slot(3).float_value = color.Height;// / 255.0f;

	return result;
}

void CRuntime_SpriteComponent::Set_Offset(CVirtualMachine* vm, CVMValue self, CVMObjectHandle color)
{
	SpriteComponent* component = reinterpret_cast<SpriteComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	component->Set_Offset(color.Get()->To_Vec2());
}

CVMObjectHandle CRuntime_SpriteComponent::Get_Offset(CVirtualMachine* vm, CVMValue self)
{
	SpriteComponent* component = reinterpret_cast<SpriteComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return vm->Create_Vec2(component->Get_Offset());
}

void CRuntime_SpriteComponent::Set_Draw_Scale(CVirtualMachine* vm, CVMValue self, CVMObjectHandle color)
{
	SpriteComponent* component = reinterpret_cast<SpriteComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	component->Set_Draw_Scale(color.Get()->To_Vec2());
}

CVMObjectHandle CRuntime_SpriteComponent::Get_Draw_Scale(CVirtualMachine* vm, CVMValue self)
{
	SpriteComponent* component = reinterpret_cast<SpriteComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return vm->Create_Vec2(component->Get_Draw_Scale());
}
void CRuntime_SpriteComponent::Set_Depth_Bias(CVirtualMachine* vm, CVMValue self, float flip)
{
	SpriteComponent* component = reinterpret_cast<SpriteComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return component->Set_Depth_Bias(flip);
}

float CRuntime_SpriteComponent::Get_Depth_Bias(CVirtualMachine* vm, CVMValue self)
{
	SpriteComponent* component = reinterpret_cast<SpriteComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return component->Get_Depth_Bias();
}

void CRuntime_SpriteComponent::Play_Oneshot_Animation(CVirtualMachine* vm, CVMValue self, CVMString name, float speed)
{
	SpriteComponent* component = reinterpret_cast<SpriteComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	AtlasAnimation* anim = ResourceFactory::Get()->Get_Atlas_Animation(name.C_Str());
	if (anim != NULL)
	{
		component->Play_Oneshot_Animation(anim, speed);
	}
	else
	{
	//	DBG_LOG("Could not find animation '%s'.", name.C_Str());
	}
}

int CRuntime_SpriteComponent::Is_Oneshot_Animation_Finished(CVirtualMachine* vm, CVMValue self)
{
	SpriteComponent* component = reinterpret_cast<SpriteComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return component->Is_Oneshot_Finished() ? 1 : 0;
}

void CRuntime_SpriteComponent::Bind(CVirtualMachine* vm)
{
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>		("SpriteComponent", "Create",					&Create);

	vm->Get_Bindings()->Bind_Method<void,int>				("SpriteComponent", "Set_Visible",				&Set_Visible);
	vm->Get_Bindings()->Bind_Method<int>					("SpriteComponent", "Get_Visible",				&Get_Visible);
	vm->Get_Bindings()->Bind_Method<void,int>				("SpriteComponent", "Set_Editor_Visible",		&Set_Editor_Visible);
	vm->Get_Bindings()->Bind_Method<int>					("SpriteComponent", "Get_Editor_Visible",		&Get_Editor_Visible);
	vm->Get_Bindings()->Bind_Method<void,int>				("SpriteComponent", "Set_Scale",				&Set_Scale);
	vm->Get_Bindings()->Bind_Method<int>					("SpriteComponent", "Get_Scale",				&Get_Scale);

	vm->Get_Bindings()->Bind_Method<void,CVMString>			("SpriteComponent", "Set_Frame",				&Set_Frame);
	vm->Get_Bindings()->Bind_Method<CVMString>				("SpriteComponent", "Get_Frame",				&Get_Frame);

	vm->Get_Bindings()->Bind_Method<void,CVMString>			("SpriteComponent", "Set_Animation",			&Set_Animation);
	vm->Get_Bindings()->Bind_Method<CVMString>				("SpriteComponent", "Get_Animation",			&Get_Animation);
	vm->Get_Bindings()->Bind_Method<void,float>				("SpriteComponent", "Set_Animation_Speed",		&Set_Animation_Speed);
	vm->Get_Bindings()->Bind_Method<float>					("SpriteComponent", "Get_Animation_Speed",		&Get_Animation_Speed);

	vm->Get_Bindings()->Bind_Method<void,int>				("SpriteComponent", "Set_Flip_Horizontal",		&Set_Flip_Horizontal);
	vm->Get_Bindings()->Bind_Method<int>					("SpriteComponent", "Get_Flip_Horizontal",		&Get_Flip_Horizontal);
	vm->Get_Bindings()->Bind_Method<void,int>				("SpriteComponent", "Set_Flip_Vertical",		&Set_Flip_Vertical);
	vm->Get_Bindings()->Bind_Method<int>					("SpriteComponent", "Get_Flip_Vertical",		&Get_Flip_Vertical);

	vm->Get_Bindings()->Bind_Method<void,int>				("SpriteComponent", "Set_Sort_By_Transparency",		&Set_Sort_By_Transparency);
	vm->Get_Bindings()->Bind_Method<int>					("SpriteComponent", "Get_Sort_By_Transparency",		&Get_Sort_By_Transparency);
	vm->Get_Bindings()->Bind_Method<void,int>				("SpriteComponent", "Set_Allow_Static",				&Set_Allow_Static);
	vm->Get_Bindings()->Bind_Method<int>					("SpriteComponent", "Get_Allow_Static",				&Get_Allow_Static);

	vm->Get_Bindings()->Bind_Method<void,float>				("SpriteComponent", "Set_Depth_Bias",			&Set_Depth_Bias);
	vm->Get_Bindings()->Bind_Method<float>					("SpriteComponent", "Get_Depth_Bias",			&Get_Depth_Bias);

	vm->Get_Bindings()->Bind_Method<void,CVMObjectHandle>	("SpriteComponent", "Set_Object_Mask",			&Set_Object_Mask);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>		("SpriteComponent", "Get_Object_Mask",			&Get_Object_Mask);
	vm->Get_Bindings()->Bind_Method<void,CVMObjectHandle>	("SpriteComponent", "Set_Color",				&Set_Color);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>		("SpriteComponent", "Get_Color",				&Get_Color);
	vm->Get_Bindings()->Bind_Method<void,CVMObjectHandle>	("SpriteComponent", "Set_Color_Scalar",			&Set_Color_Scalar);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>		("SpriteComponent", "Get_Color_Scalar",			&Get_Color_Scalar);

	vm->Get_Bindings()->Bind_Method<void,CVMObjectHandle>	("SpriteComponent", "Set_Area",					&Set_Area);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>		("SpriteComponent", "Get_Area",					&Get_Area);

	vm->Get_Bindings()->Bind_Method<void,CVMObjectHandle>	("SpriteComponent", "Set_Offset",				&Set_Offset);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>		("SpriteComponent", "Get_Offset",				&Get_Offset);
	vm->Get_Bindings()->Bind_Method<void,CVMObjectHandle>	("SpriteComponent", "Set_Draw_Scale",			&Set_Draw_Scale);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>		("SpriteComponent", "Get_Draw_Scale",			&Get_Draw_Scale);

	vm->Get_Bindings()->Bind_Method<int>					("SpriteComponent", "Is_Oneshot_Animation_Finished",				&Is_Oneshot_Animation_Finished);

	vm->Get_Bindings()->Bind_Method<void,CVMString,float>	("SpriteComponent", "Play_Oneshot_Animation",	&Play_Oneshot_Animation);
}

