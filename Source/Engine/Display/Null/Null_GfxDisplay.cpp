// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Display/Null/Null_GfxDisplay.h"
#include "Engine/Engine/GameEngine.h"

#ifdef PLATFORM_WIN32
#include <Windows.h>
//BOOL WINAPI SetConsoleTitleA(_In_ LPCTSTR lpConsoleTitle);
#endif
#include <string>

Null_GfxDisplay::Null_GfxDisplay(const char* title, int width, int height, int hertz, GfxDisplayMode::Type mode)
	: m_width(width)
	, m_height(height)
	, m_mode(mode)
	, m_hertz(hertz)
	, m_active(false)
{	
	memset(m_title, 0, MAX_TITLE_LENGTH);
	strcpy(m_title, title);
}
	
Null_GfxDisplay::~Null_GfxDisplay()
{
}

const char* Null_GfxDisplay::Get_Title()
{
	return m_title;
}

int Null_GfxDisplay::Get_Width()
{
	return m_width;
}

int Null_GfxDisplay::Get_Height()
{
	return m_height;
}

int Null_GfxDisplay::Get_Hertz()
{
	return m_hertz;
}

GfxDisplayMode::Type Null_GfxDisplay::Get_Mode()
{
	return m_mode;
}

bool Null_GfxDisplay::Is_Active()
{
	return m_active;
}

void Null_GfxDisplay::Set_Title(const char* title)
{
	if (strcmp(m_title, title) == 0)
	{
		return;
	}

	strcpy(m_title, title);
#ifdef PLATFORM_WIN32
	SetConsoleTitle(m_title);
#endif
}

void Null_GfxDisplay::Set_Hidden(bool bHidden)
{
}

bool Null_GfxDisplay::Resize(int width, int height, int hertz, GfxDisplayMode::Type mode)
{
	m_width = width;
	m_height = height;
	m_hertz = hertz;
	m_mode = mode;

	return true;
}

void Null_GfxDisplay::Tick(const FrameTime& time)
{
}

void Null_GfxDisplay::Swap_Buffers()
{
}

std::vector<GfxDisplayResolution> Null_GfxDisplay::Get_Available_Resolutions()
{
	std::vector<GfxDisplayResolution> results;
	
	// Dummy resolution.
	GfxDisplayResolution res;
	res.Width	= 80;
	res.Height	= 60;
	res.Depth	= 24;
	res.Hertz	= 60;
	results.push_back(res);

	return results;
}