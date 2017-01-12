// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RENDERER_RENDERQUERY_
#define _ENGINE_RENDERER_RENDERQUERY_

class RenderQuery
{
	MEMORY_ALLOCATOR(RenderQuery, "Rendering");

public:

	// Destructor!
	virtual ~RenderQuery() { }

	// Variable binding.
	virtual void Begin() = 0;
	virtual void Finish() = 0;
	virtual float Get_Result() = 0;

};

#endif

