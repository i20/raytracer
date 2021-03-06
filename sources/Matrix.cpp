#include <cmath>
#include <cassert>
#include <cstdint>
#include <cstring> // memset

#include "../headers/Vector.hpp"
#include "../headers/Point.hpp"
#include "../headers/Ray.hpp"

#include "../headers/Matrix.hpp"

using namespace std;

static const float degrad = M_PI / 180;

/*static*/ const Matrix Matrix::I = Matrix::IDENTITY();

/*static*/ Matrix Matrix::IDENTITY() {

    Matrix matrix;

    memset(matrix.m, 0, 16 * sizeof(float));
    memset(matrix.inv, 0, 16 * sizeof(float));

    // 4 iterations
    for (uint8_t i = 0; i < 16; i += 5)
        matrix.inv[i] = matrix.m[i] = 1;

    matrix.invertible = true;

    return matrix;
}

/*static*/ Matrix Matrix::TRANSLATION(const Vector & v) {

    Matrix matrix = Matrix::I;

    uint8_t idx = 3;

    for (uint8_t i = 0; i < 3; i++) {

        matrix.inv[idx] = -(matrix.m[idx] = v[i]);
        idx += 4;
    }

    matrix.invertible = true;

    return matrix;
}

/*static*/ Matrix Matrix::ROTATION(const Vector & v, const float d) {

    Matrix matrix;

    const Vector axis = v.normalize();

    const float r = d * degrad;
    const float c = cos(r);
    const float s = sin(r);
    const float x = axis[0];
    const float y = axis[1];
    const float z = axis[2];
    const float q = 1 - c;

    // cos(-x) = cos(x) and sin(-x) = -sin(x)

    const float xq = x * q;
    const float yq = y * q;
    const float zq = z * q;

    const float xyq = x * yq;
    const float yzq = y * zq;
    const float xzq = x * zq;

    const float xs = x * s;
    const float ys = y * s;
    const float zs = z * s;

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

        const uint8_t idx = i + 12;

        matrix.inv[idx] = matrix.m[idx] = 0;
    }

    matrix.inv[15] = matrix.m[15] = 1;
    // ----------------------------------------------------

    matrix.invertible = true;

    return matrix;
}

/*static*/ Matrix Matrix::HOMOTHETY(const Vector & v) {

    Matrix matrix = Matrix::I;

    uint8_t idx = 0;

    for (uint8_t i = 0; i < 3; i++) {

        matrix.inv[idx] = 1 / (matrix.m[idx] = v[i]);
        idx += 5;
    }

    matrix.invertible = true;

    return matrix;
}

// TODO if det = 0 not invertible
/*static*/ Matrix Matrix::TRANSFER(const Point & op, const Vector & xp, const Vector & yp, const Vector & zp) {

    /*
                          x'  y'  z'  o'

    this->m = PBB' = x    a1  a2  a3  a4
                     y    b1  b2  b3  b4
                     z    g1  g2  g3  g4
                     o    0   0   0   1
    */

    Matrix matrix;

    const float a1 = xp[0], a2 = yp[0], a3 = zp[0], a4 = op[0];
    const float b1 = xp[1], b2 = yp[1], b3 = zp[1], b4 = op[1];
    const float g1 = xp[2], g2 = yp[2], g3 = zp[2], g4 = op[2];

    const float det = a1*b2*g3 + a2*b3*g1 + a3*b1*g2 - a1*b3*g2 - a2*b1*g3 - a3*b2*g1;

    // B origin expressed in B'
    const Point o(
        (a2*b4*g3 + a3*b2*g4 + a4*b3*g2 - a2*b3*g4 - a3*b4*g2 - a4*b2*g3) / det,
        (a1*b3*g4 + a3*b4*g1 + a4*b1*g3 - a1*b4*g3 - a3*b1*g4 - a4*b3*g1) / det,
        (a1*b4*g2 + a2*b1*g4 + a4*b2*g1 - a1*b2*g4 - a2*b4*g1 - a4*b1*g2) / det
    );

    // B vectors expressed in B'
    const Vector x( (b2*g3 - g2*b3)/det, (b3*g1 - g3*b1)/det, (b1*g2 - g1*b2)/det );
    const Vector y( (a3*g2 - g3*a2)/det, (a1*g3 - g1*a3)/det, (a2*g1 - g2*a1)/det );
    const Vector z( (a2*b3 - b2*a3)/det, (a3*b1 - b3*a1)/det, (a1*b2 - b1*a2)/det );

    uint8_t idx0 = 0;

    for (uint8_t i = 0; i < 4; i++) {

        uint8_t idx1 = idx0;

        matrix.inv[idx1] = x[i]; matrix.m[idx1] = xp[i];
        idx1++;

        matrix.inv[idx1] = y[i]; matrix.m[idx1] = yp[i];
        idx1++;

        matrix.inv[idx1] = z[i]; matrix.m[idx1] = zp[i];
        idx1++;

        matrix.inv[idx1] = o[i]; matrix.m[idx1] = op[i];

        idx0 += 4;
    }

    matrix.invertible = true;

    return matrix;
}

/*static*/ Matrix Matrix::PROJECTION(const Matrix::projection_type type, const float left, const float right, const float bottom, const float top, const float near, const float far) {

    Matrix matrix;

    const float r_l = right - left;
    const float n_f = near - far;
    const float t_b = top - bottom;

    if (type == Matrix::PROJECTION_PERSPECTIVE) {

        const float _2n = -2 * near;

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

Matrix Matrix::operator*(const Matrix & matrix) const {

    Matrix r;
    r.invertible = this->invertible && matrix.invertible;

    uint8_t idx = 0;

    for(uint8_t i = 0; i < 4; i++){

        for(uint8_t j = 0; j < 4; j++){

            const uint8_t jdx = idx + j;

            float v = 0, vinv = 0;

            uint8_t kdx0 = j;

            for(uint8_t k = 0; k < 4; k++){

                const uint8_t kdx1 = idx + k;

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

Ray Matrix::operator*(const Ray & ray) const {

    return Ray(*this * ray.origin, *this * ray.direction, ray.in, ray.level, ray.norm);
}

Matrix Matrix::invert() const {

    assert(this->invertible);

    Matrix matrix;

    memcpy(matrix.m, this->inv, 16 * sizeof(float));
    memcpy(matrix.inv, this->m, 16 * sizeof(float));

    matrix.invertible = true;

    return matrix;
}
