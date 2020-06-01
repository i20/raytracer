#include "../headers/Object.hpp"
#include "../headers/Vector.hpp"
#include "../headers/Point.hpp"
#include "../headers/Ray.hpp"
#include "../headers/Color.hpp"

#include "../headers/Intersection.hpp"

using namespace std;

Intersection::Intersection() :
    point(0, 0, 0),
    normal(0, 0, 0),
    color(0, 0, 0),
    t(0),
    object(nullptr),
    ray(nullptr) {}

Intersection::Intersection(const Point & point, const Vector & normal, const Color & color, const float t, const Object * object, const Ray * ray) :
    point(point),
    normal(normal),
    color(color),
    t(t),
    object(object),
    ray(ray) {}
