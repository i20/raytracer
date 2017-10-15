#include <stdint.h>
#include <cmath>

#include "../headers/Light.hpp"
#include "../headers/Color.hpp"
#include "../headers/Intersection.hpp"
#include "../headers/Object.hpp"

#include "../headers/AmbientLight.hpp"

using namespace std;

AmbientLight::AmbientLight(const float ir, const float ig, const float ib) : Light(ir, ig, ib) {}

AmbientLight::AmbientLight(const AmbientLight & light) : Light(light) {}

AmbientLight & AmbientLight::operator=(const AmbientLight & light) {

    this->copy(light);
    return *this;
}

Color AmbientLight::compute_luminosity(const Intersection & inter, const Ray & ray, const Scene & scene) const {

    uint8_t c[3];

    for (uint8_t i = 0; i < 3; i++)
        c[i] = min((float)255, inter.object->a[i] * this->i[i] * inter.color.c[i]);

    return Color(c[0], c[1], c[2]);
}
