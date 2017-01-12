// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------

public enum StatisticDataType
{
	Int,
	Float
}

[
	Name("Statistic"), 
	Description("Base class for all statistics. Statistics can be global or local to the user. They typically include things like 'coins picked up' or 'players online today'.") 
]
public native("Statistic") class Statistic
{
	// Gets the identifier of the achievement registered with the 
	// online platform.
	property string ID
	{
		native("Get_ID") string Get();
		native("Set_ID") void Set(string value);		
	}
	
	// Gets name and description - should match platform name/description.
	property string Name
	{
		native("Get_Name") string Get();
		native("Set_Name") void Set(string value);	
	}
	property string Description
	{
		native("Get_Description") string Get();
		native("Set_Description") void Set(string value);		
	}
	
	// Category of statistic when shown in UI.	
	property string Category
	{
		native("Get_Category") string Get();
		native("Set_Category") void Set(string value);		
	}
	
	// Category of statistic when shown in UI.	
	property StatisticDataType Data_Type
	{
		native("Get_Data_Type") StatisticDataType Get();
		native("Set_Data_Type") void Set(StatisticDataType value);		
	}

	// Gets or sets if this statistic is global aggregated.
	property bool Aggregated
	{
		native("Get_Aggregated") bool Get();
		native("Set_Aggregated") void Set(bool value);			
	}
	
	// Gets or sets the aggregated time window. In days.
	property int Aggregation_Period
	{
		native("Get_Aggregation_Period") int Get();
		native("Set_Aggregation_Period") void Set(int value);			
	}
	
	// Gets or sets if this statistic is displayed to the user in UI.
	property bool UI_Display
	{
		native("Get_Display") bool Get();
		native("Set_Display") void Set(bool value);			
	}
	
	// If set then this statistic will get its value from another
	// statistic. Useful if you want to have aggregated stats mirror local stats.
	property Type Mirror_Statistic
	{
		native("Get_Mirror_Statistic") Type Get();
		native("Set_Mirror_Statistic") void Set(Type value);
	}

	// Gets or sets the value of this statistic.
	property serialized(1) float Value
	{
		native("Get_Value") float Get();
		native("Set_Value") void Set(float value);			
	}
	
	// Gets the global value of this statistic.
	property serialized(1) float Aggregated_Value
	{
		native("Get_Aggregated_Value") float Get();
		native("Set_Aggregated_Value") void Set(float value);
	}	
} 

