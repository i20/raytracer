#ifndef _MATRIX_HPP
#define _MATRIX_HPP

#include "../headers/Vector.hpp"
#include "../headers/Point.hpp"
#include "../headers/Ray.hpp"

//#define PI 3.14159265359

class Matrix {

    public:

        enum projection_type { PROJECTION_PERSPECTIVE, PROJECTION_ORTHOGRAPHIC };

        static const Matrix I;

        static Matrix IDENTITY();
        static Matrix TRANSLATION(const Vector & v);
        static Matrix ROTATION(const Vector & v, const float d);
        static Matrix HOMOTHETY(const Vector & v);
        static Matrix TRANSFER(const Point & op, const Vector & xp, const Vector & yp, const Vector & zp);
        static Matrix PROJECTION(const Matrix::projection_type type, const float left, const float right, const float bottom, const float top, const float near, const float far);

        float m[16], inv[16];
        bool invertible;

        Matrix operator*(const Matrix & matrix) const;
        Ray operator*(const Ray & ray) const;

        template <class T>
        T operator*(const T & t) const;

        Matrix invert() const;
};

template <class T>
T Matrix::operator*(const T & t) const {

    T r;

    uint8_t idx = 12;
    bool must_div = false;
    float w;

    // Must be done reversed to compute r[3] first
    for (uint8_t i = 3; i >= 0; i--) {

        float v = 0;

        for (uint8_t j = 0; j < 4; j++)
            v += this->m[idx + j] * t[j];

        if (i == 3 && v != 0 && v != 1) {

            must_div = true;
            w = v;
        }

        r[i] = must_div ? v / w : v;

        // If not for will decrease i when i = 0
        if(i == 0) break;

        // Putting this after the above condition avoids idx - 4 when idx = 0 on last loop
        idx -= 4;
    }

    return r;
}

#endif
