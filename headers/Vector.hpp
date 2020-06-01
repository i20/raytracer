#ifndef _VECTOR_HPP
#define _VECTOR_HPP

#include <cstdint>

class Point;

//__device__
class Vector {

    private:

        float v[4];

    public:

        static const Vector X;
        static const Vector Y;
        static const Vector Z;

        Vector() = default;
        Vector(const float x, const float y, const float z);
        Vector(const Point & a, const Point & b);

        float & operator[] (const uint8_t i);
        const float & operator[] (const uint8_t i) const;

        Vector normalize() const;
        float get_norm() const;

        Vector operator^(const Vector & vector) const;
        float operator*(const Vector & vector) const;
        Vector operator+(const Vector & vector) const;
        Vector operator-(const Vector & vector) const;

        Vector operator*(const float e) const;
        Vector operator/(const float e) const;
};

#endif
