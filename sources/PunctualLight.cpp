#include <cmath>
#include <cstdint>
#include <cstdlib>

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

void PunctualLight::emit_photons () const {

    // uintmax_t n_photons = <the powerfuller the light, the higher the number>;

    // for (uintmax_t i = 0; i < n_photons; i++) {

    //     // Choose a random direction across a ball (punctual light)
    //     // The while condition allow to have a uniform distribution across a ball instead of a cube
    //     // Otherwise, after direction normalization we would have more chances to have directions toward the cube's corners
    //     float x, y, z;
    //     do {
    //         x = 2 * rand() / (float)RAND_MAX - 1; // -1 <= x <= 1
    //         y = 2 * rand() / (float)RAND_MAX - 1; // -1 <= y <= 1
    //         z = 2 * rand() / (float)RAND_MAX - 1; // -1 <= z <= 1
    //     } while ( x * x + y * y + z * z > 1 );

    //     float photon_power = this->trace_photon(Photon(this->position, Vector(x, y, z))) / n_photons;
    // }
}

// Color PunctualLight::trace_photon (const Photon & photon) const {

//     Intersection ninter;

//     if ( !camera->compute_nearest_intersection(ninter, ray) )
//         return this->scene.bg_color;

//     // Photon has just hit a surface, we must decide its fate amongst "reflected" / "refracted" / "absorbed" depending on surface properties

//     if (ninter.object->is_glassy)

//         3 * rand() / (float)RAND_MAX

//     // Compute ambient, diffuse and specular reflection (Phong's 3 components)
//     // If light reflection computation counts as level + 1 then the next ray.level + 1 test becomes mandatory
//     // This calculation is only for direct enlighment, refracted must be simulated via photon mapping

//     for (const auto & light_entry : this->scene.lights)

//         // Vector dir(ninter.point, ((PunctualLight*)(light_entry.second))->position);
//         // // #TODO how do we know if shadow ray is inside or outside the object (ray can pass through the object)?
//         // Ray shadow_ray(ninter.point, dir, ray.in, ray.level);

//         clr = clr + light_entry.second->compute_luminosity(ninter, this->scene);

//     if (ray.level + 1 == this->tracing_depth)
//         return clr;

//     // From this point all rays are level + 1
//     Color cr, ct;
//     Ray rt, rr;

//     // #TODO optimize r and t color calculation in one loop
//     if ( ninter.object->compute_r_ray(rr, ninter) )
//         cr = this->radiate(rr) * ninter.object->r;

//     if ( ninter.object->compute_t_ray(rt, ninter) )
//         ct = this->radiate(rt) * (1 - ninter.object->r);

//     // TODO use operator+ from Color with variadic parameters to optimize
//     uint8_t c[3];
//     for (uint8_t i = 0; i < 3; i++) {

//         // 0 <= sum <= 1020 (255 * 4)
//         // 0 <= uint16_t <= 65 535
//         uint16_t sum = /*ca.c[i] +*/ clr.c[i] + cr.c[i] + ct.c[i];
//         c[i] = 255 < sum ? 255 : sum;
//     }

//     return Color(c[0], c[1], c[2]);
// }
