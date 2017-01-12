// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_UI_ELEMENTS_UICURVEEDITOR_
#define _ENGINE_UI_ELEMENTS_UICURVEEDITOR_

#include "Engine/UI/UIElement.h"
#include "Engine/UI/UIFrame.h"
#include "Engine/Renderer/Text/Font.h"
#include "Engine/Renderer/Text/FontRenderer.h"

#include "Engine/Renderer/Atlases/Atlas.h"
#include "Engine/Renderer/Atlases/AtlasRenderer.h"

#include "Engine/Renderer/PrimitiveRenderer.h"

class AtlasHandle;
class UIManager;
struct AtlasFrame;

struct UICurvePoint
{
	float	x;
	float	y;
	bool	persistent;
};

class UICurveEditor : public UIElement 
{
	MEMORY_ALLOCATOR(UICurveEditor, "UI");

protected:	
	friend class UILayout;

	std::vector<UICurvePoint> m_points;
	UICurvePoint* m_moving_point;

public:
	UICurveEditor();
	~UICurveEditor();
	
	void Clear_Points();
	std::vector<UICurvePoint> Get_Points() { return m_points; }
	void Add_Point(float x, float y, bool persistent);

	void Refresh();
	void After_Refresh();

	void Tick(const FrameTime& time, UIManager* manager, UIScene* scene);
	void Draw(const FrameTime& time, UIManager* manager, UIScene* scene);

};

#endif
