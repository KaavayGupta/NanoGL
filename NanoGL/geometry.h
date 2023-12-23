#pragma once

#include <cmath>

template <class T>
struct Vec2
{
	union
	{
		struct { T x, y; };
		struct { T u, v; };
		T Raw[2];
	};

	Vec2(): x(0), y(0) {}
	Vec2(T _x, T _y) : x(_x), y(_y) {}
	inline T& operator[](const int i) { return Raw[i]; }
	inline T operator[](const int i) const { return Raw[i]; }
	inline Vec2<T> operator+(const Vec2<T>& b) const { return Vec2<T>(x + b.x, y + b.y); }
	inline Vec2<T> operator-(const Vec2<T>& b) const { return Vec2<T>(x - b.x, y - b.y); }
	inline Vec2<T> operator*(float f) const { return Vec2<T>(x*f, y*f); }
	inline Vec2<T> operator/(float f) const { return Vec2<T>(x/f, y/f); }
};

template <class T>
struct Vec3
{
	union
	{
		struct { T x, y, z; };
		struct { T ivert, iuv, inorm; };
		T Raw[3];
	};

	Vec3() : x(0), y(0), z(0) {}
	Vec3(T _x, T _y, T _z) : x(_x), y(_y), z(_z) {}
	inline T& operator[](const int i) { return Raw[i]; }
	inline T operator[](const int i) const { return Raw[i]; }
	inline Vec3<T> operator+(const Vec3<T>& b) const { return Vec3<T>(x + b.x, y + b.y, z + b.z); }
	inline Vec3<T> operator-(const Vec3<T>& b) const { return Vec3<T>(x - b.x, y - b.y, z - b.z); }
	inline Vec3<T> operator*(float f) const { return Vec3<T>(x*f, y*f, z*f); }
	inline Vec3<T> operator/(float f) const { return Vec3<T>(x/f, y/f, z/f); }
	inline T operator *(const Vec3<T>& v) const { return x * v.x + y * v.y + z * v.z; }
	inline Vec3<T> operator ^(const Vec3<T>& v) const { return Vec3<T>(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x); }
	float Magnitude() const { return std::sqrt(SqrMagnitude()); }
	float SqrMagnitude() const { return (float)(x * x + y * y + z * z); }
	Vec3<T>& Normalize() { *this = (*this) / Magnitude(); return *this; }
	
};

using Vec2f = Vec2<float>;
using Vec2i = Vec2<int>;
using Vec3f = Vec3<float>;
using Vec3i = Vec3<int>;