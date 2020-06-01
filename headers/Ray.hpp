#ifndef _RAY_HPP
#define _RAY_HPP

#include <cstdint>

#include "../headers/Point.hpp"
#include "../headers/Vector.hpp"

//__device__
class Ray {

    public:
        Point origin;
        Vector direction;
        bool in;
        uintmax_t level;

        // Length of ray if finite as for shadow rays
        // -1 is a good value for "undefined" since a norm is always 0 <= norm
        float norm;

        Ray() = default; // @todo get rid of Ray default constructor (@see "Ray rt, rr;"" in Camera.cpp)
        Ray(const Point & origin, const Vector & direction, const bool in, const uintmax_t level, const float norm = -1);
};

#endif
