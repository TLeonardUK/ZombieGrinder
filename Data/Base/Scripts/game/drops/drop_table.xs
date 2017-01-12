// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using system.network;

public struct Drop_Table_Entry
{
	int Start_Index;
	int End_Index;

	Type Object_Type;
}

[
	Placeable(false), 
	Name("Drop Table"), 
	Description("Base class for all tables that describe what enemies can drop.") 
]
public class Drop_Table
{
	public float Drop_Probability; // Probability between 0-1 that a drop will be created.

	private int m_max_entries;
	private Drop_Table_Entry[] m_entries = new Drop_Table_Entry[0];

	public void Add(Type drop_type, int probability)
	{
		Drop_Table_Entry entry = new Drop_Table_Entry();
		entry.Start_Index	= m_max_entries;
		entry.End_Index		= m_max_entries + (probability - 1);
		entry.Object_Type	= drop_type;
		m_entries.AddLast(entry);

		m_max_entries += probability;
	} 

	public bool Should_Drop(float multiplier = 1.0f)
	{
		return Math.Rand(0.0f, 1.0f) <= (Drop_Probability * multiplier);
	}

	public Type Get_Drop()
	{
		int rnd = Math.Rand(0, m_max_entries);
		foreach (Drop_Table_Entry d in m_entries)
		{
			if (rnd >= d.Start_Index && rnd <= d.End_Index)
			{
				return d.Object_Type;
			}
		}
		Log.Assert(false, "Should not be possible to get here: rand index was " + rnd +" / " + m_max_entries);
	}
}

public struct Integer_Drop_Table_Entry
{
	int Start_Index;
	int End_Index;

	int Object_Type;
}

[
	Placeable(false), 
	Name("Integer Drop Table"), 
	Description("Similar to drop tables except generic-type based.") 
]
public class Integer_Drop_Table
{
	public float Drop_Probability; // Probability between 0-1 that a drop will be created.

	private int m_max_entries;
	private Integer_Drop_Table_Entry[] m_entries = new Integer_Drop_Table_Entry[0];

	public void Add(int drop_type, int probability)
	{
		Integer_Drop_Table_Entry entry = new Integer_Drop_Table_Entry();
		entry.Start_Index	= m_max_entries;
		entry.End_Index		= m_max_entries + (probability - 1);
		entry.Object_Type	= drop_type;
		m_entries.AddLast(entry);

		m_max_entries += probability;
	} 

	public bool Should_Drop(float multiplier = 1.0f)
	{
		return Math.Rand(0.0f, 1.0f) <= (Drop_Probability * multiplier);
	}

	public int Get_Drop()
	{
		int rnd = Math.Rand(0, m_max_entries);
		foreach (Integer_Drop_Table_Entry d in m_entries)
		{
			if (rnd >= d.Start_Index && rnd <= d.End_Index)
			{
				return d.Object_Type;
			}
		}
		Log.Assert(false, "Should not be possible to get here: rand index was " + rnd +" / " + m_max_entries);
	}
}
