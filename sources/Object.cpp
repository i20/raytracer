#include <cstdint>
#include <cmath>
#include <cfloat> // FLT_EPSILON
#include <cstring>

#include <string>
#include <sstream>

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
#define EPSILON .001

using namespace std;

void Object::copy(const Object & object) {

    this->color = object.color;
    this->position = object.position;
    this->base = object.base;
    this->inv = object.inv;
    this->is_closed = object.is_closed;
    this->is_glassy = object.is_glassy;
    this->octree = object.octree;

    this->image_texture = object.image_texture;
    this->image_texture_filtering = object.image_texture_filtering;

    this->normals_texture = object.normals_texture;
    this->normals_texture_filtering = object.normals_texture_filtering;

    size_t size = 3 * sizeof(float);

    memcpy(this->a, object.a, size);
    memcpy(this->d, object.d, size);
    memcpy(this->s, object.s, size);

    this->r = object.r;
    this->n = object.n;
    this->g = object.g;
}

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

Object::Object(const Object & object) :

    color(object.color),
    position(object.position),
    base(object.base),
    inv(object.inv),
    is_closed(object.is_closed),
    is_glassy(object.is_glassy),
    octree(object.octree),

    image_texture(object.image_texture),
    image_texture_filtering(object.image_texture_filtering),

    normals_texture(object.normals_texture),
    normals_texture_filtering(object.normals_texture_filtering),

    r(object.r),
    n(object.n),
    g(object.g) {

    size_t size = 3 * sizeof(float);

    memcpy(this->a, object.a, size);
    memcpy(this->d, object.d, size);
    memcpy(this->s, object.s, size);
}

Object & Object::operator=(const Object & object) {

    this->copy(object);
    return *this;
}

bool Object::compute_r_ray(Ray & rayr, const Intersection & inter) const {

    if (!this->is_glassy || this->r == 0)
        return false;

    // R = V − 2 * (V * N) * N

    Vector rayr_dir;

    float scalar = inter.ray->direction * inter.normal;
    scalar += scalar;

    float v[4];
    for (uint8_t i = 0; i < 4; i++)
        v[i] = inter.ray->direction.v[i] - inter.normal.v[i] * scalar;

    rayr_dir = Vector(v);

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

    float v[3];
    for (uint8_t i = 0; i < 3; i++)
        /* #1 */v[i] = inter.ray->direction.v[i] * dev - inter.normal.v[i] * f;
        ///* #2 */v[i] = inter.ray->direction.v[i] + inter.normal.v[i] * f;

    // inter.ray->direction * dev - inter.normal * (cos(asin(dev * sin(acos(scalar)))) + dev * scalar),
    Vector rayt_dir(v[0], v[1], v[2]);

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

    vector<const Octree *> octrees;

    if (this->octree != nullptr) {
        this->octree->find_path(octrees, ray_object);
        if (octrees.size() == 0)
            return false;
    }

    TTPairList ts = this->compute_intersection_ts(octrees, ray_object);

    if (ts.size() == 0)
        return false;

    Vector normal_object;
    Point point_object;
    const TTPair * match = nullptr;

    for (const auto & tpair : ts) {

        float p[4];
        for (uint8_t i = 0; i < 4; i++)
            p[i] = ray_object.origin.p[i] + tpair.first * ray_object.direction.v[i];

        point_object = Point(p);

        if (this->compute_intersection_final(normal_object, point_object, tpair.second)) {

            // @todo Move bump mapping here, find a way to deal with virtual template (@see Object.hpp comment)
            match = &tpair;
            break;
        }
    }

    if (match == nullptr)
        return false;

    // We have to correct the normal
    if (normal_object * ray_object.direction > 0)
        normal_object = normal_object * -1;

    normal_object = normal_object.normalize();

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

string Object::to_string() const {

    stringstream ss;
    ss << "Object[color=" << this->color.to_string() << " position=" << this->position.to_string() << "]";

    return ss.str();
}
