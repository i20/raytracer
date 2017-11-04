#ifndef _PLANE_HPP
#define _PLANE_HPP

#include <cmath>

#include "../headers/Object.hpp"
#include "../headers/Ray.hpp"
#include "../headers/Vector.hpp"
#include "../headers/Point.hpp"
#include "../headers/Triangle.hpp"
#include "../headers/Color.hpp"
#include "../headers/Texture.hpp"
#include "../headers/Octree.hpp"

#include <vector>
#include <string>
#include <cmath>

using namespace std;

class Plane : public Object {

    public:

        bool infinite;
        float height, width;

        // TODO semi finite plane width and/or height

        Plane(
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
        );

        Plane(
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
        );

        Plane(const Plane & plane);

        Plane & operator=(const Plane & plane);

        virtual string to_string() const override;

    private:

        virtual TTPairList compute_intersection_ts(const vector<const Octree *> & octrees, const Ray & ray_object) const override;
        virtual bool compute_intersection_final(Vector & normal_object, const Point & point_object, const Triangle * t) const override;
        virtual Color compute_color_shape(const Point & point_object, const Triangle * triangle) const override;

        template <class T>
        T compute_texture_texel (const Point & point_object, const Texture<T> & texture, const Triangle * triangle) const;
};

template <class T>
T Plane::compute_texture_texel (const Point & point_object, const Texture<T> & texture, const Triangle * triangle) const {

    float u, v;
    u = (this->width / 2 + point_object.p[0]) / this->width;
    v = (this->height / 2 - point_object.p[1]) / this->height;

    return texture.get_texel_by_uv(u, v, this->image_texture_filtering == Texture<Color>::FILTERING_BILINEAR);
}

#endif