#pragma once

#include <boost/numeric/ublas/matrix.hpp>

template<typename T>
class Matrix_lib
{
public:
    static boost::numeric::ublas::matrix<T>
    get4DProjectionMatrix(T r, T t, T d, T n, T f)
    {
        boost::numeric::ublas::matrix<T> projection(5, 5);

        projection(0, 0) = n / r;
        projection(0, 1) = 0;
        projection(0, 2) = 0;
        projection(0, 3) = 0;
        projection(0, 4) = 0;

        projection(1, 0) = 0;
        projection(1, 1) = n / t;
        projection(1, 2) = 0;
        projection(1, 3) = 0;
        projection(1, 4) = 0;

        projection(2, 0) = 0;
        projection(2, 1) = 0;
        projection(2, 2) = n / d;
        projection(2, 3) = 0;
        projection(2, 4) = 0;

        projection(3, 0) = 0;
        projection(3, 1) = 0;
        projection(3, 2) = 0;
        projection(3, 3) = -(f + n) / (f - n);
        projection(3, 4) = -(2 * f * n) / (f - n);

        projection(4, 0) = 0;
        projection(4, 1) = 0;
        projection(4, 2) = 0;
        projection(4, 3) = -1;
        projection(4, 4) = 0;

        return projection;
    }

    static boost::numeric::ublas::matrix<T>
    get3DProjectionMatrix(T r, T t, T n, T f)
    {
        boost::numeric::ublas::matrix<T> projection(4, 4);

        projection(0, 0) = n / r;
        projection(0, 1) = 0;
        projection(0, 2) = 0;
        projection(0, 3) = 0;

        projection(1, 0) = 0;
        projection(1, 1) = n / t;
        projection(1, 2) = 0;
        projection(1, 3) = 0;

        projection(2, 0) = 0;
        projection(2, 1) = 0;
        projection(2, 2) = -(f + n) / (f - n);
        projection(2, 3) = -(2 * f * n) / (f - n);

        projection(3, 0) = 0;
        projection(3, 1) = 0;
        projection(3, 2) = -1;
        projection(3, 3) = 0;

        return projection;
    }

    static boost::numeric::ublas::matrix<T>
    getXYRotationMatrix(T angle)
    {
        boost::numeric::ublas::matrix<T> rotation(5, 5);

        rotation(0, 0) = std::cos(angle);
        rotation(0, 1) = std::sin(angle);
        rotation(0, 2) = 0;
        rotation(0, 3) = 0;
        rotation(0, 4) = 0;

        rotation(1, 0) = -std::sin(angle);
        rotation(1, 1) = std::cos(angle);
        rotation(1, 2) = 0;
        rotation(1, 3) = 0;
        rotation(1, 4) = 0;

        rotation(2, 0) = 0;
        rotation(2, 1) = 0;
        rotation(2, 2) = 1;
        rotation(2, 3) = 0;
        rotation(2, 4) = 0;

        rotation(3, 0) = 0;
        rotation(3, 1) = 0;
        rotation(3, 2) = 0;
        rotation(3, 3) = 1;
        rotation(3, 4) = 0;

        rotation(4, 0) = 0;
        rotation(4, 1) = 0;
        rotation(4, 2) = 0;
        rotation(4, 3) = 0;
        rotation(4, 4) = 1;

        return rotation;
    }

    static boost::numeric::ublas::matrix<T>
    getYZRotationMatrix(T angle)
    {
        boost::numeric::ublas::matrix<T> rotation(5, 5);

        rotation(0, 0) = 1;
        rotation(0, 1) = 0;
        rotation(0, 2) = 0;
        rotation(0, 3) = 0;
        rotation(0, 4) = 0;

        rotation(1, 0) = 0;
        rotation(1, 1) = std::cos(angle);
        rotation(1, 2) = std::sin(angle);
        rotation(1, 3) = 0;
        rotation(1, 4) = 0;

        rotation(2, 0) = 0;
        rotation(2, 1) = -std::sin(angle);
        rotation(2, 2) = std::cos(angle);
        rotation(2, 3) = 0;
        rotation(2, 4) = 0;

        rotation(3, 0) = 0;
        rotation(3, 1) = 0;
        rotation(3, 2) = 0;
        rotation(3, 3) = 1;
        rotation(3, 4) = 0;

        rotation(4, 0) = 0;
        rotation(4, 1) = 0;
        rotation(4, 2) = 0;
        rotation(4, 3) = 0;
        rotation(4, 4) = 1;

        return rotation;
    }

    static boost::numeric::ublas::matrix<T>
    getZXRotationMatrix(T angle)
    {
        boost::numeric::ublas::matrix<T> rotation(5, 5);

        rotation(0, 0) = std::cos(angle);
        rotation(0, 1) = 0;
        rotation(0, 2) = -std::sin(angle);
        rotation(0, 3) = 0;
        rotation(0, 4) = 0;

        rotation(1, 0) = 0;
        rotation(1, 1) = 1;
        rotation(1, 2) = 0;
        rotation(1, 3) = 0;
        rotation(1, 4) = 0;

        rotation(2, 0) = std::sin(angle);
        rotation(2, 1) = 0;
        rotation(2, 2) = std::cos(angle);
        rotation(2, 3) = 0;
        rotation(2, 4) = 0;

        rotation(3, 0) = 0;
        rotation(3, 1) = 0;
        rotation(3, 2) = 0;
        rotation(3, 3) = 1;
        rotation(3, 4) = 0;

        rotation(4, 0) = 0;
        rotation(4, 1) = 0;
        rotation(4, 2) = 0;
        rotation(4, 3) = 0;
        rotation(4, 4) = 1;

        return rotation;
    }

    static boost::numeric::ublas::matrix<T>
    getXWRotationMatrix(T angle)
    {
        boost::numeric::ublas::matrix<T> rotation(5, 5);

        rotation(0, 0) = std::cos(angle);
        rotation(0, 1) = 0;
        rotation(0, 2) = 0;
        rotation(0, 3) = std::sin(angle);
        rotation(0, 4) = 0;

        rotation(1, 0) = 0;
        rotation(1, 1) = 1;
        rotation(1, 2) = 0;
        rotation(1, 3) = 0;
        rotation(1, 4) = 0;

        rotation(2, 0) = 0;
        rotation(2, 1) = 0;
        rotation(2, 2) = 1;
        rotation(2, 3) = 0;
        rotation(2, 4) = 0;

        rotation(3, 0) = -std::sin(angle);
        rotation(3, 1) = 0;
        rotation(3, 2) = 0;
        rotation(3, 3) = std::cos(angle);
        rotation(3, 4) = 0;

        rotation(4, 0) = 0;
        rotation(4, 1) = 0;
        rotation(4, 2) = 0;
        rotation(4, 3) = 0;
        rotation(4, 4) = 1;

        return rotation;
    }

    static boost::numeric::ublas::matrix<T>
    getYWRotationMatrix(T angle)
    {
        boost::numeric::ublas::matrix<T> rotation(5, 5);

        rotation(0, 0) = 1;
        rotation(0, 1) = 0;
        rotation(0, 2) = 0;
        rotation(0, 3) = 0;
        rotation(0, 4) = 0;

        rotation(1, 0) = 0;
        rotation(1, 1) = std::cos(angle);
        rotation(1, 2) = 0;
        rotation(1, 3) = -std::sin(angle);
        rotation(1, 4) = 0;

        rotation(2, 0) = 0;
        rotation(2, 1) = 0;
        rotation(2, 2) = 1;
        rotation(2, 3) = 0;
        rotation(2, 4) = 0;

        rotation(3, 0) = 0;
        rotation(3, 1) = std::sin(angle);
        rotation(3, 2) = 0;
        rotation(3, 3) = std::cos(angle);
        rotation(3, 4) = 0;

        rotation(4, 0) = 0;
        rotation(4, 1) = 0;
        rotation(4, 2) = 0;
        rotation(4, 3) = 0;
        rotation(4, 4) = 1;

        return rotation;
    }

    static boost::numeric::ublas::matrix<T>
    getZWRotationMatrix(T angle)
    {
        boost::numeric::ublas::matrix<T> rotation(5, 5);

        rotation(0, 0) = 1;
        rotation(0, 1) = 0;
        rotation(0, 2) = 0;
        rotation(0, 3) = 0;
        rotation(0, 4) = 0;

        rotation(1, 0) = 0;
        rotation(1, 1) = 1;
        rotation(1, 2) = 0;
        rotation(1, 3) = 0;
        rotation(1, 4) = 0;

        rotation(2, 0) = 0;
        rotation(2, 1) = 0;
        rotation(2, 2) = std::cos(angle);
        rotation(2, 3) = -std::sin(angle);
        rotation(2, 4) = 0;

        rotation(3, 0) = 0;
        rotation(3, 1) = 0;
        rotation(3, 2) = std::sin(angle);
        rotation(3, 3) = std::cos(angle);
        rotation(3, 4) = 0;

        rotation(4, 0) = 0;
        rotation(4, 1) = 0;
        rotation(4, 2) = 0;
        rotation(4, 3) = 0;
        rotation(4, 4) = 1;

        return rotation;
    }

    static boost::numeric::ublas::matrix<T>
    getXRotationMatrix(T angle)
    {
        boost::numeric::ublas::matrix<T> rotation(4, 4);

        rotation(0, 0) = 1;
        rotation(0, 1) = 0;
        rotation(0, 2) = 0;
        rotation(0, 3) = 0;

        rotation(1, 0) = 0;
        rotation(1, 1) = std::cos(angle);
        rotation(1, 2) = -std::sin(angle);
        rotation(1, 3) = 0;

        rotation(2, 0) = 0;
        rotation(2, 1) = std::sin(angle);
        rotation(2, 2) = std::cos(angle);
        rotation(2, 3) = 0;

        rotation(3, 0) = 0;
        rotation(3, 1) = 0;
        rotation(3, 2) = 0;
        rotation(3, 3) = 1;

        return rotation;
    }

    static boost::numeric::ublas::matrix<T>
    getYRotationMatrix(T angle)
    {
        boost::numeric::ublas::matrix<T> rotation(4, 4);

        rotation(0, 0) = std::cos(angle);
        rotation(0, 1) = 0;
        rotation(0, 2) = std::sin(angle);
        rotation(0, 3) = 0;

        rotation(1, 0) = 0;
        rotation(1, 1) = 1;
        rotation(1, 2) = 0;
        rotation(1, 3) = 0;

        rotation(2, 0) = -std::sin(angle);
        rotation(2, 1) = 0;
        rotation(2, 2) = std::cos(angle);
        rotation(2, 3) = 0;

        rotation(3, 0) = 0;
        rotation(3, 1) = 0;
        rotation(3, 2) = 0;
        rotation(3, 3) = 1;

        return rotation;
    }

    static boost::numeric::ublas::matrix<T>
    getZRotationMatrix(T angle)
    {
        boost::numeric::ublas::matrix<T> rotation(4, 4);

        rotation(0, 0) = std::cos(angle);
        rotation(0, 1) = -std::sin(angle);
        rotation(0, 2) = 0;
        rotation(0, 3) = 0;

        rotation(1, 0) = std::sin(angle);
        rotation(1, 1) = std::cos(angle);
        rotation(1, 2) = 0;
        rotation(1, 3) = 0;

        rotation(2, 0) = 0;
        rotation(2, 1) = 0;
        rotation(2, 2) = 1;
        rotation(2, 3) = 0;

        rotation(3, 0) = 0;
        rotation(3, 1) = 0;
        rotation(3, 2) = 0;
        rotation(3, 3) = 1;

        return rotation;
    }

    static boost::numeric::ublas::matrix<T>
    getRotationMatrix(T angle, T x, T y, T z)
    {
        T c = std::cos(angle);
        T s = std::sin(angle);

        boost::numeric::ublas::matrix<T> rotation(4, 4);

        T len = static_cast<T>(x) * static_cast<T>(x) +
                static_cast<T>(y) * static_cast<T>(y) +
                static_cast<T>(z) * static_cast<T>(z);

        len = std::sqrt(len);
        x /= len;
        y /= len;
        z /= len;

        T ic = 1.0 - c;
        rotation(0, 0) = x * x * ic + c;
        rotation(1, 0) = x * y * ic - z * s;
        rotation(2, 0) = x * z * ic + y * s;
        rotation(3, 0) = 0.0;
        rotation(0, 1) = y * x * ic + z * s;
        rotation(1, 1) = y * y * ic + c;
        rotation(2, 1) = y * z * ic - x * s;
        rotation(3, 1) = 0.0;
        rotation(0, 2) = x * z * ic - y * s;
        rotation(1, 2) = y * z * ic + x * s;
        rotation(2, 2) = z * z * ic + c;
        rotation(3, 2) = 0.0;
        rotation(0, 3) = 0.0;
        rotation(1, 3) = 0.0;
        rotation(2, 3) = 0.0;
        rotation(3, 3) = 1.0;

        return rotation;
    }
};

typedef Matrix_lib<float> Matrix_lib_f;
