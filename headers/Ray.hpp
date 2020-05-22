#ifndef _RAY_HPP
#define _RAY_HPP

#include <cstdint>

#include <string>

#include "../headers/Point.hpp"
#include "../headers/Vector.hpp"

using namespace std;

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

        Ray();
        Ray(const Point & origin, const Vector & direction, const bool in, const uintmax_t level, const float norm = -1);
        Ray(const Ray & ray);

        Ray & operator=(const Ray & ray);

        string to_string() const;
};

#endif
