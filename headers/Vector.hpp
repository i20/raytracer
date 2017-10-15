#ifndef _VECTOR_HPP
#define _VECTOR_HPP

#include <string>

class Point;

using namespace std;

//__device__
class Vector {

    public:

        static const Vector X;
        static const Vector Y;
        static const Vector Z;

        float v[4];

        Vector();
        Vector(const float v[4]);
        Vector(const float x, const float y, const float z);
        Vector(const Point & a, const Point & b);
        Vector(const Vector & vector);

        float & operator[] (const uint8_t i);

        Vector & operator=(const Vector & vector);

        Vector normalize() const;
        float get_norm() const;

        Vector operator^(const Vector & vector) const;
        float operator*(const Vector & vector) const;
        Vector operator+(const Vector & vector) const;
        Vector operator-(const Vector & vector) const;

        Vector operator*(const float e) const;
        Vector operator/(const float e) const;

        Vector average(const Vector & vector) const;

        string to_string() const;

    private:
        void copy(const float v[4]);
};

#endif