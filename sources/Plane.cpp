#include <cstdint>

#include <vector>
#include <string>
#include <sstream>

#include "../headers/Plane.hpp"

#include "../headers/Object.hpp"
#include "../headers/Ray.hpp"
#include "../headers/Vector.hpp"
#include "../headers/Point.hpp"
#include "../headers/Triangle.hpp"
#include "../headers/Color.hpp"
#include "../headers/Texture.hpp"
#include "../headers/Octree.hpp"

using namespace std;

/* Plane base:
 * x width
 * y height
 * z normal
 */

Plane::Plane(
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
),
    infinite(true) {}

Plane::Plane(
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
    const Vector & z_dir, const Vector & y_dir,
    const float height, const float width
) :
    Object(
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
    ),
    infinite(false),
    height(height),
    width(width) {}

Plane::Plane(const Plane & plane) :
    Object(plane),
    infinite(plane.infinite),
    height(plane.height),
    width(plane.width) {}

Plane & Plane::operator=(const Plane & plane) {

    this->copy(plane);
    this->infinite = plane.infinite;
    this->height = plane.height;
    this->width = plane.width;
    return *this;
}

TTPairList Plane::compute_intersection_ts(const vector<const Octree *> & octrees, const Ray & ray_object) const {

    TTPairList ts;

    float det = ray_object.direction.v[2];

    // No intersection if ray lies in the plane
    if (det != 0)
        Object::insert_t(-ray_object.origin.p[2] / det, nullptr, ts, ray_object);

    return ts;
}

bool Plane::compute_intersection_final(Vector & normal_object, const Point & point_object, const Triangle * t) const {

    // #TODO temp height,width/2

    if (this->infinite || (-this->width/2 <= point_object.p[0] && point_object.p[0] <= this->width/2 && -this->height/2 <= point_object.p[1] && point_object.p[1] <= this->height/2)) {

        normal_object = Vector(0, 0, 1);

        if (this->normals_texture != nullptr && !this->infinite)
            normal_object = normal_object + this->compute_texture_texel<Vector>(point_object, *this->normals_texture, nullptr);

        return true;
    }

    return false;
}

Color Plane::compute_color_shape(const Point & point_object, const Triangle * triangle) const {

    // #TODO add support for infinite plane by repeating texture
    if (this->infinite)
        return this->color;

    return this->color + this->compute_texture_texel<Color>(point_object, *this->image_texture, nullptr);
}

string Plane::to_string() const {

    stringstream ss;
    ss << "Plane[color=" << this->color.to_string() << " position=" << this->position.to_string() << " normal=" << (this->base * Vector(0, 0, 1)).to_string();

    if(!this->infinite)
        ss << " height=" << this->height << " height_along=" << (this->base * Vector(0, 1, 0)).to_string() << " width=" << this->width << " width_along=" << (this->base * Vector(1, 0, 0)).to_string();

    ss << "]";

    return ss.str();
}
