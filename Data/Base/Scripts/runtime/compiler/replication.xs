// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
//	This package contains the declarations of the default runtime functionality.
//	This should never be modified as the compiler relies on the correct content 
//  and ordering of this file.
// -----------------------------------------------------------------------------

public enum ReplicationPriority
{
	Lowest		= 1,		// Don't use zero, zero=never replicate.
	Low			= 25,
	Normal		= 50,
	High		= 75,
	Highest		= 100
} 

public enum ReplicationOwner
{
	Server			= 0,
	Client			= 1,
	ObjectOwner		= 2,
} 

public enum ReplicationMode
{
	Absolute				= 0,
	Interpolate_Linear		= 1,
	Interpolate_Smoothed	= 2,
}