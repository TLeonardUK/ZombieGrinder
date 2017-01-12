// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------

[
	Placeable(false), 
	Name("Team"), 
	Description("Defines a single team in the game. -_-") 
]
public class Team
{
	public string 	Name;
	public Vec4		Primary_Color;

	public Team(string name, Vec4 color)
	{
		Name = name;
		Primary_Color = color;
	}
}
