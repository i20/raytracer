#ifndef _INTERSECTION_HPP
#define _INTERSECTION_HPP

//#include "../headers/Object.hpp"
#include "../headers/Vector.hpp"
#include "../headers/Point.hpp"
#include "../headers/Ray.hpp"
#include "../headers/Color.hpp"

using namespace std;

// Circular reference
class Object;

class Intersection {

    public:
        Point point;
        Vector normal;
        Color color; // Color at the intersection point may be different from object.color since texture can be applied
        float t;
        const Object * object;
        const Ray * ray;

        Intersection();
        Intersection(const Point & point, const Vector & normal, const Color & color, const float t, const Object * object, const Ray * ray);
        Intersection(const Intersection & inter);

        Intersection & operator=(const Intersection & inter);
};

#endif