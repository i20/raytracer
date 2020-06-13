
#include "../headers/Point.hpp"
#include "../headers/Vector.hpp"

#include "../headers/Ray.hpp"

using namespace std;

// @todo Is it necessary to normalize here ? Cannot it be removed ?
Ray::Ray(const Point & origin, const Vector & direction, const bool in, const uintmax_t level, const float norm) :
    origin(origin), direction(direction.normalize()), in(in), level(level), norm(norm) {}
