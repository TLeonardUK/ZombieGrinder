// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_SCENE_POST_PROCESS_MANAGER_
#define _ENGINE_SCENE_POST_PROCESS_MANAGER_

#include "Engine/Renderer/RenderPipeline.h"

class PostProcessManager : public Singleton<PostProcessManager>
{
	MEMORY_ALLOCATOR(PostProcessManager, "Game");

public:
	virtual int Get_Pass_Count(int camera_index) = 0;
	virtual RenderPipeline_Shader* Get_Pass_Shader(int camera_index, int pass) = 0;
	virtual void Set_Pass_Uniforms(int camera_index, int pass) = 0;

};

#endif

