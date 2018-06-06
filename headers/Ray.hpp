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

        Ray();
        Ray(const Point & origin, const Vector & direction, const bool in, const uintmax_t level);
        Ray(const Ray & ray);

        Ray & operator=(const Ray & ray);

        string to_string() const;
};

#endif
