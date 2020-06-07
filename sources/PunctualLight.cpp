#include <cmath>
#include <cstdint>
#include <cstdlib>

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

Color PunctualLight::compute_luminosity (const Intersection & inter, const Scene & scene) const {

    const Vector pl(inter.point, this->position);
    const float d = pl.get_norm();

    //@ how do we know if shadow ray is inside or outside the object (ray can pass through the object)?
    //@ maybe shadow rays must be a subclass of rays as they are finite + their dir vector should not be normalized
    const Ray shadow_ray(inter.point, pl, inter.ray->in, inter.ray->level, d);

    bool does_light;

    // Shadow must pass through intersection surface to reach light (true self-intersection)
    // Must be computed on true (but shaded) object geometry not bumped one
    if (shadow_ray.direction * inter.true_normal <= 0)
        does_light = false;

    else {

        does_light = true;
        for (const auto & object_entry : scene.objects) {

            Intersection tmp;

            // NOTE: I once thought about replacing the same object condition (ray.direction * from.normal) with (ray.in) to speed up
            // cause obviously if the shadow ray comes from inside the object it will necessarily intersect with it to reach the light
            // which is most of the time outside BUT what if the light is inside the object?

            // Subject to false self-intersection
            if (object_entry.second->compute_intersection(tmp, shadow_ray)) {
                does_light = false;
                break;
            }
        }
    }

    if (!does_light) return Color::BLACK;

    // See lighting.pdf for details

    // Diffuse component

    // N * L
    // N = normal to surface (normalized) = inter.normal
    // L = points toward the light source (normalized) = pl.normalized
    const float scalar_d = (inter.normal * pl) / d; // Normalize L afterward (pl wasn't normalized)

    // Specular component

    // V * R
    // V = points toward the viewer (normalized) = (inter.point, ray.origin).normalized = -ray.direction
    // R = pure reflection vector (normalized) = 2(N*L)N - L = 2*scalar_d*N - L
    const float scalar_dx2 = 2 * scalar_d;
    float scalar_s = 0;
    for (uint8_t i = 0; i < 3; i++)
        scalar_s += inter.ray->direction[i] * (pl[i] / d - inter.normal[i] * scalar_dx2);

    // scalar_s <= 1
    scalar_s = 0 < scalar_s ? powf(scalar_s, inter.object->g) : 0;

    // att c : 1 - c
    // att l : 1 - ld
    // att q : 1 - qdd

    // attenuation factor should be 0 <= x <= 1
    const float attenuation = (1 - c) * (1 - min((float)1, l*d)) * (1 - min((float)1, q*d*d));

    Color c;
    for (uint8_t i = 0; i < 3; i++)
        c[i] = min((float)255, (*this)[i] * inter.color[i] * attenuation * (inter.object->d[i] * scalar_d + inter.object->s[i] * scalar_s));

    return c;
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
//         uint16_t sum = /*ca[i] +*/ clr[i] + cr[i] + ct[i];
//         c[i] = 255 < sum ? 255 : sum;
//     }

//     return Color(c[0], c[1], c[2]);
// }
