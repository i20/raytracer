#include <cmath>
#include <cstdint>

#include "../headers/Point.hpp"
#include "../headers/Vector.hpp"

using namespace std;

/*static*/ const Vector Vector::X(1, 0, 0);
/*static*/ const Vector Vector::Y(0, 1, 0);
/*static*/ const Vector Vector::Z(0, 0, 1);

Vector::Vector(const float x, const float y, const float z) {

    (*this)[0] = x;
    (*this)[1] = y;
    (*this)[2] = z;
    (*this)[3] = 0;
}

Vector::Vector(const Point & a, const Point & b) : Vector(b - a) {}

float & Vector::operator[] (const uint8_t i) {
    return this->v[i];
}

const float & Vector::operator[] (const uint8_t i) const {
    return this->v[i];
}

Vector Vector::normalize () const {

    const float norm = this->get_norm();

    return norm == 1 ? *this : *this / norm;
}

float Vector::get_norm () const {

    return sqrt(*this * *this);
}

Vector Vector::operator^ (const Vector & vector) const {

    return Vector(
        (*this)[1] * vector[2] - (*this)[2] * vector[1],
        (*this)[2] * vector[0] - (*this)[0] * vector[2],
        (*this)[0] * vector[1] - (*this)[1] * vector[0]
    );
}

float Vector::operator* (const Vector & vector) const {

    float sum = 0;

    for (uint8_t i = 0; i < 4; i++)
        sum += (*this)[i] * vector[i];

    return sum;
}

Vector Vector::operator+ (const Vector & vector) const {

    Vector v;

    for (uint8_t i = 0; i < 4; i++)
        v[i] = (*this)[i] + vector[i];

    return v;
}

Vector Vector::operator- (const Vector & vector) const {

    Vector v;

    for (uint8_t i = 0; i < 4; i++)
        v[i] = (*this)[i] - vector[i];

    return v;
}

Vector Vector::operator* (const float e) const {

    Vector v;

    for (uint8_t i = 0; i < 4; i++)
        v[i] = (*this)[i] * e;

    return v;
}

Vector Vector::operator/ (const float e) const {

    Vector v;

    for (uint8_t i = 0; i < 4; i++)
        v[i] = (*this)[i] / e;

    return v;
}
