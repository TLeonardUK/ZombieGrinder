 // -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;

public sealed native("Vec4") struct Vec4
{
	public native("ToString") override string ToString();
	
	// Constructors.
	public native("Create") 		Vec4();
	public native("Create_X") 		Vec4(float x);
	public native("Create_XY") 		Vec4(float x, float y);
	public native("Create_XYZ") 	Vec4(float x, float y, float z);
	public native("Create_XYZW") 	Vec4(float x, float y, float z, float w);
	public native("Create_V3W") 	Vec4(Vec3 xyz, float w);
	public native("Create_V2ZW") 	Vec4(Vec2 xy, float z, float w);
	public native("Create_V2V2") 	Vec4(Vec2 xy, Vec2 zw);
	
	// Operator overloads.
	public native("Operator_EQ") bool operator==(Vec4 other);
	public native("Operator_NE") bool operator!=(Vec4 other);
	public native("Operator_G")  bool operator> (Vec4 other);
	public native("Operator_GE") bool operator>=(Vec4 other);
	public native("Operator_L")  bool operator< (Vec4 other);
	public native("Operator_LE") bool operator<=(Vec4 other);
	
	public native("Operator_SubV") Vec4 operator-(Vec4 other);
	public native("Operator_SubF") Vec4 operator-(float other);
	public native("Operator_Neg")  Vec4 operator-();
	public native("Operator_AddV") Vec4 operator+(Vec4 other);
	public native("Operator_AddF") Vec4 operator+(float other);
	public native("Operator_MulV") Vec4 operator*(Vec4 other);
	public native("Operator_MulF") Vec4 operator*(float other);
	public native("Operator_DivV") Vec4 operator/(Vec4 other);	
	public native("Operator_DivF") Vec4 operator/(float other);	
		
	public native("Operator_SetIndex") void  operator[](int index, float other);
	public native("Operator_GetIndex") float operator[](int index);
	
	// General math.
	public native("Center") Vec2 Center();
	public native("Length") float Length();
	public native("Constrain") Vec4 Constrain(Vec4 bounds);
	public native("Intersects") bool Intersects(Vec4 other);
	public native("Unit") Vec4 Unit();
	public native("Dot") float Dot(Vec4 val);

	// Do not touch these fields! Order and offset assumed.
	public serialized(1) forceoffset(0) float X;
	public serialized(1) forceoffset(1) float Y;
	public serialized(1) forceoffset(2) float Z;
	public serialized(1) forceoffset(3) float W;

	[DisableInspection(true)]
	public forceoffset(2) float Width;

	[DisableInspection(true)]
	public forceoffset(3) float Height;
	
	// Swizzled access.
	[DisableInspection(true)]
	property Vec2 XY
	{
		public native("Get_XY") Vec2 Get();
		public native("Set_XY") void Set(Vec2 other);	
	}
	
	[DisableInspection(true)]
	property Vec2 ZW
	{
		public native("Get_ZW") Vec2 Get();
		public native("Set_ZW") void Set(Vec2 other);	
	}
	
	[DisableInspection(true)]
	property Vec3 XYZ
	{
		public native("Get_XYZ") Vec3 Get();
		public native("Set_XYZ") void Set(Vec3 other);
	}	

	public native("Rand") static Vec4 Rand(float min, float max);
	public native("Shift") Vec4 Shift(float x, float y);
	public native("Inflate") Vec4 Inflate(float x, float y);
	public native("RectUnion") Vec4 RectUnion(Vec4 other);
}

public sealed native("Vec3") struct Vec3
{
	public native("ToString") override string ToString();
	
	// Constructors.
	public native("Create") Vec3();
	public native("Create_X") Vec3(float x);
	public native("Create_XY") Vec3(float x, float y);
	public native("Create_XYZ") Vec3(float x, float y, float z);
	public native("Create_V2Z") Vec3(Vec2 v, float z);
	
	// Operator overloads.
	public native("Operator_EQ") bool operator==(Vec3 other);
	public native("Operator_NE") bool operator!=(Vec3 other);
	public native("Operator_G")  bool operator> (Vec3 other);
	public native("Operator_GE") bool operator>=(Vec3 other);
	public native("Operator_L")  bool operator< (Vec3 other);
	public native("Operator_LE") bool operator<=(Vec3 other);
	
	public native("Operator_SubV") Vec3 operator-(Vec3 other);
	public native("Operator_SubF") Vec3 operator-(float other);
	public native("Operator_Neg")  Vec3 operator-();
	public native("Operator_AddV") Vec3 operator+(Vec3 other);
	public native("Operator_AddF") Vec3 operator+(float other);
	public native("Operator_MulV") Vec3 operator*(Vec3 other);
	public native("Operator_MulF") Vec3 operator*(float other);
	public native("Operator_DivV") Vec3 operator/(Vec3 other);	
	public native("Operator_DivF") Vec3 operator/(float other);	
		
	public native("Operator_SetIndex") void  operator[](int index, float other);
	public native("Operator_GetIndex") float operator[](int index);

	// General math.
	public native("Length") float Length();
	public native("Unit") Vec3 Unit();
	public native("Dot") float Dot(Vec3 val);
	public native("Offset") Vec3 Offset(float dir, float length);

	public float To_Angle()
	{
		return Math.Atan2(Y, X);
	}
	
	// Do not touch these fields! Order and offset assumed.
	public serialized(1) forceoffset(0) float X;
	public serialized(1) forceoffset(1) float Y;
	public serialized(1) forceoffset(2) float Z;

	// Swizzled access.
	[DisableInspection(true)]
	property Vec2 XY
	{
		public native("Get_XY") Vec2 Get();
		public native("Set_XY") void Set(Vec2 other);	
	}
	
	public native("Rand") static Vec3 Rand(float min, float max);

}

public sealed native("Vec2") struct Vec2
{
	public native("ToString") override string ToString();
	
	// Constructors.
	public native("Create") Vec2();
	public native("Create_X") Vec2(float x);
	public native("Create_XY") Vec2(float x, float y);
	
	// Operator overloads.
	public native("Operator_EQ") bool operator==(Vec2 other);
	public native("Operator_NE") bool operator!=(Vec2 other);
	public native("Operator_G")  bool operator> (Vec2 other);
	public native("Operator_GE") bool operator>=(Vec2 other);
	public native("Operator_L")  bool operator< (Vec2 other);
	public native("Operator_LE") bool operator<=(Vec2 other);
	
	public native("Operator_SubV") Vec2 operator-(Vec2 other);
	public native("Operator_SubF") Vec2 operator-(float other);
	public native("Operator_Neg")  Vec2 operator-();
	public native("Operator_AddV") Vec2 operator+(Vec2 other);
	public native("Operator_AddF") Vec2 operator+(float other);
	public native("Operator_MulV") Vec2 operator*(Vec2 other);
	public native("Operator_MulF") Vec2 operator*(float other);
	public native("Operator_DivV") Vec2 operator/(Vec2 other);	
	public native("Operator_DivF") Vec2 operator/(float other);	
		
	public native("Operator_SetIndex") void  operator[](int index, float other);
	public native("Operator_GetIndex") float operator[](int index);

	// General math.
	public native("Length") float Length();
	public native("Direction") float Direction();
	public native("Unit") Vec2 Unit();
	public native("Dot") float Dot(Vec2 val);
	
	// Do not touch these fields! Order and offset assumed.
	public serialized(1) forceoffset(0) float X;
	public serialized(1) forceoffset(1) float Y;
	
	public native("Rand") static Vec2 Rand(float min, float max);
}

