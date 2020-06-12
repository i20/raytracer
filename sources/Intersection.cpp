#include "../headers/Object.hpp"
#include "../headers/Vector.hpp"
#include "../headers/Point.hpp"
#include "../headers/Ray.hpp"
#include "../headers/Color.hpp"

#include "../headers/Intersection.hpp"

using namespace std;

Intersection::Intersection() :
    point(0, 0, 0),
    true_normal(0, 0, 0),
    normal(0, 0, 0),
    color(0, 0, 0),
    t(0),
    object(nullptr),
    ray(nullptr) {}
