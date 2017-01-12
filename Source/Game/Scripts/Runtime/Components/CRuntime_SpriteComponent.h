// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_RUNTIME_SPRITECOMPONENT_
#define _GAME_RUNTIME_SPRITECOMPONENT_

#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMValue.h"

class CRuntime_SpriteComponent
{
public:
	static CVMObjectHandle Create(CVirtualMachine* vm, CVMValue self);

	static void Set_Visible(CVirtualMachine* vm, CVMValue self, int visible);
	static int Get_Visible(CVirtualMachine* vm, CVMValue self);
	static void Set_Editor_Visible(CVirtualMachine* vm, CVMValue self, int visible);
	static int Get_Editor_Visible(CVirtualMachine* vm, CVMValue self);
	static void Set_Scale(CVirtualMachine* vm, CVMValue self, int visible);
	static int Get_Scale(CVirtualMachine* vm, CVMValue self);

	static void Set_Frame(CVirtualMachine* vm, CVMValue self, CVMString frame);
	static CVMString Get_Frame(CVirtualMachine* vm, CVMValue self);

	static void Set_Animation(CVirtualMachine* vm, CVMValue self, CVMString frame);
	static CVMString Get_Animation(CVirtualMachine* vm, CVMValue self);
	static void Set_Animation_Speed(CVirtualMachine* vm, CVMValue self, float frame);
	static float Get_Animation_Speed(CVirtualMachine* vm, CVMValue self);

	static void Set_Flip_Horizontal(CVirtualMachine* vm, CVMValue self, int flip);
	static int Get_Flip_Horizontal(CVirtualMachine* vm, CVMValue self);
	static void Set_Flip_Vertical(CVirtualMachine* vm, CVMValue self, int flip);
	static int Get_Flip_Vertical(CVirtualMachine* vm, CVMValue self);

	static void Set_Sort_By_Transparency(CVirtualMachine* vm, CVMValue self, int flip);
	static int Get_Sort_By_Transparency(CVirtualMachine* vm, CVMValue self);

	static void Set_Allow_Static(CVirtualMachine* vm, CVMValue self, int flip);
	static int Get_Allow_Static(CVirtualMachine* vm, CVMValue self);

	static void Set_Area(CVirtualMachine* vm, CVMValue self, CVMObjectHandle color);
	static CVMObjectHandle Get_Area(CVirtualMachine* vm, CVMValue self);
	static void Set_Offset(CVirtualMachine* vm, CVMValue self, CVMObjectHandle color);
	static CVMObjectHandle Get_Offset(CVirtualMachine* vm, CVMValue self);
	static void Set_Draw_Scale(CVirtualMachine* vm, CVMValue self, CVMObjectHandle color);
	static CVMObjectHandle Get_Draw_Scale(CVirtualMachine* vm, CVMValue self);

	static void Set_Object_Mask(CVirtualMachine* vm, CVMValue self, CVMObjectHandle color);
	static CVMObjectHandle Get_Object_Mask(CVirtualMachine* vm, CVMValue self);
	static void Set_Color(CVirtualMachine* vm, CVMValue self, CVMObjectHandle color);
	static CVMObjectHandle Get_Color(CVirtualMachine* vm, CVMValue self);
	static void Set_Color_Scalar(CVirtualMachine* vm, CVMValue self, CVMObjectHandle color);
	static CVMObjectHandle Get_Color_Scalar(CVirtualMachine* vm, CVMValue self);


	static void Set_Depth_Bias(CVirtualMachine* vm, CVMValue self, float flip);
	static float Get_Depth_Bias(CVirtualMachine* vm, CVMValue self);

	static int Is_Oneshot_Animation_Finished(CVirtualMachine* vm, CVMValue self);

	static void Play_Oneshot_Animation(CVirtualMachine* vm, CVMValue self, CVMString name, float speed);

	static void Bind(CVirtualMachine* machine);

};

#endif