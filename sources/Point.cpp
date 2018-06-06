#include <cmath>
#include <cstdint>
#include <cstring>

#include <sstream>
#include <string>

#include "../headers/Vector.hpp"
#include "../headers/Point.hpp"

using namespace std;

/*static*/ const Point Point::O(0, 0, 0);

void Point::copy(const float p[4]) {

    // for (uint8_t i = 0; i < 4; i++)
    //     this->p[i] = p[i];

    memcpy(this->p, p, 4 * sizeof(float));
}

Point::Point() {}

Point::Point(const float p[4]) {

    this->copy(p);
}

Point::Point(const float x, const float y, const float z) {

    this->p[0] = x;
    this->p[1] = y;
    this->p[2] = z;
    this->p[3] = 1;
}

Point::Point(const Point & point) {

    this->copy(point.p);
}

Point & Point::operator=(const Point & point) {

    this->copy(point.p);
    return *this;
}

string Point::to_string() const {

    stringstream ss;
    ss << "Point[x=" << this->p[0] << " y=" << this->p[1] << " z=" << this->p[2] << "]";

    return ss.str();
}

Vector Point::operator-(const Point & point) const {

    float v[4];
    for (uint8_t i = 0; i < 4; i++)
        v[i] = this->p[i] - point.p[i];

    return Vector(v);
}

Point Point::operator+(const Vector & vector) const {

    float p[4];
    for (uint8_t i = 0; i < 4; i++)
        p[i] = this->p[i] + vector.v[i];

    return Point(p);
}
