#ifndef _MATRIX_HPP
#define _MATRIX_HPP

#include "../headers/Vector.hpp"
#include "../headers/Point.hpp"
#include "../headers/Ray.hpp"

#include <string>

using namespace std;

//#define PI 3.14159265359

class Matrix {

    public:

        enum projection_type { PROJECTION_PERSPECTIVE, PROJECTION_ORTHOGRAPHIC };

        static Matrix IDENTITY();
        static Matrix TRANSLATION(const Vector & v);
        static Matrix ROTATION(const Vector & v, const float d);
        static Matrix HOMOTHETY(const Vector & v);
        static Matrix TRANSFER(const Point & op, const Vector & xp, const Vector & yp, const Vector & zp);
        static Matrix PROJECTION(const Matrix::projection_type type, const float left, const float right, const float bottom, const float top, const float near, const float far);

        float m[16], inv[16];
        bool invertible;

        Matrix();
        Matrix(const Matrix & matrix);

        Matrix & operator=(const Matrix & matrix);

        Matrix operator*(const Matrix & matrix) const;
        Vector operator*(const Vector & vector) const;
        Point operator*(const Point & point) const;
        Ray operator*(const Ray & ray) const;

        Matrix invert() const;

        string to_string() const;

    private:
        void copy(const Matrix & matrix);
        void mul(float r[4], const float t[4]) const;
};

#endif