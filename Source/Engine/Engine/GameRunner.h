// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_GAMERUNNER_
#define _ENGINE_GAMERUNNER_

#include "Engine/Engine/FrameTime.h"
#include "Engine/Demo/DemoManager.h"
#include <string>
#include <vector>

class GameEngine;
class CommandLineParser;
class NetManager;
class Scene;
class EngineVirtualMachine;
class Map;
struct ParticleInstance;
class UIManager;
class PackageFile;
class BinaryStream;

class GameRunner
{
	MEMORY_ALLOCATOR(GameRunner, "Engine");

private:

protected:

	friend class GameEngine;
	friend class UIManager;

	// Base functions.
	virtual void Preload() = 0;
	virtual void Load_User_Config() = 0;
	virtual void Start() = 0;
	virtual void End() = 0;
	virtual void Tick(const FrameTime& time) = 0;
	virtual void Draw_Overlays(const FrameTime& time) = 0;
	virtual void Draw_Onscreen_Logs(const FrameTime& time) = 0;
	virtual void Draw_Watermark(const FrameTime& time) = 0;

	// Creation functions.
	virtual EngineVirtualMachine* Create_VM() = 0;

	// Config functions.
	virtual NetManager* Get_Net_Manager() = 0;
	virtual Scene* Get_Scene() = 0;
	virtual Map* Get_Map() = 0;
	
public:

	virtual std::vector<PackageFile*> Get_Restricted_Packages() = 0;
	virtual std::vector<PackageFile*> Get_Server_Enforced_Packages() = 0;
	virtual bool In_Restricted_Mode() = 0;
	virtual void Set_Map_Restricted_Mode(bool bRestricted) = 0;

	virtual void Serialize_Demo(BinaryStream* stream, DemoVersion::Type version, bool bSaving, float frameDelta) = 0;

	// Nasty, this should go elsewhere.
	virtual void Particle_Script_Event(ParticleInstance* instance, std::string event_name) = 0;
	virtual void Particle_Global_Script_Event(ParticleInstance* instance, std::string event_name) = 0;
	
};

#endif

