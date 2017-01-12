// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RENDERER_TEXTURES_RENDERTARGET_
#define _ENGINE_RENDERER_TEXTURES_RENDERTARGET_

class Texture;

struct RenderTargetBufferType
{
	enum Type
	{
		Color,
		Depth,
		Stencil
	};
};

struct OutputBufferType
{
	enum Type
	{
		BackBuffer				= 0,
		RenderTargetTexture0	= 1,
		RenderTargetTexture1	= 2,
		RenderTargetTexture2	= 3,
		RenderTargetTexture3	= 4,
		RenderTargetTexture4	= 5,
		RenderTargetTexture5	= 6,	
		RenderTargetTexture6	= 7,
		RenderTargetTexture7	= 8,
		RenderTargetTexture8	= 9,
		RenderTargetTexture9	= 10,
	};
};

class RenderTarget
{
	MEMORY_ALLOCATOR(RenderTarget, "Rendering");

private:
	
protected:

	// Constructor!
	RenderTarget();

public:

	// Destructor!
	virtual ~RenderTarget();
	
	// Binding support.
	virtual void Bind_Texture(RenderTargetBufferType::Type type, const Texture* texture) = 0;
	virtual void Validate() = 0;

	// Create shader programs.
	static RenderTarget* Create();

};

#endif

