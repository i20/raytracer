#include <cmath>
#include <ctime>
#include <cstdint>

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

using namespace std;

static const float degrad = M_PI / 180;

// static float _random() {

//     return rand() / (float)RAND_MAX;
// }

void Camera::compute_bases () {

    const Vector zc = Vector(this->eye, this->look_at).normalize();
    const Vector xc = (this->up ^ zc).normalize();
    const Vector yc = zc ^ xc;

    this->base = Matrix::TRANSFER(this->eye, xc, yc, zc);
    this->inv = this->base.invert();
}

void Camera::rotate (const Vector & axis, const float pas) {

    //@ Is there a reason I cannot juste write
    //@ this->look_at = (this->base * Matrix::ROTATION(axis, pas)) * this->look_at;
    //@ or even
    //@ this->look_at = Matrix::ROTATION(this->base * axis, pas) * this->look_at;
    //@ or maybe something is wrong with my Matrix * Matrix

    const Matrix rotation = Matrix::ROTATION(axis, pas);
    this->look_at = this->base * (rotation * (this->inv * this->look_at));
    // Do not forget to update up otherwise rotation around x (with y up) will end in a sign switch (visual reversing)
    this->up = this->base * (rotation * (this->inv * this->up));
    this->compute_bases();
}

void Camera::rotateScene (const Vector & axis, const float pas) {

    const Matrix rotation = Matrix::ROTATION(axis, pas);
    this->eye = rotation * this->eye;
    this->look_at = rotation * this->look_at;
    // Do not forget to update up otherwise rotation around x (with y up) will end in a sign switch (visual reversing)
    this->up = rotation * this->up;
    this->compute_bases();
}

void Camera::translate (const Vector & translation) {

    this->eye = this->eye + (this->base * translation);
    this->look_at = this->look_at + (this->base * translation);
    this->compute_bases();
}

void Camera::reset () {

    //@ Reset to true initialisation params
    this->eye = Point(0, 0, 5);
    this->look_at = Point::O;
    this->up = Vector::Y;
    this->compute_bases();
}

void Camera::compute_projection_size () {

    this->proj_width = 2 * this->focale * tan(this->fov / 2 * degrad);
    this->proj_height = this->proj_width * this->px_height / this->px_width;

    //@ pasx and pasy seems to be equal, merge it
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

    // Compute ambient, diffuse and specular reflection (Phong's 3 components)
    // If light reflection computation counts as level + 1 then the next ray.level + 1 test becomes mandatory
    // This calculation is only for direct enlighment, refracted must be simulated via photon mapping

    for (const auto & light_entry : this->scene.lights)

        // Vector dir(ninter.point, ((PunctualLight*)(light_entry.second))->position);
        // // #TODO how do we know if shadow ray is inside or outside the object (ray can pass through the object)?
        // Ray shadow_ray(ninter.point, dir, ray.in, ray.level);

        clr = clr + light_entry.second->compute_luminosity(ninter, this->scene);

    if (ray.level + 1 == this->tracing_depth)
        return clr;

    // From this point all rays are level + 1
    Ray rt, rr;

    // @todo Optimize r and t color calculation in one loop
    Color cr = ninter.object->compute_r_ray(rr, ninter)
        ? this->radiate(rr) * ninter.object->r
        : Color::BLACK;

    Color ct = ninter.object->compute_t_ray(rt, ninter)
        ? this->radiate(rt) * ninter.object->t
        : Color::BLACK;

    // Sum direct enlightment + reflected + refracted
    // @todo Use operator+ from Color with variadic parameters to optimize
    return clr + cr + ct;
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

// With CUDA use threads of blocks to share pixels for stencil application
void Camera::sobel_aa () {

    // Sobel gradient must be computed on original image
    const Texture<Color> base_img(this->image);
    // Save the edge detection image for debugging
    Texture<Color> edge_img(this->image.width - 2, this->image.height - 2);

    // Same as for rendering phase, x and y must be expressed in camera base
    const float x_start = this->proj_width / 2;
    const float y_start = this->proj_height / 2;

    const uint8_t aa_base = sqrt(this->aa_depth);

    // NOTE: sobel filter works only on images wider than 3x3

    #pragma omp parallel for collapse(2) schedule(dynamic)
    for (uintmax_t j = 1; j < this->image.height - 1; j++) {
        for (uintmax_t i = 1; i < this->image.width - 1; i++) {

            const Color p1 = base_img.get_texel(i-1, j-1);
            const Color p2 = base_img.get_texel(i-1, j);
            const Color p3 = base_img.get_texel(i-1, j+1);
            const Color p4 = base_img.get_texel(i, j-1);
            // const Color p5 = base_img.get_texel(i, j); /* aa'ed pixel */
            const Color p6 = base_img.get_texel(i, j+1);
            const Color p7 = base_img.get_texel(i+1, j-1);
            const Color p8 = base_img.get_texel(i+1, j);
            const Color p9 = base_img.get_texel(i+1, j+1);

            // We compute one gradient per channel r,g,b
            // http://www.hackification.com/2008/08/31/experiments-in-ray-tracing-part-8-anti-aliasing/
            // http://www.dsp.utoronto.ca/~kostas/Publications2008/pub/23.pdf

            // RGB Sobel gradient norm
            for (uint8_t ii = 0; ii < 3; ii++) {

                const float gxi = p1[ii] - p3[ii] + 2 * (p4[ii] - p6[ii]) + p7[ii] - p9[ii];
                const float gyi = p1[ii] - p7[ii] + 2 * (p2[ii] - p8[ii]) + p3[ii] - p9[ii];

                // One channel gradient norm is greater than the threshold, edge detected => aa required for the pixel
                if ( sqrt(gxi * gxi + gyi * gyi) > this->aa_threshold ) {

                    // Mark pixel as edge in edge image
                    edge_img.set_texel(i, j, Color::WHITE);

                    // Then choose an anti aliasing technic
                    // https://en.wikipedia.org/wiki/Supersampling
                    // https://computergraphics.stackexchange.com/questions/2130/anti-aliasing-filtering-in-ray-tracing

                    uint16_t c[3] = {0, 0, 0};
                    Color aa_color;

                    //

                    // // Random sampling
                    // for (uintmax_t k = 0; k < this->aa_depth; k++) {

                    //     // Random coords in pixel ij
                    //     float xr = x_start - this->pasx * (i + _random());
                    //     float yr = y_start - this->pasy * (j + _random());

                    //     Vector dir = this->base * Vector(xr, yr, this->focale);
                    //     Ray ray(this->eye, dir, false, 0);
                    //     Color newc = this->radiate(ray);

                    //     for (uint8_t ii = 0; ii < 3; ii++)
                    //         c[ii] += newc[ii];
                    // }

                    // Grid 2x2 or 4x4
                    for (uintmax_t kj = 0; kj < aa_base; kj++) {
                        for (uintmax_t ki = 0; ki < aa_base; ki++) {

                            // Fixed grid
                            const float xr = x_start - this->pasx * (i + (ki + .5) / (float)aa_base);
                            const float yr = y_start - this->pasy * (j + (kj + .5) / (float)aa_base);

                            // // Jittered grid
                            // float xr = x_start - this->pasx * (i + (ki + _random()) / (float)aa_base);
                            // float yr = y_start - this->pasy * (j + (kj + _random()) / (float)aa_base);

                            const Vector dir = this->base * Vector(xr, yr, this->focale);
                            const Ray ray(this->eye, dir, false, 0);
                            const Color newc = this->radiate(ray);

                            for (uint8_t ii = 0; ii < 3; ii++)
                                c[ii] += newc[ii];
                        }
                    }

                    //

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

// SETTERS

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

void Camera::render () {

    srand(time(nullptr));

    // Our scene's base is the standard xyz with x> and y^, z going out of the screen
    // x, y are expressed in the camera base since the projection screen is attached to it z go into screen

    // Notice that pasx (and y) is also divided by 2 since the ray is sent through the middle of the pixel
    const float x_start = (this->proj_width + this->pasx) / 2;
    const float y_start = (this->proj_height + this->pasy) / 2;

    if (this->projection == Camera::PROJECTION_PERSPECTIVE) {

        // Split image calculation between cores
        // schedule(dynamic) since image will not be homogen to compute in time (empty areas)
        // Dynamic parallelization is done on a per pixel basis
        // As shared vars are readonly accessed there's no performance concern
        // http://stackoverflow.com/questions/7418518/is-there-a-way-to-tell-openmp-that-shared-data-is-constant

        #pragma omp parallel for collapse(2) schedule(dynamic)
        for (uintmax_t j = 0; j < this->image.height; j++) {
            for (uintmax_t i = 0; i < this->image.width; i++) {

                const Vector rayd = this->base * Vector(
                    x_start - this->pasx * i,
                    y_start - this->pasy * j,
                    this->focale
                );
                // Hit center of each pixel
                // Ray must be expressed in the scene's base to compute correct intersections
                // @wonder Is initial ray always outside?
                const Ray ray(this->eye, rayd, false, 0);

                this->image.set_texel(i, j, this->radiate(ray));
            }
        }
    }

    else if (this->projection == Camera::PROJECTION_ORTHOGRAPHIC) {

        #pragma omp parallel for collapse(2) schedule(dynamic)
        for (uintmax_t j = 0; j < this->image.height; j++) {
            for (uintmax_t i = 0; i < this->image.width; i++) {

                const Point rayo = this->base * Point(
                    x_start - this->pasx * i,
                    y_start - this->pasy * j,
                    0
                );
                const Vector rayd = this->base * Vector(0, 0, 1);
                const Ray ray(rayo, rayd, false, 0);

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
