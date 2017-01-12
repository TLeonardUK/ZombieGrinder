// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
//	This package contains the declarations of the different box objects used
//	for boxing and unboxing of primitive types. 
//	This should never be modified as the compiler relies on the correct content 
//	and ordering of this file.
// -----------------------------------------------------------------------------

// Note: Compiler assumes slot(0) of the box type is the value.

// -----------------------------------------------------------------------------
//	Used to box primitive integers.
// -----------------------------------------------------------------------------
public sealed class IntBox 
{
	public int Value;

	public int GetValue()
	{
		return Value;
	}
	
	public override string ToString()
	{
		return <string>Value;
	}
	
	public IntBox(int boxedValue)
	{
		Value = boxedValue;
	}
}

// -----------------------------------------------------------------------------
//	Used to box primitive floats.
// -----------------------------------------------------------------------------
public sealed class FloatBox
{
	public float Value;
	
	public float GetValue()
	{
		return Value;
	}
	
	public override string ToString()
	{
		return <string>Value;
	}
	
	public FloatBox(float boxedValue)
	{
		Value = boxedValue;
	}
}

// -----------------------------------------------------------------------------
//	Used to box primitive bools.
// -----------------------------------------------------------------------------
public sealed class BoolBox 
{	
	public bool Value;
	
	public bool GetValue()
	{
		return Value;
	}
	
	public override string ToString()
	{
		return <string>Value;
	}
	
	public BoolBox(bool boxedValue)
	{
		Value = boxedValue;
	}
}

// -----------------------------------------------------------------------------
//	Used to box primitive strings.
// -----------------------------------------------------------------------------
public sealed class StringBox 
{
	public string Value;
	
	public string GetValue()
	{
		return Value;
	}
	
	public override string ToString()
	{
		return Value;
	}
	
	public StringBox(string boxedValue)
	{
		Value = boxedValue;
	}
}
