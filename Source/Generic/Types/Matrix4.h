// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_MATRIX4_
#define _GENERIC_MATRIX4_

#include "Vector3.h"
#include "Vector4.h"

// Matrix's are alwyas in column-major order.
// Matrix's should be considered immutable (though you can use SetColumn etc to initialize)
struct Matrix4
{
	//MEMORY_ALLOCATOR(Matrix4, "Data Types");

public:
	float Elements[16];

	// Constructors.
	Matrix4();
	Matrix4(const Matrix4* other);
	Matrix4(float c11, float c12, float c13, float c14,
			float c21, float c22, float c23, float c24,
			float c31, float c32, float c33, float c34,
			float c41, float c42, float c43, float c44);
	Matrix4(float* vals);
	
	// Operator overloads.
	Matrix4 operator* (const Matrix4 rhs) const;
	Vector4 operator* (const Vector4 rhs) const;
	void Display(const char* name);
	Matrix4 Transpose() const;
	Matrix4 Inverse() const;
	bool operator==(const Matrix4 &other) const;
	bool operator!=(const Matrix4 &other) const;

	// Column stuff.
	void SetColumn(int column, float e1, float e2, float e3, float e4);
	void SetRow(int column, float e1, float e2, float e3, float e4);

	// Perspective functions.
	static Matrix4 Identity		();
	static Matrix4 Frustum		(float l, float r, float b, float t, float n, float f);
	static Matrix4 Perspective	(float fov, float aspect_ratio, float z_near, float z_far);
	static Matrix4 Orthographic (float x_n, float x_f, float y_n, float y_f, float z_n, float z_f);
	static Matrix4 LookAt		(Vector3 eye, Vector3 center, Vector3 up);
	static Matrix4 Translate	(Vector3 vector);
	static Matrix4 RotateX		(float angle);
	static Matrix4 RotateY		(float angle);
	static Matrix4 RotateZ		(float angle);
	static Matrix4 Rotate		(Vector3 vector);

};

#endif