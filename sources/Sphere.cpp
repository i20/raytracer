#include <cstdint>
#include <cmath>

#include <vector>

#include "../headers/Sphere.hpp"

#include "../headers/Object.hpp"
#include "../headers/Ray.hpp"
#include "../headers/Vector.hpp"
#include "../headers/Point.hpp"
#include "../headers/Triangle.hpp"
#include "../headers/Color.hpp"
#include "../headers/Texture.hpp"
#include "../headers/Octree.hpp"

using namespace std;

Sphere::Sphere(
    const Color & color,
    const Point & position,
    const bool is_glassy,

    const Texture<Color> * image_texture,
    const typename Texture<Color>::filtering_type image_texture_filtering,

    const Texture<Vector> * normals_texture,
    const typename Texture<Vector>::filtering_type normals_texture_filtering,

    const float ar, const float ag, const float ab,
    const float dr, const float dg, const float db,
    const float sr, const float sg, const float sb,
    const float r, const float n, const float g,
    const float radius,
    const Vector & z_dir, const Vector & y_dir
) :
    Object(
        color,
        position,
        true,
        is_glassy,

        image_texture,
        image_texture_filtering,

        normals_texture,
        normals_texture_filtering,

        ar, ag, ab,
        dr, dg, db,
        sr, sg, sb,
        r, n, g,
        z_dir, y_dir
    ), radius(radius) {}

TTPairList Sphere::compute_intersection_ts(const vector<const Octree *> & octrees, const Ray & ray_object) const {

    float a = 0; // a = rd.rd
    float b = 0; // 2 * rd.co
    float c = -this->radius*this->radius; // co.co - r^2

    for (uint8_t i = 0; i < 3; i++) {

        float rdi = ray_object.direction[i];
        float roi = ray_object.origin[i];

        a += rdi * rdi;
        b += rdi * roi;
        c += roi * roi;
    }
    b += b;

    float det = b*b - 4*a*c;
    TTPairList ts;

    if (det >= 0) {

        float da = a+a;

        if (det == 0)
            Object::insert_t(-b / da, nullptr, ts, ray_object);

        else {

            float sq = sqrt(det);

            Object::insert_t((-b-sq) / da, nullptr, ts, ray_object);
            Object::insert_t((-b+sq) / da, nullptr, ts, ray_object);
        }
    }

    return ts;
}

bool Sphere::compute_intersection_final(Vector & normal_object, const Point & point_object, const Triangle * t, const Ray & ray_object) const {

    // @wonder Maybe normalize() is unnecessary here, remove if redundant
    normal_object = Vector(point_object[0], point_object[1], point_object[2]).normalize();

    // Detection of wether normal should be corrected must take place BEFORE bump mapping as it
    // kinda blurs tracks and normal is less indicative afterward and can generate false positives
    // Effective correction must be done AFTER though as bump map carry non corrected delta data
    bool must_correct = 0 < normal_object * ray_object.direction;

    // @todo Bump mapping fails as soon as object base is not scene base anymore, investigate why
    if (this->normals_texture != nullptr)
        normal_object = (normal_object + this->compute_texture_texel<Vector>(point_object, *this->normals_texture, nullptr)).normalize();

    if (must_correct)
        normal_object = normal_object * -1;

    return true;
}

Color Sphere::compute_color_shape(const Point & point_object, const Triangle * triangle) const {

    return this->color + this->compute_texture_texel<Color>(point_object, *this->image_texture, nullptr);
}
