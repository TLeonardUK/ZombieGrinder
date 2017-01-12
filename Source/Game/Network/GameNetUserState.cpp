// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Network/GameNetUserState.h"
#include "Game/Network/GameNetUser.h"
#include "Game/Network/Packets/GamePackets.h"

#include "Engine/Engine/GameEngine.h"

#include "Game/Scripts/GameVirtualMachine.h"

GameNetUserState::GameNetUserState(GameNetUser* user)
	: Ping(0)
	, Team_Index(0)
	, Coins(0)
	, m_user(user)
{
}

void GameNetUserState::Serialize(UserPacketState* state)
{
	Profile* profile = m_user->Get_Profile();

	state->Ping = (unsigned char)Ping;
	state->Team_Index = Team_Index;
	state->Level = profile != NULL ? profile->Level : 0;
	state->Coins = Coins;

	CVirtualMachine* vm = GameEngine::Get()->Get_VM();
	CVMContextLock lock = vm->Set_Context(vm->Get_Static_Context());

	BinaryStream stream;
	m_user->Get_Script_Object().Get()->Serialize(&stream, CVMObjectSerializeFlags::Full, &state->State_Data_Version);
	state->State_Data.Set(stream.Data(), stream.Length());
}

void GameNetUserState::Deserialize(UserPacketState* state)
{
	Profile* profile = m_user->Get_Profile();

	Ping = state->Ping;
	Team_Index = state->Team_Index;
	Coins = state->Coins;

	if (profile != NULL)
	{
		profile->Level = state->Level;
	}

	CVirtualMachine* vm = GameEngine::Get()->Get_VM();
	CVMContextLock lock = vm->Set_Context(vm->Get_Static_Context());

	BinaryStream stream(state->State_Data.Buffer(), state->State_Data.Size());
	m_user->Get_Script_Object().Get()->Deserialize(&stream, CVMObjectSerializeFlags::Full, state->State_Data_Version);
}
