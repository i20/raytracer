#include <cmath>
#include <stdint.h>
#include <sstream>
#include <cstring>


#include <string>

#include "../headers/Vector.hpp"
#include "../headers/Point.hpp"
#include "../headers/Ray.hpp"

#include "../headers/Matrix.hpp"

#include <iostream>

using namespace std;

static const float degrad = M_PI / 180;

Matrix Matrix::IDENTITY() {

    Matrix matrix;

    memset(matrix.m, 0, 16 * sizeof(float));
    memset(matrix.inv, 0, 16 * sizeof(float));

    // 4 iterations
    for (uint8_t i = 0; i < 16; i += 5)
        matrix.inv[i] = matrix.m[i] = 1;

    matrix.invertible = true;

    return matrix;
}

//#TODO Reread
Matrix Matrix::TRANSLATION(const Vector & v) {

    Matrix matrix = Matrix::IDENTITY();

    uint8_t idx = 3;

    for (uint8_t i = 0; i < 3; i++) {

        matrix.inv[idx] = -(matrix.m[idx] = v.v[i]);
        idx += 4;
    }

    matrix.invertible = true;

    return matrix;
}

Matrix Matrix::ROTATION(const Vector & v, const float d) {

    Matrix matrix;

    Vector axis = v.normalize();

    float r = d * degrad;
    float c = cos(r);
    float s = sin(r);
    float x = axis.v[0];
    float y = axis.v[1];
    float z = axis.v[2];
    float q = 1 - c;

    // cos(-x) = cos(x) and sin(-x) = -sin(x)

    float xq = x * q;
    float yq = y * q;
    float zq = z * q;

    float xyq = x * yq;
    float yzq = y * zq;
    float xzq = x * zq;

    float xs = x * s;
    float ys = y * s;
    float zs = z * s;

    // ----------------------------------------------------
    matrix.inv[0] = matrix.m[0] = x * xq + c;

    matrix.inv[1] = xyq + zs; matrix.m[1] = xyq - zs;

    matrix.inv[2] = xzq - ys; matrix.m[2] = xzq + ys;

    matrix.inv[3] = matrix.m[3] = 0;
    // ----------------------------------------------------
    matrix.inv[4] = xyq - zs; matrix.m[4] = xyq + zs;

    matrix.inv[5] = matrix.m[5] = y * yq + c;

    matrix.inv[6] = yzq + xs; matrix.m[6] = yzq - xs;

    matrix.inv[7] = matrix.m[7] = 0;
    // ----------------------------------------------------
    matrix.inv[8] = xzq + ys; matrix.m[8] = xzq - ys;

    matrix.inv[9] = yzq - xs; matrix.m[9] = yzq + xs;

    matrix.inv[10] = matrix.m[10] = z * zq + c;

    matrix.inv[11] = matrix.m[11] = 0;
    // ----------------------------------------------------
    for (uint8_t i = 0; i < 3; i++) {

        uint8_t idx = i + 12;

        matrix.inv[idx] = matrix.m[idx] = 0;
    }

    matrix.inv[15] = matrix.m[15] = 1;
    // ----------------------------------------------------

    matrix.invertible = true;

    return matrix;
}

Matrix Matrix::HOMOTHETY(const Vector & v) {

    Matrix matrix = Matrix::IDENTITY();

    uint8_t idx = 0;

    for (uint8_t i = 0; i < 3; i++) {

        matrix.inv[idx] = 1 / (matrix.m[idx] = v.v[i]);
        idx += 5;
    }

    matrix.invertible = true;

    return matrix;
}

// TODO if det = 0 not invertible
Matrix Matrix::TRANSFER(const Point & op, const Vector & xp, const Vector & yp, const Vector & zp) {

    /*
                          x'  y'  z'  o'

    this->m = PBB' = x    a1  a2  a3  a4
                     y    b1  b2  b3  b4
                     z    g1  g2  g3  g4
                     o    0   0   0   1
    */

    Matrix matrix;

    float a1 = xp.v[0], a2 = yp.v[0], a3 = zp.v[0], a4 = op.p[0];
    float b1 = xp.v[1], b2 = yp.v[1], b3 = zp.v[1], b4 = op.p[1];
    float g1 = xp.v[2], g2 = yp.v[2], g3 = zp.v[2], g4 = op.p[2];

    float det = a1*b2*g3 + a2*b3*g1 + a3*b1*g2 - a1*b3*g2 - a2*b1*g3 - a3*b2*g1;

    // B origin expressed in B'
    Point o(
        (a2*b4*g3 + a3*b2*g4 + a4*b3*g2 - a2*b3*g4 - a3*b4*g2 - a4*b2*g3) / det,
        (a1*b3*g4 + a3*b4*g1 + a4*b1*g3 - a1*b4*g3 - a3*b1*g4 - a4*b3*g1) / det,
        (a1*b4*g2 + a2*b1*g4 + a4*b2*g1 - a1*b2*g4 - a2*b4*g1 - a4*b1*g2) / det
    );

    // B vectors expressed in B'
    Vector x( (b2*g3 - g2*b3)/det, (b3*g1 - g3*b1)/det, (b1*g2 - g1*b2)/det );
    Vector y( (a3*g2 - g3*a2)/det, (a1*g3 - g1*a3)/det, (a2*g1 - g2*a1)/det );
    Vector z( (a2*b3 - b2*a3)/det, (a3*b1 - b3*a1)/det, (a1*b2 - b1*a2)/det );

    uint8_t idx0 = 0;

    for (uint8_t i = 0; i < 4; i++) {

        uint8_t idx1 = idx0;

        matrix.inv[idx1] = x.v[i]; matrix.m[idx1] = xp.v[i];
        idx1++;

        matrix.inv[idx1] = y.v[i]; matrix.m[idx1] = yp.v[i];
        idx1++;

        matrix.inv[idx1] = z.v[i]; matrix.m[idx1] = zp.v[i];
        idx1++;

        matrix.inv[idx1] = o.p[i]; matrix.m[idx1] = op.p[i];

        idx0 += 4;
    }

    matrix.invertible = true;

    return matrix;
}

Matrix Matrix::PROJECTION(const Matrix::projection_type type, const float left, const float right, const float bottom, const float top, const float near, const float far) {

    Matrix matrix;

    float r_l = right - left;
    float n_f = near - far;
    float t_b = top - bottom;

    if (type == Matrix::PROJECTION_PERSPECTIVE) {

        float _2n = -2 * near;

        matrix.m[0] = _2n / r_l;
        matrix.m[1] = 0;
        matrix.m[2] = (right + left) / r_l;
        matrix.m[3] = 0;
        matrix.m[4] = 0;
        matrix.m[5] = _2n / t_b;
        matrix.m[6] = (top + bottom) / t_b;
        matrix.m[7] = 0;
        matrix.m[8] = 0;
        matrix.m[9] = 0;
        matrix.m[10] = (near + far) / n_f;
        matrix.m[11] = _2n * far / n_f;
        matrix.m[12] = 0;
        matrix.m[13] = 0;
        matrix.m[14] = -1;
        matrix.m[15] = 0;
    }

    else if (type == Matrix::PROJECTION_ORTHOGRAPHIC) {

        matrix.m[0] = 2 / r_l;
        matrix.m[1] = 0;
        matrix.m[2] = 0;
        matrix.m[3] = -(right + left) / r_l;
        matrix.m[4] = 0;
        matrix.m[5] = 2 / t_b;
        matrix.m[6] = 0;
        matrix.m[7] = -(top + bottom) / t_b;
        matrix.m[8] = 0;
        matrix.m[9] = 0;
        matrix.m[10] = -2 / n_f;
        matrix.m[11] = -(near + far) / n_f;
        matrix.m[12] = 0;
        matrix.m[13] = 0;
        matrix.m[14] = 0;
        matrix.m[15] = 1;
    }

    matrix.invertible = false;

    return matrix;
}

void Matrix::copy(const Matrix & matrix) {

    memcpy(this->m, matrix.m, 16 * sizeof(float));
    memcpy(this->inv, matrix.inv, 16 * sizeof(float));

    this->invertible = matrix.invertible;
}

void Matrix::mul(float r[4], const float t[4]) const {

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
}

Matrix::Matrix() {}

Matrix::Matrix(const Matrix & matrix) {

    this->copy(matrix);
}

Matrix & Matrix::operator=(const Matrix & matrix) {

    this->copy(matrix);
    return *this;
}

Matrix Matrix::operator*(const Matrix & matrix) const {

    Matrix r;
    r.invertible = this->invertible && matrix.invertible;

    uint8_t idx = 0;

    for(uint8_t i = 0; i < 4; i++){

        for(uint8_t j = 0; j < 4; j++){

            uint8_t jdx = idx + j;

            float v = 0, vinv = 0;

            uint8_t kdx0 = j;

            for(uint8_t k = 0; k < 4; k++){

                uint8_t kdx1 = idx + k;

                v += this->m[kdx1] * matrix.m[kdx0];

                if(r.invertible)
                    vinv += this->inv[kdx1] * matrix.inv[kdx0];

                kdx0 += 4;
            }

            r.m[jdx] = v;
            r.inv[jdx] = vinv;
        }

        idx += 4;
    }

    return r;
}

Vector Matrix::operator*(const Vector & vector) const {

    float v[4];

    this->mul(v, vector.v);

    return Vector(v);
}

Point Matrix::operator*(const Point & point) const {

    float p[4];

    this->mul(p, point.p);

    return Point(p);
}

Ray Matrix::operator*(const Ray & ray) const {

    return Ray(*this * ray.origin, *this * ray.direction, ray.in, ray.level);
}

Matrix Matrix::invert() const {

    if (!this->invertible)
        throw string("Matrix is not invertible");

    Matrix matrix;

    memcpy(matrix.m, this->inv, 16 * sizeof(float));
    memcpy(matrix.inv, this->m, 16 * sizeof(float));

    matrix.invertible = true;

    return matrix;
}

string Matrix::to_string() const {

    stringstream ss;

    ss << "Matrix[";

    uint8_t idx = 0;

    for (uint8_t i = 0; i < 4; i++) {

        ss << "{";

        for(uint8_t j = 0; j < 4; j++)
            ss << this->m[idx + j] << " ";

        ss << "} ";

        idx += 4;
    }

    ss << "]";

    return ss.str();
}