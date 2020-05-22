#include <cstdint>
#include <cmath>

#include <string>
#include <vector>
#include <sstream>
#include <iostream>

#include "../headers/Cylinder.hpp"
#include "../headers/Object.hpp"
#include "../headers/Ray.hpp"
#include "../headers/Vector.hpp"
#include "../headers/Point.hpp"
#include "../headers/Triangle.hpp"
#include "../headers/Color.hpp"
#include "../headers/Texture.hpp"
#include "../headers/Octree.hpp"

using namespace std;

Cylinder::Cylinder(

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

) : Object(
        color,
        position,
        false,
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

    ), radius(radius), infinite(true) {}

Cylinder::Cylinder(
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
    const Vector & z_dir, const Vector & y_dir,
    const float height, const bool is_closed
) : Object(
    color,
    position,
    is_closed,
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
), radius(radius), infinite(false), height(height) {}

Cylinder::Cylinder(const Cylinder & cylinder) : Object(cylinder), radius(cylinder.radius), infinite(cylinder.infinite), height(cylinder.height) {}

Cylinder & Cylinder::operator=(const Cylinder & cylinder) {

    this->copy(cylinder);
    this->radius = cylinder.radius;
    this->infinite = cylinder.infinite;
    this->height = cylinder.height;
    return *this;
}

TTPairList Cylinder::compute_intersection_ts (const vector<const Octree *> & octrees, const Ray & ray_object) const {

    float a = 0;
    float b = 0;
    float c = -this->radius*this->radius;

    // Similar to the sphere but don't take care of the cylinder axis direction (ie z)
    for (uint8_t i = 0; i < 2; i++) {

        float rdi = ray_object.direction.v[i];
        float roi = ray_object.origin.p[i];

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

    // // Check intersections at both edges of the cylinder
    // if (this->is_closed) {

    //     float det = ray_object.direction.v[2];

    //     // No intersection if ray lies in the plane
    //     if (det != 0) {
    //         // Object::insert_t(-ray_object.origin.p[2] / det, nullptr, ts, ray_object);
    //         Object::insert_t(-ray_object.origin.p[2] / det, nullptr, ts, ray_object);
    //     }
    // }

    return ts;
}

bool Cylinder::compute_intersection_final (Vector & normal_object, const Point & point_object, const Triangle * t) const {

    // TODO support is_closed, we will need the 2 intersection points to test if the line between them passes through the edge
    // if p1 >= height && p2 < height -> ok and so on

    if (this->infinite || (0 <= point_object.p[2] && point_object.p[2] <= this->height)) {

        normal_object = Vector(point_object.p[0], point_object.p[1], 0);

        if (this->normals_texture != nullptr && !this->infinite)
            normal_object = normal_object + this->compute_texture_texel<Vector>(point_object, *this->normals_texture, nullptr);

        return true;
    }

    // No need to recheck infinite
    // if (0 <= points_object[1].p[2] && points_object[1].p[2] <= this->height) {

    //     normal_object = Vector(points_object[1].p[0], points_object[1].p[1], 0);
    //     return true;
    // }

    return false;
}

Color Cylinder::compute_color_shape(const Point & point_object, const Triangle * triangle) const {

    if (this->infinite)
        return this->color;

    return this->color + this->compute_texture_texel<Color>(point_object, *this->image_texture, nullptr);
}

string Cylinder::to_string() const {

    stringstream ss;
    ss << "Cylinder[color=" << this->color.to_string() << " position=" << this->position.to_string() << " radius=" << this->radius << "]";

    return ss.str();
}
