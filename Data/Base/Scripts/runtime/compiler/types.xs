// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
//	This package contains the declarations of all default types used by the
//	language. This should never be modified as the compiler relies on the 
//	correct content and ordering of this file.
// -----------------------------------------------------------------------------

// =============================================================================
//	Values Types
// =============================================================================

// -----------------------------------------------------------------------------
//	The boolean type allows basic boolean operations. The reason you will see
//	it as a class here when its technically a primitive is to allow the user
//	to access general properties and methods relating to it.
//		eg. bool.MinValue
// -----------------------------------------------------------------------------
public sealed native("bool") box("BoolBox") class @bool : null
{
}

// -----------------------------------------------------------------------------
//	The integer type works in the same way as the boolean one above. The size
//	of this data type depends on if the result is compiled as 64 or 32 bit. In
//	which case its 4 and 8 bytes long respectively.
// -----------------------------------------------------------------------------
public sealed native("int") box("IntBox") class @int : null
{
}

// -----------------------------------------------------------------------------
//	The integer type works in the same way as the other primitive types. 
//	Floating point numbers are always single precision.
// -----------------------------------------------------------------------------
public sealed native("float") box("FloatBox") class @float : null
{
}

// -----------------------------------------------------------------------------
//	The string class holds a single string of characters. Strings are 
//	immutable, all operations produce a new string object.
// -----------------------------------------------------------------------------
public sealed native("String") box("StringBox") class @string : null
{
	public native("ToInt")			int 			ToInt		();
	public native("ToFloat")		float 			ToFloat		();
	public native("ToChar")			int 			ToChar		();
	public native("Length")			int 			Length		();
	public native("GetIndex")		string 			operator[]	(int index);
	public native("GetSlice1")		string 			GetSlice	(int start_index);
	public native("GetSlice2")		string 			GetSlice	(int start_index, int end_index);
	public native("HexToInt")		int 			HexToInt	();
	public native("TrimStart")		string 			TrimStart	(string chars = " "); 
	public native("TrimEnd")		string 			TrimEnd		(string chars = " "); 
	public native("Trim")			string 			Trim		(string chars = " "); 
	public native("SubString") 		string 			SubString	(int offset, int count = -1);
	public native("PadRight") 		string 			PadRight	(int length, string padding = " ");
	public native("PadLeft") 		string 			PadLeft		(int length, string padding = " ");
	public native("LimitEnd") 		string 			LimitEnd	(int length, string postfix = " ...");
	public native("LimitStart") 	string 			LimitStart	(int length, string postfix = " ...");
	public native("Reverse") 		string 			Reverse		();
	public native("ToLower") 		string 			ToLower		();
	public native("ToUpper") 		string 			ToUpper		();
	public native("EndsWith") 		bool 			EndsWith	(string postfix);
	public native("StartsWith") 	bool 			StartsWith	(string prefix);
	public native("Remove") 		string 			Remove		(int start, int length);
	public native("Insert") 		string 			Insert		(string value, int index);
	public native("Replace") 		string			Replace		(string what, string with);
	public native("ReplaceSection") string 			Replace		(int start, int length, string mid);
	public native("Contains") 		bool 			Contains	(string text);
	public native("ContainsAny") 	bool 			ContainsAny	(string[] text);
	public native("IndexOf") 		int 			IndexOf		(string needle, int start_index = 0);
	public native("IndexOfAny") 	int 			IndexOfAny	(string[] needles, int start_index = 0);
	public native("LastIndexOf") 	int 			LastIndexOf	(string needle, int end_index = -1);
	public native("LastIndexOfAny") int 			LastIndexOfAny(string[] needles, int end_index = -1);
	public native("Filter") 		string 			Filter		(string allowed_chars, string replacement_char = "");
	public native("IsHex") 			bool 			IsHex		();
	public native("IsNumeric") 		bool 			IsNumeric	();
	public native("Join") 			string 			Join		(string[] haystack);
	public native("Split") 			string[] 		Split		(string seperator, int max_splits = -1, bool remove_duplicates = false);
	public native("Format") 		string 			Format		(object[] args);

	public native("FormatNumber") 	static string 			FormatNumber(float value);
	public native("IntToHex")		static string 	IntToHex	(int chr);
	public native("FromChar")		static string 	FromChar	(int chr);
	public native("FromChars")		static string 	FromChars	(int[] chr);
}

// =============================================================================
//	Reference Types
// =============================================================================

// -----------------------------------------------------------------------------
//	All objects type.
// -----------------------------------------------------------------------------
public native("Type") class Type
{
	property int ID
	{
		public native("Get_ID") int Get();
	}
	property string Name
	{
		public native("Get_Name") string Get();
	}
	public native("New") object New();
	public static native("Find") Type Find(int id);	
	public static native("Find_By_String") Type Find(string name);	
}

// -----------------------------------------------------------------------------
//	All objects derive from the base object class.
// -----------------------------------------------------------------------------
public native("Object") class @object : null
{
	public native("ToString") virtual string ToString();
	public native("GetType") virtual Type GetType();
}

/// -----------------------------------------------------------------------------
///	The array is a special class that all arrays are derived from. This class 
///	should never be instantiated or used directly, instead you should access it
//	by declaring data types in typical array syntax, eg. string[]
/// -----------------------------------------------------------------------------
public sealed native("Array") class @array<T> : @object, IEnumerable
{
	public native("Length")				int 		Length		();
	public native("Resize")				void		Resize		(int size);
	public native("SetIndex")			void	 	operator[]	(int index, T value);
	public native("GetIndex")			T		 	operator[]	(int index);
	public native("GetSlice1")			T[]			GetSlice	(int start_index);
	public native("GetSlice2")			T[]	 		GetSlice	(int start_index, int end_index);
	public native("Shift") 				void 		Shift		(int offset);
	public native("AddFirst")			void 		AddFirst	(T value);
	public native("AddArrayFirst") 		void 		AddFirst	(T[] value);
	public native("AddLast") 			void 		AddLast		(T value);
	public native("AddArrayLast")		void 		AddLast		(T[] value);
	public native("Clear") 				void 		Clear		();
	public native("Clone") 				T[] 		Clone		();
	public native("Contains") 			bool 		Contains	(T needle);
	public native("Replace") 			void 		Replace		(T needle, T other);
	public native("Reverse") 			void 		Reverse		();
	public native("RemoveIndex") 		T	 		RemoveIndex	(int index);
	public native("Remove") 			void 		Remove		(T needle);
	public native("RemoveFirst") 		T 			RemoveFirst	();
	public native("RemoveLast") 		T 			RemoveLast	();
	public native("Insert") 			void 		Insert		(T value, int index);
	public native("TrimStart") 			void 		TrimStart	(T needle);
	public native("TrimEnd") 			void 		TrimEnd		(T needle);
	public native("Trim") 				void 		Trim		(T needle);
	public native("PadLeft") 			void 		PadLeft		(int len, T padding);
	public native("PadRight") 			void		PadRight	(int len, T padding);
	public native("IndexOf") 			int 		IndexOf		(T value, int start_index = -1);
	public native("IndexOfAny") 		int 		IndexOfAny	(T[] value, int start_index = -1);
	public native("LastIndexOf") 		int 		LastIndexOf	(T value, int end_index = -1);
	public native("LastIndexOfAny") 	int 		LastIndexOfAny(T[] value, int end_index = -1);
	public native("Sort") 				void 		Sort		(Comparer<T> comparer);
	public native("ClearElements") 		void 		ClearElements(T value);
	
	public override IEnumerator GetEnumerator()
	{
		return (new ArrayEnumerator<T[]>(this));
	}
}

/// -----------------------------------------------------------------------------
///  Array enumerators are used to provide support for foreach actions against
//	 array data types.
/// -----------------------------------------------------------------------------
public sealed class ArrayEnumerator<T> : IEnumerator
{
	private T 	m_value;
	private int m_index;

	// -------------------------------------------------------------------------
	//
	// -------------------------------------------------------------------------
	public ArrayEnumerator(T value)
	{
		m_value = value;
		m_index = 0;
	}

	// -------------------------------------------------------------------------
	//
	// -------------------------------------------------------------------------
	public override object Current()
	{
		return m_value[m_index - 1];
	}
	
	// -------------------------------------------------------------------------
	//
	// -------------------------------------------------------------------------
	public override bool	Next()
	{
		m_index++;
		return (m_index <= m_value.Length());
	}
	
	// -------------------------------------------------------------------------
	//
	// -------------------------------------------------------------------------
	public override void Reset()
	{
		m_index = 0;
	}	
}

/*
/// -----------------------------------------------------------------------------
///	Simple dictionary implementation.
/// -----------------------------------------------------------------------------
public sealed native("CRuntime_Dictionary") class Dictionary<TKey, TValue> : @object
{
	public native("Count")				int 					Count			();
	public native("SetIndex")			void	 				operator[]		(int index, TValue value);
	public native("GetIndex")			TValue	 				operator[]		(int index);
	public native("Set")				void	 				operator[]		(TKey index, TValue value);
	public native("Get")				TValue	 				GetIndex		(TKey key);
	public native("Clear") 				int 					Clear			();
	public native("Clone") 				Dictionary<TKey,TValue> Clone			();
	public native("CopyTo") 			void 					CopyTo			(Dictionary<TKey,TValue> other);	
	public native("Set")				bool 					Set				(TKey key, TValue value);
	public native("Get")				bool 					Get				(TKey key);
	public native("RemoveKey")			bool 					RemoveKey		(TKey key);
	public native("RemoveValue")		bool 					RemoveValue		(TValue value);	
	public native("ContainsKey")		bool					ContainsKey		(TKey key);
	public native("ContainsValue")		bool 					ContainsValue	(TValue value);
	public native("GetKeys")			TKey[]					GetKeys			();
	public native("GetValues")			TValue[]				GetValues		();
}
*/
