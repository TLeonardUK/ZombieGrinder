// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_RUNTIME_NETWORK_
#define _GAME_RUNTIME_NETWORK_

#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMValue.h"

class CRuntime_Network
{
public:
	static bool Is_Server(CVirtualMachine* vm);
	static bool Is_Client(CVirtualMachine* vm);
	static bool Is_Local_Server(CVirtualMachine* vm);
	static bool Is_Dedicated_Server(CVirtualMachine* vm);
	static int Active_Team_Count(CVirtualMachine* vm);
	static int Get_User_Count(CVirtualMachine* vm);
	static CVMObjectHandle Get_Users(CVirtualMachine* vm);
	static CVMObjectHandle Get_User(CVirtualMachine* vm, int net_id);
	static CVMObjectHandle Get_Local_Users(CVirtualMachine* vm);
	static CVMObjectHandle Get_Primary_Local_User(CVirtualMachine* vm);
	static CVMObjectHandle Get_Local_User_By_Index(CVirtualMachine* vm, int index);
	static void Accept_Pending_Profile_Changes(CVirtualMachine* vm);
	static bool Is_Visible_To_Users(CVirtualMachine* vm, CVMObjectHandle bbox);

	static void Reset_Local_Idle_Timer(CVirtualMachine* vm);

	static void Send_Chat(CVirtualMachine* vm,  int msg_type, CVMString message, int to_net_id, int from_id);

	static void Bind(CVirtualMachine* machine);

};

#endif