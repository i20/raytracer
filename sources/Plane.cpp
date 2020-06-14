#include <cstdint>

#include <vector>

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

    const Texture<Color> * image_texture,
    const typename Texture<Color>::filtering_type image_texture_filtering,

    const Texture<Vector> * normals_texture,
    const typename Texture<Vector>::filtering_type normals_texture_filtering,

    const float ar, const float ag, const float ab,
    const float dr, const float dg, const float db,
    const float sr, const float sg, const float sb,

    const float r, const float t,

    const float n, const float g,
    const Vector & z_dir, const Vector & y_dir
) : Object(
    color,
    position,
    false,

    image_texture,
    image_texture_filtering,

    normals_texture,
    normals_texture_filtering,

    ar, ag, ab,
    dr, dg, db,
    sr, sg, sb,
    r, t, n, g,
    z_dir, y_dir
),
    infinite(true) {}

Plane::Plane(
    const Color & color,
    const Point & position,

    const Texture<Color> * image_texture,
    const typename Texture<Color>::filtering_type image_texture_filtering,

    const Texture<Vector> * normals_texture,
    const typename Texture<Vector>::filtering_type normals_texture_filtering,

    const float ar, const float ag, const float ab,
    const float dr, const float dg, const float db,
    const float sr, const float sg, const float sb,

    const float r, const float t,

    const float n, const float g,
    const Vector & z_dir, const Vector & y_dir,
    const float height, const float width
) :
    Object(
        color,
        position,
        false,

        image_texture,
        image_texture_filtering,

        normals_texture,
        normals_texture_filtering,

        ar, ag, ab,
        dr, dg, db,
        sr, sg, sb,
        r, t, n, g,
        z_dir, y_dir
    ),
    infinite(false),
    height(height),
    width(width) {}

TTPairList Plane::compute_intersection_ts(const vector<const Octree *> & octrees, const Ray & ray_object) const {

    TTPairList ts;

    const float det = ray_object.direction[2];

    // No intersection if ray lies in the plane
    if (det != 0)
        Object::insert_t(-ray_object.origin[2] / det, nullptr, ts, ray_object);

    return ts;
}

bool Plane::compute_intersection_final(Vector & true_normal_object, Vector & normal_object, const Point & point_object, const Triangle * t, const Ray & ray_object) const {

    // @todo temp height,width/2

    if (this->infinite || (-this->width/2 <= point_object[0] && point_object[0] <= this->width/2 && -this->height/2 <= point_object[1] && point_object[1] <= this->height/2)) {

        true_normal_object = Vector::Z; // ok normalized

        // Bump mapping
        if (this->normals_texture != nullptr && !this->infinite) {
            // In the particular case of plane, bump base and object base are the same (except origin)
            // Therefore, we do not need to apply matrix transfer like for the other shapes
            normal_object = this->compute_texture_texel<Vector>(point_object, *this->normals_texture, nullptr);
        }

        else normal_object = true_normal_object;

        // Detection of wether final normal should be corrected must be done on true normal as
        // bump mapping looses the information of object true geometry and leads to false positives
        if (0 < true_normal_object * ray_object.direction) {
            true_normal_object = true_normal_object * -1;
            normal_object = normal_object * -1;
        }

        return true;
    }

    return false;
}

Color Plane::compute_color_shape(const Point & point_object, const Triangle * triangle) const {

    // @todo Add support for infinite plane by repeating texture
    if (this->infinite)
        return this->color;

    return this->color + this->compute_texture_texel<Color>(point_object, *this->image_texture, nullptr);
}
