#include <cstdint>

#include "../headers/Vector.hpp"
#include "../headers/Point.hpp"

using namespace std;

/*static*/ const Point Point::O(0, 0, 0);

Point::Point(const float x, const float y, const float z) {

    (*this)[0] = x;
    (*this)[1] = y;
    (*this)[2] = z;
    (*this)[3] = 1;
}

float & Point::operator[] (const uint8_t i) {
    return this->p[i];
}

const float & Point::operator[] (const uint8_t i) const {
    return this->p[i];
}

Vector Point::operator-(const Point & point) const {

    Vector v;

    for (uint8_t i = 0; i < 4; i++)
        v[i] = (*this)[i] - point[i];

    return v;
}

Point Point::operator+(const Vector & vector) const {

    Point p;

    for (uint8_t i = 0; i < 4; i++)
        p[i] = (*this)[i] + vector[i];

    return p;
}
