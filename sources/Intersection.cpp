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
    normal(normal.normalize()),
    color(color),
    t(t),
    object(object),
    ray(ray) {}

Intersection::Intersection(const Intersection & inter) :
    point(inter.point),
    normal(inter.normal),
    color(inter.color),
    t(inter.t),
    object(inter.object),
    ray(inter.ray) {}

Intersection & Intersection::operator=(const Intersection & inter) {

    this->point = inter.point;
    this->normal = inter.normal;
    this->color = inter.color;
    this->t = inter.t;
    this->object = inter.object;
    this->ray = inter.ray;

    return *this;
}