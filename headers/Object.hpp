#ifndef _OBJECT_HPP
#define _OBJECT_HPP

#include <vector>

#include "../headers/Ray.hpp"
#include "../headers/Vector.hpp"
#include "../headers/Point.hpp"
#include "../headers/Color.hpp"
#include "../headers/Octree.hpp"
#include "../headers/Matrix.hpp"
#include "../headers/Texture.hpp"
#include "../headers/Triangle.hpp"

#include "../headers/TTPair.hpp"

//#include "../headers/Intersection.hpp"
// Circular reference
class Intersection;

// cfloat.h FLT_EPSILON = 1E-5 is too small and does not prevent self intersection
#define EPSILON .00025

class Object {

    public:

        Color color;
        Point position;
        Matrix base, inv;
        bool is_closed;
        Octree * octree;

        const Texture<Color> * image_texture;
        typename Texture<Color>::filtering_type image_texture_filtering;

        const Texture<Vector> * normals_texture;
        typename Texture<Vector>::filtering_type normals_texture_filtering;

        float a[3];   /* ambiente [0;1;inf] ambient light response */
        float d[3];   /* diffusion [0;1;inf] diffuse light response */
        float s[3];   /* specularite [0;inf] */

        float r;
        float t;

        float n;   /* refraction */
        float g;   /* brillance (smoothness, shininess, glossiness), 0 < g */

        Object(
            const Color & color,
            const Point & position,
            const bool is_closed,

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
        );

        bool compute_r_ray(Ray & rayr, const Intersection & inter) const;
        bool compute_t_ray(Ray & rayt, const Intersection & inter) const;

        bool compute_intersection(Intersection & inter, const Ray & ray) const;
        Color compute_color(const Point & point, const Triangle * triangle) const;

    protected:

        static void insert_t(const float t, const Triangle * triangle, TTPairList & list, const Ray & ray_object);

    private:

        virtual TTPairList compute_intersection_ts(const std::vector<const Octree *> & octrees, const Ray & ray_object) const = 0;
        virtual bool compute_intersection_final(Vector & true_normal_object, Vector & normal_object, const Point & point_object, const Triangle * t, const Ray & ray_object) const = 0;
        virtual Color compute_color_shape(const Point & point_object, const Triangle * triangle) const = 0;
        // I wanted to move compute_texture_texel here as a virtual but templates and virtuals are not mixable
};

#endif
