#include <cstdint>
#include <cmath>
#include <cfloat> // FLT_EPSILON
#include <cstring> // memcpy

#include "../headers/Object.hpp"

#include "../headers/Triangle.hpp"
#include "../headers/Ray.hpp"
#include "../headers/Vector.hpp"
#include "../headers/Point.hpp"
#include "../headers/Color.hpp"
#include "../headers/Octree.hpp"
#include "../headers/Matrix.hpp"
#include "../headers/Texture.hpp"
#include "../headers/Intersection.hpp"
#include "../headers/TTPair.hpp"

// cfloat.h FLT_EPSILON = 1E-5 is too small and does not prevent self intersection
// @todo Decrease EPSILON to minimum .0001 being to low
#define EPSILON .00025

using namespace std;

// @url(http://realtimecollisiondetection.net/blog/?p=89)
void Object::insert_t(const float t, const Triangle * triangle, TTPairList & list, const Ray & ray_object) {

    // Filter only relevant intersections (after ray origin and before ray end if ray is a finite one)
    // Epsilon testing is still the better of evils even if yields @img(artifacts/espilon/1589747692|1589747751|1589747930.ppm)
    // @see Comments at the end of Object::compute_intersection()
    // if (0 < t && (ray_object.norm == -1 || t <= ray_object.norm))
    // @wonder Should end edge be tested against epsilon too ?
    if (EPSILON < t && (ray_object.norm == -1 || t <= ray_object.norm))
        list.insert(TTPair(t, triangle));
}

Object::Object(

    const Color & color,
    const Point & position,
    const bool is_closed,
    const bool is_glassy,

    const Texture<Color> * image_texture,
    const typename Texture<Color>::filtering_type image_texture_filtering,

    const Texture<Vector> * normals_texture,
    const typename Texture<Vector>::filtering_type normals_texture_filtering,

    const float ar, const float ag, const float ab,
    const float dr, const float dg, const float db,
    const float sr, const float sg, const float sb,
    const float r, const float n, const float g,
    const Vector & z_dir,
    const Vector & y_dir
) :

    color(color),
    position(position),
    is_closed(is_closed),
    is_glassy(is_glassy),
    octree(nullptr),

    image_texture(image_texture),
    image_texture_filtering(image_texture_filtering),

    normals_texture(normals_texture),
    normals_texture_filtering(normals_texture_filtering),

    r(r),
    n(n),
    g(g) {

    this->a[0] = ar;
    this->a[1] = ag;
    this->a[2] = ab;

    this->d[0] = dr;
    this->d[1] = dg;
    this->d[2] = db;

    this->s[0] = sr;
    this->s[1] = sg;
    this->s[2] = sb;

    Vector z = z_dir.normalize();
    Vector x = (y_dir ^ z).normalize();

    this->base = Matrix::TRANSFER(this->position, x, z ^ x, z);
    this->inv = this->base.invert();
}

bool Object::compute_r_ray(Ray & rayr, const Intersection & inter) const {

    if (!this->is_glassy || this->r == 0)
        return false;

    // R = V − 2 * (V * N) * N

    Vector rayr_dir;

    float scalar = inter.ray->direction * inter.normal;
    scalar += scalar;

    for (uint8_t i = 0; i < 4; i++)
        rayr_dir[i] = inter.ray->direction[i] - inter.normal[i] * scalar;

    rayr = Ray(
        inter.point,
        rayr_dir,
        inter.ray->in,
        inter.ray->level + 1
    );

    return true;
}

bool Object::compute_t_ray(Ray & rayt, const Intersection & inter) const {

    if (!this->is_glassy || this->r == 1)
        return false;

    float dev = inter.ray->in ? this->n : 1 / this->n; /* n1/n2 */
    float scalar = inter.ray->direction * inter.normal; /* V*N */

    // methode #1
    float f = cos(asin(dev * sin(acos(scalar)))) + dev * scalar;

    // methode #2
    // T = V + f * N
    // f = [norm(V^N) / tan(asin(dev * norm(V^N)))] * sign(V*N) - V*N
    // V*N = cos(V, N)
    // norm(V^N) = sin(V, N) = sin(acos(V*N))
    // condition de refraction : dev * norm(V^N) < 1
    //double norm = (inter.ray->direction ^ inter.normal).get_norm();
    //if(dev * norm >= 1)
    //    return false;
    //double f = (norm / tan(asin(dev * norm))) * (scalar < 0 ? -1 : 1) - scalar;

    Vector rayt_dir;
    for (uint8_t i = 0; i < 4; i++)
        /* #1 */rayt_dir[i] = inter.ray->direction[i] * dev - inter.normal[i] * f;
        ///* #2 */rayt_dir[i] = inter.ray->direction[i] + inter.normal[i] * f;

    // inter.ray->direction * dev - inter.normal * (cos(asin(dev * sin(acos(scalar)))) + dev * scalar),

    rayt = Ray(
        inter.point,
        rayt_dir,
        this->is_closed ? !inter.ray->in : inter.ray->in,
        inter.ray->level + 1
    );

    return true;
}

bool Object::compute_intersection(Intersection & inter, const Ray & ray) const {

    Ray ray_object = this->inv * ray;

    // @wonder Why a part of the octree logic does reside in Object::compute_intersection? Isn't it a mesh thing?
    vector<const Octree *> octrees;

    if (this->octree != nullptr) {
        this->octree->find_path(octrees, ray_object);
        if (octrees.size() == 0)
            return false;
    }

    TTPairList ts = this->compute_intersection_ts(octrees, ray_object);

    if (ts.size() == 0)
        return false;

    Vector normal_object; // ok normalized
    Point point_object;
    const TTPair * match = nullptr;

    for (const auto & tpair : ts) {

        for (uint8_t i = 0; i < 4; i++)
            point_object[i] = ray_object.origin[i] + tpair.first * ray_object.direction[i];

        if (this->compute_intersection_final(normal_object, point_object, tpair.second, ray_object)) {
            // @todo Move bump mapping here, find a way to deal with virtual template (@see Object.hpp comment)
            //       Have to handle texture mapping on infinite objects in a generic way
            // @wonder Bump mapping seems to move light impact on surface @img(artifacts/bump-mapping)
            match = &tpair;
            break;
        }
    }

    if (match == nullptr)
        return false;

    // Shifting the intersection point along the normal or ray direction from an epsilon distance yields displacement artifacts
    // Not shifting and rather testing EPSILON < t in Object::insert_t() yields @img(artifacts/epsilon)
    // @doc(Wächter-Binder2019_Chapter_AFastAndRobustMethodForAvoidin)
    // Point point = this->base * (point_object + ray_object.direction * -EPSILON); @img(artifacts/epsilon/1590141040.ppm)
    // Point point = this->base * (point_object + normal_object * EPSILON); @img(artifacts/epsilon/1590141145.ppm)
    Point point = this->base * point_object;
    Vector normal = this->base * normal_object;

    inter = Intersection(
        point,
        normal,
        this->compute_color(point, match->second),
        match->first,
        this,
        &ray
    );

    return true;
}

Color Object::compute_color(const Point & point, const Triangle * triangle) const {

    // @todo Move texture mapping here, find a way to deal with virtual template (cf Object.hpp comment)
    return this->image_texture == nullptr ? this->color : this->compute_color_shape(this->inv * point, triangle);
}
