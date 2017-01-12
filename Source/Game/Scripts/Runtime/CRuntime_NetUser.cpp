// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Scripts/Runtime/CRuntime_NetUser.h"
#include "XScript/VirtualMachine/CVMBindingHelper.h"
#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMContext.h"
#include "XScript/VirtualMachine/CVMBinary.h"

#include "Game/Scene/Actors/ScriptedActor.h"

#include "Game/Network/GameNetUser.h"
#include "Game/Network/GameNetManager.h"
#include "Game/Network/GameNetServer.h"
#include "Game/Network/GameNetClient.h"

int CRuntime_NetUser::Get_Team_Index(CVirtualMachine* vm, CVMValue self)
{
	GameNetUser* user = reinterpret_cast<GameNetUser*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return user->Get_State()->Team_Index;
}

void CRuntime_NetUser::Set_Team_Index(CVirtualMachine* vm, CVMValue self, int index)
{
	GameNetUser* user = reinterpret_cast<GameNetUser*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	user->Get_State()->Team_Index = index;
}

int CRuntime_NetUser::Get_Coins(CVirtualMachine* vm, CVMValue self)
{
	GameNetUser* user = reinterpret_cast<GameNetUser*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return user->Get_State()->Coins;
}

void CRuntime_NetUser::Set_Coins(CVirtualMachine* vm, CVMValue self, int index)
{
	GameNetUser* user = reinterpret_cast<GameNetUser*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	user->Get_State()->Coins = index;
}

int CRuntime_NetUser::Get_In_Game(CVirtualMachine* vm, CVMValue self)
{
	GameNetUser* user = reinterpret_cast<GameNetUser*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return user->Get_Game_State() == GameNetClient_GameState::InGame;
}

int CRuntime_NetUser::Get_Is_Local(CVirtualMachine* vm, CVMValue self)
{
	GameNetUser* user = reinterpret_cast<GameNetUser*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return user->Get_Online_User()->Is_Local();
}

int CRuntime_NetUser::Get_Is_Premium(CVirtualMachine* vm, CVMValue self)
{
	GameNetUser* user = reinterpret_cast<GameNetUser*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
#ifdef OPT_PREMIUM_ACCOUNTS
	return user->Get_Online_User()->Get_Premium();
#else
	return 0;
#endif
}

CVMString CRuntime_NetUser::Get_Username(CVirtualMachine* vm, CVMValue self)
{
	GameNetUser* user = reinterpret_cast<GameNetUser*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return user->Get_Username().c_str();
}

CVMString CRuntime_NetUser::Get_Display_Username(CVirtualMachine* vm, CVMValue self)
{
	GameNetUser* user = reinterpret_cast<GameNetUser*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return user->Get_Display_Username().c_str();
}

CVMString CRuntime_NetUser::Get_First_Party_ID(CVirtualMachine* vm, CVMValue self)
{
	GameNetUser* user = reinterpret_cast<GameNetUser*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return user->Get_Online_User()->Get_User_ID();
}

int CRuntime_NetUser::Get_Net_ID(CVirtualMachine* vm, CVMValue self)
{
//	vm->Null_Check<GameNetUser*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	vm->Assert(self.object_value.Get() != NULL);
	GameNetUser* user = reinterpret_cast<GameNetUser*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return user->Get_Net_ID();
}

int CRuntime_NetUser::Get_Local_Index(CVirtualMachine* vm, CVMValue self)
{
	vm->Assert(self.object_value.Get() != NULL);
	GameNetUser* user = reinterpret_cast<GameNetUser*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return user->Get_Local_User_Index();
}

CVMObjectHandle CRuntime_NetUser::Get_Controller(CVirtualMachine* vm, CVMValue self)
{
	GameNetUser* user = reinterpret_cast<GameNetUser*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	ScriptedActor* poss = user->Get_Controller();
	return poss == NULL ? NULL : poss->Get_Script_Object();
}

void CRuntime_NetUser::Set_Controller(CVirtualMachine* vm, CVMValue self, CVMObjectHandle actor)
{
	vm->Assert(GameNetManager::Get()->Game_Server() != NULL);

	GameNetUser* user = reinterpret_cast<GameNetUser*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));	
	ScriptedActor* poss = reinterpret_cast<ScriptedActor*>(actor.Get()->Get_Meta_Data());
	user->Set_Controller_ID(poss == NULL ? -1 : poss->Get_Replication_Info().unique_id);
}

CVMObjectHandle CRuntime_NetUser::Get_Profile(CVirtualMachine* vm, CVMValue self)
{
	GameNetUser* user = reinterpret_cast<GameNetUser*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return user->Get_Profile() == NULL ? NULL : user->Get_Profile()->Script_Object.Get();
}

CVMObjectHandle CRuntime_NetUser::Get_Local_Profile(CVirtualMachine* vm, CVMValue self)
{
	GameNetUser* user = reinterpret_cast<GameNetUser*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return user->Get_Local_Profile() == NULL ? NULL : user->Get_Local_Profile()->Script_Object.Get();
}

int CRuntime_NetUser::Get_Profile_Change_Counter(CVirtualMachine* vm, CVMValue self)
{
	GameNetUser* user = reinterpret_cast<GameNetUser*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return user->Get_Profile_Change_Counter();
}

void CRuntime_NetUser::Update_Heading(CVirtualMachine* vm, CVMValue self, CVMObjectHandle actor)
{
	GameNetUser* user = reinterpret_cast<GameNetUser*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	ScriptedActor* poss = reinterpret_cast<ScriptedActor*>(actor.Get()->Get_Meta_Data());
	user->Update_Heading(poss);
}

void CRuntime_NetUser::Bind(CVirtualMachine* vm)
{
	vm->Get_Bindings()->Bind_Method<int>("NetUser", "Get_Profile_Change_Counter", &Get_Profile_Change_Counter);
	vm->Get_Bindings()->Bind_Method<int>("NetUser", "Get_Team_Index", &Get_Team_Index);
	vm->Get_Bindings()->Bind_Method<void,int>("NetUser", "Set_Team_Index", &Set_Team_Index);
	vm->Get_Bindings()->Bind_Method<int>("NetUser", "Get_Coins", &Get_Coins);
	vm->Get_Bindings()->Bind_Method<void,int>("NetUser", "Set_Coins", &Set_Coins);
	vm->Get_Bindings()->Bind_Method<int>("NetUser", "Get_In_Game", &Get_In_Game);
	vm->Get_Bindings()->Bind_Method<int>("NetUser", "Get_Is_Local", &Get_Is_Local);
	vm->Get_Bindings()->Bind_Method<int>("NetUser", "Get_Is_Premium", &Get_Is_Premium);
	vm->Get_Bindings()->Bind_Method<CVMString>("NetUser", "Get_Username", &Get_Username);
	vm->Get_Bindings()->Bind_Method<CVMString>("NetUser", "Get_Display_Username", &Get_Display_Username);
	vm->Get_Bindings()->Bind_Method<CVMString>("NetUser", "Get_First_Party_ID", &Get_First_Party_ID);
	vm->Get_Bindings()->Bind_Method<int>("NetUser", "Get_Local_Index", &Get_Local_Index);
	vm->Get_Bindings()->Bind_Method<int>("NetUser", "Get_Net_ID", &Get_Net_ID);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>("NetUser", "Get_Controller", &Get_Controller);
	vm->Get_Bindings()->Bind_Method<void,CVMObjectHandle>("NetUser", "Set_Controller", &Set_Controller);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>("NetUser", "Get_Profile", &Get_Profile);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>("NetUser", "Get_Local_Profile", &Get_Local_Profile);
	vm->Get_Bindings()->Bind_Method<void,CVMObjectHandle>("NetUser", "Update_Heading", &Update_Heading);
}

