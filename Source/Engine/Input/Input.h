// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_INPUT_
#define _ENGINE_INPUT_

#include "Generic/Patterns/Singleton.h"
#include "Generic/Types/Vector2.h"
#include "Engine/Engine/FrameTime.h"

#include "Engine/Input/MouseState.h"
#include "Engine/Input/KeyboardState.h"
#include "Engine/Input/JoystickState.h"

#include "Engine/Input/OutputBindings.h"
#include "Engine/Input/InputBindings.h"

#include "Engine/Localise/Locale.h"

#define MAX_INPUT_SOURCES 32

#define STEAM_CONTROLLER_SUPPORT 1

struct InputBind
{
	MEMORY_ALLOCATOR(InputBind, "Input");

public:
	InputBindings::Type  Input;
	OutputBindings::Type Output;

	InputBind(InputBindings::Type in, OutputBindings::Type out)
	{
		Input  = in;
		Output = out;
	}
};

struct JoystickMappingInputType
{
	enum Type
	{
		Axis,
		Button,
		Hat
	};
};

struct JoystickMappingProperties
{
	MEMORY_ALLOCATOR(JoystickMappingProperties, "Input");

public:
	bool Is_Mapped;

	JoystickMappingInputType::Type Destination_Type;
	int Destination_Index;

};

struct JoystickPhysicalMappingProperties
{
	MEMORY_ALLOCATOR(JoystickPhysicalMappingProperties, "Input");

public:
	bool Is_Enabled;

};
struct JoystickMapping
{
	MEMORY_ALLOCATOR(JoystickMapping, "Input");

public:
	enum
	{
		MAX_AXIS		= 10,
		MAX_BUTTONS		= 32,
		MAX_HATS		= 4 * 4,	// We treat hats as basically 4 buttons for each cardinal direction, so we need max_hats * 4 here.
	};

	std::string Name;
	std::string GUID;

	bool Is_Default;

	JoystickMappingProperties Axis_Mappings[MAX_AXIS];
	JoystickMappingProperties Button_Mappings[MAX_BUTTONS];
	JoystickMappingProperties Hat_Mappings[MAX_HATS];

	JoystickPhysicalMappingProperties Axis_Properties[MAX_AXIS];
	JoystickPhysicalMappingProperties Button_Properties[MAX_BUTTONS];
	JoystickPhysicalMappingProperties Hat_Properties[MAX_HATS];

	JoystickIconSet::Type Icon_Set;
	
};

class Input 
{
	MEMORY_ALLOCATOR(Input, "Input");

private:
	bool m_binding_state			[OutputBindings::COUNT];
	bool m_previous_binding_state	[OutputBindings::COUNT];

	int m_index;

	std::vector<InputBind> m_bindings;

	static Input* g_input_sources[MAX_INPUT_SOURCES];
	static int	  m_input_source_count;

	static Input* g_steam_input;

	static JoystickState* g_joystick_slots[MAX_INPUT_SOURCES];
	static JoystickState* g_dummy_joystick_state;

	static std::vector<JoystickMapping> g_input_mappings;

	static bool g_mark_for_device_scan;

public:	
	Input();
	virtual ~Input() {}

	// Instantiation/retrival.
	static Input*			Get							(int source_index = 0);
	static Input*			Create						(int source_index);
	static int				Get_Source_Count			();
	static void				Destroy						();
	static void				Load_Joystick_Mapping		(const char* url);
	static JoystickMapping*	Get_Joystick_Mapping		(std::string guid);
	static void				Global_Tick					(const FrameTime& time);

	// Base functions.	
	virtual void Tick(const FrameTime& time) = 0;
	
	// Mouse state.
	virtual MouseState* Get_Mouse_State() = 0;

	// Key state.
	virtual KeyboardState* Get_Keyboard_State() = 0;
	
	// Joystick state.
	static void Global_Scan_For_Devices();

	JoystickState* Get_Joystick_State();
	virtual void Scan_For_Devices() {};
	virtual std::vector<JoystickState*> Get_Known_Joysticks() = 0;

	// Binded input.
	void Update_Bindings();
	void Unbind_All		();
	void Bind			(InputBindings::Type input, OutputBindings::Type output);

	bool Is_Down		(OutputBindings::Type type);
	bool Was_Down		(OutputBindings::Type type);
	bool Was_Pressed	(OutputBindings::Type type);

	static bool Is_Down_By_Any(OutputBindings::Type type);
	static bool Was_Pressed_By_Any(OutputBindings::Type type);

	bool Is_Down		(InputBindings::Type type);
	bool Was_Down		(InputBindings::Type type);
	bool Was_Pressed	(InputBindings::Type type);

	bool Is_Action_Down(OutputBindings::Type type);
	bool Was_Action_Down(OutputBindings::Type type);
	bool Was_Action_Pressed(OutputBindings::Type type);

	// Converts an output binding back into its source input binding.
	// If multiple keys are bound to the output then the input source (joystick/keyboard/etc)
	// that had the latest input is considered the "active" one and is returned.
	static InputBindings::Type Output_To_Active_Input(OutputBindings::Type type, InputBindingType::Type key_type, JoystickState** state = NULL);

	// Gets the currently active joystick icon set.
	static JoystickIconSet::Type Get_Active_Joystick_Icon_Set();

	// Returns how long since the last input was recieved from the input source (joystick/keyboard/etc)
	// pointed to by the input binding.
	float Last_Source_Input(InputBindings::Type type);

	// Character based input.
	std::string Read_Character();
	void Flush_Character_Stack();

};

#endif

