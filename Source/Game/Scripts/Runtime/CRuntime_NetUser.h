// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_RUNTIME_NETUSER_
#define _GAME_RUNTIME_NETUSER_

#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMValue.h"

class CRuntime_NetUser
{
public:
	static int Get_Team_Index(CVirtualMachine* vm, CVMValue self);
	static void Set_Team_Index(CVirtualMachine* vm, CVMValue self, int index);
	static int Get_Coins(CVirtualMachine* vm, CVMValue self);
	static void Set_Coins(CVirtualMachine* vm, CVMValue self, int index);
	static int Get_In_Game(CVirtualMachine* vm, CVMValue self);
	static int Get_Is_Local(CVirtualMachine* vm, CVMValue self);
	static int Get_Is_Premium(CVirtualMachine* vm, CVMValue self);
	static int Get_Local_Index(CVirtualMachine* vm, CVMValue self);
	static CVMString Get_Username(CVirtualMachine* vm, CVMValue self);
	static CVMString Get_Display_Username(CVirtualMachine* vm, CVMValue self);
	static CVMString Get_First_Party_ID(CVirtualMachine* vm, CVMValue self);
	static int Get_Net_ID(CVirtualMachine* vm, CVMValue self);
	static CVMObjectHandle Get_Controller(CVirtualMachine* vm, CVMValue self);
	static void Set_Controller(CVirtualMachine* vm, CVMValue self, CVMObjectHandle actor);
	static CVMObjectHandle Get_Input(CVirtualMachine* vm, CVMValue self);
	static CVMObjectHandle Get_Profile(CVirtualMachine* vm, CVMValue self);
	static CVMObjectHandle Get_Local_Profile(CVirtualMachine* vm, CVMValue self);
	static int Get_Profile_Change_Counter(CVirtualMachine* vm, CVMValue self);	
	static void Update_Heading(CVirtualMachine* vm, CVMValue self, CVMObjectHandle actor);	

	static void Bind(CVirtualMachine* machine);

};

#endif