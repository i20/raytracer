#include <cmath>
#include <cstdint>
#include <cstring>

#include <sstream>
#include <string>

#include "../headers/Point.hpp"
#include "../headers/Vector.hpp"

using namespace std;

/*static*/ const Vector Vector::X(1, 0, 0);
/*static*/ const Vector Vector::Y(0, 1, 0);
/*static*/ const Vector Vector::Z(0, 0, 1);

void Vector::copy(const float v[4]) {

    // for (uint8_t i = 0; i < 4; i++)
    //     this->v[i] = v[i];

    memcpy(this->v, v, 4 * sizeof(float));
}

Vector::Vector() {}

Vector::Vector(const float v[4]) {

    this->copy(v);
}

Vector::Vector(const float x, const float y, const float z) {

    this->v[0] = x;
    this->v[1] = y;
    this->v[2] = z;
    this->v[3] = 0;
}

Vector::Vector(const Point & a, const Point & b) {

    this->copy((b - a).v);
}

Vector::Vector(const Vector & vector) {

    this->copy(vector.v);
}

float & Vector::operator[] (const uint8_t i) {

    return this->v[i];
}

Vector & Vector::operator=(const Vector & vector) {

    this->copy(vector.v);
    return *this;
}

Vector Vector::normalize() const {

    float norm = this->get_norm();

    return norm == 1 ? *this : *this / norm;
}

float Vector::get_norm() const {

    return sqrt(*this * *this);
}

Vector Vector::operator^(const Vector & vector) const {

    return Vector(

        this->v[1] * vector.v[2] - this->v[2] * vector.v[1],
        this->v[2] * vector.v[0] - this->v[0] * vector.v[2],
        this->v[0] * vector.v[1] - this->v[1] * vector.v[0]
    );
}

float Vector::operator*(const Vector & vector) const {

    float sum = 0;

    for (uint8_t i = 0; i < 4; i++)
        sum += this->v[i] * vector.v[i];

    return sum;
}

Vector Vector::operator+(const Vector & vector) const {

    float v[4];

    for (uint8_t i = 0; i < 4; i++)
        v[i] = this->v[i] + vector.v[i];

    return Vector(v);
}

Vector Vector::operator-(const Vector & vector) const {

    float v[4];

    for (uint8_t i = 0; i < 4; i++)
        v[i] = this->v[i] - vector.v[i];

    return Vector(v);
}

Vector Vector::operator*(const float e) const {

    float v[4];

    for (uint8_t i = 0; i < 4; i++)
        v[i] = this->v[i] * e;

    return Vector(v);
}

Vector Vector::operator/(const float e) const {

    float v[4];

    for (uint8_t i = 0; i < 4; i++)
        v[i] = this->v[i] / e;

    return Vector(v);
}

Vector Vector::average(const Vector & vector) const {

    float v[4];

    for (uint8_t i = 0; i < 4; i++)
        v[i] = (this->v[i] + vector.v[i]) / 2;

    return Vector(v);
}

string Vector::to_string() const {

    stringstream ss;
    ss << "Vector[x=" << this->v[0] << " y=" << this->v[1] << " z=" << this->v[2] << "]";

    return ss.str();
}
