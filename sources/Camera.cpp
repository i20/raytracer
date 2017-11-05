#include <cmath>
#include <ctime>

#include "../headers/Color.hpp"
#include "../headers/Ray.hpp"
#include "../headers/Intersection.hpp"
#include "../headers/Point.hpp"
#include "../headers/Vector.hpp"
#include "../headers/Scene.hpp"
#include "../headers/Matrix.hpp"
#include "../headers/Texture.hpp"

#include "../headers/PunctualLight.hpp"

#include "../headers/Camera.hpp"

#include <stdint.h>

using namespace std;

static const float degrad = M_PI / 180;

void Camera::compute_bases () {

    // camera base vectors
    Vector zc = Vector(this->eye, this->look_at).normalize();
    Vector xc = (up ^ zc).normalize();
    Vector yc = zc ^ xc;

    this->base = Matrix::TRANSFER(this->eye, xc, yc, zc);

    // zeta base vectors
    float xz[3], zz[3];
    for (uint8_t i = 0; i < 3; i++) {

        xz[i] = -xc.v[i];
        zz[i] = -zc.v[i];
    }

    this->zeta = Matrix::TRANSFER(this->look_at, Vector(xz[0], xz[1], xz[2]), yc, Vector(zz[0], zz[1], zz[2]));

    this->zinv = this->zeta.invert();
}

void Camera::compute_projection_size () {

    this->proj_width = 2 * this->focale * tan(this->fov / 2 * degrad);
    this->proj_height = this->proj_width * this->px_height / this->px_width;

    //#TODO 8
    this->pasx = this->proj_width / this->px_width;
    this->pasy = this->proj_height / this->px_height;
}

Color Camera::radiate (const Ray & ray) const {

    if (this->tracing_depth <= ray.level)
        return Color::BLACK;

    Intersection ninter;

    if ( !this->compute_nearest_intersection(ninter, ray) )
        return this->scene.bg_color;

    Color clr = Color::BLACK;

    // Compute diffuse AND specular reflection
    // If light reflection computation counts as level + 1 then the next ray.level + 1 test becomes mandatory
    // Light reflection calculation is only for direct enlighment, refracted must be simulated via photon mapping

    for (const auto & light_entry : this->scene.lights)

        // Vector dir(ninter.point, ((PunctualLight*)(light_entry.second))->position);
        // // #TODO how do we know if shadow ray is inside or outside the object (ray can pass through the object)?
        // Ray shadow_ray(ninter.point, dir, ray.in, ray.level);

        clr = clr + light_entry.second->compute_luminosity(ninter, ray, this->scene);

    if (ray.level + 1 == this->tracing_depth)
        return /*ca + */clr;

    // From this point all rays are level + 1
    Color cr, ct;
    Ray rt, rr;

    // #TODO optimize r and t color calculation in one loop
    if ( ninter.object->compute_r_ray(rr, ninter) )
        cr = this->radiate(rr) * ninter.object->r;

    if ( ninter.object->compute_t_ray(rt, ninter) )
        ct = this->radiate(rt) * (1 - ninter.object->r);

    // TODO use operator+ from Color with variadic parameters to optimize
    uint8_t c[3];
    for (uint8_t i = 0; i < 3; i++) {

        // 0 <= sum <= 1020 (255 * 4)
        // 0 <= uint16_t <= 65 535
        uint16_t sum = /*ca.c[i] +*/ clr.c[i] + cr.c[i] + ct.c[i];
        c[i] = 255 < sum ? 255 : sum;
    }

    return Color(c[0], c[1], c[2]);
}

// Used to detect intersection for full rays, those with radiate() (sight/reflected/refracted)
bool Camera::compute_nearest_intersection(Intersection & ninter, const Ray & ray) const {

    bool has_nearest = false;

    for (const auto & object_entry : this->scene.objects) {

        Intersection inter;

        if ( object_entry.second->compute_intersection(inter, ray) && (!has_nearest || inter.t < ninter.t) ) {

            has_nearest = true;
            ninter = inter;
        }
    }

    return has_nearest;
}

// // Used to detect direct enlightment only
// bool Camera::does_intersect(const Ray & ray, const Intersection & from) const {

//     for (const auto & object_entry : this->scene.objects) {

//         Intersection inter;

//         // Since shadow_ray's origin is on the intersection point compute_intersection(shadow_ray) will always be true on origin object
//         // rd * n <= 0 => ray passes through its own object to reach the light

//         // NOTE: I once thought about replacing the same object condition (ray.direction * from.normal) with (ray.in) to speed up
//         // cause obviously if the shadow ray comes from inside the object it will necessarily intersect with it to reach the light
//         // which is most of the time outside BUT what if the light is inside the object?

//         // Specific test, handles concave objects (#ALGO 1)
//         if ((object_entry.second == from.object && ray.direction * from.normal <= 0) || object_entry.second->compute_intersection(inter, ray))
//             return true;
//     }

//     return false;
// }

float Camera::random() {

    return rand() / (float)RAND_MAX;
}

// With CUDA use threads of blocks to share pixels for stencil application
void Camera::sobel_aa () {

    // Sobel gradient must be computed on original image
    Texture<Color> base_img(this->image);
    // Save the edge detection image for debugging
    Texture<Color> edge_img(this->image.width - 2, this->image.height - 2);

    // Same as for rendering phase, x and y must be expressed in camera base
    float x_start = this->proj_width / 2;
    float y_start = this->proj_height / 2;

    uint8_t aa_base = sqrt(this->aa_depth);

    // NOTE: sobel filter works only on images wider than 3x3

    #pragma omp parallel for collapse(2) schedule(dynamic)
    for (uintmax_t j = 1; j < this->image.height - 1; j++) {
        for (uintmax_t i = 1; i < this->image.width - 1; i++) {

            Color p1 = base_img.get_texel(i-1, j-1);
            Color p2 = base_img.get_texel(i-1, j);
            Color p3 = base_img.get_texel(i-1, j+1);
            Color p4 = base_img.get_texel(i, j-1);
            // Color p5 = base_img.get_texel(i, j); /* aa'ed pixel */
            Color p6 = base_img.get_texel(i, j+1);
            Color p7 = base_img.get_texel(i+1, j-1);
            Color p8 = base_img.get_texel(i+1, j);
            Color p9 = base_img.get_texel(i+1, j+1);

            // We compute one gradient per channel r,g,b
            // http://www.hackification.com/2008/08/31/experiments-in-ray-tracing-part-8-anti-aliasing/
            // http://www.dsp.utoronto.ca/~kostas/Publications2008/pub/23.pdf

            // RGB Sobel gradient norm
            for (uint8_t ii = 0; ii < 3; ii++) {

                float gxi = p1[ii] - p3[ii] + 2 * (p4[ii] - p6[ii]) + p7[ii] - p9[ii];
                float gyi = p1[ii] - p7[ii] + 2 * (p2[ii] - p8[ii]) + p3[ii] - p9[ii];

                // One channel gradient norm is greater than the threshold, edge detected => aa required for the pixel
                if ( sqrt(gxi * gxi + gyi * gyi) > this->aa_threshold ) {

                    // Mark pixel as edge in edge image
                    edge_img.set_texel(i, j, Color::WHITE);

                    // Then choose an anti aliasing technic
                    // https://en.wikipedia.org/wiki/Supersampling

                    uint16_t c[3] = {0, 0, 0};
                    Color aa_color;

                    /*
                    // Random sampling
                    for (uintmax_t k = 0; k < this->aa_depth; k++) {

                        // Random coords in pixel ij
                        float xr = x_start - this->pasx * (i + Camera::random());
                        float yr = y_start - this->pasy * (j + Camera::random());

                        Vector dir = this->base * Vector(xr, yr, this->focale);
                        Ray ray(this->eye, dir, false, 0);
                        Color newc = this->radiate(ray);

                        for (uint8_t ii = 0; ii < 3; ii++)
                            c[ii] += newc[ii];
                    }
                    /**/

                    /**/
                    // Grid 2x2 or 4x4
                    for (uintmax_t kj = 0; kj < aa_base; kj++) {
                        for (uintmax_t ki = 0; ki < aa_base; ki++) {

                            // Fixed grid
                            float xr = x_start - this->pasx * (i + (ki + .5) / (float)aa_base);
                            float yr = y_start - this->pasy * (j + (kj + .5) / (float)aa_base);

                            // // Jittered grid
                            // float xr = x_start - this->pasx * (i + (ki + Camera::random()) / (float)aa_base);
                            // float yr = y_start - this->pasy * (j + (kj + Camera::random()) / (float)aa_base);

                            Vector dir = this->base * Vector(xr, yr, this->focale);
                            Ray ray(this->eye, dir, false, 0);
                            Color newc = this->radiate(ray);

                            for (uint8_t ii = 0; ii < 3; ii++)
                                c[ii] += newc[ii];
                        }
                    }
                    /**/

                    for (uint8_t ii = 0; ii < 3; ii++) {
                        c[ii] /= this->aa_depth;
                        aa_color[ii] = c[ii] > 255 ? 255 : c[ii];
                    }

                    this->image.set_texel(i, j, aa_color);

                    // Pixel has been anti aliased, no need to continue checking its other channel gradients, go to next pixel
                    break;
                }
            }
        }
    }

    edge_img.print("./screenshots/edge.ppm");
}

// #TODO a general solution which makes us free of multiple tricky cases is to consider only the second
// intersection with the same object in does_intersect and compute_nearest_intersection

// CONSTRUCTORS

Camera::Camera(

    const Point & eye,
    const Point & look_at,
    const Vector & up,

    const float focale,
    const float fov,

    const uintmax_t px_width,
    const uintmax_t px_height,

    const Camera::projection_type projection,

    const uintmax_t tracing_depth,
    const uintmax_t aa_depth,
    const uintmax_t aa_threshold,

    Scene & scene) :

    eye(eye),
    look_at(look_at),
    up(up),

    focale(focale),
    fov(fov),

    px_width(px_width),
    px_height(px_height),

    projection(projection),

    tracing_depth(tracing_depth),
    aa_depth(aa_depth),
    aa_threshold(aa_threshold),

    scene(scene),

    // Calculated image contains hidden edge pixels for filtering
    image(px_height + 2, px_width + 2) {

    this->compute_bases();
    this->compute_projection_size();
}

Camera::Camera(const Camera & camera) :

    eye(camera.eye),
    look_at(camera.look_at),
    up(camera.up),

    focale(camera.focale),
    fov(camera.fov),

    px_width(camera.px_width),
    px_height(camera.px_height),

    projection(camera.projection),

    tracing_depth(camera.tracing_depth),
    aa_depth(camera.aa_depth),
    aa_threshold(camera.aa_threshold),

    scene(camera.scene),

    base(camera.base),
    zeta(camera.zeta),
    zinv(camera.zinv),

    proj_width(camera.proj_width),
    proj_height(camera.proj_height),

    pasx(camera.pasx),
    pasy(camera.pasy),

    image(camera.image.width, camera.image.height) {}

Camera & Camera::operator=(const Camera & camera) {

    this->eye = camera.eye;
    this->look_at = camera.look_at;
    this->up = camera.up;

    this->focale = camera.focale;
    this->fov = camera.fov;

    this->px_width = camera.px_width;
    this->px_height = camera.px_height;

    this->projection = camera.projection;

    this->tracing_depth = camera.tracing_depth;
    this->aa_depth = camera.aa_depth;
    this->aa_threshold = camera.aa_threshold;

    this->scene = camera.scene;

    this->base = camera.base;
    this->zeta = camera.zeta;
    this->zinv = camera.zinv;

    this->proj_width = camera.proj_width;
    this->proj_height = camera.proj_height;

    this->pasx = camera.pasx;
    this->pasy = camera.pasy;

    this->image = Texture<Color>(camera.image.width, camera.image.height);

    return *this;
}

// SETTERS

void Camera::set_eye(const Point & eye) {

    this->eye = eye;
    this->compute_bases();
}

void Camera::set_look_at(const Point & look_at) {

    this->look_at = look_at;
    this->compute_bases();
}

void Camera::set_up(const Vector & up) {

    this->up = up;
    this->compute_bases();
}

void Camera::set_focale(const float focale) {

    this->focale = focale;
    this->compute_projection_size();
}

void Camera::set_fov(const float fov) {

    this->fov = fov;
    this->compute_projection_size();
}

void Camera::watch(const Scene & scene) {

    this->scene = scene;
}

void Camera::set_projection_type(const Camera::projection_type projection) {

    this->projection = projection;
}

void Camera::set_tracing_depth(const uintmax_t tracing_depth) {

    this->tracing_depth = tracing_depth;
}

void Camera::set_aa_depth(const uintmax_t aa_depth) {

    this->aa_depth = aa_depth;
}

void Camera::set_aa_threshold(const uintmax_t aa_threshold) {

    this->aa_threshold = aa_threshold;
}

// RENDER

// __global__
// void cuda_render_perspective (float x_start, float y_start) {

//     Vector rayd = this->base * Vector(
//         x_start + this->pasx * blockIdx.x,
//         y_start + this->pasy * blockIdx.y,
//         this->focale
//     );

//     // Hit center of each pixel
//     // Ray must be expressed in the scene's base to compute correct intersections
//     Ray ray(this->eye, rayd, false, 0);

//     this->image.set_texel(blockIdx.x, blockIdx.y, this->radiate(ray));
// }

// __global__
// void cuda_render_orthographic (float x_start, float y_start) {

//     Point rayo = this->base * Point(
//         x_start + this->pasx * blockIdx.x,
//         y_start + this->pasy * blockIdx.y,
//         0
//     );

//     Vector rayd = this->base * Vector(0, 0, 1);
//     Ray ray(rayo, rayd, false, 0);

//     this->image.set_texel(blockIdx.x, blockIdx.y, this->radiate(ray));
// }

void Camera::render () {

    srand(time(nullptr));

    // Our scene's base is the standard xyz with x> and y^, z going out of the screen
    // x, y are expressed in the camera base since the projection screen is attached to it z go into screen

    // Notice that pasx (and y) is also divided by 2 since the ray is sent through the middle of the pixel
    float x_start = (this->proj_width + this->pasx) / 2,
          y_start = (this->proj_height + this->pasy) / 2;

    // For initial rendering use a maximum of blocks since pixels are completely independant
    // dim3 blocks(VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    if (this->projection == Camera::PROJECTION_PERSPECTIVE) {

        // Split image calculation between cores
        // schedule(dynamic) since image will not be homogen to compute in time (empty areas)
        // Dynamic parallelization is done on a per pixel basis
        // As shared vars are readonly accessed there's no performance concern
        // http://stackoverflow.com/questions/7418518/is-there-a-way-to-tell-openmp-that-shared-data-is-constant

        #pragma omp parallel for collapse(2) schedule(dynamic)
        for (uintmax_t j = 0; j < this->image.height; j++) {
            for (uintmax_t i = 0; i < this->image.width; i++) {

                Vector rayd = this->base * Vector(
                    x_start - this->pasx * i,
                    y_start - this->pasy * j,
                    this->focale
                );
                // Hit center of each pixel
                // Ray must be expressed in the scene's base to compute correct intersections
                // TODO is initial ray always outside?
                Ray ray(this->eye, rayd, false, 0);

                this->image.set_texel(i, j, this->radiate(ray));
            }
        }
    }

    else if (this->projection == Camera::PROJECTION_ORTHOGRAPHIC) {

        #pragma omp parallel for collapse(2) schedule(dynamic)
        for (uintmax_t j = 0; j < this->image.height; j++) {
            for (uintmax_t i = 0; i < this->image.width; i++) {

                Point rayo = this->base * Point(
                    x_start - this->pasx * i,
                    y_start - this->pasy * j,
                    0
                );
                Vector rayd = this->base * Vector(0, 0, 1);
                Ray ray(rayo, rayd, false, 0);

                this->image.set_texel(i, j, this->radiate(ray));
            }
        }
    }

    // Sobel antialiasing needs the image to be fully calculated once and cannot be done on the fly as discrete methods
    if (this->aa_depth) this->sobel_aa();
}

void Camera::print(const char * file_name) const {

    this->image.print(file_name, 1, 1, this->image.width - 2, this->image.height - 2);
}
