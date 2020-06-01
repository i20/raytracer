#include <cstdint>
#include <cmath>

#include "../headers/Light.hpp"
#include "../headers/Color.hpp"
#include "../headers/Intersection.hpp"
#include "../headers/Object.hpp"

#include "../headers/AmbientLight.hpp"

using namespace std;

AmbientLight::AmbientLight(const float ir, const float ig, const float ib) : Light(ir, ig, ib) {}

Color AmbientLight::compute_luminosity(const Intersection & inter, const Scene & scene) const {

    Color c;

    for (uint8_t i = 0; i < 3; i++)
        c[i] = min((float)255, inter.object->a[i] * (*this)[i] * inter.color[i]);

    return c;
}

void AmbientLight::emit_photons () const {
    // Do nothing
}
