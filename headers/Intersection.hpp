#ifndef _INTERSECTION_HPP
#define _INTERSECTION_HPP

//#include "../headers/Object.hpp"
#include "../headers/Vector.hpp"
#include "../headers/Point.hpp"
#include "../headers/Ray.hpp"
#include "../headers/Color.hpp"

// Circular reference
class Object;

class Intersection {

    public:
        Point point;
        Vector true_normal; // normal to true object geometry (no bump mapping nor shading), normalized
        Vector normal; // normalized
        Color color; // Color at the intersection point may be different from object.color since texture can be applied
        float t;
        const Object * object;
        const Ray * ray;
};

#endif
