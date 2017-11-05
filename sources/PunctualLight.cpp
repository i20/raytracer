#include <cmath>
#include <stdint.h>

#include <iostream>

#include "../headers/Light.hpp"
#include "../headers/Color.hpp"
#include "../headers/Intersection.hpp"
#include "../headers/Point.hpp"
#include "../headers/Object.hpp"
#include "../headers/Ray.hpp"

#include "../headers/Vector.hpp"
#include "../headers/PunctualLight.hpp"

using namespace std;

PunctualLight::PunctualLight(const float ir, const float ig, const float ib, const Point & position, const float c, const float l, const float q) :
    Light(ir, ig, ib),
    c(c),
    l(l),
    q(q),
    position(position) {}

PunctualLight::PunctualLight(const PunctualLight & light) :
    Light(light),
    c(light.c),
    l(light.l),
    q(light.q),
    position(light.position) {}

PunctualLight & PunctualLight::operator=(const PunctualLight & light) {

    this->copy(light);
    this->c = light.c;
    this->l = light.l;
    this->q = light.q;
    this->position = light.position;
    return *this;
}

Color PunctualLight::compute_luminosity (const Intersection & inter, const Scene & scene) const {

    Vector dir(inter.point, this->position);
    // #TODO how do we know if shadow ray is inside or outside the object (ray can pass through the object)?
    Ray shadow_ray(inter.point, dir, inter.ray->in, inter.ray->level);

    bool does_light = true;

    for (const auto & object_entry : scene.objects) {

        Intersection tmp;

        // Since shadow_ray's origin is on the intersection point compute_intersection(shadow_ray) will always be true on origin object
        // rd * n <= 0 => ray passes through its own object to reach the light

        // NOTE: I once thought about replacing the same object condition (ray.direction * from.normal) with (ray.in) to speed up
        // cause obviously if the shadow ray comes from inside the object it will necessarily intersect with it to reach the light
        // which is most of the time outside BUT what if the light is inside the object?

        // Specific test, handles concave objects (#ALGO 1)
        // tmp.t <= dir.get_norm() : handle case where light is between objects
        if ((object_entry.second == inter.object && shadow_ray.direction * inter.normal <= 0) || (object_entry.second->compute_intersection(tmp, shadow_ray) && tmp.t <= dir.get_norm())) {
            does_light = false;
            break;
        }
    }

    if (!does_light) return Color::BLACK;

    // Diffuse component

    // N = inter.normal
    // L = (inter.point, light.position).normalized
    float d = 0; /* norm pl */
    float scalar_d = 0; /* N * L */
    float pl[3];
    for (uint8_t i = 0; i < 3; i++) {

        float pli = pl[i] = this->position.p[i] - inter.point.p[i];

        d += pli * pli;
        scalar_d += inter.normal.v[i] * pli;
    }
    d = sqrt(d);
    scalar_d /= d; // Normalize L after (pl wasn't normalized)

    // Specular component

    // V = (inter.point, ray.origin).normalized = -ray.direction
    // R = rayr.direction = inter.normal * 2 * scalar_d - L
    float scalarx2 = scalar_d + scalar_d;
    float scalar_s = 0; /* V * R */
    for (uint8_t i = 0; i < 3; i++)
        scalar_s += inter.ray->direction.v[i] * (pl[i] / d - inter.normal.v[i] * scalarx2); // pl ain't normalized

    // float scalar_final = inter.object->d * scalar_d;
    // if (0 < scalar_s)
    //     scalar_final += powf(scalar_s, inter.object->g) * inter.object->s;

    scalar_s = 0 < scalar_s ? powf(scalar_s, inter.object->g) : 0;

    // Be careful when d is small
    float attenuation = 1 / (this->c + this->l * d + this->q * d * d);

    uint8_t c[3];
    for (uint8_t i = 0; i < 3; i++)
        c[i] = min((float)255, this->i[i] * inter.color.c[i] * attenuation * (inter.object->d[i] * scalar_d + inter.object->s[i] * scalar_s));

    return Color(c[0], c[1], c[2]);
}

// void emit_photons () const {

//     uintmax_t n_photons = <the powerfuller the light, the higher the number>;

//     for (uintmax_t i = 0; i < n_photons; i++) {

//         int8_t x, y, z;
//         x = 2 * rand() / (float)RAND_MAX - 1; // -1 <= x <= 1
//         y = 2 * rand() / (float)RAND_MAX - 1; // -1 <= y <= 1
//         z = 2 * rand() / (float)RAND_MAX - 1; // -1 <= z <= 1

//         float photon_power = this->trace_photon(this->position, Vector(x, y, z)) / n_photons;
//     }
// }
