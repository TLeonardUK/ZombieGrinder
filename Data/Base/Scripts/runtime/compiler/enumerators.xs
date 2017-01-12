// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
//	This package contains the declarations of any objects used for 
//	enumeration support.
//	This should never be modified as the compiler relies on the correct content 
//	and ordering of this file.
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//	Used as a base for objects that can be enumerated over.
// -----------------------------------------------------------------------------
public interface IEnumerable
{
	public abstract IEnumerator GetEnumerator();
}

// -----------------------------------------------------------------------------
//	Used as a base for objects that enumerate over collections.
// -----------------------------------------------------------------------------
public interface IEnumerator
{
	public abstract object	Current();
	public abstract bool	Next();
	public abstract void   	Reset();
}