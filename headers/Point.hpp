#ifndef _POINT_HPP
#define _POINT_HPP

#include <string>

class Vector;

using namespace std;

class Point {

    public:

        static const Point O;

        float p[4];

        Point();
        Point(const float p[4]);
        Point(const float x, const float y, const float z);
        Point(const Point & point);

        Point & operator=(const Point & point);

        string to_string() const;

        Vector operator-(const Point & point) const;
        Point operator+(const Vector & vector) const;

    private:
        void copy(const float p[4]);
};

#endif