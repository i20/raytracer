#ifndef _CYLINDER_HPP
#define _CYLINDER_HPP

#include <cmath>

#include <vector>
#include <string>

#include "../headers/Object.hpp"
#include "../headers/Ray.hpp"
#include "../headers/Vector.hpp"
#include "../headers/Point.hpp"
#include "../headers/Triangle.hpp"
#include "../headers/Color.hpp"
#include "../headers/Texture.hpp"
#include "../headers/Octree.hpp"

using namespace std;

class Cylinder : public Object {

    public:

        float radius;
        bool infinite;
        float height;

        // Constructs an infinite cylinder
        Cylinder(
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
        );

        // Constructs a finite cylinder closed or not
        Cylinder(
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
        );

        Cylinder(const Cylinder & cylinder);

        Cylinder & operator=(const Cylinder & cylinder);

        virtual string to_string() const override;

    private:

        virtual TTPairList compute_intersection_ts(const vector<const Octree *> & octrees, const Ray & ray_object) const override;
        virtual bool compute_intersection_final(Vector & normal_object, const Point & point_object, const Triangle * t) const override;
        virtual Color compute_color_shape(const Point & point_object, const Triangle * triangle) const override;

        template <class T>
        T compute_texture_texel (const Point & point_object, const Texture<T> & texture, const Triangle * triangle) const;
};

template <class T>
T Cylinder::compute_texture_texel (const Point & point_object, const Texture<T> & texture, const Triangle * triangle) const {

    float u, v, theta;

    // https://en.wikipedia.org/wiki/List_of_common_coordinate_transformations#From_Cartesian_coordinates
    theta = atan(abs(point_object.p[1] / point_object.p[0]));

    if (0 <= point_object.p[0])
        // Quadrans 1 (x positive, y positive)
        if (0 <= point_object.p[1]); // nothing to do
        // Quadrans 4 (x positive, y negative)
        else theta = 2 * M_PI - theta;
    else
        // Quadrans 2 (x negative, y positive)
        if (0 <= point_object.p[1])
            theta = M_PI - theta;
        // Quadrans 3 (x negative, y negative)
        else theta = M_PI + theta;

    u = theta / (2 * M_PI);
    v = (this->height - point_object.p[2]) / this->height;

    // TODO handle filtering conf for normals (property normals_texture_filtering)
    return texture.get_texel_by_uv(u, v, this->image_texture_filtering == Texture<Color>::FILTERING_BILINEAR);
}

#endif
