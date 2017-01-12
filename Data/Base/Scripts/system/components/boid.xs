// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------

[
	Name("BoidComponent"), 
	Description("Provides basic support for boid based flocking behaviour.") 
]
public native("BoidComponent") class BoidComponent
{
	// Creates a new collision component and binds it to the executing actor.
	// Shouldn't be called directly, use a Components {} block.
	public native("Create") BoidComponent();	
	
	// Gets/sets the center of the boid, relative to the parent.
	property serialized(1) Vec2 Center
	{
		public native("Set_Center") void Set(Vec2 val);
		public native("Get_Center") Vec2 Get();
	}
	
	// Gets/sets the distance other boids have to be inside from them
	// to be considered a neighbour.
	property serialized(1) float Neighbour_Distance
	{
		public native("Set_Neighbour_Distance") void Set(float val);
		public native("Get_Neighbour_Distance") float Get();
	}

	// Gets/sets the distance that boids try to keep apart from each other.
	property serialized(1) float Seperation_Distance
	{
		public native("Set_Seperation_Distance") void Set(float val);
		public native("Get_Seperation_Distance") float Get();
	}

	// Gets/sets the distance the maximum speed the boid can impart.
	property serialized(1) float Maximum_Speed
	{
		public native("Set_Maximum_Speed") void Set(float val);
		public native("Get_Maximum_Speed") float Get();
	}

	// Gets/sets the distance the maximum force the boid can impart.
	property serialized(1) float Maximum_Force
	{
		public native("Set_Maximum_Force") void Set(float val);
		public native("Get_Maximum_Force") float Get();
	}

	// Gets/sets the weighting for the cohesion vector.
	property serialized(1) float Cohesion_Weight
	{
		public native("Set_Cohesion_Weight") void Set(float val);
		public native("Get_Cohesion_Weight") float Get();
	}

	// Gets/sets the weighting for the avoidance vector.
	property serialized(1) float Avoidance_Weight
	{
		public native("Set_Avoidance_Weight") void Set(float val);
		public native("Get_Avoidance_Weight") float Get();
	}

	// Gets/sets the weighting for the seperation vector.
	property serialized(1) float Seperation_Weight
	{
		public native("Set_Seperation_Weight") void Set(float val);
		public native("Get_Seperation_Weight") float Get();
	}

	// Gets/sets the enabled state of this collision component.
	property serialized(1500) bool Enabled
	{
		public native("Set_Enabled") void Set(bool val);
		public native("Get_Enabled") bool Get();
	}
}