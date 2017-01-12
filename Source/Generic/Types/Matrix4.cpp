// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Generic/Types/Matrix4.h"
#include "Generic/Math/Math.h"

#include <string.h>
#include <cstring>
#include <cstdio>

Matrix4::Matrix4()
{
	memset(Elements, 0, sizeof(float) * 16);
}

Matrix4::Matrix4(float c11, float c12, float c13, float c14,
				 float c21, float c22, float c23, float c24,
				 float c31, float c32, float c33, float c34,
				 float c41, float c42, float c43, float c44)
{
	SetColumn(0, c11, c12, c13, c14);
	SetColumn(1, c21, c22, c23, c24);
	SetColumn(2, c31, c32, c33, c34);
	SetColumn(3, c41, c42, c43, c44);
}

Matrix4::Matrix4(float* vals)
{
	for (int i = 0; i < 16; i++)
	{
		Elements[i] = vals[i];
	}
}

Matrix4::Matrix4(const Matrix4* other)
{
	for (int i = 0; i < 16; i++)
	{
		Elements[i] = other->Elements[i];
	}
}

void Matrix4::Display(const char* name)
{
	printf("==== %s ====\n", name);
	for (int i = 0; i < 16; i += 4)
	{
		printf("%+4.4f %+4.4f %+4.4f %+4.4f\n", Elements[i], Elements[i + 1], Elements[i + 2], Elements[i + 3]);
	}
}

Matrix4 Matrix4::Transpose() const
{
	Matrix4 dst;
	
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			dst.Elements[i * 4 + j] = Elements[j * 4 + i];
		}
	}

	return dst;
}

bool Matrix4::operator==(const Matrix4 &other) const
{
	return (memcmp(Elements, other.Elements, sizeof(float) * 16) == 0);
}

bool Matrix4::operator!=(const Matrix4 &other) const
{
	return !operator==(other);
}

Matrix4 Matrix4::Inverse() const
{
    int i;
    float inv[16], det;
	const float* m = Elements;

    inv[0] = m[5]  * m[10] * m[15] - 
             m[5]  * m[11] * m[14] - 
             m[9]  * m[6]  * m[15] + 
             m[9]  * m[7]  * m[14] +
             m[13] * m[6]  * m[11] - 
             m[13] * m[7]  * m[10];

    inv[4] = -m[4]  * m[10] * m[15] + 
              m[4]  * m[11] * m[14] + 
              m[8]  * m[6]  * m[15] - 
              m[8]  * m[7]  * m[14] - 
              m[12] * m[6]  * m[11] + 
              m[12] * m[7]  * m[10];

    inv[8] = m[4]  * m[9] * m[15] - 
             m[4]  * m[11] * m[13] - 
             m[8]  * m[5] * m[15] + 
             m[8]  * m[7] * m[13] + 
             m[12] * m[5] * m[11] - 
             m[12] * m[7] * m[9];

    inv[12] = -m[4]  * m[9] * m[14] + 
               m[4]  * m[10] * m[13] +
               m[8]  * m[5] * m[14] - 
               m[8]  * m[6] * m[13] - 
               m[12] * m[5] * m[10] + 
               m[12] * m[6] * m[9];

    inv[1] = -m[1]  * m[10] * m[15] + 
              m[1]  * m[11] * m[14] + 
              m[9]  * m[2] * m[15] - 
              m[9]  * m[3] * m[14] - 
              m[13] * m[2] * m[11] + 
              m[13] * m[3] * m[10];

    inv[5] = m[0]  * m[10] * m[15] - 
             m[0]  * m[11] * m[14] - 
             m[8]  * m[2] * m[15] + 
             m[8]  * m[3] * m[14] + 
             m[12] * m[2] * m[11] - 
             m[12] * m[3] * m[10];

    inv[9] = -m[0]  * m[9] * m[15] + 
              m[0]  * m[11] * m[13] + 
              m[8]  * m[1] * m[15] - 
              m[8]  * m[3] * m[13] - 
              m[12] * m[1] * m[11] + 
              m[12] * m[3] * m[9];

    inv[13] = m[0]  * m[9] * m[14] - 
              m[0]  * m[10] * m[13] - 
              m[8]  * m[1] * m[14] + 
              m[8]  * m[2] * m[13] + 
              m[12] * m[1] * m[10] - 
              m[12] * m[2] * m[9];

    inv[2] = m[1]  * m[6] * m[15] - 
             m[1]  * m[7] * m[14] - 
             m[5]  * m[2] * m[15] + 
             m[5]  * m[3] * m[14] + 
             m[13] * m[2] * m[7] - 
             m[13] * m[3] * m[6];

    inv[6] = -m[0]  * m[6] * m[15] + 
              m[0]  * m[7] * m[14] + 
              m[4]  * m[2] * m[15] - 
              m[4]  * m[3] * m[14] - 
              m[12] * m[2] * m[7] + 
              m[12] * m[3] * m[6];

    inv[10] = m[0]  * m[5] * m[15] - 
              m[0]  * m[7] * m[13] - 
              m[4]  * m[1] * m[15] + 
              m[4]  * m[3] * m[13] + 
              m[12] * m[1] * m[7] - 
              m[12] * m[3] * m[5];

    inv[14] = -m[0]  * m[5] * m[14] + 
               m[0]  * m[6] * m[13] + 
               m[4]  * m[1] * m[14] - 
               m[4]  * m[2] * m[13] - 
               m[12] * m[1] * m[6] + 
               m[12] * m[2] * m[5];

    inv[3] = -m[1] * m[6] * m[11] + 
              m[1] * m[7] * m[10] + 
              m[5] * m[2] * m[11] - 
              m[5] * m[3] * m[10] - 
              m[9] * m[2] * m[7] + 
              m[9] * m[3] * m[6];

    inv[7] = m[0] * m[6] * m[11] - 
             m[0] * m[7] * m[10] - 
             m[4] * m[2] * m[11] + 
             m[4] * m[3] * m[10] + 
             m[8] * m[2] * m[7] - 
             m[8] * m[3] * m[6];

    inv[11] = -m[0] * m[5] * m[11] + 
               m[0] * m[7] * m[9] + 
               m[4] * m[1] * m[11] - 
               m[4] * m[3] * m[9] - 
               m[8] * m[1] * m[7] + 
               m[8] * m[3] * m[5];

    inv[15] = m[0] * m[5] * m[10] - 
              m[0] * m[6] * m[9] - 
              m[4] * m[1] * m[10] + 
              m[4] * m[2] * m[9] + 
              m[8] * m[1] * m[6] - 
              m[8] * m[2] * m[5];

    det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

    if (det == 0)
        return Matrix4(this);

    det = 1.0f / det;

	Matrix4 dst;
    for (i = 0; i < 16; i++)
		dst.Elements[i] = inv[i] * det;

    return dst;
}

void Matrix4::SetColumn(int column, float e1, float e2, float e3, float e4)
{
	Elements[(column * 4)]	   = e1;
	Elements[(column * 4) + 1] = e2;
	Elements[(column * 4) + 2] = e3;
	Elements[(column * 4) + 3] = e4;
}

void Matrix4::SetRow(int column, float e1, float e2, float e3, float e4)
{
	Elements[(column) + 0] = e1;
	Elements[(column) + 4] = e2;
	Elements[(column) + 8] = e3;
	Elements[(column) + 12] = e4;
}

Matrix4 Matrix4::operator*(const Matrix4 a) const
{
	Matrix4 dst;
	
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			for (int k = 0; k < 4; k++)
			{
				dst.Elements[i * 4 + j] += a.Elements[i*4 + k] * Elements[k*4 + j];
			}
		}
	}

	return dst;
}

Vector4 Matrix4::operator* (const Vector4 lhs) const
{
	return Vector4(
		(lhs.X * Elements[0 + 0]) + (lhs.Y * Elements[0 + 4]) + (lhs.Z * Elements[0 + 8]) + (lhs.W * Elements[0 + 12]),
		(lhs.X * Elements[1 + 0]) + (lhs.Y * Elements[1 + 4]) + (lhs.Z * Elements[1 + 8]) + (lhs.W * Elements[1 + 12]),
		(lhs.X * Elements[2 + 0]) + (lhs.Y * Elements[2 + 4]) + (lhs.Z * Elements[2 + 8]) + (lhs.W * Elements[2 + 12]),
		(lhs.X * Elements[3 + 0]) + (lhs.Y * Elements[3 + 4]) + (lhs.Z * Elements[3 + 8]) + (lhs.W * Elements[3 + 12]));
}

Matrix4 Matrix4::Identity()
{
	Matrix4 mat;

	mat.SetColumn(0, 1, 0, 0, 0);
	mat.SetColumn(1, 0, 1, 0, 0);
	mat.SetColumn(2, 0, 0, 1, 0);
	mat.SetColumn(3, 0, 0, 0, 1);
	
	return mat;
}

Matrix4 Matrix4::Frustum(float l, float r, float b, float t, float n, float f)
{
	float X = (2 * n) / (r - l);
	float Y = (2 * n) / (t - b);

	float A = (r + l) / (r - l);
	float B = (t + b) / (t - b);
	float C = -(f + n) / (f - n);
	float D = -(2 * f * n) / (f - n);

	Matrix4 mat;
	mat.SetColumn(0, X, 0, A, 0);
	mat.SetColumn(1, 0, Y, B, 0);
	mat.SetColumn(2, 0, 0, C, -1);
	mat.SetColumn(3, 0, 0, D, 0);

	return mat;
}

Matrix4 Matrix4::Perspective(float fov, float aspect_ratio, float z_near, float z_far)
{
	float xmin, xmax, ymin, ymax;

	ymax = z_near * tan(fov * PI / 360.0f);
	ymin = -ymax;
	xmin = ymin * aspect_ratio;
	xmax = ymax * aspect_ratio;

	return Frustum(xmin, xmax, ymin, ymax, z_near, z_far);
}	

Matrix4 Matrix4::Orthographic(float xleft, float xright, float ybottom, float ytop, float znear, float zfar)
{
    float a =  2.0f / (xright - xleft);
    float b =  2.0f / (ytop - ybottom);
    float c =  1.0f / (zfar - znear);	// Normally this would be negative and 2.0f /, but FUCK THE POLICE. This works better for 2d :P

    float tx = - ((xright + xleft)	/ (xright - xleft));
    float ty = - ((ytop + ybottom)	/ (ytop - ybottom));
    float tz = 1.0f - ((zfar - znear) / (zfar + znear));

	Matrix4 m2;
	m2.SetColumn(0, a, 0, 0, 0);
	m2.SetColumn(1, 0, b, 0, 0);
	m2.SetColumn(2, 0, 0, c, 0);
	m2.SetColumn(3,tx,ty,tz, 1);

	/*
	GLfloat mat[16];
	glMatrixMode(GL_PROJECTION_MATRIX);
	glLoadIdentity();
	glOrtho(xleft, xright, ybottom, ytop, znear, zfar);
	glGetFloatv(GL_MODELVIEW_MATRIX, mat);
	
	Matrix4 mat4(mat);
	mat4.Display("OpenGL Implementation");
	m2.Display("Our Implementation");
	*/

	return m2;
}

Matrix4 Matrix4::LookAt(Vector3 eyeVector, Vector3 centerVector, Vector3 upVector)
{
	Vector3 forward = (centerVector - eyeVector).Normalize();
	Vector3 side	= forward.Cross(upVector).Normalize();
	Vector3	up		= side.Cross(forward);

	Matrix4 m1;
	m1.SetColumn(0, side.X, up.X, -forward.X, 0);
	m1.SetColumn(1, side.Y, up.Y, -forward.Y, 0);
	m1.SetColumn(2, side.Z, up.Z, -forward.Z, 0);
	m1.SetColumn(3, 0, 0, 0, 1);

	Matrix4 m2 = Matrix4::Translate(Vector3(-eyeVector.X, -eyeVector.Y, -eyeVector.Z));

	Matrix4 result = m1 * m2;
	
	return result;
}

Matrix4 Matrix4::Translate(Vector3 vector)
{
	Matrix4 m2;
	m2.SetRow(0, 1, 0, 0, vector.X);
	m2.SetRow(1, 0, 1, 0, vector.Y);
	m2.SetRow(2, 0, 0, 1, vector.Z);
	m2.SetRow(3, 0, 0, 0, 1);
	return m2;
}

Matrix4 Matrix4::RotateX(float a)
{
	Matrix4 m2;
	m2.SetRow(0, 1, 0,		0,		0);
	m2.SetRow(1, 0, cos(a),-sin(a), 0);
	m2.SetRow(2, 0, sin(a),	cos(a), 0);
	m2.SetRow(3, 0, 0,		0,		1);
	return m2;
}

Matrix4 Matrix4::RotateY(float a)
{
	Matrix4 m2;
	m2.SetRow(0,  cos(a),	0,	sin(a),	0);
	m2.SetRow(1,  0,		1,	0,		0);
	m2.SetRow(2, -sin(a),	0,	cos(a), 0);
	m2.SetRow(3,  0,		0,	0,		1);
	return m2;
}

Matrix4 Matrix4::RotateZ(float a)
{
	Matrix4 m2;
	m2.SetRow(0,  cos(a),	-sin(a),	0,	0);
	m2.SetRow(1,  sin(a),	cos(a),		0,	0);
	m2.SetRow(2,  0,		0,			1,	0);
	m2.SetRow(3,  0,		0,			0,	1);
	return m2;
}

Matrix4 Matrix4::Rotate(Vector3 vector)
{
	return RotateX(vector.X) * RotateY(vector.Y) * RotateZ(vector.Z);
}

