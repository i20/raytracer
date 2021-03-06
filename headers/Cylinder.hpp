#ifndef _CYLINDER_HPP
#define _CYLINDER_HPP

// Always prefix standard functions with std::
// https://stackoverflow.com/questions/11085916/why-are-some-functions-in-cmath-not-in-the-std-namespace/12805610#12805610

#include <cmath>
#include <vector>

#include "../headers/Object.hpp"
#include "../headers/Ray.hpp"
#include "../headers/Vector.hpp"
#include "../headers/Point.hpp"
#include "../headers/Triangle.hpp"
#include "../headers/Color.hpp"
#include "../headers/Texture.hpp"
#include "../headers/Octree.hpp"

class Cylinder : public Object {

    public:

        float radius;
        float height;

        // Constructs an infinite cylinder
        Cylinder(
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
            const float radius,
            const Vector & z_dir, const Vector & y_dir
        );

        // Constructs a finite cylinder closed or not
        Cylinder(
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
            const float radius,
            const Vector & z_dir, const Vector & y_dir,
            const float height, const bool is_closed
        );

    private:

        virtual TTPairList compute_intersection_ts(const std::vector<const Octree *> & octrees, const Ray & ray_object) const override;
        virtual bool compute_intersection_final(Vector & true_normal_object, Vector & normal_object, const Point & point_object, const Triangle * t, const Ray & ray_object) const override;
        virtual Color compute_color_shape(const Point & point_object, const Triangle * triangle) const override;

        template <class T>
        T compute_texture_texel (const Point & point_object, const Texture<T> & texture, const Triangle * triangle) const;
};

template <class T>
T Cylinder::compute_texture_texel (const Point & point_object, const Texture<T> & texture, const Triangle * triangle) const {

    // https://en.wikipedia.org/wiki/List_of_common_coordinate_transformations#From_Cartesian_coordinates
    float theta = std::atan(std::abs(point_object[1] / point_object[0]));

    if (0 <= point_object[0])
        // Quadrans 1 (x positive, y positive)
        if (0 <= point_object[1]); // nothing to do
        // Quadrans 4 (x positive, y negative)
        else theta = 2 * M_PI - theta;
    else
        // Quadrans 2 (x negative, y positive)
        if (0 <= point_object[1])
            theta = M_PI - theta;
        // Quadrans 3 (x negative, y negative)
        else theta = M_PI + theta;

    const float u = theta / (2 * M_PI);
    const float v = (this->height - point_object[2]) / this->height;

    // @todo Handle filtering conf for normals (property normals_texture_filtering)
    return texture.get_texel_by_uv(u, v, this->image_texture_filtering == Texture<Color>::FILTERING_BILINEAR);
}

#endif
