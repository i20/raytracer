#ifndef _POINT_HPP
#define _POINT_HPP

#include <cstdint>

class Vector;

class Point {

    private:

        float p[4];

    public:

        static const Point O;

        Point() = default;
        Point(const float x, const float y, const float z);

        float & operator[] (const uint8_t i);
        const float & operator[] (const uint8_t i) const;

        Vector operator-(const Point & point) const;
        Point operator+(const Vector & vector) const;
};

#endif
