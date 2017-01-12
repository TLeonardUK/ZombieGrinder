// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_EDITOR_ACTIONS_EDITOR_ACTION_
#define _GAME_UI_SCENES_EDITOR_ACTIONS_EDITOR_ACTION_

// Editor actions are anything that modify the state of
// the map being edited - placing items, changing settings, etc.
// All actions are derived from this class and must support
// undoing and redoing their operation.

class EditorAction
{
	MEMORY_ALLOCATOR(EditorAction, "UI");

private:

protected:

public:
	virtual const char* Get_Name()	= 0;
	virtual void		Undo()		= 0;
	virtual void		Do()		= 0;

};

#endif

