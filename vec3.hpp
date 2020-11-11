#pragma once

#include <cstdint>
#include <vector>
#include <iostream>
#include <string>
#include <cmath>
#include <map>


namespace james {
struct vec3
{
	/*union{
		{double x, y, z};
		double data[3];
	} = {0};*/
	double x, y, z;

	vec3() : x(0), y(0), z(0)
	{
	}

	vec3(double x, double y, double z) : x(x), y(y), z(z)
	{
	}

	bool operator==(const vec3& other) const //equal?
	{
		return x == other.x && y == other.y && x == other.z;
	}

	bool operator!=(const vec3& other) const //neq?
	{
		return !(*this == other);
	}

	vec3 operator+(const vec3& other) const //vector sum
	{
		return { x + other.x, y + other.y, z + other.z };
	}

	vec3 operator-(const vec3& other) const //displacement
	{
		return { x - other.x, y - other.y, z - other.z };
	}

	vec3 operator*(const double scalar) const //field multiplication
	{
		return { x * scalar, y * scalar, z * scalar };
	}

	double operator*(const vec3& other) const //dot product
	{
		return x * other.x + y * other.y + z * other.z;
	}

	vec3 operator^(const vec3& other) const //cross/vector product
	{
		return{ y*other.z - z*other.y, z*other.x - x*other.z, x*other.y - y*other.x };
	}

	vec3 operator/(const double scalar) const //field division
	{
		return { x / scalar, y / scalar, z / scalar };
	}

	double dist(const vec3& other) const //distance
	{
		return std::sqrt((x - other.x)*(x - other.x) + (y - other.y)*(y - other.y) + (z - other.z)*(z - other.z));
	}

	double length() const
	{
		return std::sqrt(x*x + y*y + z*z);
	}

	bool empty() const
	{
		return x == 0 && y == 0 && z == 0;
	}
};

struct quaternion
{
	double r;
	double x;
	double y;
	double z;
	//vec3 a;

	quaternion() : r(1)//r(0), a(0.5773502692, 0.5773502692, 0.5773502692)
	{}

	//quaternion(double r, vec3 a) : r(r), a(a)
	//{}

	quaternion(double r, double x, double y, double z) : r(r), x(x), y(y), z(z)//a(x, y, z)
	{}

	quaternion operator+(const quaternion& other) const
	{
		return { r + other.r, x + other.x, y + other.y, z + other.z };
	}

	//quaternion operator*(const double scalar) const
	//{
	//	return { r * scalar, a * scalar };
	//}

	quaternion operator*(const quaternion& o) const
	{
		return { r*o.r-x*o.x-y*o.y-z*o.z, r*o.x+x*o.r+y*o.z-z*o.y, r*o.y-x*o.z+y*o.r+z*o.x, r*o.z+x*o.y-y*o.x+z*o.r };
	}

};



void matrix44_set(float * r, float * a)
{
	for (unsigned int i = 0;  i < 16; i++) r[i] = a[i];
};

void matrix44_mult(float * r, float * a, float * b)
{
  r[0]  = a[0] * b[0] +  a[4] * b[1]  + a[8]  * b[2]  + a[12] * b[3];
  r[1]  = a[1] * b[0] +  a[5] * b[1]  + a[9]  * b[2]  + a[13] * b[3];
  r[2]  = a[2] * b[0] +  a[6] * b[1]  + a[10] * b[2]  + a[14] * b[3];
  r[3]  = a[3] * b[0] +  a[7] * b[1]  + a[11] * b[2]  + a[15] * b[3];

  r[4]  = a[0] * b[4] +  a[4] * b[5]  + a[8]  * b[6]  + a[12] * b[7];
  r[5]  = a[1] * b[4] +  a[5] * b[5]  + a[9]  * b[6]  + a[13] * b[7];
  r[6]  = a[2] * b[4] +  a[6] * b[5]  + a[10] * b[6]  + a[14] * b[7];
  r[7]  = a[3] * b[4] +  a[7] * b[5]  + a[11] * b[6]  + a[15] * b[7];

  r[8]  = a[0] * b[8]  + a[4] * b[9]  + a[8]  * b[10] + a[12] * b[11];
  r[9]  = a[1] * b[8]  + a[5] * b[9]  + a[9]  * b[10] + a[13] * b[11];
  r[10] = a[2] * b[8]  + a[6] * b[9]  + a[10] * b[10] + a[14] * b[11];
  r[11] = a[3] * b[8]  + a[7] * b[9]  + a[11] * b[10] + a[15] * b[11];

  r[12] = a[0] * b[12] + a[4] * b[13] + a[8]  * b[14] + a[12] * b[15];
  r[13] = a[1] * b[12] + a[5] * b[13] + a[9]  * b[14] + a[13] * b[15];
  r[14] = a[2] * b[12] + a[6] * b[13] + a[10] * b[14] + a[14] * b[15];
  r[15] = a[3] * b[12] + a[7] * b[13] + a[11] * b[14] + a[15] * b[15];
};

void matrix44_mult(float * r, float * a)
{
  float temp[16];
  matrix44_mult(temp, r, a);
  matrix44_set(r, temp);
};

void matrix44_from_quaternion(float * r, quaternion q)
{
  r[0]  = 1 - 2 * (q.y * q.y + q.z * q.z);
  r[1]  = 2 * (q.x * q.y + q.r * q.z);
  r[2]  = 2 * (q.x * q.z - q.r * q.y);
  r[3]  = 0;

  r[4]  = 2 * (q.x * q.y - q.r * q.z);
  r[5]  = 1 - 2 * (q.x * q.x + q.z * q.z);
  r[6]  = 2 * (q.y * q.z + q.r * q.x);
  r[7]  = 0;

  r[8]  = 2 * (q.x * q.z + q.r * q.y);
  r[9]  = 2 * (q.y * q.z - q.r * q.x);
  r[10] = 1 - 2 * (q.x * q.x + q.y * q.y);
  r[11] = 0;

  r[12] = 0;
  r[13] = 0;
  r[14] = 0;
  r[15] = 1;
}

void matrix44_divide(float * r, float * a, float s)
{
	for (unsigned int i = 0;  i < 12; i++) r[i] = a[i] / s;
	for (unsigned int i = 12; i < 16; i++) r[i] = a[i];
};

void matrix44_shift(float * r, float x, float y, float z)
{
  r[0]  = 1;
  r[1]  = 0;
  r[2]  = 0;
  r[3]  = 0;

  r[4]  = 0;
  r[5]  = 1;
  r[6]  = 0;
  r[7]  = 0;

  r[8]  = 0;
  r[9]  = 0;
  r[10] = 1;
  r[11] = 0;

  r[12] = x;
  r[13] = y;
  r[14] = z;
  r[15] = 1;
};



}
