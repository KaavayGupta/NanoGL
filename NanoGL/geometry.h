#pragma once

#include <cmath>
#include <vector>
#include <cassert>
#include <iostream>

template<size_t DimCols, size_t DimRows, typename T> class Mat;

template<size_t DIM, typename T>
struct Vec
{
	Vec() { for (size_t i = DIM; i--; m_Data[i] = T()); }
	T& operator[] (const size_t i) { assert(i < DIM); return m_Data[i]; }
	const T& operator[] (const size_t i) const { assert(i < DIM); return m_Data[i]; }
private:
	T m_Data[DIM];
};

//---------------------------------------------------------------------------------------------------------------------

template <typename T>
struct Vec<2, T>
{
	Vec() : x(T()), y(T()) {}
	Vec(T X, T Y) : x(X), y(Y) {}
	template <class U> Vec<2, T>(const Vec<2, U>& v);
	T& operator[] (const size_t i) { assert(i < 2); return i <= 0 ? x : y; }
	const T& operator[] (const size_t i) const { assert(i < 2); return i <= 0 ? x : y; }

	T x, y;
};

template <typename T>
struct Vec<3, T>
{
	Vec() : x(T()), y(T()), z(T()) {}
	Vec(T X, T Y, T Z) : x(X), y(Y), z(Z) {}
	template <class U> Vec<3, T>(const Vec<3, U>& v);
	T& operator[] (const size_t i) { assert(i < 3); return i <= 0 ? x : (i == 1 ? y : z); }
	const T& operator[] (const size_t i) const { assert(i < 3); return i <= 0 ? x : (i == 1 ? y : z); }
	float Magnitude() { return std::sqrt(SqrMagnitude()); }
	float SqrMagnitude() { return float(x*x + y*y + z*z); }
	Vec<3, T>& Normalize(T l = 1) { *this = (*this) * (l / Magnitude()); return *this; }

	T x, y, z;
};

//---------------------------------------------------------------------------------------------------------------------

template <size_t DIM, typename T>
T operator*(const Vec<DIM, T>& lhs, const Vec<DIM, T>& rhs)
{
	T ret = T();
	for (size_t i = DIM; i--; ret += lhs[i] * rhs[i]);
	return ret;
}

template <size_t DIM, typename T>
Vec<DIM, T> operator+(Vec<DIM, T> lhs, const Vec<DIM, T>& rhs)
{
	for (size_t i = DIM; i--; lhs[i] += rhs[i]);
	return lhs;
}

template <size_t DIM, typename T>
Vec<DIM, T> operator-(Vec<DIM, T> lhs, const Vec<DIM, T>& rhs)
{
	for (size_t i = DIM; i--; lhs[i] -= rhs[i]);
	return lhs;
}

template<size_t DIM, typename T, typename U>
Vec<DIM, T> operator*(Vec<DIM, T> lhs, const U& rhs)
{
	for (size_t i = DIM; i--; lhs[i] *= rhs);
	return lhs;
}

template<size_t DIM, typename T, typename U>
Vec<DIM, T> operator/(Vec<DIM, T> lhs, const U& rhs)
{
	for (size_t i = DIM; i--; lhs[i] /= rhs);
	return lhs;
}

template <size_t LEN, size_t DIM, typename T>
Vec<LEN, T> Embed(const Vec<DIM, T>& v, T fill = 1)
{
	Vec<LEN, T> ret;
	for (size_t i = LEN; i--; ret[i] = i < DIM ? v[i] : fill);
	return ret;
}

template <size_t LEN, size_t DIM, typename T>
Vec<LEN, T> Proj(const Vec<DIM, T>& v)
{
	Vec<LEN, T> ret;
	for (size_t i = LEN; i--; ret[i] = v[i]);
	return ret;
}

template <typename T>
Vec<3, T> Cross(Vec<3, T> v1, Vec<3, T> v2)
{
	return Vec<3, T>(v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v1.y * v2.x);
}

template<size_t DIM, typename T>
std::ostream& operator<<(std::ostream& out, Vec<DIM, T>& v)
{
	for (unsigned int i = 0; i < DIM; i++)
	{
		out << v[i] << " ";
	}

	return out;
}

//---------------------------------------------------------------------------------------------------------------------

template<size_t DIM, typename T>
struct DT 
{
	static T Det(const Mat<DIM, DIM, T>& src) 
	{
		T ret = 0;
		for (size_t i = DIM; i--; ret += src[0][i] * src.Cofactor(0, i));
		return ret;
	}
};

template<typename T>
struct DT<1, T> 
{
	static T Det(const Mat<1, 1, T>& src) 
	{
		return src[0][0];
	}
};

//---------------------------------------------------------------------------------------------------------------------

template<size_t DimRows, size_t DimCols, typename T>
class Mat 
{
public:
	Mat() {};

	Vec<DimCols, T>& operator[] (const size_t idx)
	{
		assert(idx < DimRows);
		return m_Rows[idx];
	}

	const Vec<DimCols, T>& operator[] (const size_t idx) const
	{
		assert(idx < DimRows);
		return m_Rows[idx];
	}

	Vec<DimRows, T> Col(const size_t idx) const 
	{
		assert(idx < DimCols);
		Vec<DimRows, T> ret;
		for (size_t i = DimRows; i--; ret[i] = m_Rows[i][idx]);
		return ret;
	}

	void SetCol(size_t idx, const Vec<DimRows, T>& v)
	{
		assert(idx < DimCols);
		for (size_t i = DimRows; i--; m_Rows[i][idx] = v[i]);
	}

	static Mat<DimRows, DimCols, T> Identity() 
	{
		Mat<DimRows, DimCols, T> ret;
		for (size_t i = DimRows; i--; )
			for (size_t j = DimCols; j--; ret[i][j] = (i == j));
		return ret;
	}

	T Det() const 
	{
		return DT<DimCols, T>::Det(*this);
	}

	Mat<DimRows - 1, DimCols - 1, T> GetMinor(size_t row, size_t col) const 
	{
		Mat<DimRows - 1, DimCols - 1, T> ret;
		for (size_t i = DimRows - 1; i--; )
			for (size_t j = DimCols - 1; j--; ret[i][j] = m_Rows[i < row ? i : i + 1][j < col ? j : j + 1]);
		return ret;
	}

	T Cofactor(size_t row, size_t col) const 
	{
		return GetMinor(row, col).Det() * ((row + col) % 2 ? -1 : 1);
	}

	Mat<DimRows, DimCols, T> Adjugate() const 
	{
		Mat<DimRows, DimCols, T> ret;
		for (size_t i = DimRows; i--; )
			for (size_t j = DimCols; j--; ret[i][j] = Cofactor(i, j));
		return ret;
	}

	Mat<DimRows, DimCols, T> InvertTranspose() 
	{
		Mat<DimRows, DimCols, T> ret = Adjugate();
		T tmp = ret[0] * m_Rows[0];
		return ret / tmp;
	}


	Mat<DimRows, DimCols, T> Invert()
	{
		return InvertTranspose().Transpose();
	}

	Mat<DimCols, DimRows, T> Transpose()
	{
		Mat<DimCols, DimRows, T> ret;
		for (size_t i = DimCols; i--; ret[i] = this->Col(i));
		return ret;
	}

private:
	Vec<DimCols, T> m_Rows[DimRows];
};

//---------------------------------------------------------------------------------------------------------------------

template<size_t DimRows, size_t DimCols, typename T>
Vec<DimRows, T> operator*(const Mat<DimRows, DimCols, T>& lhs, const Vec<DimCols, T>& rhs) 
{
	Vec<DimRows, T> ret;
	for (size_t i = 0; i < DimRows; ++i) {
		ret[i] = T();
		for (size_t j = 0; j < DimCols; ++j) {
			ret[i] += lhs[i][j] * rhs[j];
		}
	}
	return ret;
}

template<size_t R1, size_t C1, size_t C2, typename T>
Mat<R1, C2, T> operator*(const Mat<R1, C1, T>& lhs, const Mat<C1, C2, T>& rhs) 
{
	Mat<R1, C2, T> result;
	for (size_t i = R1; i--; )
		for (size_t j = C2; j--; result[i][j] = lhs[i] * rhs.Col(j));
	return result;
}

template<size_t DimRows, size_t DimCols, typename T>
Mat<DimCols, DimRows, T> operator/(Mat<DimRows, DimCols, T> lhs, const T& rhs) 
{
	for (size_t i = DimRows; i--; lhs[i] = lhs[i] / rhs);
	return lhs;
}

template <size_t DimRows, size_t DimCols, class T>
std::ostream& operator<<(std::ostream& out, Mat<DimRows, DimCols, T>& m) 
{
	for (size_t i = 0; i < DimRows; i++) out << m[i] << std::endl;
	return out;
}

//---------------------------------------------------------------------------------------------------------------------

typedef Vec<2, float> Vec2f;
typedef Vec<2, int>   Vec2i;
typedef Vec<3, float> Vec3f;
typedef Vec<3, int>   Vec3i;
typedef Vec<4, float> Vec4f;
typedef Mat<4, 4, float> Mat4x4;