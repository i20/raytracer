#include <cstdint>
#include <cmath>

#include <vector>

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

    ), radius(radius), height(-1) {}

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
), radius(radius), height(height) {}

TTPairList Cylinder::compute_intersection_ts (const vector<const Octree *> & octrees, const Ray & ray_object) const {

    float a = 0;
    float b = 0;
    float c = -this->radius*this->radius;

    // Similar to the sphere but don't take care of the cylinder axis direction (ie z)
    for (uint8_t i = 0; i < 2; i++) {

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

    // @todo Implement cylinder is_closed
    // // Check intersections at both edges of the cylinder
    // if (this->height != -1 && this->is_closed) {

    //     float det = ray_object.direction[2];

    //     // No intersection if ray lies in the plane
    //     if (det != 0) {
    //         // Object::insert_t(-ray_object.origin[2] / det, nullptr, ts, ray_object);
    //         Object::insert_t(-ray_object.origin[2] / det, nullptr, ts, ray_object);
    //     }
    // }

    return ts;
}

bool Cylinder::compute_intersection_final (Vector & normal_object, const Point & point_object, const Triangle * t, const Ray & ray_object) const {

    // @todo support is_closed, we will need the 2 intersection points to test if the line between them passes through the edge
    // if p1 >= height && p2 < height -> ok and so on

    if (this->height == -1 || (0 <= point_object[2] && point_object[2] <= this->height)) {

        const Vector true_normal_object = Vector(point_object[0], point_object[1], 0).normalize();

        // Bump mapping
        if (this->normals_texture != nullptr && this->height != -1) {
            // Bump texel is expressed in normal base which is different from object base
            const Matrix bump_base = Matrix::TRANSFER(point_object, Vector::Z ^ true_normal_object, Vector::Z, true_normal_object);
            normal_object = bump_base * this->compute_texture_texel<Vector>(point_object, *this->normals_texture, nullptr).normalize();
        }

        else normal_object = true_normal_object;

        // Detection of wether final normal should be corrected must be done on true normal as
        // bump mapping looses the information of object true geometry and leads to false positives
        if (0 < true_normal_object * ray_object.direction)
            normal_object = normal_object * -1;

        return true;
    }

    return false;
}

Color Cylinder::compute_color_shape(const Point & point_object, const Triangle * triangle) const {

    if (this->height == -1)
        return this->color;

    return this->color + this->compute_texture_texel<Color>(point_object, *this->image_texture, nullptr);
}
