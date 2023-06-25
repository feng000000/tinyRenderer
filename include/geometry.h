#pragma once
#include <cmath>
#include <vector>
#include <iostream>
#include <cassert>

template <size_t DimCols, size_t DimRows, typename T> class Matrix;

// --------------- vector --------------- //

template <size_t DIM, typename T>
class Vec
{
public:
    Vec()
    {
        for (size_t i = 0; i < DIM; i ++)
            data_[i] = T();
    }

    T& operator[] (const size_t i)
    {
        assert(i < DIM);
        return data_[i];
    }

    const T& operator[] (const size_t i) const
    {
        assert(i < DIM);
        return data_[i];
    }

private:
    T data_[DIM];
};


template <typename T>
class Vec<2, T>
{
public:
    T x, y;
    Vec() : x(0), y(0) {}

    Vec(T x, T y) : x(x), y(y) {}

    template <typename U> Vec<2,T>(const Vec<2,U> &v);

    T& operator[] (const size_t i)
    {
        assert(i >= 0 && i < 2);
        return i == 0 ? x : y;
    }

    const T& operator[] (const size_t i) const
    {
        assert(i >= 0 && i < 2);
        return i == 0 ? x : y;
    }
};


template <typename T>
class Vec<3, T>
{
public:
    T x, y, z;
	Vec() : x(0), y(0), z(0) {}

    Vec(T x, T y, T z) : x(x), y(y), z(z) {}

    template <typename U> Vec<3, T>(const Vec<3,U> &v);

    T& operator[] (const size_t i)
    {
        assert(i >= 0 && i < 3);
        return i == 0 ? x : (i == 1 ? y : z);
    }

    const T& operator[] (const size_t i) const
    {
        assert(i >= 0 && i < 3);
        return i == 0 ? x : (i == 1 ? y : z);
    }

    Vec<3, T> operator^ (const Vec<3, T> v) const
    {
        return Vec<3, T>(
            y * v.z - z * v.y,
            z * v.x - x * v.z,
            x * v.y - y * v.x);
    }

    float norm()
    {
        return std::sqrt(x * x + y * y + z * z);
    }

    Vec<3, T>& normalize(T l = 1)
    {
        *this = (*this) * (l / norm());
        return *this;
    }
};

template <size_t DIM, typename T>
Vec<DIM, T> operator+ (const Vec<DIM, T> &lhs, const Vec<DIM, T> &rhs)
{
	Vec<DIM, T> res;
	for (size_t i = 0; i < DIM; i ++)
		res[i] = lhs[i] + rhs[i];
	return res;
}

template <size_t DIM, typename T>
Vec<DIM, T> operator- (const Vec<DIM, T> &lhs, const Vec<DIM, T> &rhs)
{
	Vec<DIM, T> res;
	for (size_t i = 0; i < DIM; i ++)
		res[i] = lhs[i] - rhs[i];
	return res;
}

template <size_t DIM, typename T>
T operator* (const Vec<DIM, T> &lhs, const Vec<DIM, T> &rhs)
{
	T ret = T();
	for (size_t i = 0; i < DIM; i ++)
		ret += lhs[i] * rhs[i];
	return ret;
}

template <size_t DIM, typename T, typename U>
Vec<DIM, T> operator* (const Vec<DIM, T> &lhs, const U &rhs)
{
	Vec<DIM, T> res;
	for (size_t i = 0; i < DIM; i ++)
		res[i] = lhs[i] * rhs;
	return res;
}

template <size_t DIM, typename T, typename U>
Vec<DIM, T> operator/ (const Vec<DIM, T> lhs, const U &rhs)
{
	Vec<DIM, T> res;
	for (size_t i = 0; i < DIM; i ++)
		res[i] = lhs[i] / rhs;
	return res;
}

// Convert DIM-dimensional vectors to LEN-dimensional vectors, fill with variable "fill"
template <size_t LEN, size_t DIM, typename T>
Vec<LEN, T> embed(Vec<DIM, T> lhs, T fill = 1)
{
	Vec<LEN, T> res;
	for (size_t i = 0; i < LEN; i ++)
		res[i] = (i < DIM ? lhs[i] : fill);
	return res;
}

// Project DIM-dimensional vector to LEN-dimensional
template <size_t LEN, size_t DIM, typename T>
Vec<LEN, T> proj(Vec<DIM, T> lhs)
{
	Vec<LEN, T> res;
	for (size_t i = 0; i < LEN; i ++)
		res[i] = lhs[i];
	return res;
}

template <size_t DIM, typename T>
std::ostream &operator<< (std::ostream &out, Vec<DIM, T> &v)
{
    for (unsigned int i = 0; i < DIM; i++)
    {
        out << v[i] << " ";
    }
    return out;
}


// --------------- determinant ---------------//

template <size_t DIM, typename T>
struct Determinant
{
    static T det(const Matrix<DIM, DIM, T> &src)
    {
        T res = T();
        for (size_t i = 0; i < DIM; i ++)
            res += src[0][1] * src.cofactor(0, i);
        return res;
    }
};

template <typename T>
struct Determinant<1, T>
{
    static T det(const Matrix<1, 1, T> &src)
    {
        return src[0][0];
    }
};


// --------------- matrix ---------------//

// 索引:        可以通过 operator[] 访问特定行（vec<DimCols, T>）。
// 列提取:      通过 col 方法获取指定列的矢量。
// 列设置:      通过 set_col 方法将特定向量设置为指定列的值。
// 单位矩阵:    通过静态方法 identity 获取大小为 DimRows x DimCols 的单位矩阵。
// 行列式:      通过 det 方法，获取该矩阵的行列式值。
// 余子式:      通过静态方法 get_minor 获得该矩阵的一个次矩阵。
// 代数余子式:  通过 cofactor 方法获取该矩阵的代数余子式值。
// 伴随矩阵:    通过 adjugate 方法获得该矩阵的伴随矩阵。
// 逆转置矩阵:  通过 invert_transpose 方法获取该矩阵的逆转置矩阵。

template <size_t DimRows, size_t DimCols, typename T>
class Matrix
{
public:
    Matrix() {}

    Vec<DimCols, T> &operator[] (const size_t idx)
    {
        assert(idx >= 0 && idx < DimRows);
        return rows[idx];
    }

    const Vec<DimCols, T> &operator[] (const size_t idx) const
    {
        assert(idx >= 0 && idx < DimRows);
        return rows[idx];
    }

    Vec<DimRows, T> col(const size_t idx) const
    {
        assert(idx >= 0 && idx < DimCols);
        Vec<DimRows, T> res;
        for (size_t i = 0; i < DimRows; i ++)
            res[i] = rows[i][idx];
        return res;
    }

    void set_col(size_t idx, Vec<DimRows, T> v)
    {
        assert(idx >= 0 && idx < DimCols);
        for (size_t i = 0; i < DimRows; i ++)
            rows[i][idx] = v[i];
    }

    static Matrix<DimRows, DimCols, T> identity()
    {
        Matrix<DimRows, DimCols, T> res;
        for (size_t i = 0; i < DimRows; i ++)
            for (size_t j = 0; j < DimCols; j ++)
                res[i][j] = (i == j);
        return res;
    }

    T det() const
    {
        return Determinant<DimCols, T>::det(*this);
    }

    // --------------- ??? --------------- //
    Matrix<DimRows - 1, DimCols - 1, T> get_minor(size_t row, size_t col) const
    {
        Matrix<DimRows - 1, DimCols - 1, T> res;
        for (size_t i = 0; i < DimRows - 1; i ++)
            for (size_t j = 0; j < DimCols - 1; j ++)
                res[i][j] = rows[i < row ? i : i + 1][j < col ? j : j + 1];
         return res;
    }

    T cofactor(size_t row, size_t col) const
    {
        return get_minor(row, col).det() * ((row + col) % 2 ? -1 : 1);
    }

    Matrix<DimRows, DimCols, T> adjugate() const
    {
        Matrix<DimRows, DimCols, T> ret;
        for (size_t i = DimRows; i--;)
            for (size_t j = DimCols; j--; ret[i][j] = cofactor(i, j))
                ;
        return ret;
    }

    Matrix<DimRows, DimCols, T> invert_transpose()
    {
        Matrix<DimRows, DimCols, T> ret = adjugate();
        T tmp = ret[0] * rows[0];
        return ret / tmp;
    }

private:
    Vec<DimCols, T> rows[DimRows];
};

template <size_t DimRows, size_t DimCols, typename T>
Vec<DimRows, T> operator* (const Matrix<DimRows, DimCols, T> &lhs, const Vec<DimCols, T> &rhs)
{
    Vec<DimRows, T> res;
	for (size_t i = 0; i < DimRows; i ++)
		res[i] = lhs[i] * rhs;
    return res;
}

template <size_t R1, size_t C1, size_t C2, typename T>
Matrix<R1, C2, T> operator* (const Matrix<R1, C1, T> &lhs, const Matrix<C1, C2, T> &rhs)
{
    Matrix<R1, C2, T> res;
	for (size_t i = 0; i < R1; i ++)
		for (size_t j = 0; j < C2; j ++)
			res[i][j] = lhs[i] * rhs.col(j);
    return res;
}

template <size_t DimRows, size_t DimCols, typename T>
Matrix<DimCols, DimRows, T> operator/ (Matrix<DimRows, DimCols, T> lhs, const T &rhs)
{
	for (size_t i = 0; i < DimRows; i ++)
		lhs[i] = lhs[i] / rhs;
    return lhs;
}

template <size_t DimRows, size_t DimCols, class T>
std::ostream &operator<< (std::ostream &out, Matrix<DimRows, DimCols, T> &m)
{
    for (size_t i = 0; i < DimRows; i++)
        out << m[i] << std::endl;
    return out;
}


using Vec2f     = Vec<2, float>;
using Vec2i     = Vec<2, int>;
using Vec3f     = Vec<3, float>;
using Vec3i     = Vec<3, int>;
using Vec4f     = Vec<4, float>;
using Matrix4f  = Matrix<4, 4, float>;