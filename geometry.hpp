#pragma once

#include <cmath>
#include <vector>
#include <cassert>
#include <iostream>

template <size_t DIM, typename T> struct vec {
    vec() { for (size_t i=DIM; i--; data_[i] = T()); }
    T& operator[](const size_t i)       { assert(i<DIM); return data_[i]; }
    const T& operator[](const size_t i) const { assert(i<DIM); return data_[i]; }
private:
    T data_[DIM];
};

/////////////////////////////////////////////////////////////////////////////////

template <typename T> struct vec<2,T> {
    vec() : x(T()), y(T()) {}
    vec(T X, T Y) : x(X), y(Y) {}
    template <class U> vec<2,T>(const vec<2,U> &v);
          T& operator[](const size_t i)       { assert(i<2); return i<=0 ? x : y; }
    const T& operator[](const size_t i) const { assert(i<2); return i<=0 ? x : y; }

    T x,y;
};

/////////////////////////////////////////////////////////////////////////////////

template <typename T> struct vec<3,T> {
    vec() : x(T()), y(T()), z(T()) {}
    vec(T X, T Y, T Z) : x(X), y(Y), z(Z) {}
    template <class U> vec<3,T>(const vec<3,U> &v);
          T& operator[](const size_t i)       { assert(i<3); return i<=0 ? x : (1==i ? y : z); }
    const T& operator[](const size_t i) const { assert(i<3); return i<=0 ? x : (1==i ? y : z); }
    float norm() { return std::sqrt(x*x+y*y+z*z); }
    vec<3,T> & normalize(T l=1) { *this = (*this)*(l/norm()); return *this; }

    T x,y,z;
};

/////////////////////////////////////////////////////////////////////////////////

typedef vec<2,  float> Vec2f;
typedef vec<2,  int>   Vec2i;
typedef vec<3,  float> Vec3f;
typedef vec<3,  int>   Vec3i;


/////////////////////////////////////////////////////////////////////////////////

template <typename T> struct vec<4,T> {
    vec() : x(T()), y(T()), z(T()), w(T()) {}
    vec(T X, T Y, T Z, T W) : x(X), y(Y), z(Z), w(W) {}
    vec(Vec3f input) : x(input.x), y(input.y), z(input.z), w(1) {}
    template <class U> vec<4,T>(const vec<4,U> &v);
          T& operator[](const size_t i)       { assert(i<4); return i<=0 ? x : (1==i ? y : (i == 2 ? z : w)); }
    const T& operator[](const size_t i) const { assert(i<4); return i<=0 ? x : (1==i ? y : (i == 2 ? z : w)); }
    float norm() { return std::sqrt(x*x + y*y + z*z + w*w); }
    vec<4,T> & normalize(T l=1) { *this = (*this)*(l/norm()); return *this; }
    Vec3f xyz() { return Vec3f(x, y, z); }

    T x,y,z, w;
};

typedef vec<4,  float> Vec4f;

/////////////////////////////////////////////////////////////////////////////////

template<size_t DIM,typename T> T operator*(const vec<DIM,T>& lhs, const vec<DIM,T>& rhs) {
    T ret = T();
    for (size_t i=DIM; i--; ret+=lhs[i]*rhs[i]);
    return ret;
}


template<size_t DIM,typename T>vec<DIM,T> operator+(vec<DIM,T> lhs, const vec<DIM,T>& rhs) {
    for (size_t i=DIM; i--; lhs[i]+=rhs[i]);
    return lhs;
}

template<size_t DIM,typename T>vec<DIM,T> operator-(vec<DIM,T> lhs, const vec<DIM,T>& rhs) {
    for (size_t i=DIM; i--; lhs[i]-=rhs[i]);
    return lhs;
}

template<size_t DIM,typename T,typename U> vec<DIM,T> operator*(vec<DIM,T> lhs, const U& rhs) {
    for (size_t i=DIM; i--; lhs[i]*=rhs);
    return lhs;
}

template<size_t DIM,typename T,typename U> vec<DIM,T> operator/(vec<DIM,T> lhs, const U& rhs) {
    for (size_t i=DIM; i--; lhs[i]/=rhs);
    return lhs;
}

template<size_t LEN,size_t DIM,typename T> vec<LEN,T> embed(const vec<DIM,T> &v, T fill=1) {
    vec<LEN,T> ret;
    for (size_t i=LEN; i--; ret[i]=(i<DIM?v[i]:fill));
    return ret;
}

template<size_t LEN,size_t DIM, typename T> vec<LEN,T> proj(const vec<DIM,T> &v) {
    vec<LEN,T> ret;
    for (size_t i=LEN; i--; ret[i]=v[i]);
    return ret;
}

template <typename T> vec<3,T> cross(vec<3,T> v1, vec<3,T> v2) {
    return vec<3,T>(v1.y*v2.z - v1.z*v2.y, v1.z*v2.x - v1.x*v2.z, v1.x*v2.y - v1.y*v2.x);
}

template <size_t DIM, typename T> std::ostream& operator<<(std::ostream& out, vec<DIM,T>& v) {
    for(unsigned int i=0; i<DIM; i++) {
        out << v[i] << " " ;
    }
    return out ;
}

/////////////////////////////////////////////////////////////////////////////////

const int DEFAULT_ALLOC=4;

class Matrix {
    std::vector<std::vector<float> > m;
    int rows, cols;
public:
    Matrix(int r=DEFAULT_ALLOC, int c=DEFAULT_ALLOC) {
        rows = r;
        cols = c;
        //m = std::vector<std::vector<float> >(r, std::vector<float>(c, 0.f)))
        m.resize(rows, std::vector<float>(cols, 0.0f));
    }

    inline int nrows() { return rows; }
    inline int ncols() { return cols; }

    static Matrix identity(int dimensions) {
        Matrix E(dimensions, dimensions);
        for (int i=0; i<dimensions; i++) {
            for (int j=0; j<dimensions; j++) {
                E[i][j] = (i==j ? 1.f : 0.f);
            }
        }
        return E;
    }

    std::vector<float>& operator[](const int i) {
        assert(i>=0 && i<rows);
        return m[i];
    }

    Matrix operator*(const Matrix& a) {
        assert(cols == a.rows);
        Matrix result(rows, a.cols);
        for (int i=0; i<rows; i++) {
            for (int j=0; j<a.cols; j++) {
                result.m[i][j] = 0.0f;
                for (int k=0; k<cols; k++) {
                    result.m[i][j] += m[i][k] * a.m[k][j];
                }
            }
        }
        return result;
    }

    Vec4f operator*(const Vec4f& a) {
        // assert(cols == a.rows);
        Vec4f retval;
        for (int i = 0; i < 4; i++) {
            float sum = 0;
            for (int j = 0; j < 4; j++) {
                sum += m[i][j] * a[i];
            }
            retval[i] = sum;
        }

        return retval;
    }

    Matrix transpose() {
        Matrix result(cols, rows);
        for(int i=0; i<rows; i++)
            for(int j=0; j<cols; j++)
                result[j][i] = m[i][j];
        return result;
    }

    Matrix inverse() {
        assert(rows==cols);
        // augmenting the square matrix with the identity matrix of the same dimensions a => [ai]
        Matrix result(rows, cols*2);
        for(int i=0; i<rows; i++)
            for(int j=0; j<cols; j++)
                result[i][j] = m[i][j];
        for(int i=0; i<rows; i++)
            result[i][i+cols] = 1;
        // first pass
        for (int i=0; i<rows-1; i++) {
            // normalize the first row
            for(int j=result.cols-1; j>=0; j--)
                result[i][j] /= result[i][i];
            for (int k=i+1; k<rows; k++) {
                float coeff = result[k][i];
                for (int j=0; j<result.cols; j++) {
                    result[k][j] -= result[i][j]*coeff;
                }
            }
        }
        // normalize the last row
        for(int j=result.cols-1; j>=rows-1; j--)
            result[rows-1][j] /= result[rows-1][rows-1];
        // second pass
        for (int i=rows-1; i>0; i--) {
            for (int k=i-1; k>=0; k--) {
                float coeff = result[k][i];
                for (int j=0; j<result.cols; j++) {
                    result[k][j] -= result[i][j]*coeff;
                }
            }
        }
        // cut the identity matrix back
        Matrix truncate(rows, cols);
        for(int i=0; i<rows; i++)
            for(int j=0; j<cols; j++)
                truncate[i][j] = result[i][j+cols];
        return truncate;
    }

    friend std::ostream& operator<<(std::ostream& s, Matrix& m) {
        for (int i = 0; i < m.rows; i++) {
            for (int j = 0; j < m.cols; j++) {
                s << m[i][j] << "\t\t";
            }
            s << std::endl;
        }
        return s;
    }
};

/////////////////////////////////////////////////////////////////////////////////
