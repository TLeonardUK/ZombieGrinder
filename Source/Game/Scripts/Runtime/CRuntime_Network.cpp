// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "XScript/VirtualMachine/CVMBindingHelper.h"
#include "Game/Scripts/Runtime/CRuntime_Network.h"
#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMContext.h"
#include "XScript/VirtualMachine/CVMBinary.h"

#include "Game/Network/GameNetUser.h"
#include "Game/Network/GameNetManager.h"
#include "Game/Network/GameNetServer.h"
#include "Game/Network/GameNetClient.h"

#include "Game/Game/ChatManager.h"
#include "Game/Runner/Game.h"

bool CRuntime_Network::Is_Server(CVirtualMachine* vm)
{
	GameNetManager* manager = GameNetManager::Get();
	GameNetClient* client = manager->Game_Client();
	GameNetServer* server = manager->Game_Server();
	return (server != NULL);
}

bool CRuntime_Network::Is_Client(CVirtualMachine* vm)
{
	GameNetManager* manager = GameNetManager::Get();
	GameNetClient* client = manager->Game_Client();
	GameNetServer* server = manager->Game_Server();
	return (client != NULL);
}

bool CRuntime_Network::Is_Local_Server(CVirtualMachine* vm)
{
	GameNetManager* manager = GameNetManager::Get();
	GameNetClient* client = manager->Game_Client();
	GameNetServer* server = manager->Game_Server();
	return (client != NULL && server != NULL);
}

bool CRuntime_Network::Is_Dedicated_Server(CVirtualMachine* vm)
{
	GameNetManager* manager = GameNetManager::Get();
	GameNetClient* client = manager->Game_Client();
	GameNetServer* server = manager->Game_Server();
	return (client == NULL && server != NULL);
}

CVMObjectHandle CRuntime_Network::Get_User(CVirtualMachine* vm, int net_id)
{
	GameNetManager* manager = GameNetManager::Get();
	std::vector<GameNetUser*> users = manager->Get_Game_Net_Users();

	for (std::vector<GameNetUser*>::iterator iter = users.begin(); iter != users.end(); iter++)
	{
		GameNetUser* user = *iter;
		if (user->Get_Net_ID() == net_id)
		{
			return user->Get_Script_Object();
		}
	}

	return NULL;
}

int CRuntime_Network::Get_User_Count(CVirtualMachine* vm)
{
	return (int)NetManager::Get()->Get_Net_Users().size();
}

int CRuntime_Network::Active_Team_Count(CVirtualMachine* vm)
{
	std::vector<int> active_teams;

	GameNetManager* manager = GameNetManager::Get();
	std::vector<GameNetUser*> users = manager->Get_Game_Net_Users();

	for (std::vector<GameNetUser*>::iterator iter = users.begin(); iter != users.end(); iter++)
	{
		GameNetUser* user = *iter;
		if (std::find(active_teams.begin(), active_teams.end(), user->Get_State()->Team_Index) == active_teams.end())
		{
			active_teams.push_back(user->Get_State()->Team_Index);
		}
	}

	return active_teams.size();
}

CVMObjectHandle CRuntime_Network::Get_Users(CVirtualMachine* vm)
{
	GameNetManager* manager = GameNetManager::Get();
	std::vector<GameNetUser*> users = manager->Get_Game_Net_Users();

	CVMLinkedSymbol* array_type = vm->Get_Symbol_Table_Entry(vm->Get_Active_Function()->symbol->method_data->return_data_type_class_index);
	CVMObjectHandle arr = vm->New_Array(array_type, users.size());

	int index = 0;
	for (std::vector<GameNetUser*>::iterator iter = users.begin(); iter != users.end(); iter++, index++)
	{
		GameNetUser* user = *iter;

		//CVMObjectHandle element = vm->New_Object(element_type, false, user);
		//arr.Get()->Get_Slot(index).object_value = element;
		
		arr.Get()->Get_Slot(index).object_value = user->Get_Script_Object();
	}

	return arr;
}

CVMObjectHandle CRuntime_Network::Get_Local_Users(CVirtualMachine* vm)
{
	GameNetManager* manager = GameNetManager::Get();
	std::vector<NetUser*> users = manager->Get_Local_Net_Users();

	CVMLinkedSymbol* array_type = vm->Get_Symbol_Table_Entry(vm->Get_Active_Function()->symbol->method_data->return_data_type_class_index);
	CVMObjectHandle arr = vm->New_Array(array_type, users.size());

	int index = 0;
	for (std::vector<NetUser*>::iterator iter = users.begin(); iter != users.end(); iter++, index++)
	{
		GameNetUser* user = static_cast<GameNetUser*>(*iter);

		//CVMObjectHandle element = vm->New_Object(element_type, false, user);
		//arr.Get()->Get_Slot(index).object_value = element;

		arr.Get()->Get_Slot(index).object_value = user->Get_Script_Object();
	}

	return arr;
}

CVMObjectHandle CRuntime_Network::Get_Primary_Local_User(CVirtualMachine* vm)
{
	GameNetManager* manager = GameNetManager::Get();
	GameNetUser* user = static_cast<GameNetUser*>(manager->Get_Primary_Local_Net_User());
	return user == NULL ? NULL : user->Get_Script_Object();
}

CVMObjectHandle CRuntime_Network::Get_Local_User_By_Index(CVirtualMachine* vm, int index)
{
	GameNetManager* manager = GameNetManager::Get();
	std::vector<NetUser*> users = manager->Get_Local_Net_Users();

	for (std::vector<NetUser*>::iterator iter = users.begin(); iter != users.end(); iter++)
	{
		GameNetUser* user = static_cast<GameNetUser*>(*iter);
		if (user->Get_Local_User_Index() == index)
		{
			return user->Get_Script_Object();
		}
	}

	return NULL;
}

void CRuntime_Network::Accept_Pending_Profile_Changes(CVirtualMachine* vm)
{
	GameNetServer* server = GameNetManager::Get()->Game_Server();
	vm->Assert(server != NULL);
	server->Accept_Pending_Profile_Changes();
}

void CRuntime_Network::Reset_Local_Idle_Timer(CVirtualMachine* vm)
{
	Game::Get()->Reset_Local_Idle_Timer();
}

bool CRuntime_Network::Is_Visible_To_Users(CVirtualMachine* vm, CVMObjectHandle bbox)
{
	GameNetManager* manager = GameNetManager::Get();
	std::vector<GameNetUser*> users = manager->Get_Game_Net_Users();

	bool any_in_bbox = false;
	bool is_server = manager->Server() != NULL;

	Rect2D rect_bbox = Rect2D(
		bbox.Get()->Get_Slot(0).float_value,
		bbox.Get()->Get_Slot(1).float_value,
		bbox.Get()->Get_Slot(2).float_value,
		bbox.Get()->Get_Slot(3).float_value
	);

	for (std::vector<GameNetUser*>::iterator iter = users.begin(); iter != users.end(); iter++)
	{
		GameNetUser* user = *iter;
		Rect2D view = user->Get_Estimated_Viewport();

		// If we are local we can only calculate viewports for local users.
//		if (!is_server && !user->Get_Online_User()->Is_Local())
//		{
//			continue;
//		}

		if (view.Intersects(rect_bbox))
		{
			any_in_bbox = true;
			break;
		}
	}

	return any_in_bbox;
}

void CRuntime_Network::Send_Chat(CVirtualMachine* vm,  int msg_type, CVMString message, int to_net_id, int from_id)
{
	if (GameNetManager::Get()->Game_Server() != NULL)
	{
		ChatManager::Get()->Send_Server((ChatMessageType::Type)msg_type, message.C_Str(), to_net_id, from_id);
	}
	else
	{
		ChatManager::Get()->Send_Client((ChatMessageType::Type)msg_type, message.C_Str(), to_net_id);
	}
}

void CRuntime_Network::Bind(CVirtualMachine* vm)
{
	vm->Get_Bindings()->Bind_Function<bool>("Network", "Is_Server", &Is_Server);
	vm->Get_Bindings()->Bind_Function<bool>("Network", "Is_Client", &Is_Client);
	vm->Get_Bindings()->Bind_Function<bool>("Network", "Is_Local_Server", &Is_Local_Server);
	vm->Get_Bindings()->Bind_Function<bool>("Network", "Is_Dedicated_Server", &Is_Dedicated_Server);
	vm->Get_Bindings()->Bind_Function<CVMObjectHandle,int>("Network", "Get_User", &Get_User);
	vm->Get_Bindings()->Bind_Function<int>("Network", "Get_User_Count", &Get_User_Count);
	vm->Get_Bindings()->Bind_Function<int>("Network", "Active_Team_Count", &Active_Team_Count);
	vm->Get_Bindings()->Bind_Function<CVMObjectHandle>("Network", "Get_Users", &Get_Users);
	vm->Get_Bindings()->Bind_Function<CVMObjectHandle>("Network", "Get_Local_Users", &Get_Local_Users);
	vm->Get_Bindings()->Bind_Function<CVMObjectHandle,int>("Network", "Get_Local_User_By_Index", &Get_Local_User_By_Index);
	vm->Get_Bindings()->Bind_Function<CVMObjectHandle>("Network", "Get_Primary_Local_User", &Get_Primary_Local_User);
	vm->Get_Bindings()->Bind_Function<bool,CVMObjectHandle>("Network", "Is_Visible_To_Users", &Is_Visible_To_Users);
	vm->Get_Bindings()->Bind_Function<void>("Network", "Accept_Pending_Profile_Changes", &Accept_Pending_Profile_Changes);
	vm->Get_Bindings()->Bind_Function<void>("Network", "Reset_Local_Idle_Timer", &Reset_Local_Idle_Timer);
	vm->Get_Bindings()->Bind_Function<void,int,CVMString,int,int>("Network", "Send_Chat", &Send_Chat);
}

