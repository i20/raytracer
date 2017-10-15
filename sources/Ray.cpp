#include <sstream>

#include <string>

#include "../headers/Point.hpp"
#include "../headers/Vector.hpp"

#include "../headers/Ray.hpp"

using namespace std;

Ray::Ray() : origin(0, 0, 0), direction(0, 0, 0), in(false), level(0) {}

Ray::Ray(const Point & origin, const Vector & direction, const bool in, const uintmax_t level) : origin(origin), direction(direction.normalize()), in(in), level(level) {}

Ray::Ray(const Ray & ray) : origin(ray.origin), direction(ray.direction), in(ray.in), level(ray.level) {}

Ray & Ray::operator=(const Ray & ray) {

    this->origin = ray.origin;
    this->direction = ray.direction;
    this->in = ray.in;
    this->level = ray.level;
    return *this;
}

string Ray::to_string() const {

    stringstream ss;
    ss << "Ray[origin=" << this->origin.to_string() << " direction=" << this->direction.to_string() << "]";

    return ss.str();
}